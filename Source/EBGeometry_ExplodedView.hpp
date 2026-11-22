// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: BSD-3-Clause

/**
 * @file   EBGeometry_ExplodedView.hpp
 * @brief  The viewer's exploded view of regions: each region moved by its own offset, as in an assembly drawing.
 * @details Internal to the viewer; not part of the public API.
 * @author Robert Marskar
 */

#ifndef EBGEOMETRY_EXPLODEDVIEW_HPP
#define EBGEOMETRY_EXPLODEDVIEW_HPP

// Std includes
#include <cmath>
#include <cstdint>
#include <vector>

// Our includes
#include "EBGeometry_Array.hpp"
#include "EBGeometry_BoundingVolumes.hpp"
#include "EBGeometry_Expr.hpp"
#include "EBGeometry_GPU.hpp"
#include "EBGeometry_LeafRef.hpp"
#include "EBGeometry_Math.hpp"
#include "EBGeometry_RegionNeighborhood.hpp"
#include "EBGeometry_RegionSettings.hpp"
#include "EBGeometry_RegionView.hpp"
#include "EBGeometry_Vec.hpp"

namespace EBGeometry::Render::ViewerDetail {

/**
 * @brief A view of regions drawn exploded, as in an assembly drawing: each region moved by its own offset. A shape
 * (isShape) that a renderer draws, picks and cuts with the section plane like any other.
 * @details Internal to the viewer; not part of the public API.
 *
 * Its value at p is the least, over the visible regions k below MaxRegions (the background excepted), of region k's
 * function at p - offset_k (Tape::signedDistance(point, region)): each region keeps its own surface, its interfaces
 * with the others included, so that a jacket moved off its core shows its bore. A region whose moved box lies farther
 * from p than the least value so far is not asked: its surface is at least that far, so the least value so far is still a
 * bound on the distance to the exploded surface, and its zero set is unchanged (a region is always asked near its own
 * surface, inside its box). The
 * regions from MaxRegions on stay where they are, through the view with the moved regions hidden. Rays march it (it has
 * no ray form): exact region crossings do not survive the moves.
 *
 * A surface's key (closestLeaf(), for coloring and picking) is the one the view gives region k's surface at the point
 * moved back by k's offset (regionAt()). The offsets and the moved boxes are held in the object, so that it copies to a
 * device as it is (a kernel parameter of a few kilobytes: with the render settings, beyond the 4 KB of CUDA before 12.1, as
 * every render kernel's parameters already are), and the view's pool must outlive it, as the view's does.
 *
 * Its render kernels are its own: a renderer instantiates them only where an ExplodedView is drawn (the viewer of a
 * view of regions), and no other render's code changes.
 * @tparam V A view of regions (RegionView of a Tape).
 */
template <class V>
class ExplodedView
{
public:
  /**
   * @brief Floating-point precision.
   */
  using T = typename V::T;

  /**
   * @brief A region's box, or none.
   */
  using Box = RegionNeighborhoodDetail::Box<T>;

  /**
   * @brief How many regions (the background included) can be moved; the later ones stay where they are.
   */
  static constexpr uint32_t MaxRegions = 64;

  /**
   * @brief Each region moved away from the middle: by a_factor times its box's center less the center of the boxes of
   * all regions but the background.
   * @param[in] a_view   The view, with the region settings it is drawn with (which regions are visible, the coloring).
   * @param[in] a_boxes  Each region's box, by region number (see regionBoxes()); a region without one, or with an
   * unbounded one, stays where it is.
   * @param[in] a_factor How far apart: 0 moves none; 1 moves each center away from the middle by its distance from it.
   */
  EBGEOMETRY_HOST
  ExplodedView(const V& a_view, const std::vector<Box>& a_boxes, const T a_factor)
    : ExplodedView(a_view, a_boxes, offsetsOf(a_boxes, a_factor))
  {}

