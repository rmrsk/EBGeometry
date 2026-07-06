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
 * @namespace DCEL
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
 * @brief Vertex class.
 * @tparam T    Floating-point precision type.
 * @tparam Meta User-defined metadata type.
 */
template <class T, class Meta = DefaultMetaData>
class VertexT;

/**
 * @brief Edge class.
 * @tparam T    Floating-point precision type.
 * @tparam Meta User-defined metadata type.
 */
template <class T, class Meta = DefaultMetaData>
class EdgeT;

/**
 * @brief Face class
 * @tparam T    Floating-point precision type.
 * @tparam Meta User-defined metadata type.
 */
template <class T, class Meta = DefaultMetaData>
class FaceT;

/**
 * @brief Mesh class
 * @tparam T    Floating-point precision type.
 * @tparam Meta User-defined metadata type.
 */
template <class T, class Meta = DefaultMetaData>
class MeshT;

/**
 * @brief Edge iterator class
 * @tparam T    Floating-point precision type.
 * @tparam Meta User-defined metadata type.
 */
template <class T, class Meta = DefaultMetaData>
class EdgeIteratorT;

/**
 * @brief Enum for putting some logic into how vertex normal weights are calculated
 */
enum class VertexNormalWeight
{
  None,  ///< Unweighted average of the normals of the faces incident to the vertex.
  Angle, ///< Angle-weighted pseudonormal (Baerentzen and Aanes, DOI: 10.1109/TVCG.2005.49).
};

} // namespace DCEL

} // namespace EBGeometry

#endif
