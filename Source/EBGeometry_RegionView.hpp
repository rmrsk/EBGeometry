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
#include "EBGeometry_RegionSettings.hpp"
#include "EBGeometry_Vec.hpp"

namespace EBGeometry {

/**
 * @brief The visible regions of a tape that lists TapeFeature::Regions, as a shape to look at: its surfaces are the
 * regions' surfaces, the internal ones between two regions included, found exactly along a ray.
 * @details Along a ray, its crossings are the changes of region that touch a visible region: entering one, passing
 * from one visible region to another (an internal surface, reached by seeing through the one in front), and leaving
 * one for a hidden region (its far wall); changes between hidden regions are passed through. Each is found with
 * Tape::firstRegionChange(), exactly where the shapes have ray forms, with the surface's own normal, outward of the
 * region whose surface is seen. Its value (for a section plane, contours and shading) is the visible regions' subdomain
 * function, negative exactly in them and zero only on their outer surface. Each crossing is named by the coloring
 * (RegionColoring) in HitT::m_primitive, so a renderer coloring by primitive colors by region, material, surface
 * label, surface, part or interface (the pair of regions on its two sides), and a viewer's pick reports it; a view can
 * show only the surfaces with one key (withOnlyKey()), the conductor's inner surface alone, say, passing the others
 * through. What is shown is a RegionSettings, which the renderer gives the view from its settings
 * (Render::Settings::m_regions); the view's own (withSettings(), and the with... functions) serve where it is used
 * outside a render. Up to 64 regions can be hidden; later ones are always shown. A value type: it holds the tape's handle, so the tape's pool must outlive it; rebasedView() views the same
 * tape on a mirror of its pool, so Render::view(view, pool, ...) renders it on a device.
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
   * @brief The value: the visible regions' subdomain function (see Tape::signedDistance(p, subdomains, subdomain)), half the
   * difference of the smallest function of a visible region and of a hidden one; the largest value when none is
   * visible.
   * @param[in] a_point Point.
   * @return The value.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  EBGEOMETRY_DEVICE_NOINLINE
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
   * @brief The field a section plane's contours are drawn from: the distance to the nearest surface of a visible
   * region (the smallest |function| of a visible region), zero on every visible region's surface, the interfaces
   * between two of them included, which the value (the visible regions' outer surface) does not have.
   * @param[in] a_point Point.
   * @return The field.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  EBGEOMETRY_DEVICE_NOINLINE
  inline T
  contourValue(const Vec3T<T>& a_point) const noexcept
  {
    T field = Math::Limits<T>::max();

    for (uint32_t k = 1; k < m_tape.getNumRegions(); k++) {
      if (this->isVisible(k)) {
        field = Math::min(field, std::abs(m_tape.signedDistance(a_point, k)));
      }
    }

    return field;
  }

  /**
   * @brief The key of the region a point inside the view is in (for a cut face), as a leaf reference (only m_leaf
   * is set).
   * @param[in] a_point Point.
   * @return The key in m_leaf, or LeafRef::None outside the visible regions.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  EBGEOMETRY_DEVICE_NOINLINE
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
  EBGEOMETRY_DEVICE_NOINLINE
  inline HitT<T>
  firstHit(const RayT<T>& a_ray, const T a_tMin, const T a_tMax, const RayOptions<T>& a_options) const noexcept
  {
    T t = a_tMin;

    for (uint32_t steps = 0; steps < a_options.m_maxSteps; steps++) {
      const SurfaceHitT<T> change = m_tape.firstRegionChange(a_ray, t, a_tMax, a_options);

      if (!change.m_hit.hit()) {
        return change.m_hit;
      }

      const uint32_t from = change.m_surface.m_insideRegion;
      const uint32_t to   = change.m_surface.m_outsideRegion;

      // The surface crossed lies on the later region's operand (an interface is on the surface of the region on
      // top), so it is shown as that region's, whichever way the ray goes; when that region is hidden, seen through or
      // not the one shown alone, as the other region's. So with the core seen through, the core's wall inside a
      // jacket is the jacket's bore; with the jacket seen through, the core's wall.
      const uint32_t sides[2] = {Math::max(from, to), Math::min(from, to)};

      // The surface group the surface is in, when the settings hide or highlight one.
      const uint32_t group = (m_settings.m_hiddenGroups != 0U || m_settings.m_highlightGroup != None)
                               ? m_settings.m_scheme.surfaceGroupOf(change.m_surface)
                               : None;

      // A hidden surface group's surfaces are passed through.
      const bool hidden = m_settings.isSurfaceGroupHidden(group);

      for (const uint32_t seen : sides) {
        if (hidden) {
          break;
        }

        if (!this->isVisible(seen)) {
          continue;
        }

        const uint32_t key = this->keyOf(change.m_surface, seen);

        if (this->isSeenThrough(key) || (m_settings.m_only != None && key != m_settings.m_only)) {
          continue;
        }

        const bool entering = (seen == to);

        HitT<T> hit = change.m_hit;

        // The crossing's normal points into the region entered: outward of the region left.
        hit.m_normal    = entering ? -change.m_hit.m_normal : change.m_hit.m_normal;
        hit.m_entering  = entering;
        hit.m_primitive = key;

        // The surfaces of the highlighted surface group, whatever the coloring.
        if (m_settings.m_highlightGroup != None && group == m_settings.m_highlightGroup) {
          hit.m_primitive = RegionSettings::Highlighted;
        }

        return hit;
      }

      t = change.m_hit.m_t;
    }

    return HitT<T>();
  }

  /**
   * @brief The same view of the tape rebased onto another pool, a mirror of its own (for a device: see
   * Tape::rebasedView()); the visibility, coloring and keys are kept.
   * @param[in] a_pool The pool or its mirror.
   * @return The view.
   */
  [[nodiscard]] EBGEOMETRY_HOST
  inline RegionView
  rebasedView(const Pool& a_pool) const
  {
    RegionView view = *this;

    view.m_tape = m_tape.rebasedView(a_pool);

    return view;
  }

