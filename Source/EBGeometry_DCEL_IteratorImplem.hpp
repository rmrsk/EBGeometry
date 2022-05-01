/* EBGeometry
 * Copyright © 2022 Robert Marskar
 * Please refer to Copyright.txt and LICENSE in the EBGeometry root directory.
 */

/*!
  @file   EBGeometry_DCEL_IteratorImplem.hpp
  @brief  Implementation of EBGeometry_DCEL_Iterator.hpp
  @author Robert Marskar
*/

#ifndef EBGeometry_DCEL_IteratorImplem
#define EBGeometry_DCEL_IteratorImplem

// Our includes
#include "EBGeometry_DCEL_Vertex.hpp"
#include "EBGeometry_DCEL_Edge.hpp"
#include "EBGeometry_DCEL_Face.hpp"
#include "EBGeometry_DCEL_Iterator.hpp"
#include "EBGeometry_NamespaceHeader.hpp"

namespace DCEL {

template <class T>
inline EdgeIteratorT<T>::EdgeIteratorT(Face& a_face)
{
  m_startEdge = a_face.getHalfEdge();
  m_curEdge = m_startEdge;
  m_fullLoop = false;
  m_iterMode = IterationMode::Faces;
}

template <class T>
inline EdgeIteratorT<T>::EdgeIteratorT(const Face& a_face)
{
  m_startEdge = a_face.getHalfEdge();
  m_curEdge = m_startEdge;
  m_fullLoop = false;
  m_iterMode = IterationMode::Faces;
}

template <class T>
inline EdgeIteratorT<T>::EdgeIteratorT(Vertex& a_vert)
{
  m_startEdge = a_vert.getOutgoingEdge();
  m_curEdge = m_startEdge;
  m_fullLoop = false;
  m_iterMode = IterationMode::Vertices;
}

template <class T>
inline EdgeIteratorT<T>::EdgeIteratorT(const Vertex& a_vert)
{
  m_startEdge = a_vert.getOutgoingEdge();
  m_curEdge = m_startEdge;
  m_fullLoop = false;
  m_iterMode = IterationMode::Vertices;
}

template <class T>
inline std::shared_ptr<EdgeT<T>>&
EdgeIteratorT<T>::operator()() noexcept
{
  return (m_curEdge);
}

template <class T>
inline const std::shared_ptr<EdgeT<T>>&
EdgeIteratorT<T>::operator()() const noexcept
{
  return (m_curEdge);
}

template <class T>
inline void
EdgeIteratorT<T>::reset() noexcept
{
  m_curEdge = m_startEdge;
  m_fullLoop = false;
}

template <class T>
inline void
EdgeIteratorT<T>::operator++() noexcept
{
  switch (m_iterMode) {
  case IterationMode::Faces: {
    m_curEdge = m_curEdge->getNextEdge();

    break;
  }
  case IterationMode::Vertices: {
    // For vertices, we want to compute the
    m_curEdge = m_curEdge->getPreviousEdge()->getPairEdge();

    break;
  }
  default: {
    std::cerr << "In file 'EBGeometry_DCEL_IteratorImplem.hpp function "
                 "EdgeIteratorT<T>::operator++ - logic bust\n";
  }
  }

  m_fullLoop = (m_curEdge == m_startEdge);
}

template <class T>
inline bool
EdgeIteratorT<T>::ok() const noexcept
{
  return !m_fullLoop && m_curEdge;
}
} // namespace DCEL

#include "EBGeometry_NamespaceFooter.hpp"

#endif
