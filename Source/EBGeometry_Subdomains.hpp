// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file   EBGeometry_Subdomains.hpp
 * @brief  Subdomains: the subdomain of each region of a tape, read on the host or a device.
 * @author Robert Marskar
 */

#ifndef EBGEOMETRY_SUBDOMAINS_HPP
#define EBGEOMETRY_SUBDOMAINS_HPP

// Std includes
#include <cstdint>
#include <type_traits>
#include <vector>

// Our includes
#include "EBGeometry_GPU.hpp"
#include "EBGeometry_Macros.hpp"
#include "EBGeometry_PODVector.hpp"
#include "EBGeometry_Pool.hpp"
#include "EBGeometry_SurfaceRef.hpp"

namespace EBGeometry {

/**
 * @brief What a surface is to a domain, at a point, from the regions on its two sides (see Subdomains::role()).
 */
enum class SurfaceRole : uint8_t
{
  None,     ///< Neither side is in a subdomain: the solver does not cut it.
  Boundary, ///< One side is in a subdomain: cut once, from that side; its surface group gives its meaning (a boundary condition, say).
  Interface, ///< The two sides are in two different subdomains: one surface the solver couples across.
  Internal ///< Both sides are in the same subdomain (two of its regions touching): the solver decides; usually not cut.
};

/**
 * @brief The subdomains of a domain (see Domain), read on the host or a device: the subdomain of each region of its
 * tape, and what a surface is to them.
 * @details A view of a table in a pool, a row per region of the tape: trivially copyable, a pool handle (see
 * PoolHandle), what Tape::signedDistance(p, subdomains, subdomain) and Tape::closestSurface(p, subdomains, subdomain)
 * take. A Domain builds it from a DomainSetup and returns it (Domain::subdomains()); the pool must outlive it. A
 * default-constructed one has no subdomains: every region is in none.
 *
 * A table rather than ranges of regions: on a device, finding a region's subdomain is one load whatever the number of
 * subdomains, where scanning ranges in the pool grew with them and scanning them in a kernel's parameters capped them.
 */
class Subdomains : public PoolHandle
{
public:
  /**
   * @brief No subdomain, or a region in no subdomain.
   */
  static constexpr uint32_t None = 0xFFFFFFFFU;

  /**
   * @brief The arrays, as offsets: what a Domain stores.
   */
  struct Layout
  {
    /**
     * @brief The subdomain of each region, or None.
     */
    PODVector<uint32_t> m_table;

    /**
     * @brief How many subdomains there are.
     */
    uint32_t m_numSubdomains = 0;
  };

  /**
   * @brief No subdomains: every region is in none.
   */
  Subdomains() noexcept = default;

  /**
   * @brief Build the table in a pool.
   * @details Aborts with a message, in every build, for a subdomain number not below @p a_numSubdomains.
   * @param[in,out] a_pool          Pool to reserve the table from (not frozen).
   * @param[in]     a_subdomainOf   The subdomain of each region of the tape, or None.
   * @param[in]     a_numSubdomains How many subdomains there are.
   */
  EBGEOMETRY_HOST
  Subdomains(Pool& a_pool, const std::vector<uint32_t>& a_subdomainOf, const uint32_t a_numSubdomains)
  {
    this->attach(a_pool, "Subdomains");

    m_layout.m_table.reserveSizedFrom(a_pool, static_cast<uint32_t>(a_subdomainOf.size()));
    m_layout.m_numSubdomains = a_numSubdomains;

    for (uint32_t r = 0; r < a_subdomainOf.size(); r++) {
      EBGEOMETRY_REQUIRE(a_subdomainOf[r] == None || a_subdomainOf[r] < a_numSubdomains,
                         "Subdomains: region %u is in subdomain %u of %u",
                         unsigned(r),
                         unsigned(a_subdomainOf[r]),
                         unsigned(a_numSubdomains));

      m_layout.m_table.at(this->base(), r) = a_subdomainOf[r];
    }
  }

  /**
   * @brief Rebuild a view from a stored layout and the location of the pool that holds it (as a Domain does).
   * @param[in] a_layout   The arrays.
   * @param[in] a_location Where they live.
   */
  EBGEOMETRY_HOST_DEVICE
  Subdomains(const Layout& a_layout, const PoolLocation& a_location) noexcept
    : PoolHandle(a_location), m_layout(a_layout)
  {}

  /**
   * @brief The arrays, as offsets.
   * @return The layout.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline const Layout&
  layout() const noexcept
  {
    return m_layout;
  }

  /**
   * @brief How many subdomains there are.
   * @return The count.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline uint32_t
  getNumSubdomains() const noexcept
  {
    return m_layout.m_numSubdomains;
  }

  /**
   * @brief How many regions the table has a row for: the tape's.
   * @return The count.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline uint32_t
  getNumRegions() const noexcept
  {
    return m_layout.m_table.size();
  }

  /**
   * @brief The subdomain a region is in.
   * @param[in] a_region A region of the tape; a region past its last is in none.
   * @return The subdomain, or None.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline uint32_t
  subdomainOf(const uint32_t a_region) const noexcept
  {
    return (a_region < m_layout.m_table.size()) ? m_layout.m_table.at(this->base(), a_region) : None;
  }

  /**
   * @brief What a surface is to the subdomains at a point: from the regions on its two sides there, as a tape's
   * region queries report them (Tape::closestSurface(p, region)).
   * @tparam T Floating-point precision.
   * @param[in] a_surface The surface, with its sides.
   * @return Its role; None when the sides are not set.
   */
  template <class T>
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline SurfaceRole
  role(const SurfaceRef<T>& a_surface) const noexcept
  {
    const uint32_t minus =
      (a_surface.m_ownerRegion != SurfaceRef<T>::None) ? this->subdomainOf(a_surface.m_ownerRegion) : None;
    const uint32_t plus =
      (a_surface.m_neighborRegion != SurfaceRef<T>::None) ? this->subdomainOf(a_surface.m_neighborRegion) : None;

    if (minus == None && plus == None) {
      return SurfaceRole::None;
    }
    if (minus == None || plus == None) {
      return SurfaceRole::Boundary;
    }

    return (minus == plus) ? SurfaceRole::Internal : SurfaceRole::Interface;
  }

  /**
   * @brief A view resolving against another pool of the table's mirror chain (see PoolHandle).
   * @param[in] a_pool The pool, or a mirror of it.
   * @return The view.
   */
  [[nodiscard]] EBGEOMETRY_HOST
  inline Subdomains
  rebasedView(const Pool& a_pool) const noexcept
  {
    Subdomains view = *this;

    view.setLocation(this->rebasedLocation(a_pool, m_layout.m_table.endByte(), "Subdomains"));

    return view;
  }

private:
  /**
   * @brief The arrays.
   */
  Layout m_layout;
};

static_assert(std::is_trivially_copyable_v<Subdomains>,
              "Subdomains must be trivially copyable, for a kernel to take it");

} // namespace EBGeometry

#endif
