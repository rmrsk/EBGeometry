// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: BSD-3-Clause

/**
 * @file   EBGeometry_ConnectedComponentsImplem.hpp
 * @brief  Implementation of EBGeometry_ConnectedComponents.hpp
 * @author Robert Marskar
 */

#ifndef EBGEOMETRY_CONNECTEDCOMPONENTSIMPLEM_HPP
#define EBGEOMETRY_CONNECTEDCOMPONENTSIMPLEM_HPP

// Std includes
#include <algorithm>
#include <cmath>
#include <numeric>
#include <unordered_map>
#include <utility>

// Our includes
#include "EBGeometry_ConnectedComponents.hpp"
#include "EBGeometry_Math.hpp"

namespace EBGeometry {

namespace ComponentsDetail {

/**
 * @brief The most cells along an axis: the cell coordinates of every level are packed into a 64-bit key.
 */
inline constexpr uint32_t MaxCellsPerAxis = 1U << 19U;

/**
 * @brief A leaf of the octree: a cell of 2^level cells of size h along each axis, wholly in one region (certified) or
 * of size h and in the region of its center.
 * @details Internal helper; not part of the public API.
 * @tparam T Floating-point precision.
 */
template <class T>
struct Leaf
{
  /**
   * @brief Its low corner, in cells of size h.
   */
  uint32_t m_lo[3];

  /**
   * @brief Its level: its side is 2^level cells of size h.
   */
  uint32_t m_level;

  /**
   * @brief Its region.
   */
  uint32_t m_region;

  /**
   * @brief Half the gap between the second smallest region function at its center and the smallest (its region's):
   * a lower bound on how far the center is from its region's surface.
   */
  T m_clearance;
};

/**
 * @brief The octree of a search: its grid, its leaves, and every cell by its key.
 * @details Internal helper; not part of the public API. A cell (level, a, b, c) covers the cells of size h from
 * (a, b, c) 2^level to (a + 1, b + 1, c + 1) 2^level, clipped to the box's m_n cells; cells wholly beyond the box are
 * not kept.
 * @tparam T Floating-point precision.
 */
template <class T>
struct Cells
{
  /**
   * @brief What m_cells holds for a cell that was refined.
   */
  static constexpr uint32_t Internal = 0xFFFFFFFFU;

  /**
   * @brief The box searched, grown to a whole number of cells.
   */
  BoundingVolumes::AABBT<T> m_box;

  /**
   * @brief h.
   */
  T m_h = T(0);

  /**
   * @brief How many cells of size h the box is long along each axis.
   */
  uint32_t m_n[3] = {1U, 1U, 1U};

  /**
   * @brief The root's level.
   */
  uint32_t m_rootLevel = 0U;

  /**
   * @brief The number of regions.
   */
  uint32_t m_numRegions = 0U;

  /**
   * @brief The leaves.
   */
  std::vector<Leaf<T>> m_leaves;

  /**
   * @brief Every cell kept, by key: its leaf, or Internal.
   */
  std::unordered_map<uint64_t, uint32_t> m_cells;

  /**
   * @brief A cell's key.
   * @param[in] a_level The level.
   * @param[in] a_a     Its index along x, in cells of its level.
   * @param[in] a_b     Along y.
   * @param[in] a_c     Along z.
   * @return The key.
   */
  static uint64_t
  key(const uint32_t a_level, const uint64_t a_a, const uint64_t a_b, const uint64_t a_c) noexcept
  {
    return (uint64_t(a_level) << 57U) | (a_a << 38U) | (a_b << 19U) | a_c;
  }

  /**
   * @brief A cell, if kept.
   * @param[in] a_level The level.
   * @param[in] a_index Its indices, in cells of its level.
   * @return Its leaf, Internal, or nullptr when not kept.
   */
  const uint32_t*
  find(const uint32_t a_level, const int64_t a_index[3]) const noexcept
  {
    for (int d = 0; d < 3; d++) {
      if (a_index[d] < 0 || (uint64_t(a_index[d]) << a_level) >= m_n[d]) {
        return nullptr;
      }
    }

    const auto at = m_cells.find(key(a_level, uint64_t(a_index[0]), uint64_t(a_index[1]), uint64_t(a_index[2])));

    return (at == m_cells.end()) ? nullptr : &at->second;
  }

