#ifndef KOCHOU_REGISTRY_METAL_WINDOW_HPP
#define KOCHOU_REGISTRY_METAL_WINDOW_HPP

#include <tuple>

#include <ktl/api.hpp>
#include <ktl/result.hpp>

#include <kochou/context/context.hpp>

#include <common/window_interface.hpp>

namespace kochou::registry
{
class metal_window
{
    // component requirements
public:
    static ktl::errc
    ensure(kochou::shared_context _sctx) noexcept;
    static ktl::errc
    should(kochou::shared_context _sctx) noexcept;
    static bool
    allowed(kochou::shared_context _sctx) noexcept;

public:
    ~metal_window() noexcept;
    metal_window(const metal_window &) = delete;
    metal_window(metal_window &&) noexcept;
    metal_window &
    operator=(const metal_window &) = delete;
    metal_window &
    operator=(metal_window &&) noexcept;

private:
    metal_window() noexcept;

    // interface
public:
    static ktl::result< std::tuple< metal_window, ktl::api::surface_khr >, ktl::errc >
    make(kochou::shared_context _sctx, const kochou::registry::window_input_params & _params) noexcept;

    ktl::i32
    offset_x() const noexcept;
    ktl::i32
    offset_y() const noexcept;
    ktl::u32
    width() const noexcept;
    ktl::u32
    height() const noexcept;

    ktl::errc
    hide() noexcept;
    ktl::errc
    show() noexcept;
    ktl::errc
    close() noexcept;

    bool
    is_visible() const noexcept;
    bool
    is_focused() const noexcept;
    bool
    is_fullscreen() const noexcept;
    bool
    should_close() const noexcept;

    void
    poll_events() noexcept;

private:
    kochou::shared_context sctx_;
    void *                 window_;
    void *                 view_;
    void *                 layer_;
};
} // namespace kochou::registry

static_assert(kochou::registry::is_valid_window_interface< kochou::registry::metal_window >);

#endif
