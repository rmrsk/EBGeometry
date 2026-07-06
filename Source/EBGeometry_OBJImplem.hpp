// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef EBGEOMETRY_OBJIMPLEM_HPP
#define EBGEOMETRY_OBJIMPLEM_HPP

// Std includes
#include <cstddef>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

// Our includes
#include "EBGeometry_DCEL_Mesh.hpp"
#include "EBGeometry_OBJ.hpp"
#include "EBGeometry_Soup.hpp"

namespace EBGeometry {

template <typename T>
OBJ<T>::OBJ() noexcept
{
  m_vertexCoordinates.resize(0);
  m_facets.resize(0);
  m_id = std::string();
}

template <typename T>
OBJ<T>::OBJ(const std::string& a_id) noexcept : OBJ()
{
  m_id = a_id;
}

template <typename T>
OBJ<T>::~OBJ() noexcept
{
  m_vertexCoordinates.resize(0);
  m_facets.resize(0);
}

template <typename T>
std::string&
OBJ<T>::getID() noexcept
{
  return m_id;
}

template <typename T>
const std::string&
OBJ<T>::getID() const noexcept
{
  return m_id;
}

template <typename T>
std::vector<Vec3T<T>>&
OBJ<T>::getVertexCoordinates() noexcept
{
  return m_vertexCoordinates;
}

template <typename T>
const std::vector<Vec3T<T>>&
OBJ<T>::getVertexCoordinates() const noexcept
{
  return m_vertexCoordinates;
}

template <typename T>
std::vector<std::vector<size_t>>&
OBJ<T>::getFacets() noexcept
{
  return m_facets;
}

template <typename T>
const std::vector<std::vector<size_t>>&
OBJ<T>::getFacets() const noexcept
{
  return m_facets;
}

template <typename T>
template <typename Meta>
std::shared_ptr<EBGeometry::DCEL::MeshT<T, Meta>>
OBJ<T>::convertToDCEL() const noexcept
{
  // Do a deep copy of the vertices and facets since they need to be compressed.
  std::vector<Vec3T<T>>            vertices = m_vertexCoordinates;
  std::vector<std::vector<size_t>> facets   = m_facets;

  if (Soup::containsDegeneratePolygons(vertices, facets)) {
    std::cerr << "OBJ::convertToDCEL - OBJ contains degenerate faces\n";
  }

  auto mesh = std::make_shared<EBGeometry::DCEL::MeshT<T, Meta>>();

  Soup::compress(vertices, facets);
  Soup::soupToDCEL(*mesh, vertices, facets, m_id);

  return mesh;
}

} // namespace EBGeometry

#endif
