/* EBGeometry
 * Copyright © 2026 Robert Marskar
 * Please refer to Copyright.txt and LICENSE in the EBGeometry root directory.
 */

#ifndef EBGeometry_VTKImplem
#define EBGeometry_VTKImplem

// Our includes
#include "EBGeometry_VTK.hpp"
#include "EBGeometry_Soup.hpp"
#include "EBGeometry_NamespaceHeader.hpp"

template <typename T>
VTK<T>::VTK() noexcept
{
  m_vertexCoordinates.resize(0);
  m_facets.resize(0);
  m_pointDataScalars.clear();
  m_cellDataScalars.clear();
}

template <typename T>
VTK<T>::~VTK() noexcept
{
  m_vertexCoordinates.resize(0);
  m_facets.resize(0);
  m_pointDataScalars.clear();
  m_cellDataScalars.clear();
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
VTK<T>::getPointDataScalars(const std::string a_name) noexcept
{
  return m_pointDataScalars.at(a_name);
}

template <typename T>
const std::vector<T>&
VTK<T>::getPointDataScalars(const std::string a_name) const noexcept
{
  return m_pointDataScalars.at(a_name);
}

template <typename T>
std::vector<T>&
VTK<T>::getCellDataScalars(const std::string a_name) noexcept
{
  return m_cellDataScalars.at(a_name);
}

template <typename T>
const std::vector<T>&
VTK<T>::getCellDataScalars(const std::string a_name) const noexcept
{
  return m_cellDataScalars.at(a_name);
}

template <typename T>
void
VTK<T>::setPointDataScalars(const std::string a_name, const std::vector<T>& a_data) noexcept
{
  m_pointDataScalars[a_name] = a_data;
}

template <typename T>
void
VTK<T>::setCellDataScalars(const std::string a_name, const std::vector<T>& a_data) noexcept
{
  m_cellDataScalars[a_name] = a_data;
}

template <typename T>
template <typename Meta>
std::shared_ptr<EBGeometry::DCEL::MeshT<T, Meta>>
VTK<T>::convertToDCEL() const noexcept
{
  // Do a deep copy of the vertices and facets since they might need to be compressed.
  std::vector<Vec3T<T>>            vertices = m_vertexCoordinates;
  std::vector<std::vector<size_t>> facets   = m_facets;

  if (Soup::containsDegeneratePolygons(vertices, facets)) {
    std::cerr << "VTK::convertToDCEL - VTK contains degenerate faces\n";
  }

  auto mesh = std::make_shared<EBGeometry::DCEL::MeshT<T, Meta>>();

  Soup::compress(vertices, facets);
  Soup::soupToDCEL(*mesh, vertices, facets);

  return mesh;
}

#include "EBGeometry_NamespaceFooter.hpp"

#endif
