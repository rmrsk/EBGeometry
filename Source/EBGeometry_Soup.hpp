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
#include <memory>
#include <string>
#include <vector>

// Our includes
#include "EBGeometry_DCEL.hpp"
#include "EBGeometry_ParseError.hpp"
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
 * @brief Find a polygon that visits a vertex twice.
 * @details Such a polygon corrupts the half-edge mesh built from it, so the file readers reject it
 * even when told to load other defects with a warning (Parser::OnDefect::Warn). findTopologyDefect()
 * reports it too.
 * @param[in] a_facets Index lists.
 * @return A description of the first such polygon, or an empty string if there is none.
 */
[[nodiscard]] inline static std::string
findRepeatedVertex(const std::vector<std::vector<size_t>>& a_facets);

/**
 * @brief Find a defect that stops a compressed, cleaned polygon soup from forming a half-edge mesh.
 * @details Run after compress() and removeDegeneratePolygons(). Reports the first of:
 *
 * - a polygon that visits a vertex twice;
 * - an edge traversed in the same direction by two polygons. That happens when two neighbouring
 *   polygons have opposite orientations, and whenever three or more polygons share an edge.
 *
 * An edge used by only one polygon (a hole in the surface) is not a defect here.
 * @param[in] a_facets Index lists.
 * @return A description of the defect, or an empty string if there is none.
 */
[[nodiscard]] inline static std::string
findTopologyDefect(const std::vector<std::vector<size_t>>& a_facets);

/**
 * @brief Find an edge or vertex of a half-edge mesh whose pseudonormal is zero.
 * @details Run after soupToDCEL(). An edge's pseudonormal is zero when the faces on either side of
 * it fold back onto each other, and a vertex's when the faces around it cancel out. The sign of the
 * distance near such a feature is undefined. A vertex that no face uses is not reported.
 * @tparam T    Floating-point precision type for vertex coordinates.
 * @param[in] a_mesh Mesh to check.
 * @return A description of the first such feature, or an empty string if there is none.
 */
template <typename T>
[[nodiscard]] inline static std::string
findFoldedFeature(const EBGeometry::DCEL::MeshT<T>& a_mesh);

/**
 * @brief Convert a polygon soup into a DCEL half-edge mesh.
 * @details Builds vertices, half-edges, and faces from the input arrays, reconciles
 * pair edges, runs a mesh sanity check, and computes the mesh normals (angle-weighted vertex
 * normals). a_mesh is attached to a_pool by the first reserve here
 * and is queryable from that point on, including across the reserves that follow -- a_pool need not
 * be frozen, and may stay open for further meshes.
 * @tparam T    Floating-point precision type for vertex coordinates.
 * @param[out]    a_mesh     Output DCEL mesh populated by this call.
 * @param[in,out] a_pool     Pool to reserve a_mesh's vertex/edge/face storage from.
 * @param[in]     a_vertices Compressed vertex coordinate list.
 * @param[in]     a_facets   Index lists defining each polygon face.
 * @param[in]     a_id       Identifier string used in diagnostic messages.
 */
template <typename T>
inline static void
soupToDCEL(EBGeometry::DCEL::MeshT<T>&              a_mesh,
           Pool&                                    a_pool,
           const std::vector<EBGeometry::Vec3T<T>>& a_vertices,
           const std::vector<std::vector<size_t>>&  a_facets,
           const std::string&                       a_id) noexcept;

/**
 * @brief Turn a polygon soup read from a file into a DCEL mesh, with the checks the file readers run.
 * @details The shared body of the format classes' convertToDCEL(). In order: isValid() (throws on
 * failure), compress(), removeDegeneratePolygons() (prints how many it removed),
 * findRepeatedVertex() (throws), findTopologyDefect(), soupToDCEL() and findFoldedFeature(). The last
 * two checks throw a Parser::ParseError, or with Parser::OnDefect::Warn print a warning and go on.
 * @tparam T    Floating-point precision type for vertex coordinates.
 * @param[in]     a_vertices Vertex coordinates, as read (taken by value: compressed here).
 * @param[in]     a_facets   Index lists, as read (taken by value: cleaned here).
 * @param[in,out] a_pool     Pool to reserve the mesh's storage from.
 * @param[in]     a_id       File name, for messages and the ParseError.
 * @param[in]     a_format   Name of the calling format class (e.g. "STL"), for messages.
 * @param[in]     a_onDefect Whether a topology defect or a fold throws or only warns.
 * @return The mesh.
 * @throws Parser::ParseError as described above.
 */
template <typename T>
[[nodiscard]] inline static std::shared_ptr<EBGeometry::DCEL::MeshT<T>>
readSoupIntoDCEL(std::vector<EBGeometry::Vec3T<T>> a_vertices,
                 std::vector<std::vector<size_t>>  a_facets,
                 Pool&                             a_pool,
                 const std::string&                a_id,
                 const char*                       a_format,
                 const Parser::OnDefect            a_onDefect);

/**
 * @brief Reconcile pair edges: link each half-edge with its reverse.
 * @details First builds a transient vertex-to-outgoing-half-edges table in one pass over the edges
 * (not stored on the mesh). Then, for every half-edge (u→v), it scans the half-edges starting at v
 * for one whose next edge starts at u -- the reverse half-edge (v→u) -- and sets the pair-edge index
 * on both. O(V + E) for bounded vertex valence; no circulation, so it needs no pair edges already
 * set.
 * @tparam T    Floating-point precision type.
 * @param[in,out] a_mesh Mesh whose half-edges are reconciled in place. Must already be attached to
 * the Pool its storage was reserved from, which its first reserveX() call does.
 */
template <typename T>
inline static void
reconcilePairEdgesDCEL(EBGeometry::DCEL::MeshT<T>& a_mesh) noexcept;

} // namespace Soup

} // namespace EBGeometry

#include "EBGeometry_SoupImplem.hpp"

#endif