  /**
   * @brief The piece of surface at a point a ray met (firstHit()): the surface, its part and label, and the regions on
   * its two sides there, the region whose surface was seen inside (SurfaceRef::m_insideRegion) and the one across
   * outside, with their materials; what a click picks, for a scheme's selector (SurfaceSelector::piece()).
   * @details The region seen is the one just behind the surface (against the normal, which points out of it) and the
   * one across just in front, a small step away; the surface and its sides then come from Tape::closestSurface(p,
   * region).
   * @param[in] a_point  The point, on a visible surface.
   * @param[in] a_normal The hit's normal there (outward of the region seen).
   * @return The piece, or no surface when no visible region is behind the point.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline SurfaceRef<T>
  pieceAt(const Vec3T<T>& a_point, const Vec3T<T>& a_normal) const noexcept
  {
    const BoundingVolumes::AABBT<T> box = m_tape.computeBoundingVolume();

    const T size   = (box.getHighCorner() - box.getLowCorner()).length();
    const T step   = (size > T(0) && size < Math::Limits<T>::max()) ? T(1e-4) * size : T(1e-4);
    const T length = a_normal.length();

    if (!(length > T(0))) {
      return SurfaceRef<T>();
    }

    const Vec3T<T> n = a_normal / length;

    uint32_t seen = m_tape.region(a_point - step * n);

    // Seen from inside a region (its far wall), the region in front is the visible one.
    if (!this->isVisible(seen)) {
      seen = m_tape.region(a_point + step * n);
    }

    return this->isVisible(seen) ? m_tape.closestSurface(a_point, seen) : SurfaceRef<T>();
  }

  /**
   * @brief The tape the view shows.
   * @return The tape.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline const TapeType&
  getTape() const noexcept
  {
    return m_tape;
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
   * @brief What the view shows.
   * @return The settings.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline const RegionSettings&
  getSettings() const noexcept
  {
    return m_settings;
  }

  /**
   * @brief The same view, showing what other settings say (the renderer gives it its own, Render::Settings::m_regions).
   * @param[in] a_settings The settings.
   * @return The view.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline RegionView
  withSettings(const RegionSettings& a_settings) const noexcept
  {
    RegionView view = *this;

    view.m_settings = a_settings;

    return view;
  }

  /**
   * @brief No key (RegionSettings::None).
   */
  static constexpr uint32_t None = RegionSettings::None;

