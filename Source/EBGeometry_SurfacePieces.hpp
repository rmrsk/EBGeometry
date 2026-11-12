// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file   EBGeometry_SurfacePieces.hpp
 * @brief  The pieces of surface an object with regions has, and how a domain's surface groups cover them.
 * @author Robert Marskar
 */

#ifndef EBGEOMETRY_SURFACEPIECES_HPP
#define EBGEOMETRY_SURFACEPIECES_HPP

// Std includes
#include <algorithm>
#include <cstdint>
#include <utility>
#include <vector>

// Our includes
#include "EBGeometry_DomainSetup.hpp"
#include "EBGeometry_Expr.hpp"
#include "EBGeometry_Macros.hpp"
#include "EBGeometry_Ray.hpp"
#include "EBGeometry_SurfaceRef.hpp"

namespace EBGeometry {

/**
 * @brief One piece of an object's surface, as found: a shape's surface between two regions (see
 * SurfaceSelector::piece()), with the parts of it seen.
 * @tparam T Floating-point precision.
 */
template <class T>
struct SurfacePiece
{
  /**
   * @brief The piece where it was first crossed: its surface, part and labels, and its two sides, the later region
   * (the one on top in the Overlay) its owner, the earlier its neighbor; with their materials.
   */
  SurfaceRef<T> m_surface;

  /**
   * @brief A point on it, where a ray crossed it first.
   */
  Vec3T<T> m_point = Vec3T<T>::zeros();

  /**
   * @brief The normal there, pointing outside (into m_surface.m_neighborRegion).
   */
  Vec3T<T> m_normal = Vec3T<T>::zeros();

  /**
   * @brief How many rays crossed it: a measure of its size.
   */
  uint32_t m_crossings = 0;