  /**
   * @brief Each region moved by its own offset.
   * @param[in] a_view    The view, with the region settings it is drawn with.
   * @param[in] a_boxes   Each region's box, by region number (see regionBoxes()).
   * @param[in] a_offsets Each region's offset, by region number (missing ones are zero).
   */
  EBGEOMETRY_HOST
  ExplodedView(const V& a_view, const std::vector<Box>& a_boxes, const std::vector<Vec3T<T>>& a_offsets)
    : m_view(a_view), m_rest(a_view)
  {
    const uint32_t numRegions = a_view.getNumRegions();

    m_count = Math::min(numRegions, MaxRegions);

    BoundingVolumes::AABBT<T> box;

    bool any = false;

    // The regions moved: visible ones with a box (an unbounded one is moved too: its box is all of space).
    for (uint32_t k = 0; k < MaxRegions; k++) {
      m_offset[k] = Vec3T<T>::zeros();
      m_low[k]    = Vec3T<T>::zeros();
      m_high[k]   = Vec3T<T>::zeros();
    }

    m_moved = 0U;

    for (uint32_t k = 1; k < m_count; k++) {
      if (!a_view.isVisible(k)) {
        continue;
      }

      // A region the clipping above it leaves nothing of has nothing to draw.
      if (k < a_boxes.size() && !a_boxes[k].m_some) {
        continue;
      }

      const bool hasBox = k < a_boxes.size();

      m_offset[k] = (k < a_offsets.size()) ? a_offsets[k] : Vec3T<T>::zeros();
      m_low[k]    = hasBox ? a_boxes[k].m_lo : -Vec3T<T>::infinity();
      m_high[k]   = hasBox ? a_boxes[k].m_hi : Vec3T<T>::infinity();
      m_moved |= uint64_t(1U) << k;

      const BoundingVolumes::AABBT<T> moved(m_low[k] + m_offset[k], m_high[k] + m_offset[k]);

      box = any ? BoundingVolumes::AABBT<T>(min(box.getLowCorner(), moved.getLowCorner()),
                                            max(box.getHighCorner(), moved.getHighCorner()))
                : moved;
      any = true;
    }

    // The regions from MaxRegions on, through the view with the moved ones hidden.
    m_hasRest = numRegions > MaxRegions;

    if (m_hasRest) {
      RegionSettings rest = a_view.getSettings();

      rest.m_hidden = ~uint64_t(0U);
      m_rest        = a_view.withSettings(rest);

      const BoundingVolumes::AABBT<T> whole = a_view.computeBoundingVolume();

      box = any ? BoundingVolumes::AABBT<T>(min(box.getLowCorner(), whole.getLowCorner()),
                                            max(box.getHighCorner(), whole.getHighCorner()))
                : whole;
      any = true;
    }

    m_box = any ? box : BoundingVolumes::AABBT<T>(Vec3T<T>::zeros(), Vec3T<T>::zeros());
  }

