/* EBGeometry
 * Copyright © 2026 Robert Marskar
 * Please refer to Copyright.txt and LICENSE in the EBGeometry root directory.
 */

#ifndef EBGeometry_STLImplem
#define EBGeometry_STLImplem

// Our includes
#include "EBGeometry_STL.hpp"
#include "EBGeometry_NamespaceHeader.hpp"

template <typename T>
STL2<T>::STL2() noexcept
{
  m_vertexCoordinates.resize(0);
  m_facets.resize(0);
}

template <typename T>
STL2<T>::~STL2() noexcept
{
  m_vertexCoordinates.resize(0);
  m_facets.resize(0);
}

template <typename T>
std::vector<Vec3T<T>>&
STL2<T>::getVertexCoordinates() noexcept
{
  return m_vertexCoordinates;
}

template <typename T>
const std::vector<Vec3T<T>>&
STL2<T>::getVertexCoordinates() const noexcept
{
  return m_vertexCoordinates;
}

template <typename T>
std::vector<std::vector<size_t>>&
STL2<T>::getFacets() noexcept
{
  return m_facets;
}

template <typename T>
const std::vector<std::vector<size_t>>&
STL2<T>::getFacets() const noexcept
{
  return m_facets;
}

template <typename T>
template <typename Meta>
std::shared_ptr<EBGeometry::DCEL::MeshT<T, Meta>>
STL2<T>::convertToDCEL() const noexcept
{}

#include "EBGeometry_NamespaceFooter.hpp"

#endif
