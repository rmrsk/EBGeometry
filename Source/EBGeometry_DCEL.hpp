// SPDX-FileCopyrightText: 2022 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file   EBGeometry_DCEL.hpp
 * @brief  Namespace documentation
 * @author Robert Marskar
 */

#ifndef EBGEOMETRY_DCEL_HPP
#define EBGEOMETRY_DCEL_HPP

namespace EBGeometry {

/**
 * @namespace EBGeometry::DCEL
 * @brief Namespace containing various double-connected edge list (DCEL)
 * functionality.
 * @details DCEL is short for doubly-connected edge list, and is data
 * structure for navigating surface meshes.
 */
namespace DCEL {

/**
 * @brief Default meta-data type for the DCEL primitives.
 */
using DefaultMetaData = short;

/**
 * @brief Vertex class for navigating a DCEL mesh.
 * @tparam T    Floating-point precision type.
 * @tparam Meta User-defined metadata type.
 */
template <class T, class Meta = DefaultMetaData>
class VertexT;

/**
 * @brief Half-edge class for navigating a DCEL mesh.
 * @tparam T    Floating-point precision type.
 * @tparam Meta User-defined metadata type.
 */
template <class T, class Meta = DefaultMetaData>
class EdgeT;

/**
 * @brief Face class for navigating a DCEL mesh.
 * @tparam T    Floating-point precision type.
 * @tparam Meta User-defined metadata type.
 */
template <class T, class Meta = DefaultMetaData>
class FaceT;

/**
 * @brief DCEL mesh class - stores a doubly-connected edge mesh.
 * @tparam T    Floating-point precision type.
 * @tparam Meta User-defined metadata type.
 */
template <class T, class Meta = DefaultMetaData>
class MeshT;

/**
 * @brief Half-edge iterator class for navigating the half edge mesh.
 * @tparam T    Floating-point precision type.
 * @tparam Meta User-defined metadata type.
 */
template <class T, class Meta = DefaultMetaData>
class EdgeIteratorT;

/**
 * @brief Various supported vertex normal vector calculation methods.
 */
enum class VertexNormalWeight
{
  None,  ///< Unweighted average of the normals of the faces incident to the vertex.
  Angle, ///< Angle-weighted pseudonormal (Baerentzen and Aanes, DOI: 10.1109/TVCG.2005.49).
};

/**
 * @brief Supported algorithms for testing whether a point projects to the inside of a polygon face.
 * @details A 3D point projected onto a face's plane lies inside or outside the polygon; the test
 * reduces to 2D by dropping the coordinate of the largest normal component (see FaceT). These are
 * the well-known 2D inside/outside tests.
 */
enum class InsideOutsideAlgorithm
{
  SubtendedAngle, ///< Sums the subtended angle of the point with each polygon edge; the point is
                  ///< inside if the sum is +-2*pi (360 degrees) and outside if it is 0.
  CrossingNumber, ///< Casts a ray from the point along +x and counts how many times it crosses a
                  ///< polygon edge; the point is inside if the crossing count is odd (even-odd rule).
  WindingNumber   ///< Computes the winding number of the polygon boundary around the point; the
                  ///< point is inside if the winding number is non-zero.
};

} // namespace DCEL

} // namespace EBGeometry

#endif