  /**
   * @brief The value: the least function of a visible region at the point moved back by the region's offset.
   * @param[in] a_point Point.
   * @return The value.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  EBGEOMETRY_DEVICE_NOINLINE
  inline T
  signedDistance(const Vec3T<T>& a_point) const noexcept
  {
    Vec3T<T> back;

    return this->decide(a_point, back).m_value;
  }

  /**
   * @brief The distance quality: the view's, since the value is made of its regions' functions.
   * @return The quality.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline DistanceQuality
  getDistanceQuality() const noexcept
  {
    return m_view.getDistanceQuality();
  }

  /**
   * @brief The box of the moved regions (and of the view, with regions that stay where they are).
   * @return The box.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline BoundingVolumes::AABBT<T>
  computeBoundingVolume() const noexcept
  {
    return m_box;
  }

  /**
   * @brief The region that decides the value at a point, and the point moved back into the view's frame.
   * @param[in]  a_point Point.
   * @param[out] a_back  The point moved back by the region's offset.
   * @return The region; for one of the regions that stay where they are, the one the view's tape finds at the point.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline uint32_t
  regionAt(const Vec3T<T>& a_point, Vec3T<T>& a_back) const noexcept
  {
    const Decided decided = this->decide(a_point, a_back);

    return (decided.m_region != RegionSettings::None) ? decided.m_region : m_view.getTape().region(a_point);
  }

  /**
   * @brief The key of the surface that decides the value at a point, as the view names it, as a leaf reference (only
   * m_leaf is set).
   * @param[in] a_point Point.
   * @return The key in m_leaf.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  EBGEOMETRY_DEVICE_NOINLINE
  inline LeafRef<T>
  closestLeaf(const Vec3T<T>& a_point) const noexcept
  {
    Vec3T<T>      back;
    const Decided decided = this->decide(a_point, back);

    if (decided.m_region == RegionSettings::None) {
      return m_rest.closestLeaf(a_point);
    }

    LeafRef<T> ref;

    ref.m_leaf       = this->keyOf(m_view.getTape().closestSurface(back, decided.m_region), decided.m_region);
    ref.m_localPoint = a_point;

    return ref;
  }

  /**
   * @brief What a point is in, for a section plane's fill: the material of the region the point is in, moved; the
   * background's outside them all.
   * @param[in] a_point Point.
   * @return The material's number, or none.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline uint32_t
  sectionKey(const Vec3T<T>& a_point) const noexcept
  {
    Vec3T<T>      back;
    const Decided decided = this->decide(a_point, back);

    if (decided.m_region == RegionSettings::None) {
      return m_rest.sectionKey(a_point);
    }

    return m_view.getTape().getMaterialOf((decided.m_value <= T(0)) ? decided.m_region : 0U).value();
  }

  /**
   * @brief The same exploded view of another view of the same regions: the view on a mirror of its pool (whose memory
   * the host cannot read, so that the constructors, which ask the view for its regions, could not build it), or with
   * other region settings.
   * @param[in] a_view The view, of the same tape.
   * @return The exploded view.
   */
  [[nodiscard]] EBGEOMETRY_HOST
  inline ExplodedView
  withView(const V& a_view) const noexcept
  {
    ExplodedView exploded = *this;

    exploded.m_view = a_view;
    exploded.m_rest = m_hasRest ? a_view.withSettings(m_rest.getSettings()) : a_view;

    return exploded;
  }

  /**
   * @brief The view drawn exploded.
   * @return The view.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline const V&
  getView() const noexcept
  {
    return m_view;
  }

  /**
   * @brief Each region's offset for a factor: a_factor times its box's center less the center of all boxes but the
   * background's; zero for a region without a bounded box.
   * @param[in] a_boxes  Each region's box, by region number.
   * @param[in] a_factor How far apart.
   * @return The offsets, by region number.
   */
  [[nodiscard]] EBGEOMETRY_HOST
  static inline std::vector<Vec3T<T>>
  offsetsOf(const std::vector<Box>& a_boxes, const T a_factor)
  {
    const auto bounded = [](const Box& a_box) {
      const Vec3T<T>& lo = a_box.m_lo;
      const Vec3T<T>& hi = a_box.m_hi;

      bool finite = a_box.m_some;

      for (int d = 0; d < 3; d++) {
        finite = finite && std::isfinite(lo[d]) && std::isfinite(hi[d]) && lo[d] <= hi[d];
      }

      return finite;
    };

    Vec3T<T> lo = Vec3T<T>::infinity();
    Vec3T<T> hi = -Vec3T<T>::infinity();

    for (std::size_t k = 1; k < a_boxes.size(); k++) {
      if (bounded(a_boxes[k])) {
        lo = min(lo, a_boxes[k].m_lo);
        hi = max(hi, a_boxes[k].m_hi);
      }
    }

    std::vector<Vec3T<T>> offsets(a_boxes.size(), Vec3T<T>::zeros());

    const Vec3T<T> middle = T(0.5) * (lo + hi);

    for (std::size_t k = 1; k < a_boxes.size(); k++) {
      if (bounded(a_boxes[k])) {
        offsets[k] = a_factor * (T(0.5) * (a_boxes[k].m_lo + a_boxes[k].m_hi) - middle);
      }
    }

    return offsets;
  }

private:
  /**
   * @brief The least value and the region that gives it (None: a region that stays where it is).
   */
  struct Decided
  {
    /**
     * @brief The value.
     */
    T m_value;

    /**
     * @brief The region.
     */
    uint32_t m_region;
  };

