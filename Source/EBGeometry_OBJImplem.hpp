// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file   EBGeometry_OBJImplem.hpp
 * @brief  Implementation of EBGeometry_OBJ.hpp
 * @author Robert Marskar
 */

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
#include "EBGeometry_ParseError.hpp"
#include "EBGeometry_Soup.hpp"

namespace EBGeometry {

template <typename T>
OBJ<T>::OBJ(const std::string& a_id) noexcept : OBJ()
{
  m_id = a_id;
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
OBJ<T>::convertToDCEL(Pool& a_pool, const Parser::OnDefect a_onDefect) const
{
  return Soup::readSoupIntoDCEL<T, Meta>(m_vertexCoordinates, m_facets, a_pool, m_id, "OBJ", a_onDefect);
}

} // namespace EBGeometry

#endif
