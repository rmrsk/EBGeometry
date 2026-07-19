// SPDX-FileCopyrightText: 2022 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file   EBGeometry_DCEL_EdgeImplem.hpp
 * @brief  Implementation of EBGeometry_DCEL_Edge.hpp
 * @author Robert Marskar
 */

#ifndef EBGEOMETRY_DCEL_EDGEIMPLEM_HPP
#define EBGEOMETRY_DCEL_EDGEIMPLEM_HPP

// Std includes
#include <cstddef>

// Our includes
#include "EBGeometry_DCEL_Edge.hpp"
#include "EBGeometry_Macros.hpp"

namespace EBGeometry {

namespace DCEL {

template <class T, class Meta>
inline EdgeT<T, Meta>::EdgeT(const DCELIndex a_vertex) noexcept
{
  m_vertex = a_vertex;
}

template <class T, class Meta>
inline size_t
EdgeT<T, Meta>::size() const noexcept
{
  return 2U;
}

template <class T, class Meta>
inline void
EdgeT<T, Meta>::define(const DCELIndex a_vertex, const DCELIndex a_pairEdge, const DCELIndex a_nextEdge) noexcept
{
  m_vertex   = a_vertex;
  m_pairEdge = a_pairEdge;
  m_nextEdge = a_nextEdge;
}

template <class T, class Meta>
inline void
EdgeT<T, Meta>::flipNormal() noexcept
{
  m_normal = -m_normal;
}

template <class T, class Meta>
inline void
EdgeT<T, Meta>::setVertex(const DCELIndex a_vertex) noexcept
{
  m_vertex = a_vertex;
}

template <class T, class Meta>
inline void
EdgeT<T, Meta>::setPairEdge(const DCELIndex a_pairEdge) noexcept
{
  m_pairEdge = a_pairEdge;
}

template <class T, class Meta>
inline void
EdgeT<T, Meta>::setNextEdge(const DCELIndex a_nextEdge) noexcept
{
  m_nextEdge = a_nextEdge;
}

template <class T, class Meta>
inline void
EdgeT<T, Meta>::setFace(const DCELIndex a_face) noexcept
{
  m_face = a_face;
}

template <class T, class Meta>
inline void
EdgeT<T, Meta>::setMetaData(const Meta& a_metaData) noexcept
{
  m_metaData = a_metaData;
}

template <class T, class Meta>
inline Vec3T<T>&
EdgeT<T, Meta>::getNormal() noexcept
{
  return m_normal;
}

template <class T, class Meta>
inline const Vec3T<T>&
EdgeT<T, Meta>::getNormal() const noexcept
{
  return m_normal;
}

template <class T, class Meta>
inline DCELIndex
EdgeT<T, Meta>::getVertex() const noexcept
{
  return m_vertex;
}

template <class T, class Meta>
inline DCELIndex
EdgeT<T, Meta>::getPairEdge() const noexcept
{
  return m_pairEdge;
}

template <class T, class Meta>
inline DCELIndex
EdgeT<T, Meta>::getNextEdge() const noexcept
{
  return m_nextEdge;
}

template <class T, class Meta>
inline DCELIndex
EdgeT<T, Meta>::getFace() const noexcept
{
  return m_face;
}

template <class T, class Meta>
inline Meta&
EdgeT<T, Meta>::getMetaData() noexcept
{
  return m_metaData;
}

template <class T, class Meta>
inline const Meta&
EdgeT<T, Meta>::getMetaData() const noexcept
{
  return m_metaData;
}
} // namespace DCEL

} // namespace EBGeometry

#endif
