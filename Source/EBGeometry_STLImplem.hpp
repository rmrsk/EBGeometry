// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file   EBGeometry_STLImplem.hpp
 * @brief  Implementation of EBGeometry_STL.hpp
 * @author Robert Marskar
 */

#ifndef EBGEOMETRY_STLIMPLEM_HPP
#define EBGEOMETRY_STLIMPLEM_HPP

// Std includes
#include <cstddef>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

// Our includes
#include "EBGeometry_DCEL_Mesh.hpp"
#include "EBGeometry_ParseError.hpp"
#include "EBGeometry_STL.hpp"
#include "EBGeometry_Soup.hpp"

namespace EBGeometry {

template <typename T>
STL<T>::STL(const std::string& a_id) noexcept : STL()
{
  m_id = a_id;
}

template <typename T>
std::string&
STL<T>::getID() noexcept
{
  return m_id;
}

template <typename T>
const std::string&
STL<T>::getID() const noexcept
{
  return m_id;
}

template <typename T>
std::vector<Vec3T<T>>&
STL<T>::getVertexCoordinates() noexcept
{
  return m_vertexCoordinates;
}

template <typename T>
const std::vector<Vec3T<T>>&
STL<T>::getVertexCoordinates() const noexcept
{
  return m_vertexCoordinates;
}

template <typename T>
std::vector<std::vector<size_t>>&
STL<T>::getFacets() noexcept
{
  return m_facets;
}

template <typename T>
const std::vector<std::vector<size_t>>&
STL<T>::getFacets() const noexcept
{
  return m_facets;
}

template <typename T>
template <typename Meta>
std::shared_ptr<EBGeometry::DCEL::MeshT<T, Meta>>
STL<T>::convertToDCEL(Pool& a_pool) const
{
  // Do a deep copy of the vertices and facets since they need to be compressed.
  std::vector<Vec3T<T>>            vertices = m_vertexCoordinates;
  std::vector<std::vector<size_t>> facets   = m_facets;

  auto mesh = std::make_shared<EBGeometry::DCEL::MeshT<T, Meta>>();

  std::string reason;

  if (!Soup::isValid(vertices, facets, reason)) {
    throw Parser::ParseError(m_id, 0, reason);
  }

  Soup::compress(vertices, facets);

  const size_t numRemoved = Soup::removeDegeneratePolygons(vertices, facets);

  if (numRemoved > 0) {
    std::cerr << "STL::convertToDCEL - removed " << numRemoved << " degenerate (zero-area) faces from '" << m_id
              << "', merging T-junction fillers into their neighbours\n";
  }

  // A defect here would corrupt the half-edge structure: face loops that visit a vertex twice, or
  // pair edges that cannot be matched, give wrong signs or out-of-bounds reads later on.
  reason = Soup::findTopologyDefect(facets);

  if (!reason.empty()) {
    throw Parser::ParseError(m_id, 0, reason);
  }

  Soup::soupToDCEL(*mesh, a_pool, vertices, facets, m_id);

  reason = Soup::findFoldedFeature(*mesh);

  if (!reason.empty()) {
    throw Parser::ParseError(m_id, 0, reason);
  }

  return mesh;
}

} // namespace EBGeometry

#endif
