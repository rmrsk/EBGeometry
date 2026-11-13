// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file   EBGeometry_Domain.hpp
 * @brief  Domain: the regions a downstream code computes in and its groups of surfaces, over a tape.
 * @author Robert Marskar
 */

#ifndef EBGEOMETRY_DOMAIN_HPP
#define EBGEOMETRY_DOMAIN_HPP

// Std includes
#include <cstddef>
#include <cstdint>
#include <type_traits>

// Our includes
#include "EBGeometry_DomainSetup.hpp"
#include "EBGeometry_GPU.hpp"
#include "EBGeometry_Macros.hpp"
#include "EBGeometry_Math.hpp"
#include "EBGeometry_Pool.hpp"
#include "EBGeometry_Subdomains.hpp"
#include "EBGeometry_SurfaceGroups.hpp"
#include "EBGeometry_SurfaceRef.hpp"
#include "EBGeometry_Tape.hpp"

namespace EBGeometry {

/**
 * @brief A computational domain over a tape, for a downstream code (a solver EBGeometry does not provide): the regions
 * it computes in (its subdomains) and its surface groups (each a set of surfaces the caller gives one meaning, a
 * boundary condition say: named by the caller, numbered here), with the queries a cut-cell code makes.
 * @details Built on the host from a DomainSetup, for one tape, which it holds: its queries take no tape, so a domain
 * cannot be used with a tape it was not built for. Its tables, a subdomain per region of the tape and the selectors
 * with their surface groups, live in a pool (the tables pool); there is no limit on subdomains, surface groups or
 * selectors. A trivially copyable handle (see PoolHandle): rebasedView() moves it to a device, where every query is
 * callable.
 *
 * The tables pool is usually the tape's own: one mirror, one lifetime. It may be another one, so that a domain built
 * after the tape's pool was mirrored (a viewer's edited surface groups, a solver changing its setup) costs a copy of
 * the small tables only, never the geometry. That is why a domain holds its tape as a handle, with the tape's own
 * location, beside the location of its tables: the one pool-resident class holding a second handle rather than the
 * layouts of its parts, so that several domains can share one tape whose pool is frozen.
 *
 * The domain divides space among its subdomains and Excluded, the regions in none (where it does not compute). Every
 * point is in one of them (subdomainAt()), and each has a function negative exactly in it (signedDistance()), so that
 * at a point, off the surfaces, exactly one is negative. A cut-cell code samples them all at a node
 * (signedDistances()): an edge whose ends are in different subdomains (or Excluded) crosses a surface, an interface between two
 * subdomains (where their two functions are zero, with opposite signs) or a subdomain's boundary with the excluded
 * regions (where its function is zero); its root along the edge is the crossing. Excluded's function is the negative
 * of the function of all the subdomains together. Materials and their properties are the regions' (MaterialLabel()).
 *
 * A domain and every copy and view of it need the tape's pool and the tables pool (or their mirrors) to outlive them.
 *
 * @tparam T              Floating-point precision, the tape's.
 * @tparam UserPrimitives The tape's user primitive types and features. Both deduced from the constructor.
 */
template <class T, class... UserPrimitives>
class Domain : public PoolHandle
{
public:
  /**
   * @brief The tape it is built for.
   */
  using TapeType = Tape<T, UserPrimitives...>;

  /**
   * @brief What subdomainAt() gives for a point in the regions of no subdomain.
   */
  static constexpr uint32_t Excluded = Subdomains::None;

  /**
   * @brief Build a domain over a tape.
   * @details Aborts with a message, in every build, when a subdomain of the setup holds a region the tape does not
   * have, or when the tables pool is frozen.
   * @param[in,out] a_tables The pool to hold the domain's tables: the tape's pool, or another.
   * @param[in]     a_tape   The tape, which must outlive the domain with its pool.
   * @param[in]     a_setup  How the domain is made.
   */
  EBGEOMETRY_HOST
  Domain(Pool& a_tables, const TapeType& a_tape, const DomainSetup& a_setup) : m_tape(a_tape)
  {
    this->attach(a_tables, "Domain");

    m_subdomains    = a_setup.buildSubdomains(a_tables, a_tape.getNumRegions()).layout();
    m_surfaceGroups = a_setup.buildSurfaceGroups(a_tables).layout();
  }

