#ifndef KOCHOU_REGISTRY_EXAMPLE_DYNAMIC_RENDERING_HPP
#define KOCHOU_REGISTRY_EXAMPLE_DYNAMIC_RENDERING_HPP

#include <array>

#include <ktl/errc.hpp>
#include <ktl/memory.hpp>

#include <kochou/kochou.hpp>

namespace kochou::registry
{
class example_dynamic_rendering;
using shared_example_dynamic_rendering = ktl::memory::sptr< example_dynamic_rendering >;

class example_dynamic_rendering
{
    // requirements
public:
    static ktl::errc
    ensure(kochou::shared_context _sctx) noexcept;

    static ktl::errc
    should(kochou::shared_context _sctx) noexcept;

    static bool
    allowed(kochou::shared_context _sctx) noexcept;

    // fabrics
public:
    static ktl::result< shared_example_dynamic_rendering, ktl::errc >
    make(kochou::shared_context _sctx, kochou::entity::shared_surface _surface) noexcept;

    // common
public:
    example_dynamic_rendering(
        kochou::shared_context _sctx, kochou::entity::shared_surface _surface,
        kochou::entity::shared_swapchain _swapchain, std::vector< kochou::entity::shared_image > _images,
        std::vector< kochou::entity::shared_image_view > _image_views, kochou::entity::shared_fence _fence,
        kochou::entity::shared_semaphore _image_available, kochou::entity::shared_semaphore _render_finished,
        kochou::entity::shared_command_pool _command_pool, kochou::entity::shared_command_buffer _command_buffer,
        kochou::entity::shared_queue _present_queue, kochou::entity::shared_queue _graphics_queue) noexcept;
    example_dynamic_rendering(const example_dynamic_rendering &) noexcept = delete;
    example_dynamic_rendering(example_dynamic_rendering &&) noexcept      = default;
    example_dynamic_rendering &
    operator=(const example_dynamic_rendering &) noexcept = delete;
    example_dynamic_rendering &
    operator=(example_dynamic_rendering &&) noexcept = default;

    // methods
public:
    ktl::errc
    draw(kochou::entity::shared_buffer _buffer, ktl::u32 _width, ktl::u32 _height,
         kochou::registry::shared_example_dynamic_pipeline _pipeline) noexcept;

    // shared raii
private:
    kochou::shared_context                           sctx_;
    kochou::entity::shared_surface                   surface_;
    kochou::entity::shared_swapchain                 swapchain_;
    std::vector< kochou::entity::shared_image >      images_;
    std::vector< kochou::entity::shared_image_view > image_views_;
    kochou::entity::shared_fence                     fence_;
    kochou::entity::shared_semaphore                 image_available_;
    kochou::entity::shared_semaphore                 render_finished_;
    kochou::entity::shared_command_pool              command_pool_;
    kochou::entity::shared_command_buffer            command_buffer_;
    kochou::entity::shared_queue                     present_queue_;
    kochou::entity::shared_queue                     graphics_queue_;
};
} // namespace kochou::registry

#endif
