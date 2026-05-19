#ifndef KOCHOU_REGISTRY_COMMON_WINDOW_INTERFACE_HPP
#define KOCHOU_REGISTRY_COMMON_WINDOW_INTERFACE_HPP

#include <concepts>
#include <string>

#include <ktl/api.hpp>
#include <ktl/result.hpp>
#include <ktl/type.hpp>

#include <kochou/context/context.hpp>

namespace kochou::registry
{
// TODO расширить интерфейс
struct window_input_params
{
    std::string title;
    ktl::i32    offset_x;
    ktl::i32    offset_y;
    ktl::u32    width;
    ktl::u32    height;
    bool        is_resizable;
    bool        is_visible;
    bool        is_fullscreen;
};

template < typename WINDOW >
concept is_valid_window_interface = requires(WINDOW _window, const WINDOW & _const_window, kochou::shared_context _sctx,
                                             const window_input_params & _params) {
    {
        WINDOW::make(_sctx, _params)
    } -> std::same_as< ktl::result< std::tuple< WINDOW, ktl::api::surface_khr >, ktl::errc > >;

    // position
    { _const_window.offset_x() } noexcept -> std::same_as< ktl::i32 >;
    { _const_window.offset_y() } noexcept -> std::same_as< ktl::i32 >;
    { _const_window.width() } noexcept -> std::same_as< ktl::u32 >;
    { _const_window.height() } noexcept -> std::same_as< ktl::u32 >;

    // manip
    { _window.hide() } noexcept -> std::same_as< ktl::errc >;
    { _window.show() } noexcept -> std::same_as< ktl::errc >;
    { _window.close() } noexcept -> std::same_as< ktl::errc >;

    // status
    { _const_window.is_visible() } noexcept -> std::same_as< bool >;
    { _const_window.is_focused() } noexcept -> std::same_as< bool >;
    { _const_window.is_fullscreen() } noexcept -> std::same_as< bool >;
    { _const_window.should_close() } noexcept -> std::same_as< bool >;

    { _window.poll_events() } noexcept -> std::same_as< void >;

    // TODO events
    /*
    {
        _window.on_close_request(
            []() -> bool
            {
                return true;
            })
    } noexcept -> std::same_as< bool >;
    {
        _window.on_resize(
            [](ktl::u32, ktl::u32)
            {
            })
    } noexcept -> std::same_as< bool >;
    {
        _window.on_move(
            [](ktl::i32, ktl::i32)
            {
            })
    } noexcept -> std::same_as< bool >;
    {
        _window.on_focus_gain(
            []()
            {
            })
    } noexcept -> std::same_as< bool >;
    {
        _window.on_focus_loss(
            []()
            {
            })
    } noexcept -> std::same_as< bool >;
    {
        _window.on_minimize(
            []()
            {
            })
    } noexcept -> std::same_as< bool >;
    {
        _window.on_maximize(
            []()
            {
            })
    } noexcept -> std::same_as< bool >;
    {
        _window.on_restore(
            []()
            {
            })
    } noexcept -> std::same_as< bool >;
    {
        _window.on_dpi_change(
            [](float)
            {
            })
    } noexcept -> std::same_as< bool >;
    */
};
} // namespace kochou::registry

#endif
