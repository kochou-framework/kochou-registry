#ifndef KOCHOU_REGISTRY_METAL_WINDOW_HPP
#define KOCHOU_REGISTRY_METAL_WINDOW_HPP

#include <tuple>

#include <ktl/api.hpp>
#include <ktl/result.hpp>

#include <kochou/context/context.hpp>

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
    metal_window(const metal_window &)     = delete;
    metal_window(metal_window &&) noexcept = default;
    metal_window &
    operator=(const metal_window &) = delete;
    metal_window &
    operator=(metal_window &&) noexcept = default;

private:
    metal_window() noexcept;

public:
    static ktl::result< std::tuple< metal_window, ktl::api::surface_khr >, ktl::errc >
    make(kochou::shared_context _sctx, ktl::u32 _width, ktl::u32 _height, std::string_view _title) noexcept;

    ktl::u32
    width() const noexcept;
    ktl::u32
    height() const noexcept;
    bool
    should_close() const noexcept;

private:
    kochou::shared_context sctx_;
    void *                 window_;
    void *                 view_;
    void *                 layer_;
};
} // namespace kochou::registry

#endif
