// SPDX-FileCopyrightText: 2022 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file   EBGeometry_DCEL_IteratorImplem.hpp
 * @brief  Implementation of EBGeometry_DCEL_Iterator.hpp
 * @author Robert Marskar
 */

#ifndef EBGEOMETRY_DCEL_ITERATORIMPLEM_HPP
#define EBGEOMETRY_DCEL_ITERATORIMPLEM_HPP

// Our includes
#include "EBGeometry_DCEL_Edge.hpp"
#include "EBGeometry_DCEL_Face.hpp"
#include "EBGeometry_DCEL_Iterator.hpp"
#include "EBGeometry_DCEL_Vertex.hpp"
#include "EBGeometry_Macros.hpp"

namespace EBGeometry {

namespace DCEL {

template <class T, class Meta>
inline EdgeIteratorT<T, Meta>::EdgeIteratorT(const Mesh& a_mesh, const Face& a_face) noexcept
{
  m_mesh      = &a_mesh;
  m_startEdge = a_face.getHalfEdgeIndex();
  m_curEdge   = m_startEdge;
}

template <class T, class Meta>
inline EdgeIteratorT<T, Meta>::EdgeIteratorT(const Mesh& a_mesh, const uint32_t a_startEdgeIndex) noexcept
{
  m_mesh      = &a_mesh;
  m_startEdge = a_startEdgeIndex;
  m_curEdge   = m_startEdge;
}

template <class T, class Meta>
inline uint32_t
EdgeIteratorT<T, Meta>::operator()() const noexcept
{
  return m_curEdge;
}

template <class T, class Meta>
inline void
EdgeIteratorT<T, Meta>::reset() noexcept
{
  m_curEdge  = m_startEdge;
  m_fullLoop = false;
}

template <class T, class Meta>
inline void
EdgeIteratorT<T, Meta>::operator++() noexcept
{
  EBGEOMETRY_EXPECT(m_curEdge != UINT32_MAX);
  EBGEOMETRY_EXPECT(m_mesh != nullptr);

  m_curEdge  = m_mesh->getEdges()[m_curEdge].getNextEdgeIndex();
  m_fullLoop = (m_curEdge == m_startEdge);
}

template <class T, class Meta>
inline bool
EdgeIteratorT<T, Meta>::ok() const noexcept
{
  return !m_fullLoop && m_curEdge != UINT32_MAX;
}
} // namespace DCEL

} // namespace EBGeometry

#endif
