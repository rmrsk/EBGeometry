/* EBGeometry
 * Copyright © 2026 Robert Marskar
 * Please refer to Copyright.txt and LICENSE in the EBGeometry root directory.
 */

#ifndef EBGeometry_STL
#define EBGeometry_STL

// Std includes
#include <vector>
#include <map>

// Our includes
#include "EBGeometry_Vec.hpp"
#include "EBGeometry_DCEL_Mesh.hpp"
#include "EBGeometry_NamespaceHeader.hpp"

/*!
  @brief Class for storing contents of STL files.
  @details This structure is simply used as conversion utility between STL files and various EBGeometry objects (triangle soups, DCEL meshes, etc).
  @note T is the precision used for storing the mesh.
*/
template <typename T>
class STL
{
public:
  /*!
    @brief Default constructor. Initializes empty member data holder
  */
  STL() noexcept;

  /*!
    @brief Destructor. Clears all data.
  */
  virtual ~STL() noexcept;

  /*!
    @brief Get the vertex coordinates
    @return m_vertexCoordinates
  */
  std::vector<Vec3T<T>>&
  getVertexCoordinates() noexcept;

  /*!
    @brief Get the vertex coordinates
    @return m_vertexCoordinates    
  */
  const std::vector<Vec3T<T>>&
  getVertexCoordinates() const noexcept;

  /*!
    @brief Get the triangle facet indices
    @return m_facets
  */
  std::vector<std::vector<size_t>>&
  getFacets() noexcept;

  /*!
    @brief Get the triangle facet indices
    @return m_facets
  */
  const std::vector<std::vector<size_t>>&
  getFacets() const noexcept;

  /*!
    @brief Turn the STL mesh into a DCEL mesh.
    @note This call does not populate any meta-data in the DCEL mesh structures. 
  */
  template <typename Meta>
  std::shared_ptr<EBGeometry::DCEL::MeshT<T, Meta>>
  convertToDCEL() const noexcept;

protected:
  /*!
    @brief Vertex coordinates
  */
  std::vector<Vec3T<T>> m_vertexCoordinates;

  /*!
    @brief Triangle facets
  */
  std::vector<std::vector<size_t>> m_facets;
};

#include "EBGeometry_NamespaceFooter.hpp"

#include "EBGeometry_STLImplem.hpp"

#endif
