#import <Cocoa/Cocoa.h>
#import <QuartzCore/QuartzCore.h>
#import <Metal/Metal.h>
#import <objc/runtime.h>

#include <ktl/api.hpp>
#include <ktl/memory.hpp>

#include <kochou/kochou.hpp>

#include <metal_window/metal_window.hpp>

@interface KochouMetalWindowDelegate : NSObject< NSWindowDelegate >
@property(nonatomic, assign) BOOL shouldClose;
@end

@implementation KochouMetalWindowDelegate
- (instancetype) init
{
    self = [super init];
    if (self) {
        _shouldClose = NO;
    }
    return self;
}

- (BOOL )windowShouldClose: (id) sender
{
    _shouldClose = YES;
    return YES;
}

- (void) windowWillClose: (NSNotification *) notification
{
    _shouldClose = YES;
}
@end

namespace
{
static char KOCHOU_METAL_WINDOW_DELEGATE_KEY;

static bool
kochou_is_main_thread() noexcept
{
    return [NSThread isMainThread];
}

static void
kochou_ensure_nsapp() noexcept
{
    if (NSApp == nil)
    {
        [NSApplication sharedApplication];
    }

    [NSApp setActivationPolicy:NSApplicationActivationPolicyRegular];
}

static KochouMetalWindowDelegate *
kochou_get_delegate(NSWindow * window) noexcept
{
    if (!window)
    {
        return nil;
    }

    return (KochouMetalWindowDelegate *) objc_getAssociatedObject(
        window,
        &KOCHOU_METAL_WINDOW_DELEGATE_KEY
    );
}

static void
kochou_set_delegate(NSWindow * window, KochouMetalWindowDelegate * delegate) noexcept
{
    if (!window)
    {
        return;
    }

    [window setDelegate:delegate];

    objc_setAssociatedObject(
        window,
        &KOCHOU_METAL_WINDOW_DELEGATE_KEY,
        delegate,
        OBJC_ASSOCIATION_RETAIN_NONATOMIC
    );
}

static void
kochou_clear_delegate(NSWindow * window) noexcept
{
    if (!window)
    {
        return;
    }

    [window setDelegate:nil];

    objc_setAssociatedObject(
        window,
        &KOCHOU_METAL_WINDOW_DELEGATE_KEY,
        nil,
        OBJC_ASSOCIATION_RETAIN_NONATOMIC
    );
}
} // namespace

ktl::errc
kochou::registry::metal_window::ensure(kochou::shared_context _sctx) noexcept
{
    ktl::errc rc = kochou::ensure< kochou::extension< ktl::api::extension::khr_surface > >(_sctx);
    if (rc != ktl::errc::success)
    {
        kochou::log::error("ensure ktl::api::extension::khr_surface failed");
        return rc;
    }

    rc = kochou::ensure< kochou::extension< ktl::api::extension::ext_metal_surface > >(_sctx);
    if (rc != ktl::errc::success)
    {
        kochou::log::error("ensure ktl::api::extension::ext_metal_surface failed");
        return rc;
    }

    return rc;
}

ktl::errc kochou::registry::metal_window::should(kochou::shared_context _sctx) noexcept
{
    ktl::errc rc = kochou::should< kochou::extension< ktl::api::extension::khr_surface > >(_sctx);
    if (rc != ktl::errc::success)
    {
        kochou::log::error("should ktl::api::extension::khr_surface failed");
        return rc;
    }

    rc = kochou::should< kochou::extension< ktl::api::extension::ext_metal_surface > >(_sctx);
    if (rc != ktl::errc::success)
    {
        kochou::log::error("should ktl::api::extension::ext_metal_surface failed");
        return rc;
    }

    return rc;
}

bool kochou::registry::metal_window::allowed(kochou::shared_context _sctx) noexcept
{
    return kochou::allowed< kochou::extension< ktl::api::extension::khr_surface > >(_sctx)
        && kochou::allowed< kochou::extension< ktl::api::extension::ext_metal_surface > >(_sctx);
}