  /**
   * @brief Visit every pair of leaves sharing a face, each pair once, the smaller leaf first when they differ.
   * @tparam F A callable taking (uint32_t leaf, uint32_t neighbor).
   * @param[in] a_visit What to do with each pair.
   */
  template <class F>
  void
  forEachFaceNeighbor(F&& a_visit) const
  {
    std::vector<int64_t> stack;

    for (uint32_t l = 0; l < m_leaves.size(); l++) {
      const Leaf<T>& leaf = m_leaves[l];

      for (int d = 0; d < 3; d++) {
        for (int side = 0; side < 2; side++) {
          int64_t index[3];

          for (int e = 0; e < 3; e++) {
            index[e] = int64_t(leaf.m_lo[e] >> leaf.m_level);
          }

          index[d] += (side == 0) ? -1 : 1;

          const uint32_t* other = this->find(leaf.m_level, index);

          // A neighbor not kept at this level is beyond the box, or inside a larger leaf, which finds this one.
          if (other == nullptr) {
            continue;
          }

          if (*other != Internal) {
            // A leaf of the same size: visited once, from the lower side.
            if (side == 1) {
              a_visit(l, *other);
            }

            continue;
          }

          // Smaller leaves: the children of the neighbor on the face it shares with this leaf, down to the leaves.
          stack.clear();
          stack.insert(stack.end(), {int64_t(leaf.m_level), index[0], index[1], index[2]});

          while (!stack.empty()) {
            int64_t cell[4];

            for (int k = 3; k >= 0; k--) {
              cell[k] = stack.back();
              stack.pop_back();
            }

            const uint32_t level = uint32_t(cell[0]) - 1U;

            for (int64_t i = 0; i < 2; i++) {
              for (int64_t j = 0; j < 2; j++) {
                int64_t child[3] = {2 * cell[1], 2 * cell[2], 2 * cell[3]};

                // The child's offset along d is the face's side; the two other axes run over both.
                child[d] += (side == 0) ? 1 : 0;
                child[(d + 1) % 3] += i;
                child[(d + 2) % 3] += j;

                const uint32_t* found = this->find(level, child);

                if (found == nullptr) {
                  continue;
                }

                if (*found == Internal) {
                  stack.insert(stack.end(), {int64_t(level), child[0], child[1], child[2]});
                }
                else {
                  a_visit(*found, l);
                }
              }
            }
          }
        }
      }
    }
  }

  /**
   * @brief A leaf's box in cells of size h, clipped to the box searched.
   * @param[in]  a_leaf The leaf.
   * @param[out] a_lo   Its low corner.
   * @param[out] a_hi   Its high corner.
   */
  void
  clipped(const Leaf<T>& a_leaf, uint32_t a_lo[3], uint32_t a_hi[3]) const noexcept
  {
    for (int d = 0; d < 3; d++) {
      a_lo[d] = a_leaf.m_lo[d];
      a_hi[d] = uint32_t(Math::min(uint64_t(a_leaf.m_lo[d]) + (uint64_t(1) << a_leaf.m_level), uint64_t(m_n[d])));
    }
  }

  /**
   * @brief A point in cells of size h.
   * @param[in] a_cells Its coordinates, in cells.
   * @return The point.
   */
  Vec3T<T>
  point(const T a_cells[3]) const noexcept
  {
    const Vec3T<T>& lo = m_box.getLowCorner();

    return Vec3T<T>(lo[0] + a_cells[0] * m_h, lo[1] + a_cells[1] * m_h, lo[2] + a_cells[2] * m_h);
  }
};

/**
 * @brief Stores each region's value at a point (the visitor of Tape::regionDistances()).
 * @details Internal helper; not part of the public API. A functor rather than a lambda: regionDistances() is
 * host-device, and a CUDA compile warns of a host lambda called from it.
 * @tparam T Floating-point precision.
 */
template <class T>
struct StoreValues
{
  /**
   * @brief One value per region.
   */
  T* m_values;

