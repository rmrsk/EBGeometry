// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
  @file   EBGeometry_Soup.hpp
  @brief  Declaration of polygon-soup utilities for building DCEL meshes.
  @author Robert Marskar
*/

#ifndef EBGEOMETRY_SOUP_HPP
#define EBGEOMETRY_SOUP_HPP

// Std includes
#include <vector>

// Our includes
#include "EBGeometry_Vec.hpp"
#include "EBGeometry_DCEL_Mesh.hpp"
#include "EBGeometry_NamespaceHeader.hpp"

/**
  @brief Namespace containing basic functionality for turning planar-polygon soups into DCEL meshes.
*/
namespace Soup {
  /**
    @brief Check if a polygon soup contains degenerate polygons.
    @details A polygon is degenerate if it has fewer than 3 vertices or if two or more vertices
    coincide after lexicographic sorting.
    @tparam T Floating-point precision type for vertex coordinates.
    @param[in] a_vertices Vertex coordinate list.
    @param[in] a_facets   Index lists defining each polygon face.
    @return True if any face is degenerate, false otherwise.
  */
  template <typename T>
  [[nodiscard]] inline static bool
  containsDegeneratePolygons(const std::vector<EBGeometry::Vec3T<T>>& a_vertices,
                             const std::vector<std::vector<size_t>>&  a_facets) noexcept;

  /**
    @brief Compress a polygon soup by removing duplicate vertices.
    @details After this call, `a_vertices` contains only unique vertex positions and
    `a_facets` has been updated to reference the new indices.
    @tparam T Floating-point precision type for vertex coordinates.
    @param[in,out] a_vertices Vertex coordinate list; duplicates are removed in place.
    @param[in,out] a_facets   Index lists; updated to reference the compressed vertex list.
  */
  template <typename T>
  inline static void
  compress(std::vector<EBGeometry::Vec3T<T>>& a_vertices, std::vector<std::vector<size_t>>& a_facets) noexcept;

  /**
    @brief Convert a polygon soup into a DCEL half-edge mesh.
    @details Builds vertices, half-edges, and faces from the input arrays, reconciles
    pair edges, and runs a mesh sanity check.
    @tparam T    Floating-point precision type for vertex coordinates.
    @tparam Meta Metadata type attached to DCEL vertices, edges, and faces.
    @param[out] a_mesh     Output DCEL mesh populated by this call.
    @param[in]  a_vertices Compressed vertex coordinate list.
    @param[in]  a_facets   Index lists defining each polygon face.
    @param[in]  a_id       Identifier string used in diagnostic messages.
  */
  template <typename T, typename Meta>
  inline static void
  soupToDCEL(EBGeometry::DCEL::MeshT<T, Meta>&        a_mesh,
             const std::vector<EBGeometry::Vec3T<T>>& a_vertices,
             const std::vector<std::vector<size_t>>&  a_facets,
             const std::string&                       a_id) noexcept;

  /**
    @brief Reconcile pair edges: link each half-edge with its reverse.
    @details For every half-edge (u→v) the function finds the corresponding reverse
    half-edge (v→u) by iterating over all faces incident to u and sets the pair-edge
    pointer on both.
    @tparam T    Floating-point precision type.
    @tparam Meta Metadata type attached to DCEL edges.
    @param[in,out] a_edges Collection of all half-edges to reconcile.
  */
  template <typename T, typename Meta>
  inline static void
  reconcilePairEdgesDCEL(std::vector<std::shared_ptr<EBGeometry::DCEL::EdgeT<T, Meta>>>& a_edges) noexcept;

} // namespace Soup

#include "EBGeometry_NamespaceFooter.hpp"

#include "EBGeometry_SoupImplem.hpp"

#endif
