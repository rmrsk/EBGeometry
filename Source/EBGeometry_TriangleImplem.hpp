// SPDX-FileCopyrightText: 2024 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file   EBGeometry_TriangleImplem.hpp
 * @brief  Implementation of EBGeometry_Triangle.hpp
 * @author Robert Marskar
 */

#ifndef EBGEOMETRY_TRIANGLEIMPLEM_HPP
#define EBGEOMETRY_TRIANGLEIMPLEM_HPP

// Std includes
#include <cmath>
#include <type_traits>

// Our includes
#include "EBGeometry_Array.hpp"
#include "EBGeometry_Macros.hpp"
#include "EBGeometry_Math.hpp"
#include "EBGeometry_Triangle.hpp"
#include "EBGeometry_TriangleDistance.hpp"

namespace EBGeometry {

template <class T>
Triangle<T>::Triangle(const Array<Vec3T<T>, 3>& a_vertexPositions) noexcept
{
  this->setVertexPositions(a_vertexPositions);
}

template <class T>
void
Triangle<T>::setNormal(const Vec3T<T>& a_normal) noexcept
{
  EBGEOMETRY_EXPECT(std::isfinite(a_normal[0]));
  EBGEOMETRY_EXPECT(std::isfinite(a_normal[1]));
  EBGEOMETRY_EXPECT(std::isfinite(a_normal[2]));
  EBGEOMETRY_EXPECT(std::abs(a_normal.length() - T(1)) < std::sqrt(Math::Limits<T>::epsilon()));

  this->m_triangleNormal = a_normal;
}

template <class T>
void
Triangle<T>::setVertexPositions(const Array<Vec3T<T>, 3>& a_vertexPositions) noexcept
{
  for ([[maybe_unused]] const auto& v : a_vertexPositions) {
    EBGEOMETRY_EXPECT(std::isfinite(v[0]));
    EBGEOMETRY_EXPECT(std::isfinite(v[1]));
    EBGEOMETRY_EXPECT(std::isfinite(v[2]));
  }

  m_vertexPositions = a_vertexPositions;

  this->computeNormal();
}

template <class T>
void
Triangle<T>::setVertexNormals(const Array<Vec3T<T>, 3>& a_vertexNormals) noexcept
{
  for ([[maybe_unused]] const auto& n : a_vertexNormals) {
    EBGEOMETRY_EXPECT(std::isfinite(n[0]));
    EBGEOMETRY_EXPECT(std::isfinite(n[1]));
    EBGEOMETRY_EXPECT(std::isfinite(n[2]));
    // Unit length, or zero: a mesh gives an edge or vertex on a fold a zero normal (see
    // Soup::findFoldedFeature), which the sign test treats as negative.
    EBGEOMETRY_EXPECT(n.length2() == T(0) || std::abs(n.length() - T(1)) < std::sqrt(Math::Limits<T>::epsilon()));
  }

  m_vertexNormals = a_vertexNormals;
}

template <class T>
void
Triangle<T>::setEdgeNormals(const Array<Vec3T<T>, 3>& a_edgeNormals) noexcept
{
  for ([[maybe_unused]] const auto& n : a_edgeNormals) {
    EBGEOMETRY_EXPECT(std::isfinite(n[0]));
    EBGEOMETRY_EXPECT(std::isfinite(n[1]));
    EBGEOMETRY_EXPECT(std::isfinite(n[2]));
    // Unit length, or zero: a mesh gives an edge or vertex on a fold a zero normal (see
    // Soup::findFoldedFeature), which the sign test treats as negative.
    EBGEOMETRY_EXPECT(n.length2() == T(0) || std::abs(n.length() - T(1)) < std::sqrt(Math::Limits<T>::epsilon()));
  }

  m_edgeNormals = a_edgeNormals;
}

template <class T>
void
Triangle<T>::setFaceId(const uint32_t a_faceId) noexcept
{
  this->m_faceId = a_faceId;
}

template <class T>
void
Triangle<T>::computeNormal() noexcept
{
  const Vec3T<T> x2x0 = m_vertexPositions[2] - m_vertexPositions[0];
  const Vec3T<T> x2x1 = m_vertexPositions[2] - m_vertexPositions[1];

  m_triangleNormal = cross(x2x0, x2x1);

  // Collinear or coincident vertices leave a zero normal rather than NaN. Such a triangle has no
  // interior; mesh extraction never produces one (see MeshDistanceFunctionsDetail::extractTriangles).
  const T length = m_triangleNormal.length();

  if (length > T(0)) {
    m_triangleNormal = m_triangleNormal / length;
  }
}

template <class T>
const Vec3T<T>&
Triangle<T>::getNormal() const noexcept
{
  return this->m_triangleNormal;
}

template <class T>
const Array<Vec3T<T>, 3>&
Triangle<T>::getVertexPositions() const noexcept
{
  return this->m_vertexPositions;
}

template <class T>
const Array<Vec3T<T>, 3>&
Triangle<T>::getVertexNormals() const noexcept
{
  return this->m_vertexNormals;
}

template <class T>
const Array<Vec3T<T>, 3>&
Triangle<T>::getEdgeNormals() const noexcept
{
  return this->m_edgeNormals;
}

template <class T>
uint32_t
Triangle<T>::getFaceId() const noexcept
{
  return this->m_faceId;
}

template <class T>
T
Triangle<T>::signedDistance(const Vec3T<T>& a_point) const noexcept
{
  EBGEOMETRY_EXPECT(std::isfinite(a_point[0]));
  EBGEOMETRY_EXPECT(std::isfinite(a_point[1]));
  EBGEOMETRY_EXPECT(std::isfinite(a_point[2]));

  EBGEOMETRY_EXPECT(dot(m_vertexPositions[1] - m_vertexPositions[0], m_vertexPositions[1] - m_vertexPositions[0]) >
                    T(0));
  EBGEOMETRY_EXPECT(dot(m_vertexPositions[2] - m_vertexPositions[1], m_vertexPositions[2] - m_vertexPositions[1]) >
                    T(0));
  EBGEOMETRY_EXPECT(dot(m_vertexPositions[0] - m_vertexPositions[2], m_vertexPositions[0] - m_vertexPositions[2]) >
                    T(0));

  // The kernel reads the triangle through vertex(), normal(), vertexNormal() and edgeNormal().
  struct Source
  {
    const Triangle& m_triangle;

    static TriangleDistance::Vec3V<T>
    v3(const Vec3& a_v) noexcept
    {
      return {a_v[0], a_v[1], a_v[2]};
    }

    TriangleDistance::Vec3V<T>
    vertex(const size_t a_k) const noexcept
    {
      return v3(m_triangle.m_vertexPositions[a_k]);
    }

    TriangleDistance::Vec3V<T>
    normal() const noexcept
    {
      return v3(m_triangle.m_triangleNormal);
    }

    TriangleDistance::Vec3V<T>
    vertexNormal(const size_t a_k) const noexcept
    {
      return v3(m_triangle.m_vertexNormals[a_k]);
    }

    TriangleDistance::Vec3V<T>
    edgeNormal(const size_t a_k) const noexcept
    {
      return v3(m_triangle.m_edgeNormals[a_k]);
    }
  };

  return TriangleDistance::signedDistance<T>(Source{*this}, Source::v3(a_point));
}
} // namespace EBGeometry

#endif
