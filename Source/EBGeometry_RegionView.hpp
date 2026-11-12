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
 * @brief The visible regions of a tape, as a shape to look at: its surfaces are the
 * regions' surfaces, the internal ones between two regions included, found exactly along a ray.
 * @details Along a ray, its crossings are the changes of region that touch a visible region: entering one, passing
 * from one visible region to another (an internal surface, reached by seeing through the one in front), and leaving
 * one for a hidden region (its far wall); changes between hidden regions are passed through. Each is found with
 * Tape::firstRegionChange(), exactly where the shapes have ray forms, with the surface's own normal, outward of the
 * region whose surface is seen. Its value (for a section plane, contours and shading) is the visible regions' subdomain
 * function, negative exactly in them and zero only on their outer surface. Each crossing is named by the coloring
 * (RegionColoring) in HitT::m_primitive, so a renderer coloring by primitive colors by region, material, surface
 * label, surface, part or interface (the pair of regions on its two sides), and a viewer's pick reports it; a view can
 * show only the surfaces with one key (withOnlyKey()), the interface between two regions alone, say, passing the others
 * through. What is shown is a RegionSettings, which the renderer gives the view from its settings
 * (Render::Settings::m_regions); the view's own (withSettings(), and the with... functions) serve where it is used
 * outside a render. Up to 64 regions can be hidden; later ones are always shown. A value type: it holds the tape's handle, so the tape's pool must outlive it; rebasedView() views the same
 * tape on a mirror of its pool, so Render::view(view, pool, ...) renders it on a device.
 * @tparam TapeType A Tape.
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
   * @brief The value: the function of the visible regions, as a subdomain's (see Tape::signedDistance(p, subdomains,
   * subdomain)): half the difference of the smallest function of a visible region and of one not visible; the largest
   * value when none is visible.
   * @param[in] a_point Point.
   * @return The value.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  EBGEOMETRY_DEVICE_NOINLINE
  inline T
  signedDistance(const Vec3T<T>& a_point) const noexcept
  {
    return this->functionOf(a_point, false);
  }

  /**
   * @brief The distance quality of the view's functions: the tape's (Tape::getDistanceQuality()), since they are made
   * of its regions' values.
   * @return The quality.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline DistanceQuality
  getDistanceQuality() const noexcept
  {
    return m_tape.getDistanceQuality();
  }

  /**
   * @brief The field a section plane shows (its contours and lines of equal distance), its value (see
   * contourSample()).
   * @param[in] a_point Point.
   * @return The field.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline T
  contourValue(const Vec3T<T>& a_point) const noexcept
  {
    return this->contourSample(a_point).m_value;
  }

  /**
   * @brief The field a section plane shows (its contours and lines of equal distance): with a whole domain shown
   * (RegionSettings::m_sectionDomain), the function of each point's own label (see domainSample()), secondary in the
   * excluded regions; with section
   * regions set (RegionSettings::m_sectionRegions), their function as a subdomain's, what a cut-cell code cuts for it,
   * zero on the subdomain's boundary alone, secondary outside it; without them, the distance to the nearest surface of a visible region
   * (the smallest |function| of a visible region), zero on every visible region's surface, the interfaces between two
   * of them included, which the value (the visible regions' outer surface) does not have, signed as the value.
   * Negative inside.
   * @param[in] a_point Point.
   * @return The field, and whether its lines there are secondary.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  EBGEOMETRY_DEVICE_NOINLINE
  inline ContourSample<T>
  contourSample(const Vec3T<T>& a_point) const noexcept
  {
    if (m_settings.m_sectionDomain) {
      return this->domainSample(a_point);
    }

    // A subdomain's function: what lies outside it is not computed there, its lines secondary.
    if (m_settings.m_sectionRegions != 0U) {
      const T value = this->functionOf(a_point, true);

      return ContourSample<T>{value, value > T(0)};
    }

    // The visible regions' functions, nearest to zero, signed as the visible regions' function is (signedDistance():
    // the smallest of theirs against the smallest of the others'), in the one pass over the tape.
    T field = Math::Limits<T>::max();
    T in    = Math::Limits<T>::max();
    T out   = Math::Limits<T>::max();

    m_tape.regionDistances(a_point, [&](const uint32_t a_region, const T a_value) {
      if (this->isVisible(a_region)) {
        field = Math::min(field, std::abs(a_value));
        in    = Math::min(in, a_value);
      }
      else {
        out = Math::min(out, a_value);
      }
    });

    const bool inside = in < Math::Limits<T>::max() && in < out;

    return ContourSample<T>{inside ? -field : field};
  }

  /**
   * @brief What a point is in, for a section plane's fill: its region's material, whatever the surfaces are colored by
   * (the background's included, never drawn otherwise).
   * @param[in] a_point Point.
   * @return The material's number, or none for a region without one.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline uint32_t
  sectionKey(const Vec3T<T>& a_point) const noexcept
  {
    return m_tape.getMaterialOf(m_tape.region(a_point)).value();
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
  inline HitT<T>
  firstHit(const RayT<T>& a_ray, const T a_tMin, const T a_tMax, const RayOptions<T>& a_options) const noexcept
  {
    return this->crossingSeen(a_ray, a_tMin, a_tMax, a_options, false);
  }

  /**
   * @brief The first crossing along a ray on a surface seen through (setSeenThrough()), where firstHit() passes on:
   * what a renderer draws translucent in front of what firstHit() finds.
   * @param[in] a_ray     The ray, with a unit direction.
   * @param[in] a_tMin    Start of the interval.
   * @param[in] a_tMax    End of the interval (where firstHit() met a surface, say).
   * @param[in] a_options How to march the parts without a ray form.
   * @return The crossing, as firstHit() gives it, m_primitive the coloring's key; or a miss, also at the first surface
   * firstHit() would show and when nothing is seen through.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline HitT<T>
  firstSeenThrough(const RayT<T>& a_ray, const T a_tMin, const T a_tMax, const RayOptions<T>& a_options) const noexcept
  {
    bool any = m_settings.m_throughRegions != 0U || m_settings.m_throughGroups != 0U;

    for (uint32_t i = 0; i < RegionSettings::MaxSeenThrough; i++) {
      any = any || m_settings.m_through[i] != None;
    }

    return any ? this->crossingSeen(a_ray, a_tMin, a_tMax, a_options, true) : HitT<T>();
  }

private:
  /**
   * @brief The first crossing along a ray that firstHit() shows, or with a_seenThrough the first that it passes on
   * because it is seen through (firstSeenThrough()); one body for both, a run-time choice, so that a device kernel
   * holds one copy of the march.
   * @param[in] a_ray         The ray, with a unit direction.
   * @param[in] a_tMin        Start of the interval.
   * @param[in] a_tMax        End of the interval.
   * @param[in] a_options     How to march the parts without a ray form.
   * @param[in] a_seenThrough Whether to find a surface seen through instead.
   * @return The crossing, or a miss.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  EBGEOMETRY_DEVICE_NOINLINE
  inline HitT<T>
  crossingSeen(const RayT<T>&       a_ray,
               const T              a_tMin,
               const T              a_tMax,
               const RayOptions<T>& a_options,
               const bool           a_seenThrough) const noexcept
  {
    T t = a_tMin;

    for (uint32_t steps = 0; steps < a_options.m_maxSteps; steps++) {
      const SurfaceHitT<T> change = m_tape.firstRegionChange(a_ray, t, a_tMax, a_options);

      if (!change.m_hit.hit()) {
        return change.m_hit;
      }

      const uint32_t from = change.m_surface.m_ownerRegion;
      const uint32_t to   = change.m_surface.m_neighborRegion;

      // The surface crossed lies on the later region's operand (an interface is on the surface of the region on
      // top), so it is shown as that region's, whichever way the ray goes; when that region is hidden, seen through or
      // not the one shown alone, as the other region's. So with an inner region seen through, its wall inside an outer
      // one is the outer region's bore; with the outer one seen through, the inner one's wall.
      const uint32_t sides[2] = {Math::max(from, to), Math::min(from, to)};

      // The surface group the surface is in, when the settings hide or highlight one.
      const uint32_t group =
        (m_settings.m_hiddenGroups != 0U || m_settings.m_throughGroups != 0U || m_settings.m_highlightGroup != None)
          ? m_settings.m_surfaceGroups.surfaceGroupOf(change.m_surface)
          : None;

      // With only the selected surfaces shown, the entry of the selection the surface is in.
      const uint32_t entry = m_settings.m_onlySelected ? m_settings.selectionOf(change.m_surface) : None;

      // A hidden surface group's surfaces are passed through, and with only the selected surfaces shown, the others.
      const bool hidden = m_settings.isSurfaceGroupHidden(group) || (m_settings.m_onlySelected && entry == None);

      for (const uint32_t seen : sides) {
        if (hidden) {
          break;
        }

        if (!this->isVisible(seen)) {
          continue;
        }

        const bool entering = (seen == to);

        HitT<T> hit = change.m_hit;

        // The crossing's normal points into the region entered: outward of the region left.
        hit.m_normal   = entering ? -change.m_hit.m_normal : change.m_hit.m_normal;
        hit.m_entering = entering;

        // Only the selected surfaces shown: keyed by their entry and the side seen (see RegionSettings::m_onlySelected);
        // nothing is seen through then.
        if (m_settings.m_onlySelected) {
          if (a_seenThrough) {
            return HitT<T>();
          }

          hit.m_primitive = RegionSettings::Selected + entry + (entering ? 0U : RegionSettings::MaxSelected);

          return hit;
        }

        const uint32_t key = this->keyOf(change.m_surface, seen);

        // The surfaces of the highlighted surface group or regions, whatever the coloring; a selected one, by its
        // entry. Either is never seen through: what is highlighted or picked stays solid.
        const bool lit = (m_settings.m_highlightGroup != None && group == m_settings.m_highlightGroup) ||
                         m_settings.isRegionHighlighted(from) || m_settings.isRegionHighlighted(to);
        const uint32_t chosen =
          (!lit && m_settings.m_numSelected != 0U) ? m_settings.selectionOf(change.m_surface) : None;

        const bool through = !lit && chosen == None &&
                             (this->isSeenThrough(key) || m_settings.isRegionSeenThrough(seen) ||
                              m_settings.isSurfaceGroupSeenThrough(group));

        if (m_settings.m_only != None && key != m_settings.m_only) {
          continue;
        }

        // Looking for a surface seen through: this one, or none in front of the surface shown.
        if (a_seenThrough) {
          if (through) {
            hit.m_primitive = key;

            return hit;
          }

          return HitT<T>();
        }

        if (through) {
          continue;
        }

        hit.m_primitive = lit                ? RegionSettings::Highlighted
                          : (chosen != None) ? RegionSettings::Selected + chosen
                                             : key;

        return hit;
      }

      // On past the crossing: a marched one is placed short of the surface, within the march's tolerance, so the
      // search goes on four tolerances past it (as a renderer steps past a crossing), or it would find it again.
      t = change.m_hit.m_t + (change.m_hit.m_exact ? T(0) : T(4) * a_options.m_tolerance * this->scaleOf(a_options));
    }

    return HitT<T>();
  }

  /**
   * @brief The length the march's tolerance is relative to: the options', or the tape's box's diagonal.
   * @param[in] a_options The options.
   * @return The length.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline T
  scaleOf(const RayOptions<T>& a_options) const noexcept
  {
    if (a_options.m_scale > T(0)) {
      return a_options.m_scale;
    }

    const auto box      = m_tape.computeBoundingVolume();
    const T    diagonal = (box.getHighCorner() - box.getLowCorner()).length();

    return (diagonal > T(0) && diagonal < Math::Limits<T>::max()) ? diagonal : T(1);
  }

public:
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
   * its two sides there, the region whose surface was seen the owner (SurfaceRef::m_ownerRegion) and the one across
   * the neighbor, with their materials; what a click picks, for a surface group's selector (SurfaceSelector::piece()).
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
   * @brief The first crossing of the whole object's surface along a ray, from the tape's ray program, as firstHit() on
   * the tape finds it without RayOptions::m_march (the program is traced whatever a_options.m_march says).
   * @details Through the tape's out-of-line trace, which the view's own queries call: firstHit() on the tape inlines a
   * second copy of the tape's ray code into every device kernel reaching it. With that copy, ptxas on the device render
   * tests of region views took over an hour where this takes nine minutes (CUDA, sm_70).
   * @param[in] a_ray     The ray, with a unit direction.
   * @param[in] a_tMax    End of the interval, which starts at 0.
   * @param[in] a_options How to march the parts without a ray form.
   * @return The crossing, a miss, or an unresolved query.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline HitT<T>
  objectHit(const RayT<T>& a_ray, const T a_tMax, const RayOptions<T>& a_options) const noexcept
  {
    EBGEOMETRY_EXPECT(std::abs(a_ray.m_direction.length() - T(1)) < T(1e-3));

    return m_tape.traceProgram(0U, a_ray, T(0), a_tMax, a_options);
  }

  /**
   * @brief Whether the view shows the whole object's outer surface, as the tape itself is drawn: every region shown,
   * none alone, nothing seen through, no surface group hidden, not only the selected surfaces. A ray from outside the
   * object then meets the view's surface where it meets the tape's (only the keys differ), so that a renderer can
   * trace the tape for what needs only where (shadows, occlusion).
   * @return Whether.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline bool
  showsWholeObject() const noexcept
  {
    bool through = m_settings.m_throughRegions != 0U || m_settings.m_throughGroups != 0U;

    for (uint32_t i = 0; i < RegionSettings::MaxSeenThrough; i++) {
      through = through || m_settings.m_through[i] != None;
    }

    return m_settings.m_hidden == 0U && m_settings.m_only == None && !through && m_settings.m_hiddenGroups == 0U &&
           !m_settings.m_onlySelected;
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
   * @brief The same view with a domain's surface groups (RegionColoring::SurfaceGroup colors by them).
   * @param[in] a_groups The surface groups.
   * @return The view.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline RegionView
  withSurfaceGroups(const SurfaceGroups& a_groups) const noexcept
  {
    RegionView view = *this;

    view.m_settings.m_surfaceGroups = a_groups;

    return view;
  }

private:
  /**
   * @brief The function of a point's own label in a domain (RegionSettings::m_sectionLabels), as
   * Domain::signedDistances() gives it: half the difference of the smallest function of a region with that label and
   * of one with another, never positive; in the excluded regions, theirs, secondary.
   * @param[in] a_point Point.
   * @return The value, and whether the point is in the excluded regions.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline ContourSample<T>
  domainSample(const Vec3T<T>& a_point) const noexcept
  {
    constexpr uint32_t Labels   = 33U;
    constexpr uint32_t Excluded = Labels - 1U;

    T smallest[Labels];

    for (uint32_t l = 0; l < Labels; l++) {
      smallest[l] = Math::Limits<T>::max();
    }

    m_tape.regionDistances(a_point, [&](const uint32_t a_region, const T a_value) {
      const uint8_t  label = (a_region < 64U) ? m_settings.m_sectionLabels[a_region] : RegionSettings::ExcludedLabel;
      const uint32_t l     = (label < Excluded) ? uint32_t(label) : Excluded;

      smallest[l] = Math::min(smallest[l], a_value);
    });

    // The point's own label has the smallest of all; the other term is the second smallest.
    T        first  = Math::Limits<T>::max();
    T        second = Math::Limits<T>::max();
    uint32_t own    = Excluded;

    for (uint32_t l = 0; l < Labels; l++) {
      if (smallest[l] < first) {
        second = first;
        first  = smallest[l];
        own    = l;
      }
      else {
        second = Math::min(second, smallest[l]);
      }
    }

    return ContourSample<T>{T(0.5) * (first - second), own == Excluded};
  }

  /**
   * @brief The function of a set of regions, as a subdomain's: the section's regions (RegionSettings::m_sectionRegions)
   * or the visible ones.
   * @param[in] a_point   Point.
   * @param[in] a_section Whether the set is the section's regions.
   * @return Half the difference of the smallest function of a region in the set and of one not in it; the largest
   * value when the set is empty.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline T
  functionOf(const Vec3T<T>& a_point, const bool a_section) const noexcept
  {
    T in  = Math::Limits<T>::max();
    T out = Math::Limits<T>::max();

    const uint64_t section = m_settings.m_sectionRegions;

    m_tape.regionDistances(a_point, [&](const uint32_t a_region, const T a_value) {
      const bool member =
        a_section ? (a_region < 64U && ((section >> a_region) & 1U) != 0U) : this->isVisible(a_region);

      if (member) {
        in = Math::min(in, a_value);
      }
      else {
        out = Math::min(out, a_value);
      }
    });

    return (in < Math::Limits<T>::max()) ? T(0.5) * (in - out) : Math::Limits<T>::max();
  }

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
      return RegionSettings::interfaceKey(a_surface.m_ownerRegion, a_surface.m_neighborRegion);
    }
    case RegionColoring::SurfaceGroup: {
      return m_settings.m_surfaceGroups.surfaceGroupOf(a_surface);
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
 * @tparam TapeType A Tape.
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
 * Render::Settings's region settings, and the viewer edits those and picks pieces of surface for surface groups.
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
