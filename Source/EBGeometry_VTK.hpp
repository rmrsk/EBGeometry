/* EBGeometry
 * Copyright © 2026 Robert Marskar
 * Please refer to Copyright.txt and LICENSE in the EBGeometry root directory.
 */

#ifndef EBGeometry_VTK
#define EBGeometry_VTK

// Std includes
#include <array>
#include <vector>
#include <map>

// Our includes
#include "EBGeometry_Vec.hpp"
#include "EBGeometry_NamespaceHeader.hpp"

/*!
  @brief Class for storing VTK Polydata meshes.
  @note T is the precision used for storing the mesh.
*/
template <typename T>
class VTK
{
public:
  /*!
    @brief Default constructor. Initializes empty member data holder
  */
  VTK() noexcept;

  /*!
    @brief Constructor. Initializes empty vertices and facets but sets the VTK ID (usually the file name
    @param[in] a_id Identifier for VTK object
  */
  VTK(const std::string a_id) noexcept;

  /*!
    @brief Destructor. Clears all data.
  */
  virtual ~VTK() noexcept;

  /*!
    @brief Get the identifier for this object
  */
  std::string&
  getID() noexcept;

  /*!
    @brief Get the identifier for this object
  */
  const std::string&
  getID() const noexcept;

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
    @brief Get the point data scalars
    @param[in] a_name Scalar array name
    @note Function will fail if the array does not exist
    @return m_pointDataScalars at provided name
  */
  std::vector<T>&
  getPointDataScalars(const std::string a_name) noexcept;

  /*!
    @brief Get the point data scalars
    @param[in] a_name Scalar array name
    @note Function will fail if the array does not exist
    @return m_pointDataScalars at provided name
  */
  const std::vector<T>&
  getPointDataScalars(const std::string a_name) const noexcept;

  /*!
    @brief Get the cell data scalars
    @param[in] a_name Scalar array name
    @note Function will fail if the array does not exist
    @return m_cellDataScalars at provided name
  */
  std::vector<T>&
  getCellDataScalars(const std::string a_name) noexcept;

  /*!
    @brief Get the cell data scalars
    @param[in] a_name Scalar array name
    @note Function will fail if the array does not exist
    @return m_cellDataScalars at provided name
  */
  const std::vector<T>&
  getCellDataScalars(const std::string a_name) const noexcept;

  /*!
    @brief Set point data scalars
    @param[in] a_name Array name
    @param[in] a_data Array data
  */
  void
  setPointDataScalars(const std::string a_name, const std::vector<T>& a_data) noexcept;

  /*!
    @brief Set cell data scalars
    @param[in] a_name Array name
    @param[in] a_data Array data
  */
  void
  setCellDataScalars(const std::string a_name, const std::vector<T>& a_data) noexcept;

  /*!
    @brief Turn the VTK mesh into a DCEL mesh.
    @details This call does not populate any meta-data in the DCEL mesh structures. If you need to also populate
    the meta-data on vertices and faces, you should not use this function but supply your own constructor.
  */
  template <typename Meta>
  std::shared_ptr<EBGeometry::DCEL::MeshT<T, Meta>>
  convertToDCEL() const noexcept;

protected:
  /*!
    @brief VTK object ID.
  */
  std::string m_id;

  /*!
    @brief Vertex coordinates
  */
  std::vector<Vec3T<T>> m_vertexCoordinates;

  /*!
    @brief Faces -- each entry in the outer vector contains the indices defining one face
  */
  std::vector<std::vector<size_t>> m_facets;

  /*!
    @brief Point data scalar arrays
  */
  std::map<std::string, std::vector<T>> m_pointDataScalars;

  /*!
    @brief Cell data scalar arrays
  */
  std::map<std::string, std::vector<T>> m_cellDataScalars;
};

#include "EBGeometry_NamespaceFooter.hpp"

#include "EBGeometry_VTKImplem.hpp"

#endif
