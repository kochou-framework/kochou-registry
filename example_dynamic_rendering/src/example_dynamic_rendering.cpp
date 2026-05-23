#include <example_dynamic_pipeline/example_dynamic_pipeline.hpp>
#include <example_dynamic_rendering/example_dynamic_rendering.hpp>

namespace
{
void
transition_swapchain_image(ktl::api::command_buffer _cmd, ktl::api::image _image, ktl::api::image_layout _old_layout,
                           ktl::api::image_layout _new_layout)
{
    ktl::api::image_memory_barrier barrier{};
    barrier.old_layout = _old_layout;
    barrier.new_layout = _new_layout;

    barrier.src_queue_family_index = KTL_API_QUEUE_FAMILY_IGNORED;
    barrier.dst_queue_family_index = KTL_API_QUEUE_FAMILY_IGNORED;

    barrier.image = _image;

    barrier.subresource_range.aspect_mask      = static_cast< ktl::u32 >(ktl::api::image_aspect_flag_bits::v_color_bit);
    barrier.subresource_range.base_mip_level   = 0;
    barrier.subresource_range.level_count      = 1;
    barrier.subresource_range.base_array_layer = 0;
    barrier.subresource_range.layer_count      = 1;

    ktl::api::pipeline_stage_flags src_stage{};
    ktl::api::pipeline_stage_flags dst_stage{};

    if (_old_layout == ktl::api::image_layout::v_undefined &&
        _new_layout == ktl::api::image_layout::v_color_attachment_optimal)
    {
        barrier.src_access_mask = 0;
        barrier.dst_access_mask = static_cast< ktl::u32 >(ktl::api::access_flag_bits::v_color_attachment_write_bit);

        src_stage = static_cast< ktl::u32 >(ktl::api::pipeline_stage_flag_bits::v_top_of_pipe_bit);
        dst_stage = static_cast< ktl::u32 >(ktl::api::pipeline_stage_flag_bits::v_color_attachment_output_bit);
    }
    else if (_old_layout == ktl::api::image_layout::v_color_attachment_optimal &&
             _new_layout == ktl::api::image_layout::v_present_src_khr)
    {
        barrier.src_access_mask = static_cast< ktl::u32 >(ktl::api::access_flag_bits::v_color_attachment_write_bit);
        barrier.dst_access_mask = 0;

        src_stage = static_cast< ktl::u32 >(ktl::api::pipeline_stage_flag_bits::v_color_attachment_output_bit);
        dst_stage = static_cast< ktl::u32 >(ktl::api::pipeline_stage_flag_bits::v_bottom_of_pipe_bit);
    }
    else
    {
        kochou::log::warning("transition wrong");
        return;
    }

    ktl::api::cmd_pipeline_barrier(_cmd, src_stage, dst_stage, 0, 0, nullptr, 0, nullptr, 1, &barrier);
}
} // namespace

ktl::errc
kochou::registry::example_dynamic_rendering::ensure(kochou::shared_context _sctx) noexcept
{
    ktl::errc rc = ktl::errc::success;

    rc = kochou::ensure< kochou::queue< ktl::api::queue_flag_bits::v_graphics_bit > >(_sctx);
    if (rc != ktl::errc::success)
    {
        kochou::log::error("ensure ktl::api::queue_flag_bits::v_graphics_bit failed, rc={}", rc);
        return rc;
    }

    rc = kochou::ensure< kochou::entity::swapchain >(_sctx);
    if (rc != ktl::errc::success)
    {
        kochou::log::error("ensure kochou::entity::swapchain failed, rc={}", rc);
        return rc;
    }

    rc = kochou::ensure< kochou::registry::example_dynamic_pipeline >(_sctx);
    if (rc != ktl::errc::success)
    {
        kochou::log::error("ensure kochou::registry::example_dynamic_pipeline failed, rc={}", rc);
        return rc;
    }

    return rc;
}