kochou::registry::metal_window::metal_window() noexcept
    : sctx_{}
    , surface_{}
    , window_{nullptr}
    , view_{nullptr}
    , layer_{nullptr}
{
}

kochou::registry::metal_window::metal_window(metal_window && _other) noexcept
    : sctx_{std::move(_other.sctx_)}
    , surface_{std::move(_other.surface_)}
    , window_{_other.window_}
    , view_{_other.view_}
    , layer_{_other.layer_}
{
    _other.window_ = nullptr;
    _other.view_   = nullptr;
    _other.layer_  = nullptr;
}

kochou::registry::metal_window &
kochou::registry::metal_window::operator=(metal_window && _other) noexcept
{
    if (this == &_other)
    {
        return * this;
    }

    close();

    if (layer_)
    {
        CFRelease(layer_);
    }

    if (view_)
    {
        CFRelease(view_);
    }

    if (window_)
    {
        NSWindow * window = (__bridge NSWindow *)window_;
        kochou_clear_delegate(window);
        CFRelease(window_);
    }

    sctx_    = std::move(_other.sctx_);
    surface_ = std::move(_other.surface_);
    window_  = _other.window_;
    view_    = _other.view_;
    layer_   = _other.layer_;

    _other.window_ = nullptr;
    _other.view_   = nullptr;
    _other.layer_  = nullptr;

    return *this;
}

kochou::registry::metal_window::~metal_window() noexcept
{
    @autoreleasepool
    {
        close();

        if (layer_)
        {
            CFRelease(layer_);
            layer_ = nullptr;
        }

        if (view_)
        {
            CFRelease(view_);
            view_ = nullptr;
        }

        if (window_)
        {
            NSWindow * window = (__bridge NSWindow *) window_;
            kochou_clear_delegate(window);

            CFRelease(window_);
            window_ = nullptr;
        }
    }
}

