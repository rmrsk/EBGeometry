/* EBGeometry
 * Copyright © 2026 Robert Marskar
 * Please refer to Copyright.txt and LICENSE in the EBGeometry root directory.
 */

/*!
  @file   EBGeometry_Soup.hpp
  @brief  Implementation of EBGeometry_Soup.hpp
  @author Robert Marskar
*/

#ifndef EBGeometry_Soup
#define EBGeometry_Soup

// Std includes
#include <vector>

// Our includes
#include "EBGeometry_Vec.hpp"
#include "EBGeometry_DCEL_Mesh.hpp"
#include "EBGeometry_NamespaceHeader.hpp"

/*!
  @brief Namespace contain the basic functionality for turning planar-polygon soups into DCEL meshes.
*/
namespace Soup {
  /*!
    @brief Check if triangle soup contains degenerate polygons
    @param[out] a_vertices Vertices
    @param[out] a_facets   Facets
  */
  template <typename T>
  inline static bool
  containsDegeneratePolygons(const std::vector<EBGeometry::Vec3T<T>>& a_vertices,
                             const std::vector<std::vector<size_t>>&  a_facets) noexcept;

  /*!
    @brief Compress triangle soup (removes duplicate vertices)
    @param[out] a_vertices   Vertices
    @param[out] a_facets     STL facets
  */
  template <typename T>
  inline static void
  compress(std::vector<EBGeometry::Vec3T<T>>& a_vertices, std::vector<std::vector<size_t>>& a_facets) noexcept;

  /*!
    @brief Turn raw vertices into DCEL vertices. 
    @param[out] a_mesh Output DCEL mesh.
    @param[in]  a_verticesRaw  Raw vertices
    @param[in]  a_facets       Facets
  */
  template <typename T, typename Meta>
  inline static void
  soupToDCEL(EBGeometry::DCEL::MeshT<T, Meta>&        a_mesh,
             const std::vector<EBGeometry::Vec3T<T>>& a_vertices,
             const std::vector<std::vector<size_t>>&  a_facets) noexcept;

  /*!
    @brief Reconcile pair edges, i.e. find the pair edge for every constructed
    half-edge
    @param[in,out] a_edges Half edges.
  */
  template <typename T, typename Meta>
  inline static void
  reconcilePairEdgesDCEL(std::vector<std::shared_ptr<EBGeometry::DCEL::EdgeT<T, Meta>>>& a_edges) noexcept;

} // namespace Soup

#include "EBGeometry_NamespaceFooter.hpp"

#include "EBGeometry_SoupImplem.hpp"

#endif