ktl::errc
kochou::registry::example_dynamic_rendering::should(kochou::shared_context _sctx) noexcept
{
    ktl::errc rc = ktl::errc::success;

    rc = kochou::should< kochou::queue< ktl::api::queue_flag_bits::v_graphics_bit > >(_sctx);
    if (rc != ktl::errc::success)
    {
        kochou::log::error("should ktl::api::queue_flag_bits::v_graphics_bit failed, rc={}", rc);
        return rc;
    }

    rc = kochou::should< kochou::entity::swapchain >(_sctx);
    if (rc != ktl::errc::success)
    {
        kochou::log::error("should kochou::entity::swapchain failed, rc={}", rc);
        return rc;
    }

    rc = kochou::should< kochou::registry::example_dynamic_pipeline >(_sctx);
    if (rc != ktl::errc::success)
    {
        kochou::log::error("should kochou::registry::example_dynamic_pipeline failed, rc={}", rc);
        return rc;
    }

    return rc;
}

bool
kochou::registry::example_dynamic_rendering::allowed(kochou::shared_context _sctx) noexcept
{
    return kochou::allowed< kochou::queue< ktl::api::queue_flag_bits::v_graphics_bit > >(_sctx) &&
           kochou::allowed< kochou::entity::swapchain >(_sctx) &&
           kochou::allowed< kochou::registry::example_dynamic_pipeline >(_sctx);
}

ktl::result< kochou::registry::shared_example_dynamic_rendering, ktl::errc >
kochou::registry::example_dynamic_rendering::make(kochou::shared_context         _sctx,
                                                  kochou::entity::shared_surface _surface) noexcept
{
    kochou::entity::swapchain::input_info swapchain_info_input = {
        .buffering    = 2,
        .format       = ktl::api::format::v_b_8g_8r_8a_8unorm,
        .color_space  = ktl::api::color_space_khr::v_srgb_nonlinear_khr,
        .present_mode = ktl::api::present_mode_khr::v_fifo_khr,
        .is_strict    = true};

    auto swapchain_rc = kochou::entity::swapchain::make(_sctx, _surface, swapchain_info_input);
    if (!swapchain_rc.has_value())
    {
        kochou::log::error("swapchain::make failed, rc={}", swapchain_rc.error());
        return ktl::err(swapchain_rc.error());
    }
    auto [swapchain, swapchain_output_info] = swapchain_rc.take_value();

    auto images_rc = kochou::entity::image::make(_sctx, swapchain);
    if (!images_rc.has_value())
    {
        kochou::log::error("image::make failed, rc={}", images_rc.error());
        return ktl::err(images_rc.error());
    }
    auto images = images_rc.take_value();

    auto image_views_rc = kochou::entity::image_view::make(_sctx, images, swapchain_output_info.format);
    if (!image_views_rc.has_value())
    {
        kochou::log::error("image_view::make failed, rc={}", image_views_rc.error());
        return ktl::err(image_views_rc.error());
    }
    auto image_views = image_views_rc.take_value();

    auto fence_rc = kochou::entity::fence::make(_sctx);
    if (!fence_rc.has_value())
    {
        kochou::log::error("fence::make failed, rc={}", fence_rc.error());
        return ktl::err(fence_rc.error());
    }
    auto fence = fence_rc.take_value();

    auto image_available_rc = kochou::entity::semaphore::make(_sctx);
    if (!image_available_rc.has_value())
    {
        kochou::log::error("semaphore::make failed, rc={}", image_available_rc.error());
        return ktl::err(image_available_rc.error());
    }
    auto image_available = image_available_rc.take_value();

    auto render_finished_rc = kochou::entity::semaphore::make(_sctx);
    if (!render_finished_rc.has_value())
    {
        kochou::log::error("semaphore::make failed, rc={}", render_finished_rc.error());
        return ktl::err(render_finished_rc.error());
    }
    auto render_finished = render_finished_rc.take_value();

    auto command_pool_rc = kochou::entity::command_pool::make(_sctx);
    if (!command_pool_rc.has_value())
    {
        kochou::log::error("command_pool::make failed, rc={}", command_pool_rc.error());
        return ktl::err(command_pool_rc.error());
    }
    auto command_pool = command_pool_rc.take_value();

    auto command_buffer_rc = kochou::entity::command_buffer::make(_sctx, command_pool);
    if (!command_buffer_rc.has_value())
    {
        kochou::log::error("command_buffer::make failed, rc={}", command_buffer_rc.error());
        return ktl::err(command_buffer_rc.error());
    }
    auto command_buffer = command_buffer_rc.take_value();

    auto present_queue_rc = kochou::entity::queue::make(_sctx, kochou::entity::queue::present);
    if (!present_queue_rc.has_value())
    {
        kochou::log::error("queue::make failed, rc={}", present_queue_rc.error());
        return ktl::err(present_queue_rc.error());
    }
    auto present_queue     = present_queue_rc.take_value();
    auto graphics_queue_rc = kochou::entity::queue::make(_sctx, kochou::entity::queue::graphics);
    if (!graphics_queue_rc.has_value())
    {
        kochou::log::error("queue::make failed, rc={}", graphics_queue_rc.error());
        return ktl::err(graphics_queue_rc.error());
    }
    auto graphics_queue = graphics_queue_rc.take_value();

    auto render_rc = ktl::memory::make_shared< example_dynamic_rendering >(
        std::move(_sctx), std::move(_surface), std::move(swapchain), std::move(images), std::move(image_views),
        std::move(fence), std::move(image_available), std::move(render_finished), std::move(command_pool),
        std::move(command_buffer), std::move(present_queue), std::move(graphics_queue));
    if (!render_rc.has_value())
    {
        return ktl::err(render_rc.error());
    }

    return render_rc.take_value();
}

