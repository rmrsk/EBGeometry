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
 * @brief Integer type used for topology cross-references in the index-based DCEL.
 * @details The mesh owns its vertices, half-edges, and faces in flat arrays; every topological
 * link (an edge's origin vertex, pair edge, next edge, and face; a vertex's outgoing edge and
 * incident faces; a face's half-edge) is stored as an index into the corresponding @c MeshT array
 * rather than as a pointer. This keeps the representation flat and trivially copyable -- the same
 * structure is used on the host and (after annotation) on the device.
 */
using DCELIndex = int;

/**
 * @brief Sentinel index denoting "no link" (e.g. the pair edge of a boundary half-edge).
 */
static constexpr DCELIndex InvalidIndex = -1;

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

} // namespace DCEL

} // namespace EBGeometry

#endif
