// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file   EBGeometry_PolygonSoup.hpp
 * @brief  Declaration of the polygon soup the file readers return.
 * @author Robert Marskar
 */

#ifndef EBGEOMETRY_POLYGONSOUP_HPP
#define EBGEOMETRY_POLYGONSOUP_HPP

// Std includes
#include <cstddef>
#include <map>
#include <string>
#include <type_traits>
#include <vector>

// Our includes
#include "EBGeometry_DCEL_Mesh.hpp"
#include "EBGeometry_ParseError.hpp"
#include "EBGeometry_Pool.hpp"
#include "EBGeometry_Triangle.hpp"
#include "EBGeometry_Vec.hpp"

namespace EBGeometry {

/**
 * @brief Polygons as a file stores them: vertex coordinates, faces as lists of vertex indices, and
 * named per-vertex and per-face scalar properties.
 * @details Every file reader (Parser::readSTL, readPLY, readVTK, readOBJ and readIntoPolygonSoup)
 * returns one, already cleaned (see clean()). Face i of a clean soup is face i of the DCEL mesh
 * convertToDCEL() builds, so it is the face id that MeshSDF, FlatMeshSDF and TriMeshSDF report from
 * getClosestFace(), and getFaceProperty(name)[i] is that face's property. Vertex i of a clean soup is
 * likewise vertex i of the mesh.
 *
 * The geometry is set once, at construction; properties can be added afterwards. A soup built by
 * hand is not clean until clean() is called. convertToDCEL() and convertToTriangles() clean a copy of
 * an unclean soup first, so they build the same mesh either way, but only a clean soup's own face
 * and vertex indices match the mesh's.
 * @tparam T Floating-point precision for vertex coordinates and properties.
 */
template <typename T>
class PolygonSoup
{
  static_assert(std::is_floating_point_v<T>, "PolygonSoup<T>: T must be a floating-point type");

public:
  /**
   * @brief Alias for a vector in 3D.
   */
  using Vec3 = Vec3T<T>;

  /**
   * @brief Default constructor. An empty soup with an empty identifier.
   */
  PolygonSoup() = default;

  /**
   * @brief Constructor. An empty soup with an identifier.
   * @param[in] a_id Identifier, usually the file name. ParseError reports it as the file.
   */
  explicit PolygonSoup(std::string a_id) noexcept;

  /**
   * @brief Constructor from vertex coordinates and faces, as read.
   * @details Not cleaned here, so the input may contain duplicate vertices, degenerate faces, or an
   * out-of-range index; clean() rejects or repairs them.
   * @param[in] a_id       Identifier, usually the file name. ParseError reports it as the file.
   * @param[in] a_vertices Vertex coordinates.
   * @param[in] a_facets   Faces, each a list of indices into a_vertices, in order around the face.
   */
  PolygonSoup(std::string a_id, std::vector<Vec3> a_vertices, std::vector<std::vector<size_t>> a_facets) noexcept;

  /**
   * @brief Get the identifier.
   * @return The identifier, usually the file name.
   */
  [[nodiscard]] const std::string&
  getID() const noexcept;

  /**
   * @brief Get the vertex coordinates.
   * @return The vertex coordinates.
   */
  [[nodiscard]] const std::vector<Vec3>&
  getVertexCoordinates() const noexcept;

  /**
   * @brief Get the faces.
   * @return For each face, the indices of its vertices in order around it.
   */
  [[nodiscard]] const std::vector<std::vector<size_t>>&
  getFacets() const noexcept;

  /**
   * @brief Number of vertices.
   * @return getVertexCoordinates().size().
   */
  [[nodiscard]] size_t
  numVertices() const noexcept;

  /**
   * @brief Number of faces.
   * @return getFacets().size().
   */
  [[nodiscard]] size_t
  numFacets() const noexcept;

  /**
   * @brief Whether a vertex property exists.
   * @param[in] a_name Property name.
   * @return True if the soup has a vertex property of that name.
   */
  [[nodiscard]] bool
  hasVertexProperty(const std::string& a_name) const noexcept;

  /**
   * @brief Whether a face property exists.
   * @param[in] a_name Property name.
   * @return True if the soup has a face property of that name.
   */
  [[nodiscard]] bool
  hasFaceProperty(const std::string& a_name) const noexcept;

  /**
   * @brief Get a vertex property.
   * @param[in] a_name Property name.
   * @return One value per vertex.
   * @throws std::out_of_range if the property does not exist.
   */
  [[nodiscard]] const std::vector<T>&
  getVertexProperty(const std::string& a_name) const;

  /**
   * @brief Get a face property.
   * @param[in] a_name Property name.
   * @return One value per face.
   * @throws std::out_of_range if the property does not exist.
   */
  [[nodiscard]] const std::vector<T>&
  getFaceProperty(const std::string& a_name) const;

