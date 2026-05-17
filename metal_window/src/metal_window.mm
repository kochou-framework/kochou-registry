#import <Cocoa/Cocoa.h>
#import <QuartzCore/CAMetalLayer.h>
#import <Metal/MTLDevice.h>

#include <kochou/kochou.hpp>

#include <metal_window/metal_window.hpp>

ktl::errc
kochou::registry::metal_window::ensure(kochou::shared_context _sctx) noexcept
{
    ktl::errc rc = ktl::errc::success;
    rc = kochou::ensure< kochou::extension< ktl::api::extension::khr_surface > >(_sctx);
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

ktl::errc
kochou::registry::metal_window::should(kochou::shared_context _sctx) noexcept
{
    ktl::errc rc = ktl::errc::success;
    rc = kochou::should< kochou::extension< ktl::api::extension::khr_surface > >(_sctx);
    if (rc != ktl::errc::success)
    {
        kochou::log::error("ensure ktl::api::extension::khr_surface failed");
        return rc;
    }
    rc = kochou::should< kochou::extension< ktl::api::extension::ext_metal_surface > >(_sctx);
    if (rc != ktl::errc::success)
    {
        kochou::log::error("ensure ktl::api::extension::ext_metal_surface failed");
        return rc;
    }
    return rc;
}

bool
kochou::registry::metal_window::allowed(kochou::shared_context _sctx) noexcept
{
    if (!kochou::allowed< kochou::extension< ktl::api::extension::khr_surface > >(_sctx))
    {
        return false;
    }
    if (!kochou::allowed< kochou::extension< ktl::api::extension::ext_metal_surface > >(_sctx))
    {
        return false;
    }
    return true;
}

kochou::registry::metal_window::metal_window() noexcept
    : sctx_{},
    window_{nullptr},
    view_{nullptr},
    layer_{nullptr}
{}

kochou::registry::metal_window::~metal_window() noexcept
{
    if (window_)
    {
        NSWindow * win = (__bridge_transfer NSWindow*)window_;
        [win close];
        window_ = nullptr;
    }

    view_ = nullptr;
    layer_ = nullptr;
}

ktl::result< std::tuple< kochou::registry::metal_window, ktl::api::surface_khr >, ktl::errc >
kochou::registry::metal_window::make(kochou::shared_context _sctx, 
                                     ktl::u32 _width, 
                                     ktl::u32 _height, 
                                     std::string_view _title) noexcept
{
    metal_window win;
    win.sctx_ = _sctx;

    ktl::api::instance instance = kochou::view::instance(_sctx);
    if (!instance)
    {
        return ktl::err(ktl::errc::unknown);
    }
    
    NSRect rect = NSMakeRect(0, 0, static_cast< CGFloat >(_width), static_cast< CGFloat >(_height));
    
    auto * ns_window = [[NSWindow alloc] initWithContentRect:rect
                                            styleMask:(NSWindowStyleMaskTitled | 
                                            NSWindowStyleMaskClosable | 
                                            NSWindowStyleMaskMiniaturizable | 
                                            NSWindowStyleMaskResizable)
                                            backing:NSBackingStoreBuffered
                                            defer:YES];
    [ns_window setTitle:[NSString stringWithUTF8String:_title.data()]];
    [ns_window setCollectionBehavior:NSWindowCollectionBehaviorFullScreenPrimary];

    auto * ns_view = [[NSView alloc] initWithFrame:rect];
    [ns_window setContentView:ns_view];
    
    auto * metal_layer = [CAMetalLayer layer];
    metal_layer.device = MTLCreateSystemDefaultDevice();
    metal_layer.drawableSize = CGSizeMake(_width, _height);
    metal_layer.pixelFormat = MTLPixelFormatBGRA8Unorm;

    [ns_view setLayer:metal_layer];
    [ns_view setWantsLayer:YES];

    ktl::api::metal_surface_create_info_ext surface_info{};
    surface_info.p_layer = metal_layer;
    
    ktl::api::surface_khr surface = nullptr;

    ktl::api::result rc = ktl::api::create_metal_surface_ext(instance, &surface_info, nullptr, &surface);
    if (rc != ktl::api::result::v_success)
    {
        return ktl::err(ktl::cast_api_result(rc));
    }

    [ns_window makeKeyAndOrderFront:nil];
    [ns_window center];
    [NSApp activateIgnoringOtherApps:YES];  // активируем приложение
    [NSApp run];  // бесконечный цикл обработки событий

    win.window_ = (__bridge_retained void *) ns_window;
    win.view_   = (__bridge void *) ns_view;
    win.layer_  = (__bridge void *) metal_layer;

    return std::make_tuple(std::move(win), surface);
}

ktl::u32 kochou::registry::metal_window::width() const noexcept
{
    if (!view_)
    {
        return 0;
    }
    NSView * view = (__bridge NSView *) view_;
    return static_cast< ktl::u32 >([view frame].size.width);
}

ktl::u32 kochou::registry::metal_window::height() const noexcept
{
    if (!view_)
    {
        return 0;
    }
    NSView * view = (__bridge NSView *) view_;
    return static_cast< ktl::u32 >([view frame].size.height);
}

bool kochou::registry::metal_window::should_close() const noexcept
{
    if (!window_)
    {
        return true;
    }
    // NSWindow * win = (__bridge NSWindow *) window_;
    // return [win isWindowClosed];
    return false;
}
