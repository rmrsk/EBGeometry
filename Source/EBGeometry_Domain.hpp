// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file   EBGeometry_Domain.hpp
 * @brief  Domain: a description of the regions a downstream code computes in and its tagged boundary conditions.
 * @author Robert Marskar
 */

#ifndef EBGEOMETRY_DOMAIN_HPP
#define EBGEOMETRY_DOMAIN_HPP

// Std includes
#include <cstdint>
#include <type_traits>

// Our includes
#include "EBGeometry_GPU.hpp"
#include "EBGeometry_Math.hpp"
#include "EBGeometry_Scheme.hpp"
#include "EBGeometry_Subdomains.hpp"
#include "EBGeometry_SurfaceRef.hpp"

namespace EBGeometry {

/**
 * @brief A computational domain, as a description for a downstream code (a solver EBGeometry does not provide): the
 * regions it computes in (its subdomains) and its surface groups, each a boundary condition's tag (the surface group's
 * name, kept with RegionNames) and id (its number in the scheme).
 * @details A plain value, trivially copyable, that a kernel takes whole: at a cut cell, m_subdomains says what the
 * surface is to the domain (Subdomains::role()) and m_scheme its boundary condition (Scheme::surfaceGroupOf()). The
 * surfaces it must tag are those it touches, every surface with a side in one of its regions, the internal ones
 * between two of its regions included. Materials and their properties are the regions' (MaterialLabel()).
 *
 * The domain divides space by labels: each subdomain, and Excluded, the regions in none (where it does not compute,
 * a conductor say). Every point has one label (labelAt()), and each label a function negative exactly in it
 * (signedDistance()), so that at a point, off the surfaces, exactly one is negative. A cut-cell code samples them all
 * at a node (signedDistances()): an edge whose ends have different labels crosses a surface, an interface between two
 * subdomains (where their two functions are zero, with opposite signs) or a subdomain's boundary with the excluded
 * regions (where its function is zero); its root along the edge is the crossing. Excluded's function is the negative
 * of the function of all the subdomains together.
 */
struct Domain
{
  /**
   * @brief The label of the regions in no subdomain.
   */
  static constexpr uint32_t Excluded = Subdomains::None;

  /**
   * @brief The most labels a domain has: as many subdomains as Subdomains holds ranges, and Excluded.
   */
  static constexpr uint32_t MaxLabels = Subdomains::MaxRanges + 1U;

  /**
   * @brief The regions it computes in, as subdomains.
   */
  Subdomains m_subdomains;

  /**
   * @brief Its surface groups.
   */
  Scheme m_scheme;

  /**
   * @brief Whether it computes in a region.
   * @param[in] a_region The region.
   * @return True if the region is in one of its subdomains.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline bool
  computesIn(const uint32_t a_region) const noexcept
  {
    return m_subdomains.subdomainOf(a_region) != Subdomains::None;
  }

  /**
   * @brief How many labels the domain has: its subdomains, then Excluded.
   * @return The count; Excluded's function is the last of signedDistances().
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline uint32_t
  getNumLabels() const noexcept
  {
    return m_subdomains.getNumSubdomains() + 1U;
  }

  /**
   * @brief The label of a point: the subdomain of the region it is in, or Excluded.
   * @tparam TapeType The tape of the regions (Tape).
   * @tparam T        Floating-point precision.
   * @param[in] a_tape  The tape the subdomains were built from.
   * @param[in] a_point Point.
   * @return Its subdomain, or Excluded.
   */
  template <class TapeType, class T>
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline uint32_t
  labelAt(const TapeType& a_tape, const Vec3T<T>& a_point) const noexcept
  {
    return m_subdomains.subdomainOf(a_tape.region(a_point));
  }

  /**
   * @brief A label's function: a subdomain's (Tape::signedDistance(p, subdomains, subdomain)), or Excluded's, negative
   * exactly in the regions of no subdomain.
   * @tparam TapeType The tape of the regions (Tape).
   * @tparam T        Floating-point precision.
   * @param[in] a_tape  The tape the subdomains were built from.
   * @param[in] a_point Point.
   * @param[in] a_label A subdomain (below m_subdomains.getNumSubdomains()), or Excluded.
   * @return The value: half the difference of the smallest function of a region with the label and of one without it.
   */
  template <class TapeType, class T>
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline T
  signedDistance(const TapeType& a_tape, const Vec3T<T>& a_point, const uint32_t a_label) const noexcept
  {
    return a_tape.signedDistance(a_point, m_subdomains, a_label);
  }

  /**
   * @brief Every label's function at a point, each region's function evaluated once: what a cut-cell code samples at
   * a node. The same values as signedDistance(), label by label.
   * @tparam TapeType The tape of the regions (Tape).
   * @tparam T        Floating-point precision.
   * @param[in]  a_tape   The tape the subdomains were built from.
   * @param[in]  a_point  Point.
   * @param[out] a_values The functions, getNumLabels() of them: the subdomains' in order, then Excluded's.
   */
  template <class TapeType, class T>
  EBGEOMETRY_HOST_DEVICE
  inline void
  signedDistances(const TapeType& a_tape, const Vec3T<T>& a_point, T (&a_values)[MaxLabels]) const noexcept
  {
    const uint32_t excluded = m_subdomains.getNumSubdomains();

    // Each label's smallest region function, the excluded regions' last.
    for (uint32_t l = 0; l <= excluded; l++) {
      a_values[l] = Math::Limits<T>::max();
    }

    for (uint32_t k = 0; k < a_tape.getNumRegions(); k++) {
      const uint32_t subdomain = m_subdomains.subdomainOf(k);
      const uint32_t label     = (subdomain == Subdomains::None) ? excluded : subdomain;

      a_values[label] = Math::min(a_values[label], a_tape.signedDistance(a_point, k));
    }

    // A label's other term is the smallest of the other labels': the smallest of all, or the second smallest for the
    // label that has it.
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
};

static_assert(std::is_trivially_copyable_v<Domain>, "Domain must be a plain value, for a kernel to take it");

} // namespace EBGeometry

#endif
