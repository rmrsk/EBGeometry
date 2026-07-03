// SPDX-FileCopyrightText: 2022 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
  @file   EBGeometry_DCEL_IteratorImplem.hpp
  @brief  Implementation of EBGeometry_DCEL_Iterator.hpp
  @author Robert Marskar
*/

#ifndef EBGEOMETRY_DCEL_ITERATORIMPLEM_HPP
#define EBGEOMETRY_DCEL_ITERATORIMPLEM_HPP

// Our includes
#include "EBGeometry_DCEL_Vertex.hpp"
#include "EBGeometry_DCEL_Edge.hpp"
#include "EBGeometry_DCEL_Face.hpp"
#include "EBGeometry_DCEL_Iterator.hpp"
#include "EBGeometry_NamespaceHeader.hpp"

namespace DCEL {

  template <class T, class Meta>
  inline EdgeIteratorT<T, Meta>::EdgeIteratorT(Face& a_face)
  {
    m_startEdge = a_face.getHalfEdge();
    m_curEdge   = m_startEdge;
    m_fullLoop  = false;
  }

  template <class T, class Meta>
  inline EdgeIteratorT<T, Meta>::EdgeIteratorT(const Face& a_face)
  {
    m_startEdge = a_face.getHalfEdge();
    m_curEdge   = m_startEdge;
    m_fullLoop  = false;
  }

  template <class T, class Meta>
  inline std::shared_ptr<EdgeT<T, Meta>>&
  EdgeIteratorT<T, Meta>::operator()() noexcept
  {
    return (m_curEdge);
  }

  template <class T, class Meta>
  inline const std::shared_ptr<EdgeT<T, Meta>>&
  EdgeIteratorT<T, Meta>::operator()() const noexcept
  {
    return (m_curEdge);
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
    m_curEdge  = m_curEdge->getNextEdge();
    m_fullLoop = (m_curEdge == m_startEdge);
  }

  template <class T, class Meta>
  inline bool
  EdgeIteratorT<T, Meta>::ok() const noexcept
  {
    return !m_fullLoop && m_curEdge;
  }
} // namespace DCEL

#include "EBGeometry_NamespaceFooter.hpp"

#endif
