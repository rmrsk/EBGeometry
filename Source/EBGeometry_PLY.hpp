// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef EBGEOMETRY_PLY_HPP
#define EBGEOMETRY_PLY_HPP

// Std includes
#include <cstddef>
#include <map>
#include <memory>
#include <string>
#include <type_traits>
#include <vector>

// Our includes
#include "EBGeometry_Vec.hpp"
#include "EBGeometry_DCEL.hpp"

namespace EBGeometry {

/**
  @brief Class for storing Stanford PLY meshes.
  @tparam T Floating-point precision for vertex coordinates and scalar properties.
*/
template <typename T>
class PLY
{
  static_assert(std::is_floating_point_v<T>, "PLY<T>: T must be a floating-point type");

public:
  /**
    @brief Default constructor. Initializes empty member data holder
  */
  PLY() noexcept;

  /**
    @brief Constructor. Initializes empty vertices and facets but sets the PLY ID (usually the file name
    @param[in] a_id Identifier for PLY object
  */
  PLY(const std::string a_id) noexcept;

  /**
    @brief Destructor. Clears all data.
  */
  virtual ~PLY() noexcept;

  /**
    @brief Get the identifier for this object.
    @return Reference to the PLY object identifier.
  */
  [[nodiscard]] std::string&
  getID() noexcept;

  /**
    @brief Get the identifier for this object.
    @return Const reference to the PLY object identifier.
  */
  [[nodiscard]] const std::string&
  getID() const noexcept;

  /**
    @brief Get the vertex coordinates
    @return m_vertexCoordinates
  */
  [[nodiscard]] std::vector<Vec3T<T>>&
  getVertexCoordinates() noexcept;

  /**
    @brief Get the vertex coordinates
    @return m_vertexCoordinates
  */
  [[nodiscard]] const std::vector<Vec3T<T>>&
  getVertexCoordinates() const noexcept;

  /**
    @brief Get the face indices
    @return m_facets
  */
  [[nodiscard]] std::vector<std::vector<size_t>>&
  getFacets() noexcept;

  /**
    @brief Get the face indices
    @return m_facets
  */
  [[nodiscard]] const std::vector<std::vector<size_t>>&
  getFacets() const noexcept;

  /**
    @brief Get the vertex properties
    @param[in] a_property Which property to fetch
    @note Function will fail if the property does not exist
    @return m_vertexProperties at provided property
  */
  [[nodiscard]] std::vector<T>&
  getVertexProperties(const std::string a_property);

  /**
    @brief Get the vertex properties
    @param[in] a_property Which property to fetch
    @note Function will fail if the property does not exist
    @return m_vertexProperties at provided property
  */
  [[nodiscard]] const std::vector<T>&
  getVertexProperties(const std::string a_property) const;

  /**
    @brief Get the face properties
    @param[in] a_property Which property to fetch
    @note Function will fail if the property does not exist
    @return m_faceProperties at provided property
  */
  [[nodiscard]] std::vector<T>&
  getFaceProperties(const std::string a_property);

  /**
    @brief Get the face properties (const overload).
    @param[in] a_property Which property to fetch.
    @note Function will throw if the property does not exist.
    @return Const reference to the face property array at the provided name.
  */
  [[nodiscard]] const std::vector<T>&
  getFaceProperties(const std::string a_property) const;

  /**
    @brief Set vertex properties
    @param[in] a_property Property name
    @param[in] a_data Property data
  */
  void
  setVertexProperties(const std::string a_property, const std::vector<T>& a_data);

  /**
    @brief Set face properties
    @param[in] a_property Property name
    @param[in] a_data Property data
  */
  void
  setFaceProperties(const std::string a_property, const std::vector<T>& a_data);

  /**
    @brief Turn the PLY mesh into a DCEL mesh.
    @details This call does not populate any meta-data in the DCEL mesh structures. If you need to also populate
    the meta-data on vertices and faces, you should not use this function but supply your own constructor.
    @tparam Meta Metadata type attached to DCEL vertices, edges, and faces.
    @return Shared pointer to the constructed DCEL mesh.
  */
  template <typename Meta>
  [[nodiscard]] std::shared_ptr<EBGeometry::DCEL::MeshT<T, Meta>>
  convertToDCEL() const noexcept;

  //protected:
  /**
    @brief PLY object ID.
  */
  std::string m_id;

  /**
    @brief Vertex coordinates
  */
  std::vector<Vec3T<T>> m_vertexCoordinates;

  /**
    @brief Faces -- each entry in the outer vector contains the indices defining one face
  */
  std::vector<std::vector<size_t>> m_facets;

  /**
    @brief Vertex properties
  */
  std::map<std::string, std::vector<T>> m_vertexProperties;

  /**
    @brief Face properties
  */
  std::map<std::string, std::vector<T>> m_faceProperties;
};

} // namespace EBGeometry

#include "EBGeometry_PLYImplem.hpp"

#endif
