// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
// SPDX-License-Identifier: GPL-3.0-or-later

// Test suite for EBGeometry_Octree.hpp (the generic Octree::Node tree) and
// ImplicitFunction::approximateBoundingVolumeOctree, which is built on top of it. Neither had any
// Tests/ coverage before: approximateBoundingVolumeOctree was only exercised by the
// OctreeBoundingVolume example, which prints a bounding box but never asserts it's correct.

#include "EBGeometry.hpp"
#include "TestFloatingPointUtils.hpp"

#include <catch2/catch_template_test_macros.hpp>
#include <catch2/catch_test_macros.hpp>

using namespace EBGeometry;

namespace {

// A minimal octree of plain unsigned-int levels (no physical extent tracked -- unlike
// approximateBoundingVolumeOctree's tuple metadata), used to test Octree::Node's build/traverse
// machinery in isolation.
using LevelNode = Octree::Node<unsigned int>;

std::shared_ptr<LevelNode>
buildUniformTree(const unsigned int a_maxDepth)
{
  auto root = std::make_shared<LevelNode>();

  root->getMetaData() = 0;

  const LevelNode::SplitFunction splitAlways = [a_maxDepth](const LevelNode& a_node) -> bool {
    return a_node.getMetaData() < a_maxDepth;
  };
  const LevelNode::MetaConstructor incrementLevel =
    [](const Octree::OctantIndex&, const unsigned int& a_parentMeta) -> unsigned int { return a_parentMeta + 1; };
  const LevelNode::DataConstructor noData =
    [](const Octree::OctantIndex&, const std::shared_ptr<void>&) -> std::shared_ptr<void> { return nullptr; };

  root->buildBreadthFirst(splitAlways, incrementLevel, noData);

  return root;
}

size_t
countLeaves(const std::shared_ptr<LevelNode>& a_root)
{
  size_t count = 0;

  const LevelNode::LeafEvaluator  countingUpdater = [&count](const LevelNode&) -> void { count++; };
  const LevelNode::PrunePredicate visitAll        = [](const LevelNode&) -> bool { return true; };

  a_root->traverse(countingUpdater, visitAll);

  return count;
}

} // namespace

// ─────────────────────────────────────────────────────────────────────────────
// Octree::Node
// ─────────────────────────────────────────────────────────────────────────────

TEST_CASE("Octree::Node: default-constructed node is a leaf with null children", "[Octree]")
{
  // Node() default-constructs m_meta, which for a plain (non-class) Meta type like `unsigned
  // int` leaves it indeterminate rather than zero-initialized -- give it a value before reading
  // anything back from it, so this test only exercises isLeaf()/getChildren(), not m_meta.
  LevelNode node;
  node.getMetaData() = 0;

  REQUIRE(node.isLeaf());
  for (const auto& child : node.getChildren()) {
    REQUIRE(child == nullptr);
  }
}

TEST_CASE("Octree::Node: buildBreadthFirst produces exactly 8^depth leaves for a uniform split", "[Octree]")
{
  for (unsigned int depth = 0; depth <= 3; depth++) {
    const auto   root           = buildUniformTree(depth);
    const size_t expectedLeaves = static_cast<size_t>(std::pow(8, depth));

    REQUIRE(countLeaves(root) == expectedLeaves);
  }
}

TEST_CASE("Octree::Node: a tree built to depth 0 never splits and remains a single leaf", "[Octree]")
{
  const auto root = buildUniformTree(0);

  REQUIRE(root->isLeaf());
  REQUIRE(root->getMetaData() == 0U);
}

TEST_CASE("Octree::Node: meta-data level increases by exactly one per generation", "[Octree]")
{
  const auto root = buildUniformTree(2);

  REQUIRE_FALSE(root->isLeaf());
  REQUIRE(root->getMetaData() == 0U);

  for (const auto& child : root->getChildren()) {
    REQUIRE(child != nullptr);
    REQUIRE(child->getMetaData() == 1U);
    REQUIRE_FALSE(child->isLeaf());

    for (const auto& grandchild : child->getChildren()) {
      REQUIRE(grandchild != nullptr);
      REQUIRE(grandchild->getMetaData() == 2U);
      REQUIRE(grandchild->isLeaf());
    }
  }
}

