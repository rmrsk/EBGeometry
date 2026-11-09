// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file   EBGeometry_RegionView.hpp
 * @brief  RegionView: the visible regions of a tape (see Overlay()) as one shape, for looking at them.
 * @author Robert Marskar
 */

#ifndef EBGEOMETRY_REGIONVIEW_HPP
#define EBGEOMETRY_REGIONVIEW_HPP

// Std includes
#include <cstdint>
#include <type_traits>

// Our includes
#include "EBGeometry_BoundingVolumes.hpp"
#include "EBGeometry_Expr.hpp"
#include "EBGeometry_GPU.hpp"
#include "EBGeometry_LeafRef.hpp"
#include "EBGeometry_Math.hpp"
#include "EBGeometry_Vec.hpp"

namespace EBGeometry {

/**
 * @brief The visible regions of a tape that lists TapeFeature::Regions, as one shape: the union of the visible
 * regions, whose primitives are the regions themselves.
 * @details Its value is the smallest of the visible regions' functions (Tape::signedDistance(p, region)), a distance
 * bound, so hiding the jacket of a cable shows the core inside it. closestLeaf() names the visible region a surface
 * point belongs to, as LeafRef::m_leaf, so a renderer colors by region where it colors by primitive
 * (Render::ColorBy::Primitive) and a viewer's pick reports the region. The viewer (Render::view()) gives a shape
 * with regions keys to show and hide them. Up to 64 regions can be hidden; later ones are always shown. A value
 * type: it holds the tape's handle, so the tape's pool must outlive it.
 * @tparam TapeType A Tape that lists TapeFeature::Regions.
 */
template <class TapeType>
class RegionView
{
public:
  /**
   * @brief Floating-point precision.
   */
  using T = ShapePrecision<TapeType>;

  /**
   * @brief Every region visible.
   * @param[in] a_tape The tape.
   */
  EBGEOMETRY_HOST_DEVICE
  explicit RegionView(const TapeType& a_tape) noexcept : m_tape(a_tape)
  {}

  /**
   * @brief The value: the smallest of the visible regions' functions; the largest value when none is visible.
   * @param[in] a_point Point.
   * @return The value.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline T
  signedDistance(const Vec3T<T>& a_point) const noexcept
  {
    T value = Math::Limits<T>::max();

    for (uint32_t k = 1; k < m_tape.getNumRegions(); k++) {
      if (this->isVisible(k)) {
        value = Math::min(value, m_tape.signedDistance(a_point, k));
      }
    }

    return value;
  }

  /**
   * @brief The visible region a point on the surface belongs to, as a leaf reference (only m_leaf is set).
   * @details The region just inside the surface: the point is moved a small step against the normal (the visible
   * union's gradient, by central differences) and the tape names the region there. The smallest region function
   * would not do: a region's function is a distance bound, and where a mask cuts two regions both are about as far
   * from the cut, so a cut through the core would be named the jacket.
   * @param[in] a_point Point, on or near the visible surface.
   * @return The region in m_leaf, or LeafRef::None when none is visible there.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline LeafRef<T>
  closestLeaf(const Vec3T<T>& a_point) const noexcept
  {
    const BoundingVolumes::AABBT<T> box = m_tape.computeBoundingVolume();

    const T size = (box.getHighCorner() - box.getLowCorner()).length();
    const T h    = (size > T(0) && size < Math::Limits<T>::max()) ? T(1e-4) * size : T(1e-4);

    Vec3T<T> gradient;

    for (size_t dir = 0; dir < 3; dir++) {
      Vec3T<T> step = Vec3T<T>::zeros();

      step[dir] = h;

      gradient[dir] = this->signedDistance(a_point + step) - this->signedDistance(a_point - step);
    }

    const T length = gradient.length();

    const Vec3T<T> inside = (length > T(0)) ? a_point - (T(4) * h / length) * gradient : a_point;

    const uint32_t region = m_tape.region(inside);

    LeafRef<T> ref;

    ref.m_leaf       = this->isVisible(region) ? region : LeafRef<T>::None;
    ref.m_localPoint = a_point;

    return ref;
  }

  /**
   * @brief The tape's box.
   * @return The box.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline BoundingVolumes::AABBT<T>
  computeBoundingVolume() const noexcept
  {
    return m_tape.computeBoundingVolume();
  }

  /**
   * @brief How many regions the tape has, the background included.
   * @return The count.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline uint32_t
  getNumRegions() const noexcept
  {
    return m_tape.getNumRegions();
  }

  /**
   * @brief Whether a region is shown (the background never is).
   * @param[in] a_region Region.
   * @return True if visible.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline bool
  isVisible(const uint32_t a_region) const noexcept
  {
    return a_region > 0U && (a_region >= 64U || ((m_hidden >> a_region) & 1U) == 0U);
  }

  /**
   * @brief The same view with a region shown or hidden.
   * @param[in] a_region  Region (1 to 63; others are always shown).
   * @param[in] a_visible Whether to show it.
   * @return The view.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline RegionView
  withVisible(const uint32_t a_region, const bool a_visible) const noexcept
  {
    RegionView view = *this;

    if (a_region > 0U && a_region < 64U) {
      const uint64_t bit = uint64_t(1) << a_region;

      view.m_hidden = a_visible ? (m_hidden & ~bit) : (m_hidden | bit);
    }

    return view;
  }

  /**
   * @brief The same view with every region shown.
   * @return The view.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline RegionView
  withAllVisible() const noexcept
  {
    RegionView view = *this;

    view.m_hidden = 0U;

    return view;
  }

  /**
   * @brief The same view with one region shown and the others hidden.
   * @param[in] a_region Region.
   * @return The view.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline RegionView
  withOnly(const uint32_t a_region) const noexcept
  {
    RegionView view = *this;

    view.m_hidden = ~uint64_t(0);

    return view.withVisible(a_region, true);
  }

private:
  /**
   * @brief The tape.
   */
  TapeType m_tape;

  /**
   * @brief The hidden regions, a bit each (bit k for region k).
   */
  uint64_t m_hidden = 0U;
};

/**
 * @brief Whether a shape's regions can be shown and hidden (RegionView): the primary template is false.
 * @tparam S Shape type.
 */
template <class S, class = void>
inline constexpr bool hasRegionVisibility = false;

/**
 * @brief A shape with getNumRegions(), isVisible(), withVisible(), withOnly() and withAllVisible().
 * @tparam S Shape type.
 */
template <class S>
inline constexpr bool hasRegionVisibility<S,
                                          std::void_t<decltype(std::declval<const S&>().getNumRegions()),
                                                      decltype(std::declval<const S&>().isVisible(0U)),
                                                      decltype(std::declval<const S&>().withVisible(0U, true)),
                                                      decltype(std::declval<const S&>().withOnly(0U)),
                                                      decltype(std::declval<const S&>().withAllVisible())>> = true;

} // namespace EBGeometry

#endif
