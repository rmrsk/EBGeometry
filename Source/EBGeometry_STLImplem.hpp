// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef EBGEOMETRY_STLIMPLEM_HPP
#define EBGEOMETRY_STLIMPLEM_HPP

// Std includes
#include <cstddef>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

// Our includes
#include "EBGeometry_STL.hpp"
#include "EBGeometry_Soup.hpp"

namespace EBGeometry {

template <typename T>
STL<T>::STL() noexcept
{
  m_vertexCoordinates.resize(0);
  m_facets.resize(0);
  m_id = std::string();
}

template <typename T>
STL<T>::STL(const std::string& a_id) noexcept : STL()
{
  m_id = a_id;
}

template <typename T>
STL<T>::~STL() noexcept
{
  m_vertexCoordinates.resize(0);
  m_facets.resize(0);
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
STL<T>::convertToDCEL() const noexcept
{
  // Do a deep copy of the vertices and facets since they need to be compressed.
  std::vector<Vec3T<T>>            vertices = m_vertexCoordinates;
  std::vector<std::vector<size_t>> facets   = m_facets;

  if (Soup::containsDegeneratePolygons(vertices, facets)) {
    std::cerr << "STL::convertToDCEL - STL contains degenerate faces\n";
  }

  auto mesh = std::make_shared<EBGeometry::DCEL::MeshT<T, Meta>>();

  Soup::compress(vertices, facets);
  Soup::soupToDCEL(*mesh, vertices, facets, m_id);

  return mesh;
}

} // namespace EBGeometry

#endif