  /**
   * @brief The parts of it the rays crossed (a box's faces, a mesh's faces), sorted.
   */
  std::vector<uint32_t> m_parts;
};

/**
 * @brief The pieces of surface an object with regions has, as rays find them: every surface between every pair of
 * regions they cross, each once, in the order first found.
 * @details Host only. Rays along x, y and z on a grid over the tape's box, a_resolution by a_resolution of each,
 * cross the object, and every change of region they meet (Tape::firstRegionChange(), exact where the shapes have ray
 * forms) is a crossing of a piece; crossings of the same surface between the same two regions, whichever way and
 * through whichever part, are one piece. The grid is offset by an irrational fraction of a cell, so that faces aligned
 * with the axes are not grazed. Every piece found is real; a piece no ray crosses (smaller than a cell, or thinner
 * than one along all three axes) is missed, so the pieces an object may have are possiblePieces(), and these confirm
 * which of them it has. The cost is 3 a_resolution^2 rays.
 * @tparam TapeType A Tape.
 * @param[in] a_tape       The tape.
 * @param[in] a_resolution Rays per side of the grid along each axis.
 * @return The pieces.
 */
template <class TapeType>
[[nodiscard]] EBGEOMETRY_HOST
std::vector<SurfacePiece<ShapePrecision<TapeType>>>
findSurfacePieces(const TapeType& a_tape, const uint32_t a_resolution = 64)
{
  using T = ShapePrecision<TapeType>;

  std::vector<SurfacePiece<T>> pieces;

  const BoundingVolumes::AABBT<T> box = a_tape.computeBoundingVolume();

  const Vec3T<T> low  = box.getLowCorner();
  const Vec3T<T> high = box.getHighCorner();
  const Vec3T<T> size = high - low;

  if (!(size.length() > T(0)) || !(size.length() < Math::Limits<T>::max()) || a_resolution == 0U) {
    return pieces;
  }

  // Past the box on both ends, so that every crossing is inside the interval.
  const T margin = T(0.01) * size.length();

  // The cell offset, irrational so that no ray lies in an axis-aligned face.
  const T jitter = T(0.4142135623730950488);

  for (int axis = 0; axis < 3; axis++) {
    const int u = (axis + 1) % 3;
    const int v = (axis + 2) % 3;

    for (uint32_t i = 0; i < a_resolution; i++) {
      for (uint32_t j = 0; j < a_resolution; j++) {
        Vec3T<T> origin = low;

        origin[axis] = low[axis] - margin;
        origin[u]    = low[u] + (T(i) + jitter) * size[u] / T(a_resolution);
        origin[v]    = low[v] + (T(j) + T(1) - jitter) * size[v] / T(a_resolution);

        Vec3T<T> direction = Vec3T<T>::zeros();

        direction[axis] = T(1);

        const RayT<T> ray{origin, direction};
        const T       tMax = size[axis] + T(2) * margin;

        T t = T(0);

        for (uint32_t crossing = 0; crossing < 4096U; crossing++) {
          const SurfaceHitT<T> change = a_tape.firstRegionChange(ray, t, tMax);

          if (!change.m_hit.hit()) {
            break;
          }

          // On past it: a marched crossing is placed short of the surface, within the march's tolerance.
          t = change.m_hit.m_t + (change.m_hit.m_exact ? T(0) : T(4) * RayOptions<T>().m_tolerance * size.length());

          // The piece the same whichever way it was crossed: the later region the owner, the normal pointing out of it.
          SurfaceRef<T> surface = change.m_surface;
          Vec3T<T>      normal  = change.m_hit.m_normal;

          if (surface.m_ownerRegion < surface.m_neighborRegion) {
            const uint32_t        region   = surface.m_ownerRegion;
            const MaterialLabelId material = surface.m_ownerMaterial;

            surface.m_ownerRegion      = surface.m_neighborRegion;
            surface.m_ownerMaterial    = surface.m_neighborMaterial;
            surface.m_neighborRegion   = region;
            surface.m_neighborMaterial = material;

            // The crossing's normal points into the region entered, here the inside.
            normal = -normal;
          }

          SurfacePiece<T>* known = nullptr;

          for (SurfacePiece<T>& piece : pieces) {
            const SurfaceRef<T>& other = piece.m_surface;

            if (other.m_surface == surface.m_surface && other.m_ownerRegion == surface.m_ownerRegion &&
                other.m_neighborRegion == surface.m_neighborRegion) {
              known = &piece;

              break;
            }
          }

          if (known == nullptr) {
            pieces.push_back(SurfacePiece<T>{surface, change.m_hit.m_point, normal, 0U, {}});

            known = &pieces.back();
          }

          known->m_crossings++;

          const auto at = std::lower_bound(known->m_parts.begin(), known->m_parts.end(), surface.m_part);

          if (at == known->m_parts.end() || *at != surface.m_part) {
            known->m_parts.insert(at, surface.m_part);
          }
        }
      }
    }
  }

  return pieces;
}

/**
 * @brief What an expression's index says about its surfaces, kept for checking surface groups against them without the
 * index: each surface's shape (leaf), each shape's parts and their labels, the pairs of regions each may separate,
 * and each region's material.
 * @details Host only. Built from ExprIndex (from the expression alone, no geometry), so it holds every surface and
 * every pair of regions a surface may separate (ExprIndex::getSidesOfLeaf(), conservative: some pairs listed may not
 * meet).
 */
class SurfaceInventory
{
public:
  /**
   * @brief An empty inventory: no surfaces.
   */
  SurfaceInventory() = default;

  /**
   * @brief The inventory of an expression's index.
   * @tparam T Floating-point precision.
   * @param[in] a_index The index.
   */
  template <class T>
  EBGEOMETRY_HOST
  explicit SurfaceInventory(const ExprIndex<T>& a_index)
  {
    for (uint32_t leaf = 0; leaf < a_index.getNumLeaves(); leaf++) {
      m_leafParts.push_back(a_index.getNumPartsOfLeaf(leaf));
      m_leafLabels.push_back(a_index.getSurfaceLabel(leaf));
      m_leafPartLabels.push_back(a_index.getPartSurfaceLabels(leaf));
      m_leafSides.push_back(a_index.getSidesOfLeaf(leaf));
    }

    for (uint32_t surface = 0; surface < a_index.getNumSurfaces(); surface++) {
      m_surfaceLeaves.push_back(a_index.getLeafOfSurface(surface));
    }

    for (uint32_t region = 0; region < a_index.getNumRegions(); region++) {
      m_regionMaterials.push_back(a_index.getMaterialOf(region));
    }
  }

  /**
   * @brief How many surfaces there are.
   * @return The count.
   */
  [[nodiscard]] EBGEOMETRY_HOST
  uint32_t
  getNumSurfaces() const noexcept
  {
    return static_cast<uint32_t>(m_surfaceLeaves.size());
  }