  /**
   * @brief The tape the domain was built for.
   * @return The tape.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline const TapeType&
  tape() const noexcept
  {
    return m_tape;
  }

  /**
   * @brief The subdomains, for the tape's own queries that take them (Tape::firstSurface(ray, ..., subdomains, ...)).
   * @return A view of the table.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline Subdomains
  subdomains() const noexcept
  {
    return Subdomains(m_subdomains, this->location());
  }

  /**
   * @brief The surface groups.
   * @return A view of the selectors.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline SurfaceGroups
  surfaceGroups() const noexcept
  {
    return SurfaceGroups(m_surfaceGroups, this->location());
  }

  /**
   * @brief How many functions signedDistances() gives: one per subdomain, then Excluded's.
   * @return The count, the number of subdomains plus one.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline uint32_t
  getNumSubdomainFunctions() const noexcept
  {
    return m_subdomains.m_numSubdomains + 1U;
  }

  /**
   * @brief Whether it computes in a region.
   * @param[in] a_region The region.
   * @return True if the region is in one of its subdomains.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline bool
  computesIn(const uint32_t a_region) const noexcept
  {
    return this->subdomains().subdomainOf(a_region) != Subdomains::None;
  }

  /**
   * @brief Whether a surface between two regions is one it must give a condition: one side or both in its regions.
   * @param[in] a_first  A region on one side.
   * @param[in] a_second The region on the other.
   * @return True if it touches the domain's regions.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline bool
  touches(const uint32_t a_first, const uint32_t a_second) const noexcept
  {
    return this->computesIn(a_first) || this->computesIn(a_second);
  }

  /**
   * @brief The subdomain a point is in: that of the region it is in, or Excluded.
   * @param[in] a_point Point.
   * @return Its subdomain, or Excluded.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline uint32_t
  subdomainAt(const Vec3T<T>& a_point) const noexcept
  {
    return this->subdomains().subdomainOf(m_tape.region(a_point));
  }

  /**
   * @brief A subdomain's function (Tape::signedDistance(p, subdomains, subdomain)), or Excluded's, negative exactly in
   * the regions of no subdomain.
   * @param[in] a_point     Point.
   * @param[in] a_subdomain A subdomain (below getNumSubdomainFunctions() - 1), or Excluded.
   * @return The value: half the difference of the smallest function of a region in it and of one not in it.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline T
  signedDistance(const Vec3T<T>& a_point, const uint32_t a_subdomain) const noexcept
  {
    return m_tape.signedDistance(a_point, this->subdomains(), a_subdomain);
  }

  /**
   * @brief Every subdomain's function at a point, and Excluded's, each region's function evaluated once: what a
   * cut-cell code samples at a node. The same values as signedDistance(), one by one.
   * @details The buffer's size is checked in every build on the host, and with EBGEOMETRY_ENABLE_ASSERTIONS on a
   * device, where this is called once per point.
   * @param[in]  a_point    Point.
   * @param[out] a_values   The functions, getNumSubdomainFunctions() of them: the subdomains' in order, then Excluded's.
   * @param[in]  a_capacity The buffer's size: at least getNumSubdomainFunctions().
   */
  EBGEOMETRY_HOST_DEVICE
  inline void
  signedDistances(const Vec3T<T>& a_point, T* a_values, const uint32_t a_capacity) const noexcept
  {
    const uint32_t excluded = m_subdomains.m_numSubdomains;

#if defined(EBGEOMETRY_DEVICE_COMPILE)
    EBGEOMETRY_EXPECT(a_capacity > excluded);
#else
    EBGEOMETRY_REQUIRE(a_capacity > excluded,
                       "Domain::signedDistances: room for %u values, the domain has %u functions",
                       unsigned(a_capacity),
                       unsigned(excluded + 1U));
#endif

    const Subdomains subdomains = this->subdomains();

    // Each subdomain's smallest region function, the excluded regions' last.
    for (uint32_t l = 0; l <= excluded; l++) {
      a_values[l] = Math::Limits<T>::max();
    }

    m_tape.regionDistances(a_point, [&](const uint32_t a_region, const T a_value) {
      const uint32_t subdomain = subdomains.subdomainOf(a_region);
      const uint32_t slot      = (subdomain == Subdomains::None) ? excluded : subdomain;

      a_values[slot] = Math::min(a_values[slot], a_value);
    });

    // A function's other term is the smallest of the others': the smallest of all, or the second smallest for the one
    // that has it.
    T        first    = Math::Limits<T>::max();
    T        second   = Math::Limits<T>::max();
    uint32_t smallest = 0U;

    for (uint32_t l = 0; l <= excluded; l++) {
      if (a_values[l] < first) {
        second   = first;
        first    = a_values[l];
        smallest = l;
      }
      else {
        second = Math::min(second, a_values[l]);
      }
    }

    for (uint32_t l = 0; l <= excluded; l++) {
      a_values[l] = T(0.5) * (a_values[l] - ((l == smallest) ? second : first));
    }
  }