kochou::registry::example_dynamic_rendering::example_dynamic_rendering(
    kochou::shared_context _sctx, kochou::entity::shared_surface _surface, kochou::entity::shared_swapchain _swapchain,
    std::vector< kochou::entity::shared_image > _images, std::vector< kochou::entity::shared_image_view > _image_views,
    kochou::entity::shared_fence _fence, kochou::entity::shared_semaphore _image_available,
    kochou::entity::shared_semaphore _render_finished, kochou::entity::shared_command_pool _command_pool,
    kochou::entity::shared_command_buffer _command_buffer, kochou::entity::shared_queue _present_queue,
    kochou::entity::shared_queue _graphics_queue) noexcept
    : sctx_(std::move(_sctx)), surface_(std::move(_surface)), swapchain_(std::move(_swapchain)),
      images_(std::move(_images)), image_views_(std::move(_image_views)), fence_(std::move(_fence)),
      image_available_(std::move(_image_available)), render_finished_(std::move(_render_finished)),
      command_pool_(std::move(_command_pool)), command_buffer_(std::move(_command_buffer)),
      present_queue_(std::move(_present_queue)), graphics_queue_(std::move(_graphics_queue))
{
}

ktl::errc
kochou::registry::example_dynamic_rendering::draw(kochou::entity::shared_buffer _buffer, ktl::u32 _width,
                                                  ktl::u32                                          _height,
                                                  kochou::registry::shared_example_dynamic_pipeline _pipeline) noexcept
{
    fence_->wait(std::numeric_limits< ktl::u64 >::max());
    fence_->reset();

    ktl::u32         image_index;
    ktl::api::result acquire_rc = ktl::api::acquire_next_image_khr(kochou::view::device(sctx_), swapchain_->raw,
                                                                   std::numeric_limits< ktl::u64 >::max(),
                                                                   image_available_->raw, nullptr, &image_index);
    if (acquire_rc == ktl::api::result::v_error_out_of_date_khr)
    {
        kochou::log::info("require swapchain recreate");
        return ktl::cast_api_result(acquire_rc);
    }

    command_buffer_->begin();
    transition_swapchain_image(command_buffer_->raw, images_[image_index]->raw, ktl::api::image_layout::v_undefined,
                               ktl::api::image_layout::v_color_attachment_optimal);

    ktl::api::rendering_info_khr rendering_info;
    rendering_info.render_area.offset     = {0, 0};
    rendering_info.render_area.extent     = {_width, _height};
    rendering_info.layer_count            = 1;
    rendering_info.color_attachment_count = 1;

    ktl::api::rendering_attachment_info_khr color_attachment;
    color_attachment.image_view        = image_views_[image_index]->raw;
    color_attachment.image_layout      = ktl::api::image_layout::v_attachment_optimal_khr;
    color_attachment.load_op           = ktl::api::attachment_load_op::v_clear;
    color_attachment.store_op          = ktl::api::attachment_store_op::v_store;
    color_attachment.clear_value.color = {{0.0f, 0.0f, 0.0f, 1.0f}};

    rendering_info.p_color_attachments = &color_attachment;

    ktl::api::cmd_begin_rendering(command_buffer_->raw, &rendering_info);
    _pipeline->bind(command_buffer_);
    ktl::api::viewport vp{};
    vp.x         = 0.0f;
    vp.y         = 0.0f;
    vp.width     = rendering_info.render_area.extent.width;
    vp.height    = rendering_info.render_area.extent.height;
    vp.min_depth = 0.0f;
    vp.max_depth = 1.0f;
    ktl::api::cmd_set_viewport(command_buffer_->raw, 0, 1, &vp);
    ktl::api::rect_2d sc{};
    sc.offset = {0, 0};
    sc.extent = rendering_info.render_area.extent;
    ktl::api::cmd_set_scissor(command_buffer_->raw, 0, 1, &sc);
    ktl::api::buffer vertext_buffer[] = {_buffer->raw};
    ktl::api::dvsize offsets[]        = {0};
    ktl::api::cmd_bind_vertex_buffers(command_buffer_->raw, 0, 1, vertext_buffer, offsets);
    ktl::api::cmd_draw(command_buffer_->raw, 3, 1, 0, 0);
    ktl::api::cmd_end_rendering(command_buffer_->raw);
    transition_swapchain_image(command_buffer_->raw, images_[image_index]->raw,
                               ktl::api::image_layout::v_color_attachment_optimal,
                               ktl::api::image_layout::v_present_src_khr);
    command_buffer_->end();

    ktl::api::submit_info          submit_info;
    ktl::api::pipeline_stage_flags wait_stage =
        static_cast< ktl::u32 >(ktl::api::pipeline_stage_flag_bits::v_color_attachment_output_bit);
    submit_info.wait_semaphore_count   = 1;
    submit_info.p_wait_semaphores      = &image_available_->raw;
    submit_info.p_wait_dst_stage_mask  = &wait_stage;
    submit_info.command_buffer_count   = 1;
    submit_info.p_command_buffers      = &command_buffer_->raw;
    submit_info.signal_semaphore_count = 1;
    submit_info.p_signal_semaphores    = &render_finished_->raw;

    ktl::errc submit_rc = graphics_queue_->submit(submit_info, fence_);
    if (submit_rc != ktl::errc::success)
    {
        kochou::log::error("submit_rc={}", submit_rc);
    }

    ktl::api::present_info_khr present_info;
    present_info.wait_semaphore_count = 1;
    present_info.p_wait_semaphores    = &render_finished_->raw;
    present_info.swapchain_count      = 1;
    present_info.p_swapchains         = &swapchain_->raw;
    present_info.p_image_indices      = &image_index;

    auto rc = ktl::api::queue_present_khr(present_queue_->raw, &present_info);
    if (rc != ktl::api::result::v_success)
    {
        kochou::log::error("pr={}", static_cast< ktl::i32 >(rc));
    }

    return ktl::errc::success;
}