ktl::result< std::tuple< kochou::registry::metal_window, kochou::entity::shared_surface >, ktl::errc >
kochou::registry::metal_window::make(kochou::shared_context _sctx, const window_input_params & _params) noexcept
{
    @autoreleasepool
    {
        if (!kochou_is_main_thread())
        {
            kochou::log::error("metal_window must be created on main thread");
            return ktl::err(ktl::errc::unknown);
        }

        kochou_ensure_nsapp();

        metal_window win;

        ktl::api::instance instance = kochou::view::instance(_sctx);

        if (!instance)
        {
            return ktl::err(_sctx->status());
        }

        NSScreen * screen = [NSScreen mainScreen];

        NSWindowStyleMask style = NSWindowStyleMaskClosable
                                | NSWindowStyleMaskTitled
                                | NSWindowStyleMaskMiniaturizable;
        if (_params.is_resizable)
        {
            style |= NSWindowStyleMaskResizable;
        }

        NSRect content_rect = NSMakeRect(
            static_cast< CGFloat >(_params.offset_x),
            static_cast< CGFloat >(_params.offset_y),
            static_cast< CGFloat >(_params.width),
            static_cast< CGFloat >(_params.height)
        );

        NSWindow * window = [[NSWindow alloc]
            initWithContentRect:content_rect
            styleMask:style
            backing:NSBackingStoreBuffered
            defer:NO
            screen:screen];

        if (!window)
        {
            kochou::log::error("NSWindow creation failed");
            return ktl::err(ktl::errc::unknown);
        }

        [window setTitle:[NSString stringWithUTF8String:_params.title.c_str()]];
        [window setMinSize:NSMakeSize(100.0, 100.0)];
        [window setReleasedWhenClosed:NO];

        NSRect view_rect = NSMakeRect(
            0.0,
            0.0,
            static_cast< CGFloat >(_params.width),
            static_cast< CGFloat >(_params.height)
        );

        NSView * view = [[NSView alloc] initWithFrame:view_rect];

        if (!view)
        {
            [window close];
            kochou::log::error("NSView creation failed");
            return ktl::err(ktl::errc::unknown);
        }

        [view setWantsLayer:YES];

        CAMetalLayer * metal_layer = [CAMetalLayer layer];

        if (!metal_layer)
        {
            [window close];
            kochou::log::error("CAMetalLayer creation failed");
            return ktl::err(ktl::errc::unknown);
        }

        id< MTLDevice > metal_device = MTLCreateSystemDefaultDevice();

        if (!metal_device)
        {
            [window close];
            kochou::log::error("MTLCreateSystemDefaultDevice failed");
            return ktl::err(ktl::errc::unknown);
        }

        metal_layer.device = metal_device;
        metal_layer.opaque = YES;
        metal_layer.framebufferOnly = YES;
        metal_layer.contentsScale = screen ? [screen backingScaleFactor] : 1.0;
        metal_layer.drawableSize = CGSizeMake(
            static_cast< CGFloat >(_params.width) * metal_layer.contentsScale,
            static_cast< CGFloat >(_params.height) * metal_layer.contentsScale
        );

        [view setLayer:metal_layer];
        [window setContentView:view];

        KochouMetalWindowDelegate * delegate = [[KochouMetalWindowDelegate alloc] init];
        kochou_set_delegate(window, delegate);

        ktl::api::metal_surface_create_info_ext surface_info{};
        surface_info.p_layer          = metal_layer;
        ktl::api::surface_khr surface = nullptr;
        ktl::api::result rc           = ktl::api::create_metal_surface_ext(
            instance,
            &surface_info,
            nullptr,
            &surface
        );
        if (rc != ktl::api::result::v_success)
        {
            [window close];
            kochou::log::error("create_metal_surface_ext failed");
            return ktl::err(ktl::cast_api_result(rc));
        }

        win.sctx_    = _sctx;
        win.surface_ = ktl::memory::make_shared< kochou::entity::surface >(surface, _params.width, _params.height);
        win.window_  = (__bridge_retained void *) window;
        win.view_    = (__bridge_retained void *) view;
        win.layer_   = (__bridge_retained void *) metal_layer;

        if (_params.is_fullscreen)
        {
            [window toggleFullScreen:nil];
        }

        if (_params.is_visible)
        {
            [window makeKeyAndOrderFront:nil];
            [NSApp activateIgnoringOtherApps:YES];
        }

        return std::make_tuple(std::move(win), win.surface_);
    }
}

ktl::i32 kochou::registry::metal_window::offset_x() const noexcept
{
    if (!window_)
    {
        return 0;
    }

    NSWindow * window = (__bridge NSWindow *)window_;
    NSRect frame = [window frame];

    return static_cast< ktl::i32 >(frame.origin.x);
}

ktl::i32 kochou::registry::metal_window::offset_y() const noexcept
{
    if (!window_)
    {
        return 0;
    }

    NSWindow * window = (__bridge NSWindow *)window_;
    NSRect frame = [window frame];

    return static_cast< ktl::i32 >(frame.origin.y);
}

ktl::u32 kochou::registry::metal_window::width() const noexcept
{
    if (!window_)
    {
        return 0;
    }

    NSWindow * window = (__bridge NSWindow *)window_;
    NSView * content_view = [window contentView];

    if (!content_view)
    {
        return 0;
    }

    NSRect bounds = [content_view bounds];

    return static_cast< ktl::u32 >(bounds.size.width);
}

ktl::u32 kochou::registry::metal_window::height() const noexcept
{
    if (!window_)
    {
        return 0;
    }

    NSWindow * window = (__bridge NSWindow *) window_;
    NSView * content_view = [window contentView];

    if (!content_view)
    {
        return 0;
    }

    NSRect bounds = [content_view bounds];

    return static_cast< ktl::u32 >(bounds.size.height);
}

