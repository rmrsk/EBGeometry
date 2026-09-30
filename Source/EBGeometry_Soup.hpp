// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file   EBGeometry_Soup.hpp
 * @brief  Declaration of polygon-soup utilities for building DCEL meshes.
 * @author Robert Marskar
 */

#ifndef EBGEOMETRY_SOUP_HPP
#define EBGEOMETRY_SOUP_HPP

// Std includes
#include <cstddef>
#include <string>
#include <vector>

// Our includes
#include "EBGeometry_DCEL.hpp"
#include "EBGeometry_Pool.hpp"
#include "EBGeometry_Vec.hpp"

namespace EBGeometry {

/**
 * @brief Namespace containing basic functionality for turning planar-polygon soups into DCEL meshes.
 */
namespace Soup {
/**
 * @brief Check if a polygon soup contains degenerate polygons.
 * @details A polygon is degenerate if it has fewer than 3 vertices, if two or more of its vertices
 * coincide, or if it has zero area (see isZeroArea()).
 * @tparam T Floating-point precision type for vertex coordinates.
 * @param[in] a_vertices Vertex coordinate list.
 * @param[in] a_facets   Index lists defining each polygon face.
 * @return True if any face is degenerate, false otherwise.
 */
template <typename T>
[[nodiscard]] inline static bool
containsDegeneratePolygons(const std::vector<EBGeometry::Vec3T<T>>& a_vertices,
                           const std::vector<std::vector<size_t>>&  a_facets) noexcept;

/**
 * @brief Check that a polygon soup can describe a mesh at all.
 * @details Every vertex index must refer to an existing vertex, and every vertex coordinate must be
 * finite. A soup read from a corrupted file typically fails one of these; the other Soup functions
 * assume both.
 * @tparam T Floating-point precision type for vertex coordinates.
 * @param[in]  a_vertices Vertex coordinate list.
 * @param[in]  a_facets   Index lists defining each polygon face.
 * @param[out] a_reason   Why the soup is invalid; untouched if it is valid.
 * @return True if the soup is valid.
 */
template <typename T>
[[nodiscard]] inline static bool
isValid(const std::vector<EBGeometry::Vec3T<T>>& a_vertices,
        const std::vector<std::vector<size_t>>&  a_facets,
        std::string&                             a_reason) noexcept;

/**
 * @brief Compress a polygon soup by removing duplicate vertices.
 * @details After this call, `a_vertices` contains only unique vertex positions and
 * `a_facets` has been updated to reference the new indices.
 * @tparam T Floating-point precision type for vertex coordinates.
 * @param[in,out] a_vertices Vertex coordinate list; duplicates are removed in place.
 * @param[in,out] a_facets   Index lists; updated to reference the compressed vertex list.
 */
template <typename T>
inline static void
compress(std::vector<EBGeometry::Vec3T<T>>& a_vertices, std::vector<std::vector<size_t>>& a_facets) noexcept;

/**
 * @brief Whether a polygon has zero area to within rounding.
 * @details Uses the polygon's Newell normal, whose length is twice the area of a planar polygon, and
 * compares it against 64 machine epsilons times the square of the polygon's longest edge. This
 * catches exactly collinear vertices after rounding, not merely thin polygons.
 * @tparam T Floating-point precision type for vertex coordinates.
 * @param[in] a_vertices Vertex coordinate list.
 * @param[in] a_facet    Index list of one polygon.
 * @return True if the polygon's area is zero to within rounding.
 */
template <typename T>
[[nodiscard]] inline static bool
isZeroArea(const std::vector<EBGeometry::Vec3T<T>>& a_vertices, const std::vector<size_t>& a_facet) noexcept;

/**
 * @brief Remove degenerate polygons from a compressed polygon soup, repairing the mesh around them.
 * @details Run after compress(), so that coincident vertices share an index. For each facet:
 *
 * - Repeated consecutive vertex indices are merged. A facet left with fewer than three vertices is
 *   removed; the facets around it pair up with each other directly.
 * - A zero-area triangle (three collinear vertices) is a T-junction filler: its middle vertex lies on
 *   its longest edge, and the facet across that edge has one fewer vertex than the geometry needs.
 *   The triangle is removed and its middle vertex is inserted into that facet between the longest
 *   edge's endpoints. The neighbour becomes a planar polygon with one straight-angle vertex, and the
 *   mesh stays closed. Without the repair, the triangle has no normal, and every edge and vertex
 *   pseudonormal next to it is wrong.
 * - Any other zero-area facet, and a zero-area triangle with no facet across its longest edge, is
 *   removed.
 *
 * @tparam T Floating-point precision type for vertex coordinates.
 * @param[in]     a_vertices Compressed vertex coordinate list.
 * @param[in,out] a_facets   Index lists; degenerate facets are removed, and neighbours of repaired
 * T-junctions gain a vertex.
 * @return Number of facets removed.
 */
template <typename T>
inline static size_t
removeDegeneratePolygons(const std::vector<EBGeometry::Vec3T<T>>& a_vertices,
                         std::vector<std::vector<size_t>>&        a_facets) noexcept;

/**
 * @brief Convert a polygon soup into a DCEL half-edge mesh.
 * @details Builds vertices, half-edges, and faces from the input arrays, reconciles
 * pair edges, and runs a mesh sanity check. a_mesh is attached to a_pool by the first reserve here
 * and is queryable from that point on, including across the reserves that follow -- a_pool need not
 * be frozen, and may stay open for further meshes.
 * @tparam T    Floating-point precision type for vertex coordinates.
 * @tparam Meta Metadata type attached to DCEL vertices, edges, and faces.
 * @param[out]    a_mesh     Output DCEL mesh populated by this call.
 * @param[in,out] a_pool     Pool to reserve a_mesh's vertex/edge/face storage from.
 * @param[in]     a_vertices Compressed vertex coordinate list.
 * @param[in]     a_facets   Index lists defining each polygon face.
 * @param[in]     a_id       Identifier string used in diagnostic messages.
 */
template <typename T, typename Meta>
inline static void
soupToDCEL(EBGeometry::DCEL::MeshT<T, Meta>&        a_mesh,
           Pool&                                    a_pool,
           const std::vector<EBGeometry::Vec3T<T>>& a_vertices,
           const std::vector<std::vector<size_t>>&  a_facets,
           const std::string&                       a_id) noexcept;

/**
 * @brief Reconcile pair edges: link each half-edge with its reverse.
 * @details For every half-edge (u→v) the function finds the corresponding reverse
 * half-edge (v→u) by circulating the half-edges around u (via pair/next edges of
 * whichever edges already have their pair set, falling back to a scan of u's
 * outgoing edges discovered so far) and sets the pair-edge index on both.
 * @tparam T    Floating-point precision type.
 * @tparam Meta Metadata type attached to DCEL edges.
 * @param[in,out] a_mesh Mesh whose half-edges are reconciled in place. Must already be attached to
 * the Pool its storage was reserved from, which its first reserveX() call does.
 */
template <typename T, typename Meta>
inline static void
reconcilePairEdgesDCEL(EBGeometry::DCEL::MeshT<T, Meta>& a_mesh) noexcept;

} // namespace Soup

} // namespace EBGeometry

#include "EBGeometry_SoupImplem.hpp"

#endif
