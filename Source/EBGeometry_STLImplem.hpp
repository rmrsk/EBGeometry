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
std::shared_ptr<EBGeometry::DCEL::MeshT<T>>
STL<T>::convertToDCEL(Pool& a_pool, const Parser::OnDefect a_onDefect) const
{
  return Soup::readSoupIntoDCEL<T>(m_vertexCoordinates, m_facets, a_pool, m_id, "STL", a_onDefect);
}

} // namespace EBGeometry

#endif
