/* EBGeometry
 * Copyright © 2023 Robert Marskar
 * Please refer to Copyright.txt and LICENSE in the EBGeometry root directory.
 */

/*!
  @file   EBGeometry_ImplicitFunctionImplem.hpp
  @brief  Implementation of EBGeometry_ImplicitFunction.hpp
  @author Robert Marskar
*/

#ifndef EBGeometry_ImplicitFunctionImplem
#define EBGeometry_ImplicitFunctionImplem

// Std includes
#include <tuple>

// Our includes
#include "EBGeometry_Octree.hpp"
#include "EBGeometry_ImplicitFunction.hpp"
#include "EBGeometry_NamespaceHeader.hpp"

template <class T>
T
ImplicitFunction<T>::operator()(const Vec3T<T>& a_point) const noexcept
{
  return this->value(a_point);
}

template <class T>
BoundingVolumes::AABBT<T>
ImplicitFunction<T>::computeAABB(const Vec3T<T>&    a_initialLowCorner,
                                 const Vec3T<T>&    a_initialHighCorner,
                                 const unsigned int a_maxTreeDepth,
                                 const T&           a_safetyFactor) const noexcept
{

  using namespace Octree;

  // Some declarative shortcuts. The octree nodes do not contain data, but they do contain meta-data constiting of the physical corners
  // of the octs, their tree level, and
  using Vec3     = Vec3T<T>;
  using MetaData = std::tuple<Vec3, Vec3, unsigned int, bool>;
  using Data     = void;
  using Node     = Node<MetaData>;

  // The node metadata is encoded as a tuple (loCorner, hiCorner, level, contains_intersection). We split the node if it contains an intersection
  // and it's level is below the max tree depth.
  auto splitNode = [&a_maxTreeDepth](const Node& a_node) -> bool {
    const bool containsIntersection = std::get<3>(a_node.getMeta());
    const bool nodeLevel            = std::get<2>(a_node.getMeta());

    return (nodeLevel < a_maxTreeDepth) && containsIntersection;
  };

  auto metaBuild = [](const OctantIndex& a_index, const MetaData& a_parentMeta) -> MetaData {
    MetaData meta;

    const Vec3 parentLo = std::get<0>(a_parentMeta);
    const Vec3 parentHi = std::get<1>(a_parentMeta);

    //    bool hasIntersection =

    return meta;
  };

  auto dataBuild = [](const OctantIndex& a_index, const std::shared_ptr<Data>& a_parentData) -> std::shared_ptr<Data> {
    return nullptr;
  };

  return BoundingVolumes::AABBT<T>(a_initialLowCorner, a_initialHighCorner);

  // Recall errors: 1) not an sdf. 2) bad settings. 3) initial box too small.
}

#include "EBGeometry_NamespaceFooter.hpp"

#endif