  /**
   * @brief Every subdomain's function at a point, and Excluded's, into an array whose size is the capacity (see
   * signedDistances()).
   * @tparam N The array's size: at least getNumSubdomainFunctions().
   * @param[in]  a_point  Point.
   * @param[out] a_values The functions: the subdomains' in order, then Excluded's.
   */
  template <std::size_t N>
  EBGEOMETRY_HOST_DEVICE
  inline void
  signedDistances(const Vec3T<T>& a_point, T (&a_values)[N]) const noexcept
  {
    this->signedDistances(a_point, a_values, static_cast<uint32_t>(N));
  }

  /**
   * @brief The surface of a subdomain closest to a point, with what is on its other side (Tape::closestSurface(p,
   * subdomains, subdomain)): never a contact between two of its own regions.
   * @param[in] a_point     Point.
   * @param[in] a_subdomain The subdomain.
   * @return The surface, with its sides and their materials.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline SurfaceRef<T>
  closestSurface(const Vec3T<T>& a_point, const uint32_t a_subdomain) const noexcept
  {
    return m_tape.closestSurface(a_point, this->subdomains(), a_subdomain);
  }

  /**
   * @brief What a surface is to the domain (Subdomains::role()).
   * @param[in] a_surface The surface, with its sides.
   * @return None, Boundary, Interface or Internal.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline SurfaceRole
  role(const SurfaceRef<T>& a_surface) const noexcept
  {
    return this->subdomains().role(a_surface);
  }

  /**
   * @brief A surface's group (SurfaceGroups::surfaceGroupOf()).
   * @param[in] a_surface The surface, with its sides and their materials.
   * @return The surface group, or SurfaceGroups::None.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline uint32_t
  surfaceGroupOf(const SurfaceRef<T>& a_surface) const noexcept
  {
    return this->surfaceGroups().surfaceGroupOf(a_surface);
  }

  /**
   * @brief A view resolving against mirrors, the tape and the tables in one pool.
   * @param[in] a_pool The pool holding both, or a mirror of it.
   * @return The view.
   */
  [[nodiscard]] EBGEOMETRY_HOST
  inline Domain
  rebasedView(const Pool& a_pool) const
  {
    return this->rebasedView(a_pool, a_pool);
  }

  /**
   * @brief A view resolving against mirrors, the tape and the tables in two pools.
   * @param[in] a_tapePool   The tape's pool, or a mirror of it.
   * @param[in] a_tablesPool The tables pool, or a mirror of it.
   * @return The view.
   */
  [[nodiscard]] EBGEOMETRY_HOST
  inline Domain
  rebasedView(const Pool& a_tapePool, const Pool& a_tablesPool) const
  {
    Domain view = *this;

    view.m_tape = m_tape.rebasedView(a_tapePool);

    const uint64_t endByte = Math::max(m_subdomains.m_table.endByte(), m_surfaceGroups.m_groupOf.endByte());

    view.setLocation(this->rebasedLocation(a_tablesPool, endByte, "Domain"));

    return view;
  }

private:
  /**
   * @brief The tape, a handle with its own location.
   */
  TapeType m_tape;

  /**
   * @brief The subdomain of each region.
   */
  Subdomains::Layout m_subdomains;

  /**
   * @brief The selectors and their surface groups.
   */
  SurfaceGroups::Layout m_surfaceGroups;
};

/**
 * @brief Deduce a domain's tape from the constructor.
 */
template <class T, class... UserPrimitives>
Domain(Pool&, const Tape<T, UserPrimitives...>&, const DomainSetup&) -> Domain<T, UserPrimitives...>;

} // namespace EBGeometry

#endif
