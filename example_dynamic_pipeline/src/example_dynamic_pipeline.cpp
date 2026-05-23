#include <example_dynamic_pipeline/example_dynamic_pipeline.hpp>

namespace
{
static constexpr ktl::api::pipeline_input_assembly_state_create_info input_assembly = {
    .topology = ktl::api::primitive_topology::v_triangle_list, .primitive_restart_enable = false};

static constexpr ktl::api::pipeline_viewport_state_create_info viewport_state = {.viewport_count = 1,
                                                                                 .scissor_count  = 1};

static constexpr ktl::api::pipeline_rasterization_state_create_info rasterizer = {
    .polygon_mode = ktl::api::polygon_mode::v_fill,
    .cull_mode    = static_cast< ktl::api::flag32 >(ktl::api::cull_mode_flag_bits::v_none),
    .front_face   = ktl::api::front_face::v_counter_clockwise,
    .line_width   = 1.0f};

static constexpr ktl::api::pipeline_multisample_state_create_info multisample = {
    .rasterization_samples = ktl::api::sample_count_flag_bits::v_1bit};

static constexpr ktl::api::pipeline_color_blend_attachment_state blend_attachment = {
    .blend_enable     = false,
    .color_write_mask = static_cast< ktl::api::flag32 >(ktl::api::color_component_flag_bits::v_r_bit) |
                        static_cast< ktl::api::flag32 >(ktl::api::color_component_flag_bits::v_g_bit) |
                        static_cast< ktl::api::flag32 >(ktl::api::color_component_flag_bits::v_b_bit) |
                        static_cast< ktl::api::flag32 >(ktl::api::color_component_flag_bits::v_a_bit)};

static constexpr ktl::api::pipeline_color_blend_state_create_info color_blend = {.attachment_count = 1,
                                                                                 .p_attachments    = &blend_attachment};

static constexpr ktl::api::dynamic_state dynamic_states[2] = {ktl::api::dynamic_state::v_viewport,
                                                              ktl::api::dynamic_state::v_scissor};

static constexpr ktl::api::pipeline_dynamic_state_create_info dynamic_state = {.dynamic_state_count = 2,
                                                                               .p_dynamic_states    = dynamic_states};

static constexpr ktl::api::format color_format = ktl::api::format::v_b_8g_8r_8a_8unorm;

static constexpr ktl::api::pipeline_rendering_create_info rendering_info = {
    .color_attachment_count     = 1,
    .p_color_attachment_formats = &color_format,
    .depth_attachment_format    = ktl::api::format::v_undefined,
    .stencil_attachment_format  = ktl::api::format::v_undefined};
} // namespace

ktl::errc
kochou::registry::example_dynamic_pipeline::ensure(kochou::shared_context _sctx) noexcept
{
    ktl::errc rc = ktl::errc::success;

    rc = kochou::ensure< kochou::extension< ktl::api::extension::khr_dynamic_rendering > >(_sctx);
    if (rc != ktl::errc::success)
    {
        kochou::log::error("ensure ktl::api::extension::khr_dynamic_rendering failed, rc={}", rc);
        return rc;
    }

    rc = kochou::ensure< kochou::feature< ktl::api::feature::dynamic_rendering > >(_sctx);
    if (rc != ktl::errc::success)
    {
        kochou::log::error("ensure ktl::api::feature::dynamic_rendering failed, rc={}", rc);
        return rc;
    }

    return rc;
}

ktl::errc
kochou::registry::example_dynamic_pipeline::should(kochou::shared_context _sctx) noexcept
{
    ktl::errc rc = ktl::errc::success;

    rc = kochou::should< kochou::extension< ktl::api::extension::khr_dynamic_rendering > >(_sctx);
    if (rc != ktl::errc::success)
    {
        kochou::log::error("should ktl::api::extension::khr_dynamic_rendering failed, rc={}", rc);
        return rc;
    }

    rc = kochou::should< kochou::feature< ktl::api::feature::dynamic_rendering > >(_sctx);
    if (rc != ktl::errc::success)
    {
        kochou::log::error("should ktl::api::feature::dynamic_rendering failed, rc={}", rc);
        return rc;
    }

    return rc;
}

bool
kochou::registry::example_dynamic_pipeline::allowed(kochou::shared_context _sctx) noexcept
{
    return kochou::allowed< kochou::extension< ktl::api::extension::khr_dynamic_rendering > >(_sctx) &&
           kochou::allowed< kochou::feature< ktl::api::feature::dynamic_rendering > >(_sctx);
}

ktl::result< kochou::registry::shared_example_dynamic_pipeline, ktl::errc >
kochou::registry::example_dynamic_pipeline::make(kochou::shared_context _sctx, input_info && _info) noexcept
{
    ktl::api::graphics_pipeline_create_info pipeline_info = {};
    pipeline_info.p_vertex_input_state                    = &_info.vertex_info;
    pipeline_info.p_input_assembly_state                  = &input_assembly;
    pipeline_info.p_viewport_state                        = &viewport_state;
    pipeline_info.p_rasterization_state                   = &rasterizer;
    pipeline_info.p_multisample_state                     = &multisample;
    pipeline_info.p_depth_stencil_state                   = nullptr;
    pipeline_info.p_color_blend_state                     = &color_blend;
    pipeline_info.p_dynamic_state                         = &dynamic_state;
    pipeline_info.render_pass                             = nullptr;
    pipeline_info.pnext                                   = &rendering_info;
    pipeline_info.stage_count                             = _info.shader_stages.size();
    pipeline_info.p_stages                                = _info.shader_stages.data();
    pipeline_info.layout                                  = _info.pipeline_layout->raw;

    auto pipeline_rc =
        kochou::entity::pipeline::make(_sctx, _info.pipeline_layout, _info.shader_modules, pipeline_info);
    if (!pipeline_rc.has_value())
    {
        kochou::log::error("pipeline::make failed, rc={}", pipeline_rc.error());
        return ktl::err(pipeline_rc.error());
    }
    auto pipeline = pipeline_rc.take_value();

    auto shared_dynamic_pipeline_rc = ktl::memory::make_shared< example_dynamic_pipeline >(std::move(pipeline));
    if (!shared_dynamic_pipeline_rc.has_value())
    {
        return ktl::err(shared_dynamic_pipeline_rc.error());
    }

    return shared_dynamic_pipeline_rc.take_value();
}

kochou::registry::example_dynamic_pipeline::example_dynamic_pipeline(kochou::entity::shared_pipeline _pipeline) noexcept
    : pipeline_(std::move(_pipeline))
{
}

void
kochou::registry::example_dynamic_pipeline::bind(kochou::entity::shared_command_buffer _command_buffer) noexcept
{
    ktl::api::cmd_bind_pipeline(_command_buffer->raw, ktl::api::pipeline_bind_point::v_graphics, pipeline_->raw);
}