  /**
   * @brief The key of the interface between two regions (RegionSettings::interfaceKey()).
   * @param[in] a_first  A region.
   * @param[in] a_second The region on the other side.
   * @return The key.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  static inline uint32_t
  interfaceKey(const uint32_t a_first, const uint32_t a_second) noexcept
  {
    return RegionSettings::interfaceKey(a_first, a_second);
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
    return m_settings.isVisible(a_region, m_tape.getNumRegions());
  }

  /**
   * @brief Whether the surfaces with a key are seen through.
   * @param[in] a_key The key.
   * @return True if they are.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline bool
  isSeenThrough(const uint32_t a_key) const noexcept
  {
    return m_settings.isSeenThrough(a_key);
  }

  /**
   * @brief The same view with a region shown or hidden (RegionSettings::setVisible()).
   * @param[in] a_region  Region.
   * @param[in] a_visible Whether to show it.
   * @return The view.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline RegionView
  withVisible(const uint32_t a_region, const bool a_visible) const noexcept
  {
    RegionView view = *this;

    view.m_settings.setVisible(a_region, a_visible);

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

    view.m_settings.m_hidden = 0U;

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

    view.m_settings.showOnly(a_region);

    return view;
  }

  /**
   * @brief The same view, showing only the surfaces with a key (by the coloring); None shows them all.
   * @param[in] a_key The key, as HitT::m_primitive reports it (a pick), or None.
   * @return The view.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline RegionView
  withOnlyKey(const uint32_t a_key) const noexcept
  {
    RegionView view = *this;

    view.m_settings.m_only = a_key;

    return view;
  }

  /**
   * @brief The same view with the surfaces of a key seen through, or opaque again (RegionSettings::setSeenThrough()).
   * @param[in] a_key     The key.
   * @param[in] a_through Whether to see through it.
   * @return The view.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline RegionView
  withSeenThrough(const uint32_t a_key, const bool a_through) const noexcept
  {
    RegionView view = *this;

    view.m_settings.setSeenThrough(a_key, a_through);

    return view;
  }

  /**
   * @brief The same view with every surface opaque.
   * @return The view.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline RegionView
  withNoneSeenThrough() const noexcept
  {
    RegionView view = *this;

    view.m_settings.clearSeenThrough();

    return view;
  }

  /**
   * @brief The same view, naming surfaces by another key (RegionSettings::setColoring()).
   * @param[in] a_coloring The coloring.
   * @return The view.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline RegionView
  withColoring(const RegionColoring a_coloring) const noexcept
  {
    RegionView view = *this;

    view.m_settings.setColoring(a_coloring);

    return view;
  }

  /**
   * @brief The same view with a solver's scheme (RegionColoring::SurfaceGroup colors by its groups).
   * @param[in] a_scheme The scheme.
   * @return The view.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline RegionView
  withScheme(const Scheme& a_scheme) const noexcept
  {
    RegionView view = *this;

    view.m_settings.m_scheme = a_scheme;

    return view;
  }

private:
  /**
   * @brief The tape.
   */
  TapeType m_tape;

  /**
   * @brief What is shown.
   */
  RegionSettings m_settings;

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
    switch (m_settings.m_coloring) {
    case RegionColoring::Interface: {
      return RegionSettings::interfaceKey(a_surface.m_insideRegion, a_surface.m_outsideRegion);
    }
    case RegionColoring::SurfaceGroup: {
      return m_settings.m_scheme.surfaceGroupOf(a_surface);
    }
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
 * @brief Whether a shape's regions are shown by region settings (RegionView): the primary template is false.
 * @tparam S Shape type.
 */
template <class S, class = void>
inline constexpr bool hasRegionVisibility = false;

/**
 * @brief A shape with getNumRegions(), getSettings(), withSettings(), getTape() and pieceAt(): the renderer gives it
 * Render::Settings's region settings, and the viewer edits those and picks pieces of surface for a scheme.
 * @tparam S Shape type.
 */
template <class S>
inline constexpr bool hasRegionVisibility<
  S,
  std::void_t<decltype(std::declval<const S&>().getNumRegions()),
              decltype(std::declval<const S&>().getSettings()),
              decltype(std::declval<const S&>().withSettings(std::declval<const RegionSettings&>())),
              decltype(std::declval<const S&>().getTape()),
              decltype(std::declval<const S&>().pieceAt(std::declval<const Vec3T<typename S::T>&>(),
                                                        std::declval<const Vec3T<typename S::T>&>()))>> = true;

} // namespace EBGeometry

#endif
