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
    @return m_facets
  */
  std::vector<std::vector<size_t>>&
  getFacets() noexcept;

  /*!
    @brief Get the face indices
    @return m_facets
  */
  const std::vector<std::vector<size_t>>&
  getFacets() const noexcept;

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

  /*!
    @brief Set vertex properties
    @param[in] a_property Property name
    @param[in] a_data Property data
  */
  void
  setVertexProperties(const std::string a_property, const std::vector<T>& a_data) noexcept;

  /*!
    @brief Set face properties
    @param[in] a_property Property name
    @param[in] a_data Property data
  */
  void
  setFaceProperties(const std::string a_property, const std::vector<T>& a_data) noexcept;

  /*!
    @brief Turn the PLY mesh into a DCEL mesh.
    @details This call does not populate any meta-data in the DCEL mesh structures. If you need to also populate
    the meta-data on vertices and faces, you should not use this function but supply your own constructor. 
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
    @brief Faces -- each entry in the outer vector contains the indices defining one face
  */
  std::vector<std::vector<size_t>> m_facets;

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