  /**
   * @brief Store a region's value.
   * @param[in] a_region The region.
   * @param[in] a_value  Its value.
   */
  EBGEOMETRY_HOST_DEVICE
  void
  operator()(const uint32_t a_region, const T a_value) const noexcept
  {
    m_values[a_region] = a_value;
  }
};

/**
 * @brief The grown box of a search.
 * @details Internal helper; not part of the public API.
 * @tparam T              Floating-point precision.
 * @tparam UserPrimitives The tape's user primitives and features.
 * @param[in]  a_tape    The tape.
 * @param[in]  a_options The options.
 * @param[out] a_cells   The cells, whose grid is set.
 */
template <class T, class... UserPrimitives>
void
setGrid(const Tape<T, UserPrimitives...>& a_tape, const ComponentOptions<T>& a_options, Cells<T>& a_cells)
{
  const T h = a_options.m_resolution;

  EBGEOMETRY_REQUIRE(h > T(0), "connectedComponents: the resolution must be positive, it is %g", double(h));

  EBGEOMETRY_REQUIRE(a_tape.getDistanceQuality() != DistanceQuality::NotADistance,
                     "connectedComponents: the tape's value is not a distance, so no cell can be shown to lie in one "
                     "region");

  const bool given = a_options.m_box.getLowCorner()[0] <= a_options.m_box.getHighCorner()[0];

  Vec3T<T> lo = given ? a_options.m_box.getLowCorner() : a_tape.computeBoundingVolume().getLowCorner();
  Vec3T<T> hi = given ? a_options.m_box.getHighCorner() : a_tape.computeBoundingVolume().getHighCorner();

  for (int d = 0; d < 3; d++) {
    // An unbounded object's box reaches the largest values (Vec3T::max()), or infinity.
    const T far = T(0.5) * Math::Limits<T>::max();

    EBGEOMETRY_REQUIRE(std::abs(lo[d]) < far && std::abs(hi[d]) < far && lo[d] <= hi[d],
                       "connectedComponents: the box searched must be bounded and not empty (%s); an unbounded "
                       "object needs a box (ComponentOptions::m_box)",
                       given ? "the box given" : "the tape's bounding box");
  }

  if (!given) {
    for (int d = 0; d < 3; d++) {
      lo[d] -= T(2) * h;
      hi[d] += T(2) * h;
    }
  }

  uint32_t longest = 1U;

  for (int d = 0; d < 3; d++) {
    const double cells = std::ceil(double(hi[d] - lo[d]) / double(h));

    EBGEOMETRY_REQUIRE(cells <= double(MaxCellsPerAxis),
                       "connectedComponents: the box is %g cells of size h long, more than %u",
                       cells,
                       unsigned(MaxCellsPerAxis));

    a_cells.m_n[d] = Math::max(1U, uint32_t(cells));
    hi[d]          = lo[d] + T(a_cells.m_n[d]) * h;
    longest        = Math::max(longest, a_cells.m_n[d]);
  }

  a_cells.m_box        = BoundingVolumes::AABBT<T>(lo, hi);
  a_cells.m_h          = h;
  a_cells.m_numRegions = a_tape.getNumRegions();
  a_cells.m_rootLevel  = 0U;

  while ((1U << a_cells.m_rootLevel) < longest) {
    a_cells.m_rootLevel++;
  }
}

/**
 * @brief Search a tape's regions: the octree, refined where no region holds a cell whole.
 * @details Internal helper; not part of the public API. A cell's clearance is half the gap between the second
 * smallest region function at its center and the smallest, f_r of its region r: the negated function of r as a
 * subdomain alone (Tape::signedDistance(p, subdomains, s)). It is at most the distance d from the center to r's
 * surface, needing only that each region function is a distance bound (|f| at most the distance to that region's
 * surface, which every quality but NotADistance gives), not that it is Lipschitz: f_r >= -d, and the region across
 * r's surface at its nearest point is at distance d, so its function, and the second smallest, is at most d. A cell
 * whose clearance is at least its half-diagonal therefore lies wholly in r.
 * @tparam T              Floating-point precision.
 * @tparam UserPrimitives The tape's user primitives and features.
 * @param[in] a_tape    The tape.
 * @param[in] a_options The options.
 * @param[in] a_abort   What happens when the search reaches ComponentOptions::m_maxCells cells: abort with a message
 *                      (in every build), or return nullptr.
 * @return The cells, or nullptr.
 */
template <class T, class... UserPrimitives>
std::shared_ptr<const Cells<T>>
search(const Tape<T, UserPrimitives...>& a_tape, const ComponentOptions<T>& a_options, const bool a_abort = true)
{
  auto cells = std::make_shared<Cells<T>>();

  setGrid(a_tape, a_options, *cells);

  const T sqrt3 = std::sqrt(T(3));

  std::vector<T> values(cells->m_numRegions);

  // Cells to look at: level, then the indices along x, y and z in cells of that level.
  std::vector<uint32_t> stack = {cells->m_rootLevel, 0U, 0U, 0U};

  while (!stack.empty()) {
    const uint32_t c     = stack.back();
    const uint32_t b     = stack[stack.size() - 2];
    const uint32_t a     = stack[stack.size() - 3];
    const uint32_t level = stack[stack.size() - 4];

    stack.resize(stack.size() - 4);

    // The cap on the cells kept, which bounds the memory the search takes.
    if (uint64_t(cells->m_cells.size()) >= a_options.m_maxCells) {
      EBGEOMETRY_REQUIRE(!a_abort,
                         "connectedComponents: the search at resolution %g reached its cap of %llu cells "
                         "(ComponentOptions::m_maxCells) before it finished: give a coarser resolution, a smaller box, "
                         "or a larger cap",
                         double(cells->m_h),
                         static_cast<unsigned long long>(a_options.m_maxCells));

      return nullptr;
    }

    const uint32_t lo[3]  = {a << level, b << level, c << level};
    const T        side   = T(1U << level);
    const T        mid[3] = {T(lo[0]) + side / T(2), T(lo[1]) + side / T(2), T(lo[2]) + side / T(2)};

    std::fill(values.begin(), values.end(), Math::Limits<T>::max());

    a_tape.regionDistances(cells->point(mid), StoreValues<T>{values.data()});

    T        first  = Math::Limits<T>::max();
    T        second = Math::Limits<T>::max();
    uint32_t region = 0U;

    for (uint32_t r = 0; r < cells->m_numRegions; r++) {
      if (values[r] < first) {
        second = first;
        first  = values[r];
        region = r;
      }
      else {
        second = Math::min(second, values[r]);
      }
    }

    const T        clearance = T(0.5) * (second - first);
    const T        reach     = T(0.5) * sqrt3 * side * cells->m_h;
    const uint64_t key       = Cells<T>::key(level, a, b, c);

    if (level == 0U || clearance >= reach) {
      cells->m_cells.emplace(key, uint32_t(cells->m_leaves.size()));
      cells->m_leaves.push_back(Leaf<T>{{lo[0], lo[1], lo[2]}, level, region, clearance});

      continue;
    }

    cells->m_cells.emplace(key, Cells<T>::Internal);

    for (uint32_t child = 0; child < 8U; child++) {
      const uint32_t ca = 2U * a + (child & 1U);
      const uint32_t cb = 2U * b + ((child >> 1U) & 1U);
      const uint32_t cc = 2U * c + ((child >> 2U) & 1U);

      // Children wholly beyond the box are not kept.
      if ((ca << (level - 1U)) < cells->m_n[0] && (cb << (level - 1U)) < cells->m_n[1] &&
          (cc << (level - 1U)) < cells->m_n[2]) {
        stack.insert(stack.end(), {level - 1U, ca, cb, cc});
      }
    }
  }

  return cells;
}

/**
 * @brief The root of a union-find set, with path halving.
 * @details Internal helper; not part of the public API.
 * @param[in,out] a_parent The parents.
 * @param[in]     a_x      An element.
 * @return Its root.
 */
inline uint32_t
findRoot(std::vector<uint32_t>& a_parent, uint32_t a_x) noexcept
{
  while (a_parent[a_x] != a_x) {
    a_parent[a_x] = a_parent[a_parent[a_x]];
    a_x           = a_parent[a_x];
  }

  return a_x;
}

template <class T>
ConnectedComponents<T>
label(const std::shared_ptr<const Cells<T>>& a_cells,
      const std::vector<uint32_t>&           a_groupOf,
      const uint32_t                         a_numGroups)
{
  const Cells<T>& cells = *a_cells;
  const uint32_t  num   = uint32_t(cells.m_leaves.size());

  EBGEOMETRY_REQUIRE(a_groupOf.size() == cells.m_numRegions,
                     "connectedComponents: %u groups given for %u regions",
                     unsigned(a_groupOf.size()),
                     unsigned(cells.m_numRegions));

  const auto groupOfLeaf = [&](const uint32_t a_leaf) { return a_groupOf[cells.m_leaves[a_leaf].m_region]; };

  // Join the leaves of one group that share a face.
  std::vector<uint32_t> parent(num);

  std::iota(parent.begin(), parent.end(), 0U);

  cells.forEachFaceNeighbor([&](const uint32_t a_leaf, const uint32_t a_other) {
    if (groupOfLeaf(a_leaf) == groupOfLeaf(a_other)) {
      const uint32_t x = findRoot(parent, a_leaf);
      const uint32_t y = findRoot(parent, a_other);

      parent[Math::max(x, y)] = Math::min(x, y);
    }
  });

  // Each set's component: its group, volume, box, seed and whether it reaches the box.
  std::vector<uint32_t>              setOfRoot(num, ConnectedComponents<T>::None);
  std::vector<ConnectedComponent<T>> sets;
  std::vector<T>                     best;

  const T volumeOfCell = cells.m_h * cells.m_h * cells.m_h;

  std::vector<uint32_t> setOfLeaf(num);

  for (uint32_t l = 0; l < num; l++) {
    const uint32_t root = findRoot(parent, l);

    if (setOfRoot[root] == ConnectedComponents<T>::None) {
      setOfRoot[root] = uint32_t(sets.size());

      ConnectedComponent<T> component;

      component.m_group = groupOfLeaf(l);

      sets.push_back(component);
      best.push_back(-Math::Limits<T>::max());
    }

    const uint32_t         s    = setOfRoot[root];
    const Leaf<T>&         leaf = cells.m_leaves[l];
    ConnectedComponent<T>& set  = sets[s];

    setOfLeaf[l] = s;

    uint32_t lo[3];
    uint32_t hi[3];

    cells.clipped(leaf, lo, hi);

    const T loCells[3]  = {T(lo[0]), T(lo[1]), T(lo[2])};
    const T hiCells[3]  = {T(hi[0]), T(hi[1]), T(hi[2])};
    const T midCells[3] = {
      T(0.5) * (loCells[0] + hiCells[0]), T(0.5) * (loCells[1] + hiCells[1]), T(0.5) * (loCells[2] + hiCells[2])};

    set.m_volume += T(uint64_t(hi[0] - lo[0]) * uint64_t(hi[1] - lo[1]) * uint64_t(hi[2] - lo[2])) * volumeOfCell;
    set.m_box = set.m_box.merged(BoundingVolumes::AABBT<T>(cells.point(loCells), cells.point(hiCells)));

    for (int d = 0; d < 3; d++) {
      set.m_reachesBox = set.m_reachesBox || lo[d] == 0U || hi[d] == cells.m_n[d];
    }

    // The seed: the center of the leaf deepest inside it. A leaf held whole by its region holds its clipped center.
    if (leaf.m_clearance > best[s]) {
      best[s]    = leaf.m_clearance;
      set.m_seed = cells.point(midCells);
    }
  }

  // Number the components by group, those reaching the box first, then by decreasing volume.
  std::vector<uint32_t> order(sets.size());

  std::iota(order.begin(), order.end(), 0U);

  std::stable_sort(order.begin(), order.end(), [&](const uint32_t a_x, const uint32_t a_y) {
    const ConnectedComponent<T>& x = sets[a_x];
    const ConnectedComponent<T>& y = sets[a_y];

    if (x.m_group != y.m_group) {
      return x.m_group < y.m_group;
    }
    if (x.m_reachesBox != y.m_reachesBox) {
      return x.m_reachesBox;
    }

    return x.m_volume > y.m_volume;
  });

  std::vector<uint32_t> numberOfSet(sets.size());

  for (uint32_t i = 0; i < order.size(); i++) {
    numberOfSet[order[i]] = i;
  }

  ConnectedComponents<T> result;

  result.m_cells = a_cells;

  result.m_componentOfCell.resize(num);

  for (uint32_t l = 0; l < num; l++) {
    result.m_componentOfCell[l] = numberOfSet[setOfLeaf[l]];
  }

  for (const uint32_t s : order) {
    result.m_components.push_back(sets[s]);
  }

  uint32_t numGroups = a_numGroups;

  for (const uint32_t group : a_groupOf) {
    numGroups = Math::max(numGroups, group + 1U);
  }

  result.m_firstOfGroup.assign(numGroups + 1U, 0U);

  for (const ConnectedComponent<T>& component : result.m_components) {
    result.m_firstOfGroup[component.m_group + 1U]++;
  }

  for (uint32_t g = 0; g < numGroups; g++) {
    result.m_firstOfGroup[g + 1U] += result.m_firstOfGroup[g];
  }

  // The components that touch: leaves of different components sharing a face.
  result.m_neighbors.resize(result.m_components.size());

  cells.forEachFaceNeighbor([&](const uint32_t a_leaf, const uint32_t a_other) {
    const uint32_t x = result.m_componentOfCell[a_leaf];
    const uint32_t y = result.m_componentOfCell[a_other];

    if (x != y) {
      result.m_neighbors[x].push_back(y);
      result.m_neighbors[y].push_back(x);
    }
  });

  for (std::vector<uint32_t>& neighbors : result.m_neighbors) {
    std::sort(neighbors.begin(), neighbors.end());

    neighbors.erase(std::unique(neighbors.begin(), neighbors.end()), neighbors.end());
  }

  return result;
}

} // namespace ComponentsDetail

template <class T>
inline uint32_t
ConnectedComponents<T>::getNumComponents() const noexcept
{
  return uint32_t(m_components.size());
}

template <class T>
inline const ConnectedComponent<T>&
ConnectedComponents<T>::component(const uint32_t a_component) const
{
  EBGEOMETRY_REQUIRE(a_component < m_components.size(),
                     "ConnectedComponents: no component %u of %u",
                     unsigned(a_component),
                     unsigned(m_components.size()));

  return m_components[a_component];
}

template <class T>
inline uint32_t
ConnectedComponents<T>::getNumGroups() const noexcept
{
  return m_firstOfGroup.empty() ? 0U : uint32_t(m_firstOfGroup.size() - 1U);
}

template <class T>
inline std::vector<uint32_t>
ConnectedComponents<T>::componentsOf(const uint32_t a_group) const
{
  EBGEOMETRY_REQUIRE(a_group < this->getNumGroups(),
                     "ConnectedComponents: no group %u of %u",
                     unsigned(a_group),
                     unsigned(this->getNumGroups()));

  std::vector<uint32_t> components(m_firstOfGroup[a_group + 1U] - m_firstOfGroup[a_group]);

  std::iota(components.begin(), components.end(), m_firstOfGroup[a_group]);

  return components;
}

template <class T>
inline const std::vector<uint32_t>&
ConnectedComponents<T>::neighbors(const uint32_t a_component) const
{
  EBGEOMETRY_REQUIRE(a_component < m_components.size(),
                     "ConnectedComponents: no component %u of %u",
                     unsigned(a_component),
                     unsigned(m_components.size()));

  return m_neighbors[a_component];
}

template <class T>
inline uint32_t
ConnectedComponents<T>::componentAt(const Vec3T<T>& a_point) const noexcept
{
  const ComponentsDetail::Cells<T>& cells = *m_cells;

  int64_t fine[3];

  for (int d = 0; d < 3; d++) {
    const T x = (a_point[d] - cells.m_box.getLowCorner()[d]) / cells.m_h;

    if (!(x >= T(0)) || !(x <= T(cells.m_n[d]))) {
      return None;
    }

    fine[d] = Math::min(int64_t(x), int64_t(cells.m_n[d]) - 1);
  }

  for (uint32_t level = cells.m_rootLevel + 1U; level-- > 0U;) {
    const int64_t index[3] = {fine[0] >> level, fine[1] >> level, fine[2] >> level};

    const uint32_t* found = cells.find(level, index);

    if (found == nullptr) {
      return None;
    }
    if (*found != ComponentsDetail::Cells<T>::Internal) {
      return m_componentOfCell[*found];
    }
  }

  return None;
}

template <class T>
inline T
ConnectedComponents<T>::resolution() const noexcept
{
  return m_cells->m_h;
}

template <class T>
inline const BoundingVolumes::AABBT<T>&
ConnectedComponents<T>::box() const noexcept
{
  return m_cells->m_box;
}

template <class T>
inline uint64_t
ConnectedComponents<T>::getNumCells() const noexcept
{
  return uint64_t(m_cells->m_cells.size());
}

template <class T, class... UserPrimitives>
inline ConnectedComponents<T>
connectedComponents(const Tape<T, UserPrimitives...>& a_tape, const ComponentOptions<T>& a_options)
{
  std::vector<uint32_t> groupOf(a_tape.getNumRegions());

  std::iota(groupOf.begin(), groupOf.end(), 0U);

  return ComponentsDetail::label(ComponentsDetail::search(a_tape, a_options), groupOf, 0U);
}

template <class T, class... UserPrimitives>
inline ConnectedComponents<T>
connectedComponents(const Tape<T, UserPrimitives...>& a_tape,
                    const std::vector<uint32_t>&      a_groupOf,
                    const ComponentOptions<T>&        a_options)
{
  EBGEOMETRY_REQUIRE(a_groupOf.size() == a_tape.getNumRegions(),
                     "connectedComponents: %u groups given for %u regions",
                     unsigned(a_groupOf.size()),
                     unsigned(a_tape.getNumRegions()));

  return ComponentsDetail::label(ComponentsDetail::search(a_tape, a_options), a_groupOf, 0U);
}

template <class T, class... UserPrimitives>
inline ConnectedComponents<T>
connectedComponents(const Domain<T, UserPrimitives...>& a_domain, const ComponentOptions<T>& a_options)
{
  const uint32_t   excluded   = a_domain.getNumSubdomainFunctions() - 1U;
  const Subdomains subdomains = a_domain.subdomains();

  std::vector<uint32_t> groupOf(a_domain.tape().getNumRegions());

  for (uint32_t r = 0; r < groupOf.size(); r++) {
    const uint32_t subdomain = subdomains.subdomainOf(r);

    groupOf[r] = (subdomain == Subdomains::None) ? excluded : subdomain;
  }

  return ComponentsDetail::label(ComponentsDetail::search(a_domain.tape(), a_options), groupOf, excluded + 1U);
}

} // namespace EBGeometry

#endif