  /**
   * @brief A surface's shape.
   * @param[in] a_surface The surface (below getNumSurfaces()).
   * @return Its leaf.
   */
  [[nodiscard]] EBGEOMETRY_HOST
  uint32_t
  getLeaf(const uint32_t a_surface) const
  {
    EBGEOMETRY_REQUIRE(a_surface < m_surfaceLeaves.size(),
                       "SurfaceInventory: surface %u of %zu",
                       unsigned(a_surface),
                       m_surfaceLeaves.size());

    return m_surfaceLeaves[a_surface];
  }

  /**
   * @brief How many parts a surface's shape has (ExprIndex::getNumPartsOfLeaf()): 0 for a collection's member.
   * @param[in] a_surface The surface.
   * @return The count.
   */
  [[nodiscard]] EBGEOMETRY_HOST
  uint32_t
  getNumParts(const uint32_t a_surface) const
  {
    return m_leafParts[this->getLeaf(a_surface)];
  }

  /**
   * @brief A part's surface label: its face group's, or its shape's.
   * @param[in] a_surface The surface.
   * @param[in] a_part    The part.
   * @return The label, or none.
   */
  [[nodiscard]] EBGEOMETRY_HOST
  SurfaceLabelId
  getLabel(const uint32_t a_surface, const uint32_t a_part) const
  {
    const uint32_t                     leaf  = this->getLeaf(a_surface);
    const std::vector<SurfaceLabelId>& parts = m_leafPartLabels[leaf];

    return (a_part < parts.size()) ? parts[a_part] : m_leafLabels[leaf];
  }

  /**
   * @brief Whether a surface's parts have labels of their own (face groups).
   * @param[in] a_surface The surface.
   * @return True if they do.
   */
  [[nodiscard]] EBGEOMETRY_HOST
  bool
  hasPartLabels(const uint32_t a_surface) const
  {
    return !m_leafPartLabels[this->getLeaf(a_surface)].empty();
  }

  /**
   * @brief The pairs of regions a surface may separate, smaller region first.
   * @param[in] a_surface The surface.
   * @return The pairs.
   */
  [[nodiscard]] EBGEOMETRY_HOST
  const std::vector<std::pair<uint32_t, uint32_t>>&
  getSides(const uint32_t a_surface) const
  {
    return m_leafSides[this->getLeaf(a_surface)];
  }

  /**
   * @brief A region's material.
   * @param[in] a_region The region.
   * @return Its material label, or none.
   */
  [[nodiscard]] EBGEOMETRY_HOST
  MaterialLabelId
  getMaterial(const uint32_t a_region) const noexcept
  {
    return (a_region < m_regionMaterials.size()) ? m_regionMaterials[a_region] : MaterialLabelId::none();
  }

private:
  /**
   * @brief Each surface's leaf.
   */
  std::vector<uint32_t> m_surfaceLeaves;

  /**
   * @brief Each leaf's number of parts (0 for a collection).
   */
  std::vector<uint32_t> m_leafParts;

  /**
   * @brief Each leaf's label.
   */
  std::vector<SurfaceLabelId> m_leafLabels;

  /**
   * @brief Each leaf's parts' labels, empty when its parts have none of their own.
   */
  std::vector<std::vector<SurfaceLabelId>> m_leafPartLabels;

  /**
   * @brief Each leaf's pairs of regions.
   */
  std::vector<std::vector<std::pair<uint32_t, uint32_t>>> m_leafSides;

  /**
   * @brief Each region's material.
   */
  std::vector<MaterialLabelId> m_regionMaterials;
};

/**
 * @brief A piece of surface an object may have: a surface between two regions, the later inside.
 */
struct PossiblePiece
{
  /**
   * @brief The surface.
   */
  uint32_t m_surface = 0;

  /**
   * @brief The later region.
   */
  uint32_t m_ownerRegion = 0;

