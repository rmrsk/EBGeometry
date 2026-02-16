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
STL<T>::STL() noexcept
{
  m_vertexCoordinates.resize(0);
  m_triangleNormals.resize(0);
}

template <typename T>
STL<T>::~STL() noexcept
{
  m_vertexCoordinates.resize(0);
  m_triangleNormals.resize(0);
}

template <typename T>
std::vector<std::array<Vec3T<T>, 3>>&
STL<T>::getVertexCoordinates() noexcept
{
  return m_vertexCoordinates;
}

template <typename T>
const std::vector<std::array<Vec3T<T>, 3>>&
STL<T>::getVertexCoordinates() const noexcept
{
  return m_vertexCoordinates;
}

template <typename T>
std::vector<Vec3T<T>>&
STL<T>::getTriangleNormals() noexcept
{
  return m_triangleNormals;
}

template <typename T>
const std::vector<Vec3T<T>>&
STL<T>::getTriangleNormals() const noexcept
{
  return m_triangleNormals;
}

#include "EBGeometry_NamespaceFooter.hpp"

#endif