TEST_CASE("Octree::Node: buildDepthFirst and buildBreadthFirst produce trees with the same leaf count", "[Octree]")
{
  constexpr unsigned int depth = 3;

  auto depthFirstRoot           = std::make_shared<LevelNode>();
  depthFirstRoot->getMetaData() = 0;

  const LevelNode::SplitFunction splitAlways = [](const LevelNode& a_node) -> bool {
    return a_node.getMetaData() < depth;
  };
  const LevelNode::MetaConstructor incrementLevel =
    [](const Octree::OctantIndex&, const unsigned int& a_parentMeta) -> unsigned int { return a_parentMeta + 1; };
  const LevelNode::DataConstructor noData =
    [](const Octree::OctantIndex&, const std::shared_ptr<void>&) -> std::shared_ptr<void> { return nullptr; };

  depthFirstRoot->buildDepthFirst(splitAlways, incrementLevel, noData);

  const auto breadthFirstRoot = buildUniformTree(depth);

  REQUIRE(countLeaves(depthFirstRoot) == countLeaves(breadthFirstRoot));
  REQUIRE(countLeaves(depthFirstRoot) == static_cast<size_t>(std::pow(8, depth)));
}

TEST_CASE("Octree::Node::traverse: a prunePredicate that prunes a subtree stops that subtree's leaves "
          "from being visited",
          "[Octree]")
{
  const auto root = buildUniformTree(2);

  size_t visitedCount = 0;

  const LevelNode::LeafEvaluator counting = [&visitedCount](const LevelNode&) -> void { visitedCount++; };

  // Only descend into the root and its first child (octant 0); every other direct child of the
  // root is pruned outright, so only the first child's 8 grandchildren should ever be visited.
  const LevelNode::PrunePredicate pruneAllButFirstChild = [&root](const LevelNode& a_node) -> bool {
    if (&a_node == root.get() || &a_node == root->getChildren()[0].get()) {
      return true;
    }
    for (size_t i = 1; i < 8; i++) {
      if (&a_node == root->getChildren()[i].get()) {
        return false;
      }
    }
    // Anything below the first child (its own children) is always visited.
    return true;
  };

  root->traverse(counting, pruneAllButFirstChild);

  // Only the first child's 8 grandchildren (depth-2 leaves) should have been visited.
  REQUIRE(visitedCount == 8);
}

TEST_CASE("Octree::Node::traverse: a custom childOrderer changes the visitation order", "[Octree]")
{
  const auto root = buildUniformTree(1);

  std::vector<const void*> defaultOrder;
  std::vector<const void*> reversedOrder;

  const LevelNode::LeafEvaluator recordDefault = [&defaultOrder](const LevelNode& a_node) -> void {
    defaultOrder.push_back(static_cast<const void*>(&a_node));
  };
  const LevelNode::LeafEvaluator recordReversed = [&reversedOrder](const LevelNode& a_node) -> void {
    reversedOrder.push_back(static_cast<const void*>(&a_node));
  };
  const LevelNode::PrunePredicate visitAll = [](const LevelNode&) -> bool { return true; };
  const LevelNode::ChildOrderer   reverseChildren =
    [](std::array<std::shared_ptr<const LevelNode>, 8>& a_children) -> void {
    std::reverse(a_children.begin(), a_children.end());
  };

  root->traverse(recordDefault, visitAll);
  root->traverse(recordReversed, visitAll, reverseChildren);

  REQUIRE(defaultOrder.size() == 8);
  REQUIRE(reversedOrder.size() == 8);

  std::vector<const void*> manuallyReversed(defaultOrder.rbegin(), defaultOrder.rend());
  REQUIRE(reversedOrder == manuallyReversed);
}

// ─────────────────────────────────────────────────────────────────────────────
// ImplicitFunction::approximateBoundingVolumeOctree
// ─────────────────────────────────────────────────────────────────────────────

