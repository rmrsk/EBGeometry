// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef EBGEOMETRY_VTK_HPP
#define EBGEOMETRY_VTK_HPP

// Std includes
#include <cstddef>
#include <map>
#include <memory>
#include <string>
#include <type_traits>
#include <vector>

// Our includes
#include "EBGeometry_DCEL.hpp"
#include "EBGeometry_Vec.hpp"

namespace EBGeometry {

/**
 * @brief Class for storing VTK Polydata meshes.
 * @tparam T Floating-point precision for vertex coordinates and scalar arrays.
 */
template <typename T>
class VTK
{
  static_assert(std::is_floating_point_v<T>, "VTK<T>: T must be a floating-point type");

public:
  /**
   * @brief Default constructor. Initializes empty member data holder
   */
  VTK() noexcept;

  /**
   * @brief Constructor. Initializes empty vertices and facets but sets the VTK ID (usually the file name
   * @param[in] a_id Identifier for VTK object
   */
  VTK(const std::string& a_id) noexcept;

  /**
   * @brief Destructor. Clears all data.
   */
  virtual ~VTK() noexcept;

  /**
   * @brief Get the identifier for this object.
   * @return Reference to the VTK object identifier.
   */
  [[nodiscard]] std::string&
  getID() noexcept;

  /**
   * @brief Get the identifier for this object.
   * @return Const reference to the VTK object identifier.
   */
  [[nodiscard]] const std::string&
  getID() const noexcept;

  /**
   * @brief Get the vertex coordinates
   * @return m_vertexCoordinates
   */
  [[nodiscard]] std::vector<Vec3T<T>>&
  getVertexCoordinates() noexcept;

  /**
   * @brief Get the vertex coordinates
   * @return m_vertexCoordinates
   */
  [[nodiscard]] const std::vector<Vec3T<T>>&
  getVertexCoordinates() const noexcept;

  /**
   * @brief Get the face indices
   * @return m_facets
   */
  [[nodiscard]] std::vector<std::vector<size_t>>&
  getFacets() noexcept;

  /**
   * @brief Get the face indices
   * @return m_facets
   */
  [[nodiscard]] const std::vector<std::vector<size_t>>&
  getFacets() const noexcept;

  /**
   * @brief Get the point data scalars
   * @param[in] a_name Scalar array name
   * @note Function will fail if the array does not exist
   * @return m_pointDataScalars at provided name
   */
  [[nodiscard]] std::vector<T>&
  getPointDataScalars(const std::string a_name);

  /**
   * @brief Get the point data scalars
   * @param[in] a_name Scalar array name
   * @note Function will fail if the array does not exist
   * @return m_pointDataScalars at provided name
   */
  [[nodiscard]] const std::vector<T>&
  getPointDataScalars(const std::string a_name) const;

  /**
   * @brief Get the cell data scalars
   * @param[in] a_name Scalar array name
   * @note Function will fail if the array does not exist
   * @return m_cellDataScalars at provided name
   */
  [[nodiscard]] std::vector<T>&
  getCellDataScalars(const std::string a_name);

  /**
   * @brief Get the cell data scalars
   * @param[in] a_name Scalar array name
   * @note Function will fail if the array does not exist
   * @return m_cellDataScalars at provided name
   */
  [[nodiscard]] const std::vector<T>&
  getCellDataScalars(const std::string a_name) const;

  /**
   * @brief Set point data scalars
   * @param[in] a_name Array name
   * @param[in] a_data Array data
   */
  void
  setPointDataScalars(const std::string a_name, const std::vector<T>& a_data);

  /**
   * @brief Set cell data scalars
   * @param[in] a_name Array name
   * @param[in] a_data Array data
   */
  void
  setCellDataScalars(const std::string a_name, const std::vector<T>& a_data);

  /**
   * @brief Turn the VTK mesh into a DCEL mesh.
   * @details This call does not populate any meta-data in the DCEL mesh structures. If you need to also populate
   * the meta-data on vertices and faces, you should not use this function but supply your own constructor.
   * @tparam Meta Metadata type attached to DCEL vertices, edges, and faces.
   * @return Shared pointer to the constructed DCEL mesh.
   */
  template <typename Meta>
  [[nodiscard]] std::shared_ptr<EBGeometry::DCEL::MeshT<T, Meta>>
  convertToDCEL() const noexcept;

  // protected:
  /**
   * @brief VTK object ID.
   */
  std::string m_id;

  /**
   * @brief Vertex coordinates
   */
  std::vector<Vec3T<T>> m_vertexCoordinates;

  /**
   * @brief Faces -- each entry in the outer vector contains the indices defining one face
   */
  std::vector<std::vector<size_t>> m_facets;

  /**
   * @brief Point data scalar arrays
   */
  std::map<std::string, std::vector<T>> m_pointDataScalars;

  /**
   * @brief Cell data scalar arrays
   */
  std::map<std::string, std::vector<T>> m_cellDataScalars;
};

} // namespace EBGeometry

#include "EBGeometry_VTKImplem.hpp"

#endif