  /**
   * @brief Names of the vertex properties.
   * @return The names, sorted.
   */
  [[nodiscard]] std::vector<std::string>
  getVertexPropertyNames() const;

  /**
   * @brief Names of the face properties.
   * @return The names, sorted.
   */
  [[nodiscard]] std::vector<std::string>
  getFacePropertyNames() const;

  /**
   * @brief Add or replace a vertex property.
   * @param[in] a_name Property name.
   * @param[in] a_data One value per vertex. Its size must be numVertices().
   */
  void
  setVertexProperty(const std::string& a_name, std::vector<T> a_data);

  /**
   * @brief Add or replace a face property.
   * @param[in] a_name Property name.
   * @param[in] a_data One value per face. Its size must be numFacets().
   */
  void
  setFaceProperty(const std::string& a_name, std::vector<T> a_data);

  /**
   * @brief Whether the soup is clean: clean() has run, and nothing has changed the geometry since.
   * @return True if the soup is clean.
   */
  [[nodiscard]] bool
  isClean() const noexcept;

  /**
   * @brief Merge duplicate vertices and remove degenerate faces, carrying the properties along.
   * @details In order:
   *
   * - Soup::isValid(): a vertex index out of range, or a non-finite coordinate, throws.
   * - Soup::compress(): vertices at the same position merge into one, and the vertices are sorted
   *   lexicographically. A merged vertex takes its properties from the first of them in the input.
   * - Soup::removeDegeneratePolygons(): faces with fewer than three distinct vertices, and zero-area
   *   faces, are removed, and the T-junctions such a face filled are repaired. The remaining faces
   *   keep their order, and their properties.
   *
   * Cleaning a clean soup changes nothing.
   * @return The number of faces removed.
   * @throws Parser::ParseError, naming getID() as the file, if the soup fails Soup::isValid().
   */
  size_t
  clean();

  /**
   * @brief Build a DCEL mesh from the soup.
   * @details Cleans a copy first if the soup is not clean (see clean()). Face i and vertex i of a
   * clean soup are face i and vertex i of the mesh. The mesh is attached to a_pool, which must
   * outlive it.
   * @param[in,out] a_pool     Pool to reserve the mesh's vertex, edge and face storage from.
   * @param[in]     a_onDefect Whether faces oriented inconsistently, an edge shared by three or more
   * faces, or faces that fold back onto each other throw (the default) or only print a warning.
   * @return The mesh, by value.
   * @throws Parser::ParseError if the soup cannot describe a mesh: it fails Soup::isValid(), it has
   * no faces, a face visits a vertex twice, the faces cannot be joined into a half-edge mesh (see
   * Soup::findTopologyDefect()), or faces fold back onto each other (see Soup::findFoldedFeature()).
   * With Parser::OnDefect::Warn the last two only print a warning.
   */
  [[nodiscard]] DCEL::MeshT<T>
  convertToDCEL(Pool& a_pool, Parser::OnDefect a_onDefect = Parser::OnDefect::Throw) const;

  /**
   * @brief Build flat triangles from the soup.
   * @details Builds the DCEL mesh as convertToDCEL() does, in a Pool private to this call, and cuts
   * each face into triangles, a polygon by a fan from its first vertex. Each triangle carries the
   * index of the mesh face it was cut from, and the mesh's edge and vertex normals.
   * @param[in] a_onDefect As for convertToDCEL().
   * @return The triangles, by value.
   * @throws Parser::ParseError as convertToDCEL().
   */
  [[nodiscard]] std::vector<Triangle<T>>
  convertToTriangles(Parser::OnDefect a_onDefect = Parser::OnDefect::Throw) const;

protected:
  /**
   * @brief Identifier, usually the file name.
   */
  std::string m_id;

  /**
   * @brief Vertex coordinates.
   */
  std::vector<Vec3> m_vertexCoordinates;

  /**
   * @brief Faces, each a list of indices into m_vertexCoordinates.
   */
  std::vector<std::vector<size_t>> m_facets;

  /**
   * @brief Vertex properties, one value per vertex.
   */
  std::map<std::string, std::vector<T>> m_vertexProperties;

  /**
   * @brief Face properties, one value per face.
   */
  std::map<std::string, std::vector<T>> m_faceProperties;

  /**
   * @brief Whether clean() has run.
   * @details The geometry is set only by the constructors, so nothing can make a clean soup unclean
   * again. A member function that changes the vertices or faces must reset this flag.
   */
  bool m_isClean = false;
};

} // namespace EBGeometry

#include "EBGeometry_PolygonSoupImplem.hpp"

#endif
