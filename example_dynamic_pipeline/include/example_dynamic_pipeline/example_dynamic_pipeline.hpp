#ifndef KOCHOU_REGISTRY_EXAMPLE_DYNAMIC_PIPELINE_HPP
#define KOCHOU_REGISTRY_EXAMPLE_DYNAMIC_PIPELINE_HPP

#include <ktl/memory.hpp>

#include <kochou/kochou.hpp>

namespace kochou::registry
{
class example_dynamic_pipeline;
using shared_example_dynamic_pipeline = ktl::memory::sptr< example_dynamic_pipeline >;

class example_dynamic_pipeline
{
public:
    struct vertex
    {
        float pos[3];
        float color[3];
    };

    struct input_info
    {
        ktl::api::pipeline_vertex_input_state_create_info          vertex_info;
        kochou::entity::shared_pipeline_layout                     pipeline_layout;
        std::vector< kochou::entity::shared_shader_module >        shader_modules;
        std::vector< ktl::api::pipeline_shader_stage_create_info > shader_stages;
    };

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
    static ktl::result< shared_example_dynamic_pipeline, ktl::errc >
    make(kochou::shared_context, input_info && _info) noexcept;

    // common
public:
    example_dynamic_pipeline(kochou::entity::shared_pipeline _pipeline) noexcept;
    example_dynamic_pipeline(const example_dynamic_pipeline &) noexcept = delete;
    example_dynamic_pipeline(example_dynamic_pipeline &&) noexcept      = default;
    example_dynamic_pipeline &
    operator=(const example_dynamic_pipeline &) noexcept = delete;
    example_dynamic_pipeline &
    operator=(example_dynamic_pipeline &&) noexcept = default;
    ~example_dynamic_pipeline()                     = default;

    // methods
public:
    void
    bind(kochou::entity::shared_command_buffer _command_buffer) noexcept;

    // shared_raii
private:
    kochou::entity::shared_pipeline pipeline_;
};
} // namespace kochou::registry

#endif
