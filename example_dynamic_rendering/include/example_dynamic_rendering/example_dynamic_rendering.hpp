#ifndef KOCHOU_REGISTRY_EXAMPLE_DYNAMIC_RENDERING_HPP
#define KOCHOU_REGISTRY_EXAMPLE_DYNAMIC_RENDERING_HPP

#include <array>

#include <ktl/errc.hpp>
#include <ktl/memory.hpp>

#include <kochou/kochou.hpp>

namespace kochou::registry
{
class dynamic_rendering;
using shared_dynamic_rendering = ktl::memory::sptr< shared_dynamic_rendering >;

class dynamic_rendering
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
    // common
public:
private:
    std::array< ktl::memory::sptr< void >, 10 > shared_raii_;
};
} // namespace kochou::registry

#endif
