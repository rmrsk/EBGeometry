// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef EBGEOMETRY_OBJ_HPP
#define EBGEOMETRY_OBJ_HPP

// Std includes
#include <cstddef>
#include <memory>
#include <string>
#include <type_traits>
#include <vector>

// Our includes
#include "EBGeometry_DCEL_Mesh.hpp"
#include "EBGeometry_Vec.hpp"

namespace EBGeometry {

/**
 * @brief Class for storing contents of Wavefront OBJ files.
 * @details This structure is simply used as conversion utility between OBJ files and various EBGeometry objects (triangle soups, DCEL meshes, etc). Only vertex ('v') and face ('f') records are retained; texture/normal indices, groups, and materials are ignored.
 * @tparam T Floating-point precision for vertex coordinates.
 */
template <typename T>
class OBJ
{
  static_assert(std::is_floating_point_v<T>, "OBJ<T>: T must be a floating-point type");

public:
  /**
   * @brief Default constructor. Initializes empty member data holder
   */
  OBJ() noexcept;

  /**
   * @brief Constructor. Initializes empty vertices and facets but sets the OBJ ID (usually the file name
   * @param[in] a_id Identifier for OBJ object
   */
  OBJ(const std::string& a_id) noexcept;

  /**
   * @brief Destructor. Clears all data.
   */
  virtual ~OBJ() noexcept;

  /**
   * @brief Get the identifier for this object.
   * @return Reference to the OBJ object identifier.
   */
  [[nodiscard]] std::string&
  getID() noexcept;

  /**
   * @brief Get the identifier for this object.
   * @return Const reference to the OBJ object identifier.
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
   * @brief Get the polygon facet indices
   * @return m_facets
   */
  [[nodiscard]] std::vector<std::vector<size_t>>&
  getFacets() noexcept;

  /**
   * @brief Get the polygon facet indices
   * @return m_facets
   */
  [[nodiscard]] const std::vector<std::vector<size_t>>&
  getFacets() const noexcept;

  /**
   * @brief Turn the OBJ mesh into a DCEL mesh.
   * @details This call does not populate any meta-data in the DCEL mesh structures.
   * @tparam Meta Metadata type attached to DCEL vertices, edges, and faces.
   * @return Shared pointer to the constructed DCEL mesh.
   */
  template <typename Meta>
  [[nodiscard]] std::shared_ptr<EBGeometry::DCEL::MeshT<T, Meta>>
  convertToDCEL() const noexcept;

protected:
  /**
   * @brief OBJ object ID.
   */
  std::string m_id;

  /**
   * @brief Vertex coordinates
   */
  std::vector<Vec3T<T>> m_vertexCoordinates;

  /**
   * @brief Polygon facets
   */
  std::vector<std::vector<size_t>> m_facets;
};

} // namespace EBGeometry

#include "EBGeometry_OBJImplem.hpp"

#endif
