// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file   EBGeometry_PoolImplem.hpp
 * @brief  Implementation of EBGeometry_Pool.hpp
 * @author Robert Marskar
 */

#ifndef EBGEOMETRY_POOLIMPLEM_HPP
#define EBGEOMETRY_POOLIMPLEM_HPP

// Std includes
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <utility>

// Our includes
#include "EBGeometry_GPU.hpp"
#include "EBGeometry_GPURuntime.hpp"
#include "EBGeometry_Macros.hpp"
#include "EBGeometry_MemoryResource.hpp"
#include "EBGeometry_Pool.hpp"

namespace EBGeometry {

inline Pool::Pool(MemoryResource& a_resource, size_t a_initialBytes) : m_resource(&a_resource), m_control(makeControl())
{
  // A build pool must be host-accessible: reserve()/push_back()/grow() write through base() with the
  // host CPU. Device-resident pools are produced only by mirror() (via the private MirrorTag
  // constructor). A host store into device memory would be silent undefined behaviour.
  EBGEOMETRY_REQUIRE(a_resource.isHostAccessible(),
                     "Pool: a build pool requires a host-accessible MemoryResource; a device-resident pool is "
                     "produced only by Pool::mirror");

  if (a_initialBytes > 0) {
    const size_t rounded = (a_initialBytes + (PoolBaseAlign - 1)) & ~(PoolBaseAlign - 1);

    m_control->m_base = m_resource->allocate(rounded, PoolBaseAlign);
    m_capacity        = rounded;
  }
}

inline Pool::~Pool() noexcept
{
  if (m_control != nullptr && m_control->m_base != nullptr) {
    m_resource->deallocate(m_control->m_base, m_capacity, PoolBaseAlign);
  }
}

// The move steals the control block itself, so the block's *address* is unchanged and every
// pool-resident object pointing at it keeps resolving -- that is the control block's entire reason
// for existing. Nothing about the base needs fixing up here.
inline Pool::Pool(Pool&& a_other) noexcept
  : m_resource(a_other.m_resource),
    m_control(std::move(a_other.m_control)),
    m_size(a_other.m_size),
    m_capacity(a_other.m_capacity),
    m_mirrorOf(a_other.m_mirrorOf),
    m_frozen(a_other.m_frozen)
{
  a_other.m_size     = 0;
  a_other.m_capacity = 0;
  a_other.m_mirrorOf = 0;
  a_other.m_frozen   = false;
}

inline Pool&
Pool::operator=(Pool&& a_other) noexcept
{
  if (this != &a_other) {
    if (m_control != nullptr && m_control->m_base != nullptr) {
      m_resource->deallocate(m_control->m_base, m_capacity, PoolBaseAlign);
    }

    m_resource = a_other.m_resource;
    m_control  = std::move(a_other.m_control);
    m_size     = a_other.m_size;
    m_capacity = a_other.m_capacity;
    m_mirrorOf = a_other.m_mirrorOf;
    m_frozen   = a_other.m_frozen;

    a_other.m_size     = 0;
    a_other.m_capacity = 0;
    a_other.m_mirrorOf = 0;
    a_other.m_frozen   = false;
  }

  return *this;
}

inline uint64_t
Pool::reserve(size_t a_count, size_t a_elemSize, size_t a_alignment)
{
  // A moved-from pool owns no control block, so every path below (and every base resolution by an
  // object reserved here) would dereference null.
  EBGEOMETRY_REQUIRE(m_control != nullptr, "Pool::reserve: cannot reserve from a moved-from pool");

  // A frozen block may already have been mirrored, and growing it would silently diverge from the
  // mirror (and invalidate a captured base).
  EBGEOMETRY_REQUIRE(!m_frozen, "Pool::reserve: cannot reserve from a frozen pool");

  // Zero would underflow the mask below; the base is only PoolBaseAlign-aligned, so a larger
  // alignment cannot be honoured.
  EBGEOMETRY_REQUIRE(a_alignment > 0 && (a_alignment & (a_alignment - 1)) == 0 && a_alignment <= PoolBaseAlign,
                     "Pool::reserve: the alignment must be a power of two no larger than %zu (%zu)",
                     PoolBaseAlign,
                     a_alignment);

  const size_t bytes   = a_count * a_elemSize;
  const size_t aligned = (m_size + (a_alignment - 1)) & ~(a_alignment - 1);
  const size_t need    = aligned + bytes;

  if (need > m_capacity) {
    this->grow(need);
  }

  m_size = need;

  return static_cast<uint64_t>(aligned);
}

inline void
Pool::grow(size_t a_need)
{
  EBGEOMETRY_EXPECT(m_resource->isHostAccessible()); // device blocks are never grown

  size_t newCap = (m_capacity > 0) ? (2 * m_capacity) : PoolBaseAlign;

  if (a_need > newCap) {
    newCap = a_need;
  }

  newCap = (newCap + (PoolBaseAlign - 1)) & ~(PoolBaseAlign - 1);

  void* newBase = m_resource->allocate(newCap, PoolBaseAlign);

  if (m_control->m_base != nullptr) {
    std::memcpy(newBase, m_control->m_base, m_size);
    m_resource->deallocate(m_control->m_base, m_capacity, PoolBaseAlign);
  }

  // Publishing the new base through the control block is what makes pool-resident objects immune to
  // a grow: they re-read it on every access. Addresses already resolved against the old base are
  // dangling from here on -- see the warning on reserve().
  m_control->m_base = newBase;
  m_capacity        = newCap;
}

inline void
Pool::freeze() noexcept
{
  m_frozen = true;
}

inline Pool
Pool::mirror(const Pool& a_src, MemoryResource& a_dstResource)
{
  // An unfrozen source could still grow after the copy, leaving the mirror stale.
  EBGEOMETRY_REQUIRE(a_src.isFrozen(), "Pool::mirror: the source pool must be frozen before it is mirrored");

  // MirrorTag: the destination may be device-resident (not host-accessible), so it must NOT go
  // through the public constructor's host-accessibility guard. mirror allocates its exact block
  // directly below and freezes it; it is never reserved into.
  Pool dst(a_dstResource, MirrorTag{});

  if (a_src.m_size > 0) {
    void* const srcBase = a_src.m_control->m_base;
    void* const dstBase = a_dstResource.allocate(a_src.m_size, PoolBaseAlign); // exact size, no slack

    dst.m_control->m_base = dstBase;

    // The copy itself belongs to the resources: only a device resource (defined only in translation
    // units compiled with a GPU backend) knows how to reach device memory. Backend #ifs here would
    // give this non-template inline function different definitions in host and device translation
    // units, and the linker would keep just one of them.
    const MemoryResource& copier = !a_dstResource.isHostAccessible()       ? a_dstResource
                                   : !a_src.m_resource->isHostAccessible() ? *a_src.m_resource
                                                                           : a_dstResource;

    copier.copy(dstBase, a_dstResource, srcBase, *a_src.m_resource, a_src.m_size);
  }

  dst.m_capacity = a_src.m_size;
  dst.m_size     = a_src.m_size;
  dst.m_frozen   = true;

  // Record the *root* of the mirror chain, not the immediate source: mirroring host -> pinned
  // staging -> device must leave the device pool naming the original host pool, so that an object
  // built in that host pool can still validate a rebase onto the device pool.
  dst.m_mirrorOf          = (a_src.m_mirrorOf != 0) ? a_src.m_mirrorOf : a_src.id();
  dst.m_control->m_rootId = a_src.m_control->m_rootId;

  return dst;
}

inline bool
PoolLocation::isAttachedTo(const Pool& a_pool) const noexcept
{
  return m_control != nullptr && m_control == a_pool.control();
}

inline void
PoolLocation::attach(const Pool& a_pool, const char* a_who) noexcept
{
  EBGEOMETRY_REQUIRE(m_control == nullptr || m_control == a_pool.control(),
                     "%s: all of its arrays must be reserved from the same Pool",
                     a_who);

  m_control        = a_pool.control();
  m_base           = nullptr;
  m_hostAccessible = false;
}

inline PoolLocation
PoolLocation::rebasedOnto(const Pool& a_pool, const uint64_t a_endByte, const char* a_who) const noexcept
{
  EBGEOMETRY_REQUIRE(m_control != nullptr,
                     "%s::rebasedView: rebase the original, not a view onto a device-accessible pool or an object "
                     "that was never built",
                     a_who);
  EBGEOMETRY_REQUIRE(a_pool.rootId() == m_control->m_rootId,
                     "%s::rebasedView: the pool must be the object's own pool or a mirror of it (its mirror chain "
                     "starts at pool %llu, the object's at pool %llu)",
                     a_who,
                     static_cast<unsigned long long>(a_pool.rootId()),
                     static_cast<unsigned long long>(m_control->m_rootId));

  // A mirror taken before the object's last reserve is too small to hold its arrays.
  EBGEOMETRY_REQUIRE(a_endByte <= a_pool.usedBytes(),
                     "%s::rebasedView: the object's arrays must fit inside the pool (they end at byte %llu, the pool "
                     "holds %zu)",
                     a_who,
                     static_cast<unsigned long long>(a_endByte),
                     a_pool.usedBytes());

  PoolLocation location;

  if (a_pool.resource().isDeviceAccessible()) {
    // A kernel cannot follow a host control block, so the base is captured by value. That is safe only
    // for a frozen pool, whose base cannot move: a mirror is frozen when it is made, and a pool built
    // directly in managed memory must be frozen first.
    EBGEOMETRY_REQUIRE(a_pool.isFrozen(), "%s::rebasedView: a device-accessible pool must be frozen", a_who);

    location.m_base           = a_pool.base();
    location.m_hostAccessible = a_pool.resource().isHostAccessible();
  }
  else {
    // Host pool: follow its control block rather than snapshot its base, so the view is immune to
    // growth exactly like the original.
    location.m_control = a_pool.control();
  }

  return location;
}

} // namespace EBGeometry

#endif // EBGEOMETRY_POOLIMPLEM_HPP