  /**
   * @brief The least function of a visible region at the point moved back by its offset, the region, and the point
   * moved back.
   * @param[in]  a_point Point.
   * @param[out] a_back  The point moved back by the deciding region's offset (a_point for none).
   * @return The value and the region.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline Decided
  decide(const Vec3T<T>& a_point, Vec3T<T>& a_back) const noexcept
  {
    Decided best{Math::Limits<T>::max(), RegionSettings::None};

    a_back = a_point;

    for (uint32_t k = 1; k < m_count; k++) {
      if (((m_moved >> k) & 1U) == 0U) {
        continue;
      }

      const Vec3T<T> q = a_point - m_offset[k];

      // The distance from q to the region's box: nothing nearer than that can come from it.
      T outside = T(0);

      for (int d = 0; d < 3; d++) {
        const T beyond = Math::max(m_low[k][d] - q[d], q[d] - m_high[k][d]);

        outside += (beyond > T(0)) ? beyond * beyond : T(0);
      }

      if ((best.m_value > T(0)) ? (outside >= best.m_value * best.m_value) : (outside > T(0))) {
        continue;
      }

      const T value = m_view.getTape().signedDistance(q, k);

      if (value < best.m_value) {
        best   = Decided{value, k};
        a_back = q;
      }
    }

    if (m_hasRest) {
      const T value = m_rest.signedDistance(a_point);

      if (value < best.m_value) {
        best   = Decided{value, RegionSettings::None};
        a_back = a_point;
      }
    }

    return best;
  }

  /**
   * @brief A region's surface's key, by the view's coloring, as the view names it.
   * @param[in] a_surface The surface.
   * @param[in] a_region  The region whose surface it is.
   * @return The key.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline uint32_t
  keyOf(const SurfaceRef<T>& a_surface, const uint32_t a_region) const noexcept
  {
    const RegionSettings& settings = m_view.getSettings();

    switch (settings.m_coloring) {
    case RegionColoring::Interface: {
      return RegionSettings::interfaceKey(a_surface.m_ownerRegion, a_surface.m_neighborRegion);
    }
    case RegionColoring::SurfaceGroup: {
      return settings.m_surfaceGroups.surfaceGroupOf(a_surface);
    }
    case RegionColoring::Material: {
      return m_view.getTape().getMaterialOf(a_region).value();
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

  /**
   * @brief The view.
   */
  V m_view;

  /**
   * @brief The view with the moved regions hidden, for the regions that stay where they are.
   */
  V m_rest;

  /**
   * @brief Whether some regions stay where they are (the view has more than MaxRegions).
   */
  bool m_hasRest = false;

  /**
   * @brief How many regions are looked at for moving: the view's, at most MaxRegions.
   */
  uint32_t m_count = 0;

  /**
   * @brief The regions moved, a bit each.
   */
  uint64_t m_moved = 0U;

  /**
   * @brief Each region's offset.
   */
  Array<Vec3T<T>, MaxRegions> m_offset;

  /**
   * @brief Each region's box, low corners (unmoved).
   */
  Array<Vec3T<T>, MaxRegions> m_low;

  /**
   * @brief See m_low.
   */
  Array<Vec3T<T>, MaxRegions> m_high;

  /**
   * @brief The box of the exploded view.
   */
  BoundingVolumes::AABBT<T> m_box;
};

/**
 * @brief Each region's box, by region number, from the expression a tape was compiled from (the background's, region
 * 0, is empty; so is one that the clipping above it leaves nothing of).
 * @details Internal to the viewer; not part of the public API.
 * @tparam T Floating-point precision.
 * @param[in] a_expr The expression.
 * @return The boxes.
 */
template <class T>
[[nodiscard]] inline std::vector<RegionNeighborhoodDetail::Box<T>>
regionBoxes(const Expr<T>& a_expr)
{
  std::vector<RegionNeighborhoodDetail::Box<T>> boxes(
    1, RegionNeighborhoodDetail::Box<T>{Vec3T<T>::zeros(), Vec3T<T>::zeros(), false});

  RegionNeighborhoodDetail::regionBoxes(a_expr, boxes);

  return boxes;
}

} // namespace EBGeometry::Render::ViewerDetail

#endif
