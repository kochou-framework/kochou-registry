#ifndef KOCHOU_REGISTRY_RCU_FENCE_FRAME_HPP
#define KOCHOU_REGISTRY_RCU_FENCE_FRAME_HPP

#include <atomic>
#include <limits>
#include <span>

#include <ktl/errc.hpp>
#include <ktl/result.hpp>

#include <kochou/kochou.hpp>

namespace kochou::registry
{
// light struct
struct entity_collection
{
    std::atomic< entity_collection * > next;
    std::atomic< ktl::u32 >            ref;

    kochou::entity::swapchain               swapchain;
    std::span< kochou::entity::image >      images;
    std::span< kochou::entity::image_view > image_views;

    inline ktl::errc
    inc() noexcept
    {
        ktl::u32 old = ref.fetch_add(1, std::memory_order::relaxed);
        if (old == std::numeric_limits< ktl::u32 >::max())
        {
            return ktl::errc::overflow;
        }
        return ktl::errc::success;
    }

    inline void
    dec() noexcept
    {
        ktl::u32 old = ref.fetch_sub(1, std::memory_order::acq_rel);
        if (old == 1)
        {
            destroy(this);
        }
    }

    static ktl::result< entity_collection *, ktl::errc >
    make(kochou::shared_context _sctx, const kochou::entity::surface & _surface,
         const kochou::entity::swapchain::io_info _io_info) noexcept;

    static void
    destroy(entity_collection * _collection);
};

template < typename FRAME_DATA >
struct rcu_fence_frame
{
    kochou::entity::fence fence;
    entity_collection *   collection;
    FRAME_DATA            data;
};

class rcu_fence_frame1
{
public:
    static ktl::errc
    ensure(kochou::shared_context _sctx) noexcept;
    static ktl::errc
    should(kochou::shared_context _sctx) noexcept;
    static bool
    allowed(kochou::shared_context _sctx) noexcept;

public:
    rcu_fence_frame(kochou::shared_context _sctx) noexcept;
    rcu_fence_frame(const rcu_fence_frame &) noexcept = delete;
    rcu_fence_frame(rcu_fence_frame &&) noexcept;
    rcu_fence_frame &
    operator=(const rcu_fence_frame &) noexcept = delete;
    rcu_fence_frame &
    operator=(rcu_fence_frame &&) noexcept;
    ~rcu_fence_frame() noexcept;

private:
};
} // namespace kochou::registry

#endif
