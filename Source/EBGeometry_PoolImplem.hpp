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

inline Pool::Pool(MemoryResource& a_resource, size_t a_initialBytes) : m_resource(&a_resource)
{
  // A build pool must be host-accessible: reserve()/push_back()/grow() write through base() with the
  // host CPU. Device-resident pools are produced only by mirror() (via the private MirrorTag
  // constructor). This guard is always-on, NOT EBGEOMETRY_EXPECT, because a host store into device
  // memory in a release build is silent undefined behaviour, not a recoverable precondition slip.
  if (!a_resource.isHostAccessible()) {
    std::fprintf(stderr,
                 "EBGeometry::Pool: a build pool requires a host-accessible MemoryResource; a "
                 "device-resident pool is produced only by Pool::mirror\n");
    std::abort();
  }

  if (a_initialBytes > 0) {
    const size_t rounded = (a_initialBytes + (PoolBaseAlign - 1)) & ~(PoolBaseAlign - 1);

    m_base     = m_resource->allocate(rounded, PoolBaseAlign);
    m_capacity = rounded;
  }
}

inline Pool::~Pool() noexcept
{
  if (m_base != nullptr) {
    m_resource->deallocate(m_base, m_capacity, PoolBaseAlign);
  }
}

inline Pool::Pool(Pool&& a_other) noexcept
  : m_resource(a_other.m_resource),
    m_base(a_other.m_base),
    m_size(a_other.m_size),
    m_capacity(a_other.m_capacity),
    m_frozen(a_other.m_frozen)
{
  a_other.m_base     = nullptr;
  a_other.m_size     = 0;
  a_other.m_capacity = 0;
  a_other.m_frozen   = false;
}

inline Pool&
Pool::operator=(Pool&& a_other) noexcept
{
  if (this != &a_other) {
    if (m_base != nullptr) {
      m_resource->deallocate(m_base, m_capacity, PoolBaseAlign);
    }

    m_resource = a_other.m_resource;
    m_base     = a_other.m_base;
    m_size     = a_other.m_size;
    m_capacity = a_other.m_capacity;
    m_frozen   = a_other.m_frozen;

    a_other.m_base     = nullptr;
    a_other.m_size     = 0;
    a_other.m_capacity = 0;
    a_other.m_frozen   = false;
  }

  return *this;
}

inline uint64_t
Pool::reserve(size_t a_count, size_t a_elemSize, size_t a_alignment)
{
  EBGEOMETRY_EXPECT(!m_frozen);                              // no reserve after freeze
  EBGEOMETRY_EXPECT(a_alignment <= PoolBaseAlign);           // base is 256-aligned; larger unsupported
  EBGEOMETRY_EXPECT((a_alignment & (a_alignment - 1)) == 0); // power of two

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

  if (m_base != nullptr) {
    std::memcpy(newBase, m_base, m_size);
    m_resource->deallocate(m_base, m_capacity, PoolBaseAlign);
  }

  m_base     = newBase;
  m_capacity = newCap;
}

inline void
Pool::freeze() noexcept
{
  m_frozen = true;
}

inline Pool
Pool::mirror(const Pool& a_src, MemoryResource& a_dstResource)
{
  EBGEOMETRY_EXPECT(a_src.isFrozen());

  // MirrorTag: the destination may be device-resident (not host-accessible), so it must NOT go
  // through the public constructor's host-accessibility guard. mirror allocates its exact block
  // directly below and freezes it; it is never reserved into.
  Pool dst(a_dstResource, MirrorTag{});

  if (a_src.m_size > 0) {
    dst.m_base = a_dstResource.allocate(a_src.m_size, PoolBaseAlign); // exact size, no slack

    const bool srcHost = a_src.m_resource->isHostAccessible();
    const bool dstHost = a_dstResource.isHostAccessible();

    if (srcHost && dstHost) {
      std::memcpy(dst.m_base, a_src.m_base, a_src.m_size);
    }
#if defined(EBGEOMETRY_CUDA) || defined(EBGEOMETRY_HIP)
    else if (srcHost && !dstHost) {
      EBGEOMETRY_GPU_CHECK(GPU::memcpy(dst.m_base, a_src.m_base, a_src.m_size, GPU::MemcpyHostToDevice));
    }
    else if (!srcHost && dstHost) {
      EBGEOMETRY_GPU_CHECK(GPU::memcpy(dst.m_base, a_src.m_base, a_src.m_size, GPU::MemcpyDeviceToHost));
    }
    else {
      // Device-to-device is out of scope for the mirror (the foundation only builds on host and
      // uploads once); the alias layer exposes no device-to-device direction. Always-on abort, NOT
      // EBGEOMETRY_EXPECT, so a release build fails hard here instead of returning a frozen pool
      // whose block was allocated but never copied (silent garbage).
      std::fprintf(stderr, "EBGeometry::Pool::mirror: device-to-device mirroring is not supported\n");
      std::abort();
    }
#else
    else {
      // With no offload backend active there is no device placement, so a non-host source or
      // destination is impossible here. Always-on abort for the same reason as above.
      std::fprintf(stderr, "EBGeometry::Pool::mirror: non-host memory requires a GPU backend\n");
      std::abort();
    }
#endif
  }

  dst.m_capacity = a_src.m_size;
  dst.m_size     = a_src.m_size;
  dst.m_frozen   = true;

  return dst;
}

} // namespace EBGeometry

#endif // EBGEOMETRY_POOLIMPLEM_HPP
