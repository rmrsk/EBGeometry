// SPDX-FileCopyrightText: 2023 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file   EBGeometry_ImplicitFunctionImplem.hpp
 * @brief  Implementation of EBGeometry_ImplicitFunction.hpp
 * @author Robert Marskar
 */

#ifndef EBGEOMETRY_IMPLICITFUNCTIONIMPLEM_HPP
#define EBGEOMETRY_IMPLICITFUNCTIONIMPLEM_HPP

// Std includes
#include <cmath>
#include <cstddef>
#include <iostream>
#include <memory>
#include <string>
#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>

// Our includes
#include "EBGeometry_ImplicitFunction.hpp"
#include "EBGeometry_Macros.hpp"
#include "EBGeometry_Octree.hpp"
#include "EBGeometry_Vec.hpp"

namespace EBGeometry {

template <class T>
T
ImplicitFunction<T>::operator()(const Vec3T<T>& a_point) const noexcept
{
  EBGEOMETRY_EXPECT(std::isfinite(a_point[0]));
  EBGEOMETRY_EXPECT(std::isfinite(a_point[1]));
  EBGEOMETRY_EXPECT(std::isfinite(a_point[2]));

  return this->value(a_point);
}

template <class T>
template <class BV>
BV
ImplicitFunction<T>::approximateBoundingVolumeOctree(const Vec3T<T>&    a_initialLowCorner,
                                                     const Vec3T<T>&    a_initialHighCorner,
                                                     const unsigned int a_maxTreeDepth,
                                                     const T&           a_safetyFactor) const
{
  // Qualified, so that lookup finds the free function rather than this member.
  return EBGeometry::approximateBoundingVolumeOctree<BV>(
    *this, a_initialLowCorner, a_initialHighCorner, a_maxTreeDepth, a_safetyFactor);
}

namespace ImplicitFunctionDetail {

/**
 * @brief Detects whether F has a signedDistance(const Vec3T<T>&) member.
 */
template <class F, class T, class = void>
struct HasSignedDistance : std::false_type
{
};

/**
 * @brief Detects whether F has a signedDistance(const Vec3T<T>&) member (positive case).
 */
template <class F, class T>
struct HasSignedDistance<
  F,
  T,
  std::void_t<decltype(std::declval<const F&>().signedDistance(std::declval<const Vec3T<T>&>()))>> : std::true_type
{
};

/**
 * @brief Detects whether F has a value(const Vec3T<T>&) member.
 */
template <class F, class T, class = void>
struct HasValue : std::false_type
{
};

/**
 * @brief Detects whether F has a value(const Vec3T<T>&) member (positive case).
 */
template <class F, class T>
struct HasValue<F, T, std::void_t<decltype(std::declval<const F&>().value(std::declval<const Vec3T<T>&>()))>>
  : std::true_type
{
};

/**
 * @brief Evaluate a_function at a_point through signedDistance(), value() or operator(), in that order
 * of preference.
 * @tparam T Floating-point precision.
 * @tparam F Function type.
 * @param[in] a_function Function to evaluate.
 * @param[in] a_point    Evaluation point.
 * @return Function value at a_point.
 */
template <class T, class F>
T
evaluate(const F& a_function, const Vec3T<T>& a_point) noexcept
{
  if constexpr (HasSignedDistance<F, T>::value) {
    return static_cast<T>(a_function.signedDistance(a_point));
  }
  else if constexpr (HasValue<F, T>::value) {
    return static_cast<T>(a_function.value(a_point));
  }
  else {
    return static_cast<T>(a_function(a_point));
  }
}

} // namespace ImplicitFunctionDetail

template <class BV, class F, class T>
BV
approximateBoundingVolumeOctree(const F&           a_function,
                                const Vec3T<T>&    a_initialLowCorner,
                                const Vec3T<T>&    a_initialHighCorner,
                                const unsigned int a_maxTreeDepth,
                                const T&           a_safety)
{
  EBGEOMETRY_EXPECT(std::isfinite(a_initialLowCorner[0]));
  EBGEOMETRY_EXPECT(std::isfinite(a_initialLowCorner[1]));
  EBGEOMETRY_EXPECT(std::isfinite(a_initialLowCorner[2]));
  EBGEOMETRY_EXPECT(std::isfinite(a_initialHighCorner[0]));
  EBGEOMETRY_EXPECT(std::isfinite(a_initialHighCorner[1]));
  EBGEOMETRY_EXPECT(std::isfinite(a_initialHighCorner[2]));
  EBGEOMETRY_EXPECT(a_safety >= T(0));

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
    const auto containsIntersection = std::get<3>(a_node.getMetaData());
    const auto nodeLevel            = std::get<2>(a_node.getMetaData());

    return containsIntersection && (nodeLevel < a_maxTreeDepth);
  };

