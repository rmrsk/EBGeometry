// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: BSD-3-Clause

/**
 * @file   EBGeometry_RegionNeighborhoodImplem.hpp
 * @brief  Implementation of EBGeometry_RegionNeighborhood.hpp
 * @author Robert Marskar
 */

#ifndef EBGEOMETRY_REGIONNEIGHBORHOODIMPLEM_HPP
#define EBGEOMETRY_REGIONNEIGHBORHOODIMPLEM_HPP

// Std includes
#include <memory>
#include <optional>
#include <utility>

// Our includes
#include "EBGeometry_RegionNeighborhood.hpp"

namespace EBGeometry {

namespace RegionNeighborhoodDetail {

/**
 * @brief A region's box, or none when the clipping above it leaves nothing of it.
 * @details Internal helper; not part of the public API.
 * @tparam T Floating-point precision.
 */
template <class T>
struct Box
{
  /**
   * @brief Its low corner.
   */
  Vec3T<T> m_lo;

  /**
   * @brief Its high corner.
   */
  Vec3T<T> m_hi;

  /**
   * @brief Whether there is a box.
   */
  bool m_some = true;
};

/**
 * @brief The boxes of the regions of a partition, in region order, in the partition's frame.
 * @details Internal helper; not part of the public API. Walks the partition depth first, as ExprIndex numbers the
 * regions: an Overlay's operands in order; a map carries the boxes below it into its own frame, and a clipping
 * intersection clips them by its own box.
 * @tparam T Floating-point precision.
 * @param[in]     a_expr  A partition, or an operand of one.
 * @param[in,out] a_boxes The boxes found so far.
 */
template <class T>
inline void
regionBoxes(const Expr<T>& a_expr, std::vector<Box<T>>& a_boxes)
{
  using ExprDetail::Kind;

  const ExprDetail::Node<T>& node = a_expr.node();

  if (!node.m_partition) {
    const BoundingVolumes::AABBT<T> box = a_expr.computeBoundingVolume();

    a_boxes.push_back(Box<T>{box.getLowCorner(), box.getHighCorner(), true});

    return;
  }

  if (node.m_overlay) {
    for (const Expr<T>& child : node.m_children) {
      regionBoxes(child, a_boxes);
    }

    return;
  }

  const size_t first = a_boxes.size();

  regionBoxes(node.m_children.front(), a_boxes);

  for (size_t i = first; i < a_boxes.size(); i++) {
    Box<T>& box = a_boxes[i];

    if (!box.m_some) {
      continue;
    }

    if (node.m_kind == Kind::Affine) {
      const BoundingVolumes::AABBT<T> carried = node.m_inverse.transform(BoundingVolumes::AABBT<T>(box.m_lo, box.m_hi));

      box.m_lo = carried.getLowCorner();
      box.m_hi = carried.getHighCorner();
    }
    else {
      // A clipping intersection: the region lies inside its box too.
      box.m_lo = max(box.m_lo, node.m_boundingVolume.getLowCorner());
      box.m_hi = min(box.m_hi, node.m_boundingVolume.getHighCorner());

      for (size_t d = 0; d < 3; d++) {
        if (box.m_lo[d] > box.m_hi[d]) {
          box.m_some = false;
        }
      }
    }
  }
}

/**
 * @brief Whether two boxes meet, touching included, within a tolerance.
 * @details Internal helper; not part of the public API.
 * @tparam T Floating-point precision.
 * @param[in] a_first     A box.
 * @param[in] a_second    Another.
 * @param[in] a_tolerance How far apart they may be and still meet (rounding in the maps that carried them).
 * @return Whether they meet.
 */
template <class T>
inline bool
meet(const Box<T>& a_first, const Box<T>& a_second, const T a_tolerance) noexcept
{
  if (!a_first.m_some || !a_second.m_some) {
    return false;
  }

  for (size_t d = 0; d < 3; d++) {
    if (a_first.m_lo[d] > a_second.m_hi[d] + a_tolerance || a_second.m_lo[d] > a_first.m_hi[d] + a_tolerance) {
      return false;
    }
  }

  return true;
}

/**
 * @brief A partition with only the regions kept, or none if none of its regions is.
 * @details Internal helper; not part of the public API. Walks the partition as regionBoxes() does; a node keeps its
 * own fields (names, labels, maps), with the children kept and its box made again from theirs.
 * @tparam T Floating-point precision.
 * @param[in]     a_expr   A partition, or an operand of one.
 * @param[in]     a_kept   Whether each region is kept (region 0, the background, first).
 * @param[in,out] a_region The last region numbered so far.
 * @return The partition, or none.
 */
template <class T>
inline std::optional<Expr<T>>
keepRegions(const Expr<T>& a_expr, const std::vector<bool>& a_kept, uint32_t& a_region)
{
  using ExprDetail::Kind;

  const ExprDetail::Node<T>& node = a_expr.node();

  if (!node.m_partition) {
    a_region++;

    return a_kept[a_region] ? std::optional<Expr<T>>(a_expr) : std::nullopt;
  }

  std::vector<Expr<T>> children;

  if (node.m_overlay) {
    for (const Expr<T>& child : node.m_children) {
      auto kept = keepRegions(child, a_kept, a_region);

      if (kept) {
        children.push_back(std::move(*kept));
      }
    }

    if (children.empty()) {
      return std::nullopt;
    }
  }
  else {
    auto kept = keepRegions(node.m_children.front(), a_kept, a_region);

    if (!kept) {
      return std::nullopt;
    }

    children = node.m_children;

    children.front() = std::move(*kept);
  }

  auto copy = std::make_shared<ExprDetail::Node<T>>(node);

  copy->m_children = std::move(children);

  if (node.m_overlay) {
    BoundingVolumes::AABBT<T> box;
    DistanceQuality           quality = DistanceQuality::Bound;

    for (const Expr<T>& child : copy->m_children) {
      box     = box.merged(child.computeBoundingVolume());
      quality = ExprDetail::worse(quality, child.getDistanceQuality());
    }

    copy->m_boundingVolume = box;
    copy->m_quality        = quality;
  }
  else if (node.m_kind == Kind::Affine) {
    copy->m_boundingVolume = node.m_inverse.transform(copy->m_children.front().computeBoundingVolume());
  }
  else {
    // A clipping intersection: its box, as small as the kept regions' allow.
    const BoundingVolumes::AABBT<T> child = copy->m_children.front().computeBoundingVolume();

    Vec3T<T> lo = max(node.m_boundingVolume.getLowCorner(), child.getLowCorner());
    Vec3T<T> hi = min(node.m_boundingVolume.getHighCorner(), child.getHighCorner());

    for (size_t d = 0; d < 3; d++) {
      hi[d] = Math::max(lo[d], hi[d]);
    }

    copy->m_boundingVolume = BoundingVolumes::AABBT<T>(lo, hi);
  }

  return Expr<T>(std::shared_ptr<const ExprDetail::Node<T>>(std::move(copy)));
}

} // namespace RegionNeighborhoodDetail

template <class T>
inline const Expr<T>&
RegionNeighborhood<T>::object() const noexcept
{
  return m_object;
}

template <class T>
inline uint32_t
RegionNeighborhood<T>::getNumRegions() const noexcept
{
  return static_cast<uint32_t>(m_wholeRegion.size());
}

template <class T>
inline uint32_t
RegionNeighborhood<T>::wholeRegion(const uint32_t a_region) const
{
  EBGEOMETRY_REQUIRE(a_region < m_wholeRegion.size(),
                     "RegionNeighborhood: region %u of %u",
                     unsigned(a_region),
                     unsigned(m_wholeRegion.size()));

  return m_wholeRegion[a_region];
}

template <class T>
inline uint32_t
RegionNeighborhood<T>::region(const uint32_t a_wholeRegion) const
{
  return (a_wholeRegion < m_region.size()) ? m_region[a_wholeRegion] : None;
}

template <class T>
inline uint32_t
RegionNeighborhood<T>::surface(const uint32_t a_wholeSurface) const
{
  return (a_wholeSurface < m_surface.size()) ? m_surface[a_wholeSurface] : None;
}

template <class T>
inline uint32_t
RegionNeighborhood<T>::wholeSurface(const uint32_t a_surface) const
{
  EBGEOMETRY_REQUIRE(a_surface < m_wholeSurface.size(),
                     "RegionNeighborhood: surface %u of %u",
                     unsigned(a_surface),
                     unsigned(m_wholeSurface.size()));

  return m_wholeSurface[a_surface];
}

template <class T>
inline DomainSetup
RegionNeighborhood<T>::setup(const DomainSetup& a_wholeSetup) const
{
  DomainSetup setup;

  for (uint32_t s = 0; s < a_wholeSetup.getNumSubdomains(); s++) {
    std::vector<uint32_t> regions;

    for (const uint32_t r : a_wholeSetup.getRegions(s)) {
      const uint32_t mine = this->region(r);

      EBGEOMETRY_REQUIRE(mine != None,
                         "RegionNeighborhood: region %u of subdomain %u is not in the neighborhood; build it from the "
                         "regions of the setup's subdomains",
                         unsigned(r),
                         unsigned(s));

      regions.push_back(mine);
    }

    setup.addSubdomain(std::move(regions));
  }

  for (uint32_t g = 0; g < a_wholeSetup.getNumSurfaceGroups(); g++) {
    setup.addSurfaceGroup();
  }

  // Each condition of a selector translated; Any stays Any, and a condition on something dropped drops the selector.
  const auto translate = [](const uint32_t a_value, const std::vector<uint32_t>& a_map, uint32_t& a_out) -> bool {
    if (a_value == SurfaceSelector::Any) {
      a_out = SurfaceSelector::Any;

      return true;
    }

    a_out = (a_value < a_map.size()) ? a_map[a_value] : None;

    return a_out != None;
  };

  for (uint32_t i = 0; i < a_wholeSetup.getNumSelectors(); i++) {
    const SurfaceSelector& whole    = a_wholeSetup.getSelector(i);
    SurfaceSelector        selector = whole;

    const bool kept = translate(whole.m_surface, m_surface, selector.m_surface) &&
                      translate(whole.m_regionA, m_region, selector.m_regionA) &&
                      translate(whole.m_regionB, m_region, selector.m_regionB) &&
                      translate(whole.m_label, m_surfaceLabel, selector.m_label) &&
                      translate(whole.m_materialA, m_material, selector.m_materialA) &&
                      translate(whole.m_materialB, m_material, selector.m_materialB);

    if (kept) {
      setup.add(a_wholeSetup.getSurfaceGroupOfSelector(i), selector);
    }
  }

  return setup;
}

template <class T>
inline RegionNeighborhood<T>
regionNeighborhood(const Expr<T>& a_expr, const std::vector<uint32_t>& a_regions)
{
  using Box = RegionNeighborhoodDetail::Box<T>;

  EBGEOMETRY_REQUIRE(!a_regions.empty(), "regionNeighborhood: no regions given");

  const ExprIndex<T> whole(a_expr);

  const uint32_t numRegions = whole.getNumRegions();

  for (const uint32_t r : a_regions) {
    EBGEOMETRY_REQUIRE(r < numRegions,
                       "regionNeighborhood: region %u is not one of the expression's (0 to %u)",
                       unsigned(r),
                       unsigned(numRegions - 1U));
  }

  // The regions' boxes, region 1 first, and which regions are kept: those given, and those whose boxes meet theirs;
  // every one with the background, which meets them all.
  std::vector<Box> boxes;

  RegionNeighborhoodDetail::regionBoxes(a_expr, boxes);

  EBGEOMETRY_EXPECT(boxes.size() + 1U == numRegions);

  const BoundingVolumes::AABBT<T> scene = a_expr.computeBoundingVolume();

  const T tolerance = T(1.E-5) * (scene.getHighCorner() - scene.getLowCorner()).length();

  std::vector<bool> kept(numRegions, false);

  kept[0] = true;

  for (const uint32_t r : a_regions) {
    kept[r] = true;

    if (r == 0U) {
      kept.assign(numRegions, true);
    }
  }

  for (uint32_t r = 1; r < numRegions; r++) {
    for (const uint32_t given : a_regions) {
      if (!kept[r] && given > 0U && RegionNeighborhoodDetail::meet(boxes[r - 1U], boxes[given - 1U], tolerance)) {
        kept[r] = true;
      }
    }
  }

  uint32_t numbered = 0;

  auto object = RegionNeighborhoodDetail::keepRegions(a_expr, kept, numbered);

  EBGEOMETRY_EXPECT(object.has_value() && numbered + 1U == numRegions);

  RegionNeighborhood<T> neighborhood{std::move(*object)};

  const ExprIndex<T> mine(neighborhood.m_object);

  // The regions both ways: the kept ones keep their order.
  neighborhood.m_region.assign(numRegions, RegionNeighborhood<T>::None);

  for (uint32_t r = 0; r < numRegions; r++) {
    if (kept[r]) {
      neighborhood.m_region[r] = static_cast<uint32_t>(neighborhood.m_wholeRegion.size());

      neighborhood.m_wholeRegion.push_back(r);
    }
  }

  EBGEOMETRY_EXPECT(neighborhood.m_wholeRegion.size() == mine.getNumRegions());

  // The surfaces both ways: a leaf is kept with its region (a mask's leaves, in no region, always), and the leaves
  // kept keep their order and their surfaces.
  neighborhood.m_surface.assign(whole.getNumSurfaces(), RegionNeighborhood<T>::None);

  for (uint32_t l = 0; l < whole.getNumLeaves(); l++) {
    const uint32_t region = whole.getRegionOfLeaf(l);

    if (region == ExprIndex<T>::None || kept[region]) {
      for (uint32_t s = whole.getFirstSurface(l); s < whole.getFirstSurface(l + 1U); s++) {
        neighborhood.m_surface[s] = static_cast<uint32_t>(neighborhood.m_wholeSurface.size());

        neighborhood.m_wholeSurface.push_back(s);
      }
    }
  }

  EBGEOMETRY_EXPECT(neighborhood.m_wholeSurface.size() == mine.getNumSurfaces());

  // Labels and materials by their names.
  neighborhood.m_surfaceLabel.assign(whole.getNumSurfaceLabels(), RegionNeighborhood<T>::None);

  for (uint32_t l = 0; l < whole.getNumSurfaceLabels(); l++) {
    const SurfaceLabelId label = mine.findSurfaceLabel(whole.getSurfaceLabelName(SurfaceLabelId(l)));

    if (!label.isNone()) {
      neighborhood.m_surfaceLabel[l] = label.value();
    }
  }

  neighborhood.m_material.assign(whole.getNumMaterialLabels(), RegionNeighborhood<T>::None);

  for (uint32_t m = 0; m < whole.getNumMaterialLabels(); m++) {
    const MaterialLabelId material = mine.findMaterialLabel(whole.getMaterialLabelName(MaterialLabelId(m)));

    if (!material.isNone()) {
      neighborhood.m_material[m] = material.value();
    }
  }

  return neighborhood;
}

} // namespace EBGeometry

#endif
