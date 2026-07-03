// SPDX-FileCopyrightText: 2022 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
  @file   EBGeometry_DCEL.hpp
  @brief  Namespace documentation
  @author Robert Marskar
*/

#ifndef EBGEOMETRY_DCEL_HPP
#define EBGEOMETRY_DCEL_HPP

#include "EBGeometry_NamespaceHeader.hpp"

/**
  @namespace DCEL
  @brief Namespace containing various double-connected edge list (DCEL)
  functionality.
*/
namespace DCEL {

  /**
    @brief Default meta-data type for the DCEL primitives
  */
  using DefaultMetaData = short;

  /**
    @brief Vertex class
    @tparam T    Floating-point precision type.
    @tparam Meta User-defined metadata type.
  */
  template <class T, class Meta = DefaultMetaData>
  class VertexT;

  /**
    @brief Edge class
    @tparam T    Floating-point precision type.
    @tparam Meta User-defined metadata type.
  */
  template <class T, class Meta = DefaultMetaData>
  class EdgeT;

  /**
    @brief Face class
    @tparam T    Floating-point precision type.
    @tparam Meta User-defined metadata type.
  */
  template <class T, class Meta = DefaultMetaData>
  class FaceT;

  /**
    @brief Mesh class
    @tparam T    Floating-point precision type.
    @tparam Meta User-defined metadata type.
  */
  template <class T, class Meta = DefaultMetaData>
  class MeshT;

  /**
    @brief Edge iterator class
    @tparam T    Floating-point precision type.
    @tparam Meta User-defined metadata type.
  */
  template <class T, class Meta = DefaultMetaData>
  class EdgeIteratorT;

  /**
    @brief Enum for putting some logic into how vertex normal weights are calculated
  */
  enum class VertexNormalWeight
  {
    None,
    Angle
  };

} // namespace DCEL

#include "EBGeometry_NamespaceFooter.hpp"

#endif