  // Update meta-information for the leaf node. This updates the two corners of the node, the node level, and
  // computes whether or not this particular node intersects the geometry.
  auto metaBuild = [&a_function, a_safety](const OctantIndex& a_index, const MetaData& a_parentMeta) -> MetaData {
    if (!(std::get<3>(a_parentMeta))) {
      std::cerr << "approximateBoundingVolumeOctree -- logic bust, parent did not have an intersection\n";
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

    intersection =
      std::abs(ImplicitFunctionDetail::evaluate<T>(a_function, center)) <= (T(1.0) + a_safety) * dx.length();

    return meta;
  };

  // Data builder for leaf nodes. Our nodes don't contain data so return a null pointer instead.
  auto dataBuild = [](const OctantIndex&, const std::shared_ptr<Data>&) -> std::shared_ptr<Data> { return nullptr; };

  // Initialize the root node and build the octree.
  auto root = std::make_shared<Node>();

  MetaData& metaRoot = root->getMetaData();

  const Vec3 initialCenter = 0.5 * (a_initialHighCorner + a_initialLowCorner);
  const Vec3 initialDx     = 0.5 * (a_initialHighCorner - a_initialLowCorner);

  std::get<0>(metaRoot) = a_initialLowCorner;
  std::get<1>(metaRoot) = a_initialHighCorner;
  std::get<2>(metaRoot) = 0;
  std::get<3>(metaRoot) = std::abs(ImplicitFunctionDetail::evaluate<T>(a_function, initialCenter)) <=
                          (T(1.0) + a_safety) * initialDx.length();

  std::vector<Vec3> vertices;

  // Handle potential errors.
  const std::string baseError = "approximateBoundingVolumeOctree error: ";
  // Invalid if the box is empty or inverted along any axis, not only along all three.
  if (!(a_initialLowCorner[0] < a_initialHighCorner[0] && a_initialLowCorner[1] < a_initialHighCorner[1] &&
        a_initialLowCorner[2] < a_initialHighCorner[2])) {
    std::cerr << baseError + "'a_initialLowCorner' must be below 'a_initialHighCorner' in every component\n";

    vertices.emplace_back(-Vec3::max());
    vertices.emplace_back(+Vec3::max());
  }
  else if (!(std::get<3>(metaRoot))) {
    std::cerr << baseError + "'input volume does not contain surface.'\n";

    vertices.emplace_back(-Vec3::max());
    vertices.emplace_back(+Vec3::max());
  }
  else {
    root->buildBreadthFirst(splitNode, metaBuild, dataBuild);

    // Traverse the octree and collect the vertex coordinates of each node that contains an
    // intersection.
    auto leafEvaluator = [&vertices](const Node& a_node) -> void {
      if (std::get<3>(a_node.getMetaData())) {
        const Vec3 lo = std::get<0>(a_node.getMetaData());
        const Vec3 hi = std::get<1>(a_node.getMetaData());
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

    auto prunePredicate = [](const Node& a_node) -> bool {
      const MetaData& meta = a_node.getMetaData();

      return std::get<3>(meta);
    };

    root->traverse(leafEvaluator, prunePredicate);
  }

  return BV(vertices);
}

} // namespace EBGeometry

#endif