  /**
   * @brief The earlier region.
   */
  uint32_t m_neighborRegion = 0;
};

/**
 * @brief Every piece of surface an object may have: each surface between each pair of regions its index says it may
 * separate. A superset: every piece the object has is listed (findSurfacePieces() confirms which), and some listed may
 * not occur.
 * @param[in] a_inventory The surfaces (SurfaceInventory of the expression's index).
 * @return The pieces, by surface, then by pair.
 */
[[nodiscard]] EBGEOMETRY_HOST
inline std::vector<PossiblePiece>
possiblePieces(const SurfaceInventory& a_inventory)
{
  std::vector<PossiblePiece> pieces;

  for (uint32_t surface = 0; surface < a_inventory.getNumSurfaces(); surface++) {
    for (const auto& [first, second] : a_inventory.getSides(surface)) {
      pieces.push_back(PossiblePiece{surface, Math::max(first, second), Math::min(first, second)});
    }
  }

  return pieces;
}

/**
 * @brief How much of a piece a domain's surface groups hold.
 */
enum class Coverage : uint8_t
{
  None,   ///< No part of it.
  Partly, ///< Some parts, not all.
  Full    ///< All of it.
};

/**
 * @brief How surface groups cover a piece: how much of it they hold, and the surface group when one holds
 * all of it.
 */
struct PieceCoverage
{
  /**
   * @brief How much.
   */
  Coverage m_coverage = Coverage::None;

  /**
   * @brief The surface group holding all of it, or SurfaceGroups::None (none, or several).
   */
  uint32_t m_surfaceGroup = SurfaceGroups::None;
};

/**
 * @brief How a domain's surface groups cover a piece an object may have: exactly, part by part when its selectors tell parts apart.
 * @details Host only. A piece is checked once when no selector depends on the part (a part, or a label
 * that differs between parts); otherwise every part of its shape is (a mesh's faces: one check each). A member of a
 * collection has parts the inventory does not count: it is checked once, as a part no part condition matches, so
 * only selectors without one cover it.
 * @param[in] a_piece     The piece.
 * @param[in] a_inventory The object's surfaces.
 * @param[in] a_setup     The domain whose surface groups cover it (its setup).
 * @return The coverage.
 */
[[nodiscard]] EBGEOMETRY_HOST
inline PieceCoverage
coverageOf(const PossiblePiece& a_piece, const SurfaceInventory& a_inventory, const DomainSetup& a_setup)
{
  bool partConditions  = false;
  bool labelConditions = false;

  for (uint32_t i = 0; i < a_setup.getNumSelectors(); i++) {
    partConditions  = partConditions || a_setup.getSelector(i).m_part != SurfaceSelector::Any;
    labelConditions = labelConditions || a_setup.getSelector(i).m_label != SurfaceSelector::Any;
  }

  const uint32_t parts = a_inventory.getNumParts(a_piece.m_surface);
  const bool     perPart =
    parts > 1U && (partConditions || (labelConditions && a_inventory.hasPartLabels(a_piece.m_surface)));

  // The selectors match surface references; their precision does not enter.
  SurfaceRef<double> surface;

  surface.m_surface          = a_piece.m_surface;
  surface.m_leaf             = a_inventory.getLeaf(a_piece.m_surface);
  surface.m_ownerRegion      = a_piece.m_ownerRegion;
  surface.m_neighborRegion   = a_piece.m_neighborRegion;
  surface.m_ownerMaterial    = a_inventory.getMaterial(a_piece.m_ownerRegion);
  surface.m_neighborMaterial = a_inventory.getMaterial(a_piece.m_neighborRegion);

  const uint32_t checks  = perPart ? parts : 1U;
  uint32_t       covered = 0;
  uint32_t       group   = SurfaceGroups::None;
  bool           mixed   = false;

  for (uint32_t part = 0; part < checks; part++) {
    // A collection's member: a part no part condition matches.
    surface.m_part         = (parts == 0U) ? SurfaceSelector::Any - 1U : part;
    surface.m_surfaceLabel = a_inventory.getLabel(a_piece.m_surface, (parts == 0U) ? 0U : part);

    const uint32_t g = a_setup.surfaceGroupOf(surface);

    if (g != SurfaceGroups::None) {
      mixed = mixed || (covered > 0U && g != group);
      group = g;

      covered++;
    }
  }

  PieceCoverage coverage;

  coverage.m_coverage     = (covered == checks) ? Coverage::Full : (covered > 0U) ? Coverage::Partly : Coverage::None;
  coverage.m_surfaceGroup = (coverage.m_coverage == Coverage::Full && !mixed) ? group : SurfaceGroups::None;

  return coverage;
}

} // namespace EBGeometry

#endif
