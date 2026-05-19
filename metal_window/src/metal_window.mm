#import <Cocoa/Cocoa.h>
#import <QuartzCore/QuartzCore.h>
#import <Metal/Metal.h>

#include <kochou/kochou.hpp>

#include <metal_window/metal_window.hpp>

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

ktl::errc
kochou::registry::metal_window::should(kochou::shared_context _sctx) noexcept
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

bool
kochou::registry::metal_window::allowed(kochou::shared_context _sctx) noexcept
{
    return kochou::allowed< kochou::extension< ktl::api::extension::khr_surface > >(_sctx) &&
           kochou::allowed< kochou::extension< ktl::api::extension::ext_metal_surface > >(_sctx);
}

kochou::registry::metal_window::metal_window() noexcept
    : sctx_{}
    , window_{nullptr}
    , view_{nullptr}
    , layer_{nullptr}
{}

kochou::registry::metal_window::~metal_window() noexcept
{
    if (window_) {
        NSWindow *win = (__bridge_transfer NSWindow *)window_;
        [win close];
        window_ = nullptr;
    }
    view_  = nullptr;
    layer_ = nullptr;
}

ktl::result< std::tuple< kochou::registry::metal_window, ktl::api::surface_khr >, ktl::errc >
kochou::registry::metal_window::make(kochou::shared_context           _sctx,
                                     const window_input_params &      _params) noexcept
{
    metal_window win;
    win.sctx_ = _sctx;

    ktl::api::instance instance = kochou::view::instance(_sctx);
    if (!instance)
    {
        return ktl::err(_sctx->status());
    }

    NSScreen *          screen = [NSScreen mainScreen];
    NSWindowStyleMask   style  = NSWindowStyleMaskClosable
                               | NSWindowStyleMaskTitled
                               | NSWindowStyleMaskMiniaturizable;
    if (_params.is_resizable)
    {
        style |= NSWindowStyleMaskResizable;
    }
    NSRect     content_rect = NSMakeRect(_params.offset_x, _params.offset_y, _params.width, _params.height);
    NSWindow * window       = [[NSWindow alloc] initWithContentRect:content_rect
                                                styleMask:style
                                                backing:NSBackingStoreBuffered
                                                defer:NO
                                                screen:screen];
    if (!window)
    {
        return ktl::err(ktl::errc::unknown);
    }
    window.minSize            = NSMakeSize(100, 100);
    window.releasedWhenClosed = NO;
    
    if (_params.is_fullscreen)
    {
        [window toggleFullScreen:nil];
    }

    win.window_ = (__bridge void *) window;
    win.view_   = nullptr;
    win.layer_  = nullptr;

    CAMetalLayer * metalLayer = [[CAMetalLayer alloc] init]
    metalLayer.device         = MTLCreateSystemDefaultDevice();
    metalLayer.opaque         = YES;

    ktl::api::surface_khr surface = nullptr;
    return std::make_tuple(std::move(win), surface);
}

ktl::i32 kochou::registry::metal_window::offset_x() const noexcept
{
    return 0;
}

ktl::i32 kochou::registry::metal_window::offset_y() const noexcept
{
    return 0;
}

ktl::u32 kochou::registry::metal_window::width() const noexcept
{
    return 0;
}

ktl::u32 kochou::registry::metal_window::height() const noexcept
{
    return 0;
}

ktl::errc kochou::registry::metal_window::show() noexcept
{
    if (!window_)
    {
        return ktl::errc::unknown;
    }
    NSWindow * window = (__bridge NSWindow *) window_;

    [window setIsVisible:true];
    [window makeKeyAndOrderFront:nil];

    return ktl::errc::success;
}

ktl::errc kochou::registry::metal_window::hide() noexcept
{
    if (!window_)
    {
        return ktl::errc::unknown;
    }
    NSWindow * window = (__bridge NSWindow *) window_;

    [window setIsVisible:false];
    [window makeKeyAndOrderFront:nil];

    return ktl::errc::success;
}

ktl::errc kochou::registry::metal_window::close() noexcept
{
    return ktl::errc::success;
}

bool kochou::registry::metal_window::is_visible() const noexcept
{
    return false;
}

bool kochou::registry::metal_window::is_focused() const noexcept
{
    return false;
}

bool kochou::registry::metal_window::is_fullscreen() const noexcept
{
    return false;
}

bool kochou::registry::metal_window::should_close() const noexcept
{
    return false;
}

void kochou::registry::metal_window::poll_events() noexcept
{
}
