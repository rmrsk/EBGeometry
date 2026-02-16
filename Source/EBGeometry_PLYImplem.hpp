/* EBGeometry
 * Copyright © 2026 Robert Marskar
 * Please refer to Copyright.txt and LICENSE in the EBGeometry root directory.
 */

#ifndef EBGeometry_PLYImplem
#define EBGeometry_PLYImplem

// Our includes
#include "EBGeometry_PLY.hpp"
#include "EBGeometry_NamespaceHeader.hpp"

template <typename T>
PLY<T>::PLY() noexcept
{
  m_vertexCoordinates.resize(0);
  m_faceIndices.resize(0);
  m_vertexProperties.resize(0);
  m_faceProperties.resize(0);
}

template <typename T>
PLY<T>::~PLY() noexcept
{
  m_vertexCoordinates.resize(0);
  m_faceIndices.resize(0);
  m_vertexProperties.resize(0);
  m_faceProperties.resize(0);
}

template <typename T>
std::vector<Vec3T<T>>&
PLY<T>::getVertexCoordinates() noexcept
{
  return m_vertexCoordinates;
}

template <typename T>
const std::vector<Vec3T<T>>&
PLY<T>::getVertexCoordinates() const noexcept
{
  return m_vertexCoordinates;
}

template <typename T>
std::vector<std::vector<size_t>>&
PLY<T>::getFaceIndices() noexcept
{
  return m_faceIndices;
}

template <typename T>
const std::vector<std::vector<size_t>>&
PLY<T>::getFaceIndices() const noexcept
{
  return m_faceIndices;
}

template <typename T>
std::vector<T>&
PLY<T>::getVertexProperties(const std::string a_property) noexcept
{
  return m_vertexProperties.at(a_property);
}

template <typename T>
const std::vector<T>&
PLY<T>::getVertexProperties(const std::string a_property) const noexcept
{
  return m_vertexProperties.at(a_property);
}

template <typename T>
std::vector<T>&
PLY<T>::getFaceProperties(const std::string a_property) noexcept
{
  return m_faceProperties.at(a_property);
}

template <typename T>
const std::vector<T>&
PLY<T>::getFaceProperties(const std::string a_property) const noexcept
{
  return m_faceProperties.at(a_property);
}

#include "EBGeometry_NamespaceFooter.hpp"

#endif
