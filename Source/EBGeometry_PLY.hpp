/* EBGeometry
 * Copyright © 2026 Robert Marskar
 * Please refer to Copyright.txt and LICENSE in the EBGeometry root directory.
 */

#ifndef EBGeometry_PLY
#define EBGeometry_PLY

// Std includes
#include <array> 
#include <vector>

// Our includes
#include "EBGeometry_Vec.hpp"
#include "EBGeometry_NamespaceHeader.hpp"

/*!
  @brief Class for storing Stanford PLY meshes.
  @note T is the precision used for storing the mesh.
*/
template <typename T>
class PLY
{
public:
  /*!
    @brief Default constructor. Initializes empty member data holder
  */
  PLY() noexcept;

  /*!
    @brief Destructor. Clears all data.
  */
  virtual ~PLY() noexcept;

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
    @brief Get the face indices
    @return m_faceIndices
  */
  std::vector<std::vector<size_t>>&
  getFaceIndices() noexcept;

  /*!
    @brief Get the face indices
    @return m_faceIndices
  */
  const std::vector<std::vector<size_t>>&
  getFaceIndices() const noexcept;

  /*!
    @brief Get the vertex properties
    @param[in] a_property Which property to fetch
    @note Function will fail if the property does not exist
    @return m_vertexProperties at provided property
  */
  std::vector<T>&
  getVertexProperties(const std::string a_property) noexcept;

  /*!
    @brief Get the vertex properties
    @param[in] a_property Which property to fetch
    @note Function will fail if the property does not exist    
    @return m_vertexProperties at provided property
  */
  const std::vector<T>&
  getVertexProperties(const std::string a_property) const noexcept;

  /*!
    @brief Get the face properties
    @param[in] a_property Which property to fetch
    @note Function will fail if the property does not exist
    @return m_faceProperties at provided property
  */
  std::vector<T>&
  getFaceProperties(const std::string a_property) noexcept;

  /*!
    @brief Get the vertex properties
    @param[in] a_property Which property to fetch
    @note Function will fail if the property does not exist    
    @return m_vertexProperties at provided property
  */
  const std::vector<T>&
  getFaceProperties(const std::string a_property) const noexcept;

protected:
  /*!
    @brief Vertex coordinates
  */
  std::vector<Vec3T<T>> m_vertexCoordinates;

  /*!
    @brief Faces -- each entry in the outer vector contains the indices defining one face
  */
  std::vector<std::vector<size_t>> m_faceIndices;

  /*!
    @brief Vertex properties
  */
  std::map<std::string, std::vector<T>> m_vertexProperties;

  /*!
    @brief Face properties
  */
  std::map<std::string, std::vector<T>> m_faceProperties;
};

#include "EBGeometry_NamespaceFooter.hpp"

#include "EBGeometry_PLYImplem.hpp"

#endif