TEMPLATE_TEST_CASE("approximateBoundingVolumeOctree: encloses a sphere within a margin that "
                   "shrinks as depth increases",
                   "[Octree][ImplicitFunction]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T      = TestType;
  using Vec3   = Vec3T<T>;
  using Sphere = SphereSDF<T>;
  using BV     = BoundingVolumes::AABBT<T>;

  const T slop = T(1.0e4) * std::numeric_limits<T>::epsilon();

  const Sphere sphere(Vec3::zeros(), T(1));

  const Vec3 initLo = -2 * Vec3::ones();
  const Vec3 initHi = +2 * Vec3::ones();

  T previousMargin = std::numeric_limits<T>::max();

  for (const unsigned int depth : {2U, 4U, 6U, 8U}) {
    const auto bv = sphere.template approximateBoundingVolumeOctree<BV>(initLo, initHi, depth, T(0.0));

    // Must actually enclose the true bounding box [-1, 1]^3 (never smaller than the real shape).
    for (int i = 0; i < 3; i++) {
      REQUIRE(bv.getLowCorner()[i] <= T(-1.0) + slop);
      REQUIRE(bv.getHighCorner()[i] >= T(1.0) - slop);
    }

    // The margin beyond the true bounds should shrink (or stay the same) as depth increases.
    const T margin = bv.getHighCorner()[0] - T(1.0);
    REQUIRE(margin <= previousMargin + slop);
    previousMargin = margin;
  }

  // At depth 8, the approximation should be tight to well within a tenth of the sphere's radius.
  const auto tightBV = sphere.template approximateBoundingVolumeOctree<BV>(initLo, initHi, 8U, T(0.0));
  REQUIRE(tightBV.getHighCorner()[0] < T(1.1));
}

TEMPLATE_TEST_CASE("approximateBoundingVolumeOctree: an inverted initial box (lo >= hi) falls "
                   "back to the maximal bounding volume",
                   "[Octree][ImplicitFunction]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T      = TestType;
  using Vec3   = Vec3T<T>;
  using Sphere = SphereSDF<T>;
  using BV     = BoundingVolumes::AABBT<T>;

  const Sphere sphere(Vec3::zeros(), T(1));

  const auto bv = sphere.template approximateBoundingVolumeOctree<BV>(Vec3::ones(), Vec3::zeros(), 4U, T(0.0));

  // The fallback uses -Vec3::max()/+Vec3::max(), so the result is far larger than any reasonable
  // search box.
  REQUIRE(bv.getLowCorner()[0] < T(-1.0e10));
  REQUIRE(bv.getHighCorner()[0] > T(1.0e10));
}

TEMPLATE_TEST_CASE("approximateBoundingVolumeOctree: an initial box that never touches the "
                   "surface falls back to the maximal bounding volume",
                   "[Octree][ImplicitFunction]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T      = TestType;
  using Vec3   = Vec3T<T>;
  using Sphere = SphereSDF<T>;
  using BV     = BoundingVolumes::AABBT<T>;

  const Sphere sphere(Vec3::zeros(), T(1));

  // Centered far away from the unit sphere, and small enough that it can't possibly reach it.
  const Vec3 farLo(100, 100, 100);
  const Vec3 farHi(101, 101, 101);

  const auto bv = sphere.template approximateBoundingVolumeOctree<BV>(farLo, farHi, 4U, T(0.0));

  REQUIRE(bv.getLowCorner()[0] < T(-1.0e10));
  REQUIRE(bv.getHighCorner()[0] > T(1.0e10));
}

TEMPLATE_TEST_CASE("approximateBoundingVolumeOctree: a larger safety factor never shrinks the "
                   "sphere out of the result",
                   "[Octree][ImplicitFunction]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T      = TestType;
  using Vec3   = Vec3T<T>;
  using Sphere = SphereSDF<T>;
  using BV     = BoundingVolumes::AABBT<T>;

  const T slop = T(1.0e4) * std::numeric_limits<T>::epsilon();

  const Sphere sphere(Vec3::zeros(), T(1));

  const Vec3 initLo = -2 * Vec3::ones();
  const Vec3 initHi = +2 * Vec3::ones();

  for (const T safety : {T(0.0), T(0.25), T(1.0)}) {
    const auto bv = sphere.template approximateBoundingVolumeOctree<BV>(initLo, initHi, 6U, safety);

    for (int i = 0; i < 3; i++) {
      REQUIRE(bv.getLowCorner()[i] <= T(-1.0) + slop);
      REQUIRE(bv.getHighCorner()[i] >= T(1.0) - slop);
    }
  }
}
