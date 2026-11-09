// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file   EBGeometry_SurfacePieces.hpp
 * @brief  findSurfacePieces(): the pieces of surface an object with regions has, for checking that a scheme covers them.
 * @author Robert Marskar
 */

#ifndef EBGEOMETRY_SURFACEPIECES_HPP
#define EBGEOMETRY_SURFACEPIECES_HPP

// Std includes
#include <cstdint>
#include <vector>

// Our includes
#include "EBGeometry_Macros.hpp"
#include "EBGeometry_Ray.hpp"
#include "EBGeometry_Scheme.hpp"
#include "EBGeometry_SurfaceRef.hpp"

namespace EBGeometry {

/**
 * @brief One piece of an object's surface: a shape's surface and part, between two regions (see SurfaceSelector::piece()).
 * @tparam T Floating-point precision.
 */
template <class T>
struct SurfacePiece
{
  /**
   * @brief The piece: its surface, part and labels, and its two sides, the later region (the one on top in the
   * Overlay) inside, the earlier outside; with their materials.
   */
  SurfaceRef<T> m_surface;

  /**
   * @brief A point on it, where a ray crossed it first.
   */
  Vec3T<T> m_point = Vec3T<T>::zeros();

  /**
   * @brief The normal there, pointing outside (into m_surface.m_outsideRegion).
   */
  Vec3T<T> m_normal = Vec3T<T>::zeros();

  /**
   * @brief How many rays crossed it: a measure of its size.
   */
  uint32_t m_crossings = 0;
};

/**
 * @brief The pieces of surface an object with regions has: every surface and part between every pair of regions it
 * separates, each once, in the order first found.
 * @details Host only. Rays along x, y and z on a grid over the tape's box, a_resolution by a_resolution of each,
 * cross the object, and every change of region they meet (Tape::firstRegionChange(), exact where the shapes have ray
 * forms) is a crossing of a piece; pieces with the same surface, part and pair of regions, whichever way a ray crossed
 * them, are one. The grid is offset by an irrational fraction of a cell, so that faces aligned with the axes are not
 * grazed. A piece no ray crosses (thinner than a cell along all three axes, or smaller than a cell) is missed: raise
 * a_resolution for small pieces. The cost is 3 a_resolution^2 rays.
 * @tparam TapeType A Tape that lists TapeFeature::Regions.
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

          t = change.m_hit.m_t;

          // The piece the same whichever way it was crossed: the later region inside, the normal pointing out of it.
          SurfaceRef<T> surface = change.m_surface;
          Vec3T<T>      normal  = change.m_hit.m_normal;

          if (surface.m_insideRegion < surface.m_outsideRegion) {
            const uint32_t        region   = surface.m_insideRegion;
            const MaterialLabelId material = surface.m_insideMaterial;

            surface.m_insideRegion    = surface.m_outsideRegion;
            surface.m_insideMaterial  = surface.m_outsideMaterial;
            surface.m_outsideRegion   = region;
            surface.m_outsideMaterial = material;

            // The crossing's normal points into the region entered, here the inside.
            normal = -normal;
          }

          bool known = false;

          for (SurfacePiece<T>& piece : pieces) {
            const SurfaceRef<T>& other = piece.m_surface;

            if (other.m_surface == surface.m_surface && other.m_part == surface.m_part &&
                other.m_insideRegion == surface.m_insideRegion && other.m_outsideRegion == surface.m_outsideRegion) {
              piece.m_crossings++;
              known = true;

              break;
            }
          }

          if (!known) {
            pieces.push_back(SurfacePiece<T>{surface, change.m_hit.m_point, normal, 1U});
          }
        }
      }
    }
  }

  return pieces;
}

/**
 * @brief The pieces no surface group of a scheme holds: what a solver would meet with no boundary condition.
 * @tparam T Floating-point precision.
 * @param[in] a_pieces The pieces (findSurfacePieces()).
 * @param[in] a_scheme The scheme.
 * @return The pieces in no surface group, in their order.
 */
template <class T>
[[nodiscard]] EBGEOMETRY_HOST
std::vector<SurfacePiece<T>>
unassignedPieces(const std::vector<SurfacePiece<T>>& a_pieces, const Scheme& a_scheme)
{
  std::vector<SurfacePiece<T>> unassigned;

  for (const SurfacePiece<T>& piece : a_pieces) {
    if (a_scheme.surfaceGroupOf(piece.m_surface) == Scheme::None) {
      unassigned.push_back(piece);
    }
  }

  return unassigned;
}

} // namespace EBGeometry

#endif
