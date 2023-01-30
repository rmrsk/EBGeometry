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
template <class BV>
BV
ImplicitFunction<T>::approximateBoundingVolumeOctree(const Vec3T<T>&    a_initialLowCorner,
                                                     const Vec3T<T>&    a_initialHighCorner,
                                                     const unsigned int a_maxTreeDepth,
                                                     const T&           a_safetyFactor) const noexcept
{
  using namespace Octree;

  // TLDR: This routine computes a bounding volume using octree subdivision of the implicit function surface. This code
  //       may look complicated, but we are simply using the EBGeometry::Octree signatures for hierarchically partitioning
  //       the surface using spatial subdivision, and then we later gather the vertices of the leaf nodes/volumes that
  //       intersected the implicit function surface.

  // Some declarative shortcuts.
  // Vec3:     Just a 3D vector with provided precision.
  // Data:     Empty, since the octree nodes do not actually contain any data.
  // MetaData: Meta-data in octree nodes, consisting of the physical corners, the node level, and an intersection flag.
  // Node:     Just a shortcut for the octree node type we're dealing with.
  using Vec3     = Vec3T<T>;
  using MetaData = std::tuple<Vec3, Vec3, unsigned int, bool>;
  using Data     = void;
  using Node     = Node<MetaData>;

  // The node metadata is encoded as a tuple (loCorner, hiCorner, level, contains_intersection). We split the node if it
  // contains an intersection AND if the branch has not reached the maximum permitted depth.
  auto splitNode = [&a_maxTreeDepth](const Node& a_node) -> bool {
    const auto containsIntersection = std::get<3>(a_node.getMeta());
    const auto nodeLevel            = std::get<2>(a_node.getMeta());

    return (nodeLevel >= a_maxTreeDepth) || !containsIntersection;
  };

  // Update meta-information for the leaf node. This updates the two corners of the node, the node level, and
  // computes whether or not this particular node intersects the geometry.
  auto metaBuild = [this, a_safetyFactor](const OctantIndex& a_index, const MetaData& a_parentMeta) -> MetaData {
    if (!(std::get<3>(a_parentMeta))) {
      std::cerr << "ImplicitFunction<T>::computeAABB -- logic bust, parent did not have an intersection\n";
    }

    // The two physical corners of the parent node.
    const Vec3 parentLo = std::get<0>(a_parentMeta);
    const Vec3 parentHi = std::get<1>(a_parentMeta);
    const Vec3 delta    = parentHi - parentLo;

    // Create the meta-data.
    MetaData meta;

    auto& loCorner     = std::get<0>(meta);
    auto& hiCorner     = std::get<1>(meta);
    auto& treeLevel    = std::get<2>(meta);
    auto& intersection = std::get<3>(meta);

    loCorner  = parentLo + Octree::LowCorner<T>[a_index] * delta;
    hiCorner  = parentLo + Octree::HighCorner<T>[a_index] * delta;
    treeLevel = std::get<2>(a_parentMeta) + 1;

    const Vec3 center = 0.5 * (loCorner + hiCorner);
    const Vec3 dx     = 0.5 * (hiCorner - loCorner);

    intersection = std::abs(this->value(center)) <= (1.0 + a_safetyFactor) * dx.length();

    return meta;
  };

  // Data builder for leaf nodes. Our nodes don't contain data so return a null pointer instead.
  auto dataBuild = [](const OctantIndex& a_index, const std::shared_ptr<Data>& a_parentData) -> std::shared_ptr<Data> {
    return nullptr;
  };

  // Initialize the root node and build the octree.
  auto root = std::make_shared<Node>();

  MetaData& metaRoot = root->getMeta();

  const Vec3 initialCenter = 0.5 * (a_initialHighCorner + a_initialLowCorner);
  const Vec3 initialDx     = 0.5 * (a_initialHighCorner - a_initialLowCorner);

  std::get<0>(metaRoot) = a_initialLowCorner;
  std::get<1>(metaRoot) = a_initialHighCorner;
  std::get<2>(metaRoot) = 0;
  std::get<3>(metaRoot) = std::abs(this->value(initialCenter)) <= (1.0 + a_safetyFactor) * initialDx.length();

  root->build(splitNode, metaBuild, dataBuild);

  // Traverse the octree and collect the vertex coordinates of each node that contains an
  // intersection.
  std::vector<Vec3> vertices;

  auto updater = [&vertices](const Node& a_node) -> void {
    if (std::get<3>(a_node.getMeta())) {
      const Vec3 lo = std::get<0>(a_node.getMeta());
      const Vec3 hi = std::get<1>(a_node.getMeta());
      const Vec3 dx = hi - lo;

      for (size_t i = 0; i <= 1; i++) {
        for (size_t j = 0; j <= 1; j++) {
          for (size_t k = 0; k <= 1; k++) {
            vertices.emplace_back(lo + Vec3(i * dx[0], j * dx[1], k * dx[2]));
          }
        }
      }
    }
  };

  auto visiter = [](const Node& a_node) -> bool {
    const MetaData& meta = a_node.getMeta();

    return std::get<3>(meta);
  };

  root->traverse(updater, visiter);

  return BV(vertices);
}

#include "EBGeometry_NamespaceFooter.hpp"

#endif
