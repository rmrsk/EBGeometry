// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file   EBGeometry_RegionView.hpp
 * @brief  RegionView: the visible regions of a tape (see Overlay()) as a shape to look at, its surfaces those of the
 * regions, internal ones included.
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
#include "EBGeometry_Ray.hpp"
#include "EBGeometry_Vec.hpp"

namespace EBGeometry {

/**
 * @brief What a RegionView names a surface by, the key a renderer colors by (Render::ColorBy::Primitive).
 */
enum class RegionColoring : uint8_t
{
  Region,       ///< The region the surface belongs to, as seen.
  Material,     ///< That region's material.
  SurfaceLabel, ///< The surface's label (SurfaceLabel()).
  Surface,      ///< The surface itself (SurfaceRef::m_surface): each shape, each member of a collection.
  Part          ///< The surface's part: a box's face, a mesh's face.
};

/**
 * @brief The visible regions of a tape that lists TapeFeature::Regions, as a shape to look at: its surfaces are the
 * regions' surfaces, the internal ones between two regions included, found exactly along a ray.
 * @details Along a ray, its crossings are the changes of region that touch a visible region: entering one, passing
 * from one visible region to another (an internal surface, reached by seeing through the one in front), and leaving
 * one for a hidden region (its far wall); changes between hidden regions are passed through. Each is found with
 * Tape::firstRegionChange(), exactly where the shapes have ray forms, with the surface's own normal, outward of the
 * region whose surface is seen. Its value (for a section plane, contours and shading) is the visible regions' phase
 * function, negative exactly in them and zero only on their outer surface. Each crossing is named by the coloring
 * (RegionColoring) in HitT::m_primitive, so a renderer coloring by primitive colors by region, material, surface
 * label, surface or part, and a viewer's pick reports it. Up to 64 regions can be hidden; later ones are always
 * shown. A value type: it holds the tape's handle, so the tape's pool must outlive it.
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
   * @brief Every region visible, colored by region.
   * @param[in] a_tape The tape.
   */
  EBGEOMETRY_HOST_DEVICE
  explicit RegionView(const TapeType& a_tape) noexcept : m_tape(a_tape)
  {}

  /**
   * @brief The value: the visible regions' phase function (see Tape::signedDistance(p, phases, phase)), half the
   * difference of the smallest function of a visible region and of a hidden one; the largest value when none is
   * visible.
   * @param[in] a_point Point.
   * @return The value.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline T
  signedDistance(const Vec3T<T>& a_point) const noexcept
  {
    T in  = Math::Limits<T>::max();
    T out = Math::Limits<T>::max();

    for (uint32_t k = 0; k < m_tape.getNumRegions(); k++) {
      const T d = m_tape.signedDistance(a_point, k);

      if (this->isVisible(k)) {
        in = Math::min(in, d);
      }
      else {
        out = Math::min(out, d);
      }
    }

    return (in < Math::Limits<T>::max()) ? T(0.5) * (in - out) : Math::Limits<T>::max();
  }

  /**
   * @brief The key of the region a point inside the view is in (for a cut face), as a leaf reference (only m_leaf
   * is set).
   * @param[in] a_point Point.
   * @return The key in m_leaf, or LeafRef::None outside the visible regions.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline LeafRef<T>
  closestLeaf(const Vec3T<T>& a_point) const noexcept
  {
    LeafRef<T> ref;

    const uint32_t region = m_tape.region(a_point);

    if (this->isVisible(region)) {
      ref.m_leaf = this->keyOf(m_tape.closestSurface(a_point, region), region);
    }

    ref.m_localPoint = a_point;

    return ref;
  }

  /**
   * @brief The first crossing along a ray that touches a visible region (see the class description).
   * @param[in] a_ray     The ray, with a unit direction.
   * @param[in] a_tMin    Start of the interval.
   * @param[in] a_tMax    End of the interval.
   * @param[in] a_options How to march the parts without a ray form.
   * @return The crossing: its normal outward of the region whose surface is seen, m_entering whether that region is
   * the one entered, m_primitive the coloring's key; or a miss.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline HitT<T>
  firstHit(const RayT<T>& a_ray, const T a_tMin, const T a_tMax, const RayOptions<T>& a_options) const noexcept
  {
    T t = a_tMin;

    for (uint32_t steps = 0; steps < a_options.m_maxSteps; steps++) {
      const SurfaceHitT<T> change = m_tape.firstRegionChange(a_ray, t, a_tMax, a_options);

      if (!change.m_hit.hit()) {
        return change.m_hit;
      }

      const uint32_t from = change.m_surface.m_minusRegion;
      const uint32_t to   = change.m_surface.m_plusRegion;

      if (this->isVisible(from) || this->isVisible(to)) {
        // The surface seen is the region entered's when it is visible, else the one left's.
        const bool     entering = this->isVisible(to);
        const uint32_t seen     = entering ? to : from;

        HitT<T> hit = change.m_hit;

        // The crossing's normal points into the region entered: outward of the region left.
        hit.m_normal    = entering ? -change.m_hit.m_normal : change.m_hit.m_normal;
        hit.m_entering  = entering;
        hit.m_primitive = this->keyOf(change.m_surface, seen);

        return hit;
      }

      t = change.m_hit.m_t;
    }

    return HitT<T>();
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
    return a_region > 0U && a_region < m_tape.getNumRegions() &&
           (a_region >= 64U || ((m_hidden >> a_region) & 1U) == 0U);
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

  /**
   * @brief What the view names a surface by.
   * @return The coloring.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline RegionColoring
  getColoring() const noexcept
  {
    return m_coloring;
  }

  /**
   * @brief The same view, naming surfaces by another key.
   * @param[in] a_coloring The coloring.
   * @return The view.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline RegionView
  withColoring(const RegionColoring a_coloring) const noexcept
  {
    RegionView view = *this;

    view.m_coloring = a_coloring;

    return view;
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

  /**
   * @brief What a surface is named by.
   */
  RegionColoring m_coloring = RegionColoring::Region;

