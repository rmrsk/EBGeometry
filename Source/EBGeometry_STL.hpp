/* EBGeometry
 * Copyright © 2026 Robert Marskar
 * Please refer to Copyright.txt and LICENSE in the EBGeometry root directory.
 */

#ifndef EBGeometry_STL
#define EBGeometry_STL

// Std includes
#include <array>
#include <vector>
#include <map>

// Our includes
#include "EBGeometry_Vec.hpp"
#include "EBGeometry_NamespaceHeader.hpp"

/*!
  @brief Class for storing STL meshes.
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
  std::vector<std::array<Vec3T<T>, 3>>&
  getVertexCoordinates() noexcept;

  /*!
    @brief Get the vertex coordinates
    @return m_vertexCoordinates    
  */
  const std::vector<std::array<Vec3T<T>, 3>>&
  getVertexCoordinates() const noexcept;

  /*!
    @brief Get the triangle normals
    @return m_normals
  */
  std::vector<Vec3T<T>>&
  getTriangleNormals() noexcept;

  /*!
    @brief Get the triangle normals
    @return m_normals
  */
  const std::vector<Vec3T<T>>&
  getTriangleNormals() const noexcept;

  /*!
    @brief Turn the STL mesh into a DCEL mesh. 
  */
  template <typename Meta>
  std::shared_ptr<EBGeometry::DCEL::MeshT<T, Meta>>
  convertToDCEL() const noexcept;

protected:
  /*!
    @brief Vertex coordinates
  */
  std::vector<std::array<Vec3T<T>, 3>> m_vertexCoordinates;

  /*!
    @brief Triangle normals
  */
  std::vector<Vec3T<T>> m_triangleNormals;
};

#include "EBGeometry_NamespaceFooter.hpp"

#include "EBGeometry_STLImplem.hpp"

#endif
