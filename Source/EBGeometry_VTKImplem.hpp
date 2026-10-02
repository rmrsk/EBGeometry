// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file   EBGeometry_VTKImplem.hpp
 * @brief  Implementation of EBGeometry_VTK.hpp
 * @author Robert Marskar
 */

#ifndef EBGEOMETRY_VTKIMPLEM_HPP
#define EBGEOMETRY_VTKIMPLEM_HPP

// Std includes
#include <cstddef>
#include <iostream>
#include <memory>
#include <string>
#include <utility>
#include <vector>

// Our includes
#include "EBGeometry_DCEL_Mesh.hpp"
#include "EBGeometry_ParseError.hpp"
#include "EBGeometry_Soup.hpp"
#include "EBGeometry_VTK.hpp"

namespace EBGeometry {

template <typename T>
VTK<T>::VTK(const std::string& a_id) noexcept : VTK()
{
  m_id = a_id;
}

template <typename T>
std::string&
VTK<T>::getID() noexcept
{
  return m_id;
}

template <typename T>
const std::string&
VTK<T>::getID() const noexcept
{
  return m_id;
}

template <typename T>
std::vector<Vec3T<T>>&
VTK<T>::getVertexCoordinates() noexcept
{
  return m_vertexCoordinates;
}

template <typename T>
const std::vector<Vec3T<T>>&
VTK<T>::getVertexCoordinates() const noexcept
{
  return m_vertexCoordinates;
}

template <typename T>
std::vector<std::vector<size_t>>&
VTK<T>::getFacets() noexcept
{
  return m_facets;
}

template <typename T>
const std::vector<std::vector<size_t>>&
VTK<T>::getFacets() const noexcept
{
  return m_facets;
}

template <typename T>
std::vector<T>&
VTK<T>::getPointDataScalars(const std::string a_name)
{
  return m_pointDataScalars.at(a_name);
}

template <typename T>
const std::vector<T>&
VTK<T>::getPointDataScalars(const std::string a_name) const
{
  return m_pointDataScalars.at(a_name);
}

template <typename T>
std::vector<T>&
VTK<T>::getCellDataScalars(const std::string a_name)
{
  return m_cellDataScalars.at(a_name);
}

template <typename T>
const std::vector<T>&
VTK<T>::getCellDataScalars(const std::string a_name) const
{
  return m_cellDataScalars.at(a_name);
}

template <typename T>
void
VTK<T>::setPointDataScalars(const std::string a_name, std::vector<T> a_data)
{
  m_pointDataScalars[a_name] = std::move(a_data);
}

template <typename T>
void
VTK<T>::setCellDataScalars(const std::string a_name, std::vector<T> a_data)
{
  m_cellDataScalars[a_name] = std::move(a_data);
}

template <typename T>
template <typename Meta>
std::shared_ptr<EBGeometry::DCEL::MeshT<T, Meta>>
VTK<T>::convertToDCEL(Pool& a_pool) const
{
  // Do a deep copy of the vertices and facets since they might need to be compressed.
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
    std::cerr << "VTK::convertToDCEL - removed " << numRemoved << " degenerate (zero-area) faces from '" << m_id
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
