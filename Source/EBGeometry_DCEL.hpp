/* EBGeometry
 * Copyright © 2022 Robert Marskar
 * Please refer to Copyright.txt and LICENSE in the EBGeometry root directory.
 */

/*!
  @file   EBGeometry_DCEL.hpp
  @brief  Namespace documentation
  @author Robert Marskar
*/

#ifndef EBGeometry_DCEL
#define EBGeometry_DCEL

#include "EBGeometry_NamespaceHeader.hpp"

/*!
  @namespace DCEL
  @brief Namespace containing various double-connected edge list (DCEL)
  functionality.
*/
namespace DCEL {

  /*!
    @brief Default meta-data type for the DCEL primitives
  */
  using DefaultMetaData = short;

  /*!
    @brief Vertex class
  */
  template <class T, class Meta = DefaultMetaData>
  class VertexT;

  /*!
    @brief Edge class
  */
  template <class T, class Meta = DefaultMetaData>
  class EdgeT;

  /*!
    @brief Face class
  */
  template <class T, class Meta = DefaultMetaData>
  class FaceT;
  /*!
    @brief Mesh class
  */
  template <class T, class Meta = DefaultMetaData>
  class MeshT;

  /*!
    @brief Edge iterator class
  */
  template <class T, class Meta = DefaultMetaData>
  class EdgeIteratorT;

  /*!
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