  /**
   * @brief A surface's key, by the coloring.
   * @param[in] a_surface The surface.
   * @param[in] a_region  The region whose surface it is, as seen.
   * @return The key.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline uint32_t
  keyOf(const SurfaceRef<T>& a_surface, const uint32_t a_region) const noexcept
  {
    switch (m_coloring) {
    case RegionColoring::Material: {
      return m_tape.getMaterialOf(a_region).value();
    }
    case RegionColoring::SurfaceLabel: {
      return a_surface.m_surfaceLabel.value();
    }
    case RegionColoring::Surface: {
      return a_surface.m_surface;
    }
    case RegionColoring::Part: {
      return a_surface.m_part;
    }
    default: {
      return a_region;
    }
    }
  }
};

/**
 * @brief A RegionView traced along a ray by its own firstHit() (see RegionView).
 * @tparam TapeType A Tape that lists TapeFeature::Regions.
 */
template <class TapeType>
struct RayForm<RegionView<TapeType>>
{
  /**
   * @brief Floating-point precision.
   */
  using T = ShapePrecision<TapeType>;

  /**
   * @brief The first crossing that touches a visible region (RegionView::firstHit()).
   * @param[in] a_view    The view.
   * @param[in] a_ray     The ray, with a unit direction.
   * @param[in] a_tMin    Start of the interval.
   * @param[in] a_tMax    End of the interval.
   * @param[in] a_options How to march the parts without a ray form.
   * @return The crossing, or a miss.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  static inline HitT<T>
  firstHit(const RegionView<TapeType>& a_view,
           const RayT<T>&              a_ray,
           const T                     a_tMin,
           const T                     a_tMax,
           const RayOptions<T>&        a_options) noexcept
  {
    return a_view.firstHit(a_ray, a_tMin, a_tMax, a_options);
  }
};

/**
 * @brief Whether a shape's regions can be shown, hidden and colored (RegionView): the primary template is false.
 * @tparam S Shape type.
 */
template <class S, class = void>
inline constexpr bool hasRegionVisibility = false;

/**
 * @brief A shape with getNumRegions(), isVisible(), withVisible(), withOnly(), withAllVisible(), getColoring() and
 * withColoring().
 * @tparam S Shape type.
 */
template <class S>
inline constexpr bool
  hasRegionVisibility<S,
                      std::void_t<decltype(std::declval<const S&>().getNumRegions()),
                                  decltype(std::declval<const S&>().isVisible(0U)),
                                  decltype(std::declval<const S&>().withVisible(0U, true)),
                                  decltype(std::declval<const S&>().withOnly(0U)),
                                  decltype(std::declval<const S&>().withAllVisible()),
                                  decltype(std::declval<const S&>().getColoring()),
                                  decltype(std::declval<const S&>().withColoring(RegionColoring::Region))>> = true;

} // namespace EBGeometry

#endif
