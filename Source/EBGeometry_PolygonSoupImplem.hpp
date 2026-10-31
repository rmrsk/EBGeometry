// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file   EBGeometry_PolygonSoupImplem.hpp
 * @brief  Implementation of EBGeometry_PolygonSoup.hpp
 * @author Robert Marskar
 */

#ifndef EBGEOMETRY_POLYGONSOUPIMPLEM_HPP
#define EBGEOMETRY_POLYGONSOUPIMPLEM_HPP

// Std includes
#include <cstddef>
#include <iostream>
#include <string>
#include <utility>
#include <vector>

// Our includes
#include "EBGeometry_Macros.hpp"
#include "EBGeometry_Math.hpp"
#include "EBGeometry_MeshDistanceFunctions.hpp"
#include "EBGeometry_PolygonSoup.hpp"
#include "EBGeometry_Soup.hpp"

namespace EBGeometry {

template <typename T>
PolygonSoup<T>::PolygonSoup(std::string a_id) noexcept : m_id(std::move(a_id))
{}

template <typename T>
PolygonSoup<T>::PolygonSoup(std::string                      a_id,
                            std::vector<Vec3>                a_vertices,
                            std::vector<std::vector<size_t>> a_facets) noexcept
  : m_id(std::move(a_id)), m_vertexCoordinates(std::move(a_vertices)), m_facets(std::move(a_facets))
{}

template <typename T>
const std::string&
PolygonSoup<T>::getID() const noexcept
{
  return m_id;
}

template <typename T>
const std::vector<Vec3T<T>>&
PolygonSoup<T>::getVertexCoordinates() const noexcept
{
  return m_vertexCoordinates;
}

template <typename T>
const std::vector<std::vector<size_t>>&
PolygonSoup<T>::getFacets() const noexcept
{
  return m_facets;
}

template <typename T>
size_t
PolygonSoup<T>::numVertices() const noexcept
{
  return m_vertexCoordinates.size();
}

template <typename T>
size_t
PolygonSoup<T>::numFacets() const noexcept
{
  return m_facets.size();
}

template <typename T>
bool
PolygonSoup<T>::hasVertexProperty(const std::string& a_name) const noexcept
{
  return m_vertexProperties.find(a_name) != m_vertexProperties.end();
}

template <typename T>
bool
PolygonSoup<T>::hasFaceProperty(const std::string& a_name) const noexcept
{
  return m_faceProperties.find(a_name) != m_faceProperties.end();
}

template <typename T>
const std::vector<T>&
PolygonSoup<T>::getVertexProperty(const std::string& a_name) const
{
  return m_vertexProperties.at(a_name);
}

template <typename T>
const std::vector<T>&
PolygonSoup<T>::getFaceProperty(const std::string& a_name) const
{
  return m_faceProperties.at(a_name);
}

template <typename T>
std::vector<std::string>
PolygonSoup<T>::getVertexPropertyNames() const
{
  std::vector<std::string> names;

  names.reserve(m_vertexProperties.size());

  for (const auto& property : m_vertexProperties) {
    names.push_back(property.first);
  }

  return names;
}

template <typename T>
std::vector<std::string>
PolygonSoup<T>::getFacePropertyNames() const
{
  std::vector<std::string> names;

  names.reserve(m_faceProperties.size());

  for (const auto& property : m_faceProperties) {
    names.push_back(property.first);
  }

  return names;
}

template <typename T>
void
PolygonSoup<T>::setVertexProperty(const std::string& a_name, std::vector<T> a_data)
{
  EBGEOMETRY_REQUIRE(a_data.size() == m_vertexCoordinates.size(),
                     "PolygonSoup: vertex property '%s' has %zu values for %zu vertices",
                     a_name.c_str(),
                     a_data.size(),
                     m_vertexCoordinates.size());

  m_vertexProperties[a_name] = std::move(a_data);
}

template <typename T>
void
PolygonSoup<T>::setFaceProperty(const std::string& a_name, std::vector<T> a_data)
{
  EBGEOMETRY_REQUIRE(a_data.size() == m_facets.size(),
                     "PolygonSoup: face property '%s' has %zu values for %zu faces",
                     a_name.c_str(),
                     a_data.size(),
                     m_facets.size());

  m_faceProperties[a_name] = std::move(a_data);
}

template <typename T>
bool
PolygonSoup<T>::isClean() const noexcept
{
  return m_isClean;
}

template <typename T>
size_t
PolygonSoup<T>::clean()
{
  std::string reason;

  if (!Soup::isValid(m_vertexCoordinates, m_facets, reason)) {
    throw Parser::ParseError(m_id, 0, reason);
  }

  const size_t numOriginalVertices = m_vertexCoordinates.size();
  const size_t numOriginalFacets   = m_facets.size();

  const std::vector<size_t> newVertex     = Soup::compress(m_vertexCoordinates, m_facets);
  const std::vector<size_t> originalFacet = Soup::removeDegeneratePolygons(m_vertexCoordinates, m_facets);

  // A merged vertex takes its properties from the first of the input vertices it merges.
  constexpr size_t unset = Math::Limits<size_t>::max();

  std::vector<size_t> firstOriginal(m_vertexCoordinates.size(), unset);

  for (size_t v = 0; v < numOriginalVertices; v++) {
    if (firstOriginal[newVertex[v]] == unset) {
      firstOriginal[newVertex[v]] = v;
    }
  }

  for (auto& property : m_vertexProperties) {
    std::vector<T> remapped(m_vertexCoordinates.size());

    for (size_t v = 0; v < remapped.size(); v++) {
      remapped[v] = property.second[firstOriginal[v]];
    }

    property.second = std::move(remapped);
  }

  for (auto& property : m_faceProperties) {
    std::vector<T> kept(m_facets.size());

    for (size_t f = 0; f < kept.size(); f++) {
      kept[f] = property.second[originalFacet[f]];
    }

    property.second = std::move(kept);
  }

  m_isClean = true;

  return numOriginalFacets - m_facets.size();
}

template <typename T>
DCEL::MeshT<T>
PolygonSoup<T>::convertToDCEL(Pool& a_pool, const Parser::OnDefect a_onDefect) const
{
  if (!m_isClean) {
    PolygonSoup<T> cleaned(m_id, m_vertexCoordinates, m_facets);

    cleaned.clean();

    return cleaned.convertToDCEL(a_pool, a_onDefect);
  }

  // A defect the mesh survives: thrown, or reported and loaded anyway.
  const auto onDefect = [this, a_onDefect](const std::string& a_reason) {
    if (a_onDefect == Parser::OnDefect::Throw) {
      throw Parser::ParseError(m_id, 0, a_reason);
    }

    std::cerr << "PolygonSoup::convertToDCEL - warning: '" << m_id << "': " << a_reason
              << "; loading it anyway, so the sign of the distance near it is unreliable\n";
  };

  // A mesh with no faces describes no object, and every distance function built from it would be
  // empty. An empty file, or one whose every face was degenerate, ends up here.
  if (m_facets.empty()) {
    throw Parser::ParseError(m_id, 0, "the mesh has no faces");
  }

  // A face that visits a vertex twice corrupts the half-edge mesh built from it: never loaded.
  std::string reason = Soup::findRepeatedVertex(m_facets);

  if (!reason.empty()) {
    throw Parser::ParseError(m_id, 0, reason);
  }

  // Faces oriented inconsistently, or three or more on one edge, leave edges unpaired or paired
  // one-sidedly; the mesh still builds, but the signs near them are unreliable.
  reason = Soup::findTopologyDefect(m_facets);

  if (!reason.empty()) {
    onDefect(reason);
  }

  DCEL::MeshT<T> mesh;

  Soup::soupToDCEL(mesh, a_pool, m_vertexCoordinates, m_facets, m_id);

  reason = Soup::findFoldedFeature(mesh);

  if (!reason.empty()) {
    onDefect(reason);
  }

  return mesh;
}

template <typename T>
std::vector<Triangle<T>>
PolygonSoup<T>::convertToTriangles(const Parser::OnDefect a_onDefect) const
{
  // The DCEL mesh is only a step on the way to the triangles, which are plain values. Keeping it in a
  // Pool private to this call frees it on return; in the caller's Pool it would stay reserved (a
  // Pool never frees individual reservations) and be mirrored to the device with everything else.
  Pool scratch(hostMemoryResource());

  const DCEL::MeshT<T> mesh = this->convertToDCEL(scratch, a_onDefect);

  // The same extraction TriMeshSDF's mesh constructor uses: real half-edge normals and the face ids,
  // so a TriMeshSDF built from these triangles and one built from the mesh are identical.
  return MeshDistanceFunctionsDetail::extractTriangles(mesh);
}

} // namespace EBGeometry

#endif
