// SPDX-FileCopyrightText: 2022 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file   EBGeometry_DCEL_FaceImplem.hpp
 * @brief  Implementation of EBGeometry_DCEL_Face.hpp
 * @author Robert Marskar
 */

#ifndef EBGEOMETRY_DCEL_FACEIMPLEM_HPP
#define EBGEOMETRY_DCEL_FACEIMPLEM_HPP

// Std includes
#include <cstddef>
#include <limits>

// Our includes
#include "EBGeometry_DCEL_Face.hpp"
#include "EBGeometry_Macros.hpp"

namespace EBGeometry {

namespace DCEL {

template <class T, class Meta>
inline FaceT<T, Meta>::FaceT(const DCELIndex a_edge)
{
  m_halfEdge = a_edge;
}

template <class T, class Meta>
inline void
FaceT<T, Meta>::define(const Vec3& a_normal, const DCELIndex a_edge) noexcept
{
  EBGEOMETRY_EXPECT(std::isfinite(a_normal[0]));
  EBGEOMETRY_EXPECT(std::isfinite(a_normal[1]));
  EBGEOMETRY_EXPECT(std::isfinite(a_normal[2]));

  // a_edge == InvalidIndex is valid here (e.g. a freshly-created face not yet wired into the mesh).
  m_normal   = a_normal;
  m_halfEdge = a_edge;
}

template <class T, class Meta>
inline void
FaceT<T, Meta>::flipNormal() noexcept
{
  m_normal = -m_normal;
}

template <class T, class Meta>
inline void
FaceT<T, Meta>::normalizeNormalVector() noexcept
{
  EBGEOMETRY_EXPECT(m_normal.length() > std::numeric_limits<T>::epsilon());

  m_normal = m_normal / m_normal.length();
}

template <class T, class Meta>
inline void
FaceT<T, Meta>::setHalfEdge(const DCELIndex a_halfEdge) noexcept
{
  m_halfEdge = a_halfEdge;
}

template <class T, class Meta>
inline void
FaceT<T, Meta>::setMetaData(const Meta& a_metaData) noexcept
{
  m_metaData = a_metaData;
}

template <class T, class Meta>
inline void
FaceT<T, Meta>::setPolygon2D(const Polygon2D<T>& a_poly2) noexcept
{
  m_poly2 = a_poly2;
}

template <class T, class Meta>
inline void
FaceT<T, Meta>::setInsideOutsideAlgorithm(const InsideOutsideAlgorithm a_algorithm) noexcept
{
  m_poly2Algorithm = a_algorithm;
}

template <class T, class Meta>
inline T&
FaceT<T, Meta>::getArea() noexcept
{
  return m_area;
}

template <class T, class Meta>
inline const T&
FaceT<T, Meta>::getArea() const noexcept
{
  return m_area;
}

template <class T, class Meta>
inline T&
FaceT<T, Meta>::getCentroid(const size_t a_dir) noexcept
{
  return m_centroid[a_dir];
}

template <class T, class Meta>
inline const T&
FaceT<T, Meta>::getCentroid(const size_t a_dir) const noexcept
{
  return m_centroid[a_dir];
}

template <class T, class Meta>
inline Vec3T<T>&
FaceT<T, Meta>::getCentroid() noexcept
{
  return m_centroid;
}

template <class T, class Meta>
inline const Vec3T<T>&
FaceT<T, Meta>::getCentroid() const noexcept
{
  return m_centroid;
}

template <class T, class Meta>
inline Vec3T<T>&
FaceT<T, Meta>::getNormal() noexcept
{
  return m_normal;
}

template <class T, class Meta>
inline const Vec3T<T>&
FaceT<T, Meta>::getNormal() const noexcept
{
  return m_normal;
}

template <class T, class Meta>
inline DCELIndex
FaceT<T, Meta>::getHalfEdge() const noexcept
{
  return m_halfEdge;
}

template <class T, class Meta>
inline Meta&
FaceT<T, Meta>::getMetaData() noexcept
{
  return m_metaData;
}

template <class T, class Meta>
inline const Meta&
FaceT<T, Meta>::getMetaData() const noexcept
{
  return m_metaData;
}

template <class T, class Meta>
inline const Polygon2D<T>&
FaceT<T, Meta>::getPolygon2D() const noexcept
{
  return m_poly2;
}

template <class T, class Meta>
inline typename FaceT<T, Meta>::InsideOutsideAlgorithm
FaceT<T, Meta>::getInsideOutsideAlgorithm() const noexcept
{
  return m_poly2Algorithm;
}
} // namespace DCEL

} // namespace EBGeometry

#endif