ktl::errc kochou::registry::metal_window::show() noexcept
{
    @autoreleasepool
    {
        if (!window_)
        {
            return ktl::errc::unknown;
        }

        if (!kochou_is_main_thread())
        {
            kochou::log::error("metal_window::show must be called on main thread");
            return ktl::errc::unknown;
        }

        NSWindow * window = (__bridge NSWindow *)window_;

        [window makeKeyAndOrderFront:nil];
        [NSApp activateIgnoringOtherApps:YES];

        return ktl::errc::success;
    }
}

ktl::errc kochou::registry::metal_window::hide() noexcept
{
    @autoreleasepool
    {
        if (!window_)
        {
            return ktl::errc::unknown;
        }

        if (!kochou_is_main_thread())
        {
            kochou::log::error("metal_window::hide must be called on main thread");
            return ktl::errc::unknown;
        }

        NSWindow * window = (__bridge NSWindow *) window_;

        [window orderOut:nil];

        return ktl::errc::success;
    }
}

ktl::errc kochou::registry::metal_window::close() noexcept
{
    @autoreleasepool
    {
        if (!window_)
        {
            return ktl::errc::success;
        }

        if (!kochou_is_main_thread())
        {
            kochou::log::error("metal_window::close must be called on main thread");
            return ktl::errc::unknown;
        }

        NSWindow * window = (__bridge NSWindow *)window_;
        KochouMetalWindowDelegate * delegate = kochou_get_delegate(window);

        if (delegate)
        {
            delegate.shouldClose = YES;
        }

        if ([window isVisible])
        {
            [window close];
        }

        return ktl::errc::success;
    }
}

bool kochou::registry::metal_window::is_visible() const noexcept
{
    if (!window_)
    {
        return false;
    }

    NSWindow * window = (__bridge NSWindow *) window_;

    return [window isVisible];
}

bool kochou::registry::metal_window::is_focused() const noexcept
{
    if (!window_)
    {
        return false;
    }

    NSWindow * window = (__bridge NSWindow *) window_;

    return [window isKeyWindow];
}

bool kochou::registry::metal_window::is_fullscreen() const noexcept
{
    if (!window_)
    {
        return false;
    }

    NSWindow * window = (__bridge NSWindow *) window_;

    return ([window styleMask] & NSWindowStyleMaskFullScreen) != 0;
}

bool kochou::registry::metal_window::should_close() const noexcept
{
    if (!window_)
    {
        return true;
    }

    NSWindow * window = (__bridge NSWindow *) window_;
    KochouMetalWindowDelegate* delegate = kochou_get_delegate(window);

    if (!delegate)
    {
        return false;
    }

    return delegate.shouldClose == YES;
}

void kochou::registry::metal_window::poll_events() noexcept
{
    @autoreleasepool
    {
        if (!kochou_is_main_thread())
        {
            kochou::log::error("metal_window::poll_events must be called on main thread");
            return;
        }

        kochou_ensure_nsapp();

        for (;;)
        {
            NSEvent * event = [NSApp
                nextEventMatchingMask:NSEventMaskAny
                untilDate:[NSDate distantPast]
                inMode:NSDefaultRunLoopMode
                dequeue:YES];

            if (!event)
            {
                break;
            }

            [NSApp sendEvent:event];
        }

        [NSApp updateWindows];

        if (window_ && layer_)
        {
            NSWindow * window = (__bridge NSWindow *) window_;
            NSView * content_view = [window contentView];
            CAMetalLayer * metal_layer = (__bridge CAMetalLayer *) layer_;

            if (content_view && metal_layer)
            {
                NSRect bounds = [content_view bounds];
                CGFloat scale = [window backingScaleFactor];

                metal_layer.contentsScale = scale;
                metal_layer.drawableSize = CGSizeMake(
                    bounds.size.width * scale,
                    bounds.size.height * scale
                );
            }
        }
    }
}
