// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

/*!
  @file    EBGeometry_PointCloudBVHImplem.hpp
  @brief   Implementation of EBGeometry_PointCloudBVH.hpp
  @author  Robert Marskar
*/

#ifndef EBGeometry_PointCloudBVHImplem
#define EBGeometry_PointCloudBVHImplem

// Std includes
#include <algorithm>
#include <array>
#include <numeric>
#include <utility>

// Our includes
#include "EBGeometry_Macros.hpp"
#include "EBGeometry_PointCloudBVH.hpp"
#include "EBGeometry_SFC.hpp"

namespace EBGeometry {

template <class T, class Meta, size_t K, size_t W>
inline PointCloudBVH<T, Meta, K, W>::PointCloudBVH(const std::vector<Vec3T<T>>& a_positions,
                                                   const std::vector<Meta>&     a_metadata,
                                                   std::size_t                  a_targetLeafSize)
  : PointCloudBVH(buildTree(a_positions, a_targetLeafSize), a_positions, a_metadata)
{
  EBGEOMETRY_EXPECT(a_positions.size() == a_metadata.size());
}

template <class T, class Meta, size_t K, size_t W>
inline PointCloudBVH<T, Meta, K, W>::PointCloudBVH(BuildResult&&                a_build,
                                                   const std::vector<Vec3T<T>>& a_positions,
                                                   const std::vector<Meta>&     a_metadata)
  : Base(std::move(a_build.nodes), std::move(a_build.primitives)),
    m_positions(a_positions),
    m_metadata(a_metadata),
    m_leafOff(std::move(a_build.leafOff)),
    m_leafCnt(std::move(a_build.leafCnt))
{}

template <class T, class Meta, size_t K, size_t W>
inline typename PointCloudBVH<T, Meta, K, W>::BuildResult
PointCloudBVH<T, Meta, K, W>::buildTree(const std::vector<Vec3T<T>>& a_positions, std::size_t a_leafSize)
{
  const std::size_t n = a_positions.size();

  BuildResult res;
  res.leafOff.assign(n, 0);
  res.leafCnt.assign(n, 0);
  res.nodes.reserve(n > 0 ? 2 * n / std::max<std::size_t>(a_leafSize, 1) + 4 : 4);
  res.primitives.reserve(n / W + 4);

  std::vector<std::uint32_t> idx(n);
  std::iota(idx.begin(), idx.end(), std::uint32_t{0});

  // Recursion over index ranges [lo, hi) of idx. All partitioning is in place on idx; leaves pack
  // their particles into ParticleGroups inline. Kept in a local struct so the recursive calls do
  // not thread every argument.
  struct Builder
  {
    const std::vector<Vec3T<T>>& pos;
    std::vector<std::uint32_t>&  idx;
    BuildResult&                 res;
    const std::size_t            leafSize;

    // Recursively split idx[lo, hi) into a_kk index ranges by longest-axis midpoint (in place).
    void
    partition(std::uint32_t                                         a_lo,
              std::uint32_t                                         a_hi,
              int                                                   a_kk,
              std::vector<std::pair<std::uint32_t, std::uint32_t>>& a_out)
    {
      if (a_kk <= 1 || a_hi <= a_lo) {
        a_out.emplace_back(a_lo, a_hi);
        return;
      }

      Vec3T<T> clo = +Vec3T<T>::max();
      Vec3T<T> chi = -Vec3T<T>::max();
      for (std::uint32_t i = a_lo; i < a_hi; i++) {
        const Vec3T<T>& p = pos[idx[i]];
        clo               = min(clo, p);
        chi               = max(chi, p);
      }

      const int axis = (chi - clo).maxDir(true);
      const T   mid  = T(0.5) * (clo[axis] + chi[axis]);

      const std::vector<Vec3T<T>>& P = pos;
      auto m = std::partition(idx.begin() + a_lo, idx.begin() + a_hi, [&P, axis, mid](std::uint32_t a_id) noexcept {
        return P[a_id][axis] < mid;
      });

      std::uint32_t sp = static_cast<std::uint32_t>(m - idx.begin());

      const int k1 = a_kk / 2;
      const int k2 = a_kk - k1;

      // Keep each half populated enough to yield its share of non-empty leaves.
      sp = std::max<std::uint32_t>(a_lo + k1, std::min<std::uint32_t>(a_hi - k2, sp));

      partition(a_lo, sp, k1, a_out);
      partition(sp, a_hi, k2, a_out);
    }

    // Build the subtree over idx[lo, hi); returns its node index. Fills node bbox bottom-up.
    std::uint32_t
    build(std::uint32_t a_lo, std::uint32_t a_hi)
    {
      const std::uint32_t ni = static_cast<std::uint32_t>(res.nodes.size());
      res.nodes.emplace_back();

      if (a_hi - a_lo <= leafSize) {
        const std::uint32_t primOff = static_cast<std::uint32_t>(res.primitives.size());

        Vec3T<T> blo = +Vec3T<T>::max();
        Vec3T<T> bhi = -Vec3T<T>::max();

        for (std::uint32_t g = a_lo; g < a_hi; g += static_cast<std::uint32_t>(W)) {
          const std::uint32_t count = std::min<std::uint32_t>(static_cast<std::uint32_t>(W), a_hi - g);

          std::array<Vec3T<T>, W>    posArr;
          std::array<std::size_t, W> metaArr;
          for (std::uint32_t j = 0; j < count; j++) {
            posArr[j]  = pos[idx[g + j]];
            metaArr[j] = idx[g + j];
            blo        = min(blo, posArr[j]);
            bhi        = max(bhi, posArr[j]);
          }

          ParticleGroup group;
          group.pack(posArr.data(), metaArr.data(), count);
          res.primitives.push_back(group);
        }

        const std::uint32_t primCnt = static_cast<std::uint32_t>(res.primitives.size()) - primOff;
        for (std::uint32_t i = a_lo; i < a_hi; i++) {
          res.leafOff[idx[i]] = primOff;
          res.leafCnt[idx[i]] = primCnt;
        }

        res.nodes[ni].setBoundingVolume(AABB(blo, bhi));
        res.nodes[ni].setPrimitivesOffset(primOff);
        res.nodes[ni].setNumPrimitives(primCnt);

        return ni;
      }

      std::vector<std::pair<std::uint32_t, std::uint32_t>> ranges;
      ranges.reserve(K);
      this->partition(a_lo, a_hi, static_cast<int>(K), ranges);

      Vec3T<T>                     blo = +Vec3T<T>::max();
      Vec3T<T>                     bhi = -Vec3T<T>::max();
      std::array<std::uint32_t, K> children;
      for (std::size_t k = 0; k < K; k++) {
        children[k]      = this->build(ranges[k].first, ranges[k].second);
        const AABB& cbox = res.nodes[children[k]].getBoundingVolume();
        blo              = min(blo, cbox.getLowCorner());
        bhi              = max(bhi, cbox.getHighCorner());
      }
      for (std::size_t k = 0; k < K; k++) {
        res.nodes[ni].setChildOffset(children[k], k); // interior: setNumPrimitives stays 0
      }
      res.nodes[ni].setBoundingVolume(AABB(blo, bhi));

      return ni;
    }
  };

  if (n > 0) {
    Builder builder{a_positions, idx, res, std::max<std::size_t>(a_leafSize, 1)};
    builder.build(0, static_cast<std::uint32_t>(n));
  }

  return res;
}

template <class T, class Meta, size_t K, size_t W>
inline void
PointCloudBVH<T, Meta, K, W>::query(const Vec3T<T>& a_query,
                                    std::size_t     a_k,
                                    Hit*            a_out,
                                    std::size_t&    a_found,
                                    std::size_t     a_exclude,
                                    std::uint32_t   a_seedOff,
                                    std::uint32_t   a_seedCnt) const noexcept
{
  // Fast path for the single-nearest case (closestPoint / nearestNeighbor / all-NN with k==1), the
  // common one: a lean {distance, index} state instead of the general k-best set, so the per-lane
  // work is just a compare-and-maybe-store.
  if (a_k == 1) {
    struct Best
    {
      T           d2  = std::numeric_limits<T>::max();
      std::size_t idx = s_none;
    };
    Best       best;
    const auto scan1 = [this, &a_query, a_exclude](Best& a_best, std::size_t a_off, std::size_t a_cnt) noexcept {
      for (std::size_t g = 0; g < a_cnt; g++) {
        const ParticleGroup&   group     = this->m_primitives[a_off + g];
        const std::array<T, W> distances = group.getDistances2(a_query);
        for (std::size_t lane = 0; lane < W; lane++) {
          const std::size_t c = group.getMetaData(lane);
          if (c != a_exclude && distances[lane] < a_best.d2) {
            a_best.d2  = distances[lane];
            a_best.idx = c;
          }
        }
      }
    };
    if (a_seedCnt > 0) {
      // Seeded self-query: scanning the query particle's own leaf first gives a tight prune bound
      // immediately, so a plain unordered scalar DFS prunes just as hard as an ordered descent --
      // without paying pruneTraverse's per-interior-node child-sort and its separate m_childAabbSoA
      // SIMD load. For these cheap SoA point leaves that makes the unordered DFS ~20% faster. (This
      // holds ONLY because of the seed: see the external branch below.)
      scan1(best, a_seedOff, a_seedCnt);

      std::uint32_t stack[64];
      int           sp = 0;
      stack[sp++]      = 0U;
      while (sp > 0) {
        const Node& node = this->m_linearNodes[stack[--sp]];
        if (node.getDistanceToBoundingVolume2(a_query) >= best.d2) {
          continue;
        }
        if (node.isLeaf()) {
          const std::uint32_t off = node.getPrimitivesOffset();
          if (off != a_seedOff) {
            scan1(best, off, node.getNumPrimitives());
          }
        }
        else {
          const auto& kids = node.getChildOffsets();
          for (std::size_t k = 0; k < K; k++) {
            stack[sp++] = kids[k];
          }
        }
      }
    }
    else {
      // Unseeded external query: the prune bound starts at infinity, so the order in which leaves are
      // visited is decisive -- pruneTraverse's near-first ordered descent tightens the bound fast,
      // whereas an unordered DFS would explore huge far-away regions before finding a close point (it
      // is ~17x slower here). The ordering cost that does not pay for seeded self-queries is exactly
      // what makes external queries fast, so this branch keeps the base traversal.
      const auto eval1 = [&scan1](Best& a_best, std::size_t a_off, std::size_t a_cnt) noexcept {
        scan1(a_best, a_off, a_cnt);
      };
      const auto prune1 = [](const Best& a_best) noexcept -> T { return a_best.d2; };
      this->pruneTraverse(a_query, best, eval1, prune1);
    }
    if (best.idx != s_none) {
      a_out[0] = Hit{best.idx, best.d2};
      a_found  = 1;
    }
    return;
  }

  // A running a_k-best set (ascending by distance) plus the config the leaf scan needs. It is the
  // pruneTraverse() state, so the inherited SIMD-optimized traversal prunes off its current bound.
  struct QState
  {
    Hit*        out;
    std::size_t k;
    std::size_t exclude;
    std::size_t found = 0;
    T           bound = std::numeric_limits<T>::max();

    inline void
    insert(T a_d2, std::size_t a_idx) noexcept
    {
      if (a_idx == exclude || a_d2 >= bound) {
        return; // self, or farther than the current worst -- cheap reject before the de-dupe scan
      }
      if (k > 1) {
        for (std::size_t i = 0; i < found; i++) {
          if (out[i].index == a_idx) {
            return; // already held (padded lanes / seed-then-traverse would otherwise double-count)
          }
        }
      }
      std::size_t pos = (found < k) ? found++ : (k - 1);
      out[pos]        = Hit{a_idx, a_d2};
      while (pos > 0 && out[pos].distanceSquared < out[pos - 1].distanceSquared) {
        std::swap(out[pos], out[pos - 1]);
        pos--;
      }
      bound = (found < k) ? std::numeric_limits<T>::max() : out[k - 1].distanceSquared;
    }
  };

  QState state{a_out, a_k, a_exclude};

  const auto processLeaf = [this, &a_query](QState& a_state, std::size_t a_off, std::size_t a_cnt) noexcept {
    for (std::size_t g = 0; g < a_cnt; g++) {
      const ParticleGroup&   group     = this->m_primitives[a_off + g];
      const std::array<T, W> distances = group.getDistances2(a_query);
      for (std::size_t lane = 0; lane < W; lane++) {
        a_state.insert(distances[lane], group.getMetaData(lane));
      }
    }
  };

  // Seed the bound from the query particle's own leaf (self queries only), then skip that leaf below
  // so the descent prunes with an already-tight bound instead of starting from +inf.
  if (a_seedCnt > 0) {
    processLeaf(state, a_seedOff, a_seedCnt);
  }

  const auto evalLeaf = [&](QState& a_state, std::size_t a_off, std::size_t a_cnt) noexcept {
    if (a_seedCnt > 0 && static_cast<std::uint32_t>(a_off) == a_seedOff) {
      return; // own leaf already scanned in the seed
    }
    processLeaf(a_state, a_off, a_cnt);
  };
  const auto pruneDist2 = [](const QState& a_state) noexcept -> T { return a_state.bound; };

  this->pruneTraverse(a_query, state, evalLeaf, pruneDist2);

  a_found = state.found;
}

template <class T, class Meta, size_t K, size_t W>
inline typename PointCloudBVH<T, Meta, K, W>::Hit
PointCloudBVH<T, Meta, K, W>::closestPoint(const Vec3T<T>& a_query) const noexcept
{
  Hit         hit;
  std::size_t found = 0;
  this->query(a_query, 1, &hit, found, s_none, 0, 0);
  return hit;
}

template <class T, class Meta, size_t K, size_t W>
inline std::size_t
PointCloudBVH<T, Meta, K, W>::closestPoints(const Vec3T<T>& a_query, std::size_t a_k, Hit* a_out) const noexcept
{
  std::size_t found = 0;
  this->query(a_query, a_k, a_out, found, s_none, 0, 0);
  return found;
}

template <class T, class Meta, size_t K, size_t W>
inline typename PointCloudBVH<T, Meta, K, W>::Hit
PointCloudBVH<T, Meta, K, W>::nearestNeighbor(std::size_t a_particle) const noexcept
{
  Hit         hit;
  std::size_t found = 0;
  this->query(m_positions[a_particle], 1, &hit, found, a_particle, m_leafOff[a_particle], m_leafCnt[a_particle]);
  return hit;
}

template <class T, class Meta, size_t K, size_t W>
inline std::size_t
PointCloudBVH<T, Meta, K, W>::nearestNeighbors(std::size_t a_particle, std::size_t a_k, Hit* a_out) const noexcept
{
  std::size_t found = 0;
  this->query(m_positions[a_particle], a_k, a_out, found, a_particle, m_leafOff[a_particle], m_leafCnt[a_particle]);
  return found;
}

template <class T, class Meta, size_t K, size_t W>
inline std::vector<typename PointCloudBVH<T, Meta, K, W>::Hit>
PointCloudBVH<T, Meta, K, W>::allNearestNeighbors(std::size_t a_k) const
{
  const std::size_t n = m_positions.size();

  std::vector<Hit> result(n * a_k);

  // Process particles in Hilbert order: consecutive queries touch nearby leaves, so the tree nodes
  // and each seeded own-leaf stay hot in cache. Ordering affects only speed, not results.
  const std::vector<std::uint32_t> order = SFC::order<SFC::Hilbert>(m_positions);

  for (const std::uint32_t p : order) {
    std::size_t found = 0;
    this->query(m_positions[p], a_k, &result[p * a_k], found, p, m_leafOff[p], m_leafCnt[p]);
  }

  return result;
}

template <class T, class Meta, size_t K, size_t W>
inline typename PointCloudBVH<T, Meta, K, W>::Hit
PointCloudBVH<T, Meta, K, W>::bruteForceOne(const Vec3T<T>& a_query, std::size_t a_exclude) const noexcept
{
  Hit best;
  for (std::size_t i = 0; i < m_positions.size(); i++) {
    if (i == a_exclude) {
      continue;
    }
    const T d2 = (m_positions[i] - a_query).length2();
    if (d2 < best.distanceSquared) {
      best.distanceSquared = d2;
      best.index           = i;
    }
  }
  return best;
}

template <class T, class Meta, size_t K, size_t W>
inline std::size_t
PointCloudBVH<T, Meta, K, W>::bruteForceK(const Vec3T<T>& a_query,
                                          std::size_t     a_k,
                                          Hit*            a_out,
                                          std::size_t     a_exclude) const
{
  // Full scan into a scratch buffer, then partial_sort the a_k smallest to the front. Deliberately
  // simple and obviously correct -- this is the reference path, not a hot one.
  std::vector<Hit> all;
  all.reserve(m_positions.size());
  for (std::size_t i = 0; i < m_positions.size(); i++) {
    if (i == a_exclude) {
      continue;
    }
    all.push_back(Hit{i, (m_positions[i] - a_query).length2()});
  }

  const std::size_t k = std::min(a_k, all.size());
  std::partial_sort(
    all.begin(),
    all.begin() + static_cast<std::ptrdiff_t>(k),
    all.end(),
    [](const Hit& a_lhs, const Hit& a_rhs) noexcept { return a_lhs.distanceSquared < a_rhs.distanceSquared; });
  for (std::size_t j = 0; j < k; j++) {
    a_out[j] = all[j];
  }
  return k;
}

template <class T, class Meta, size_t K, size_t W>
inline typename PointCloudBVH<T, Meta, K, W>::Hit
PointCloudBVH<T, Meta, K, W>::closestPointBruteForce(const Vec3T<T>& a_query) const noexcept
{
  return this->bruteForceOne(a_query, s_none);
}

template <class T, class Meta, size_t K, size_t W>
inline std::size_t
PointCloudBVH<T, Meta, K, W>::closestPointsBruteForce(const Vec3T<T>& a_query, std::size_t a_k, Hit* a_out) const
{
  return this->bruteForceK(a_query, a_k, a_out, s_none);
}

template <class T, class Meta, size_t K, size_t W>
inline typename PointCloudBVH<T, Meta, K, W>::Hit
PointCloudBVH<T, Meta, K, W>::nearestNeighborBruteForce(std::size_t a_particle) const noexcept
{
  return this->bruteForceOne(m_positions[a_particle], a_particle);
}

template <class T, class Meta, size_t K, size_t W>
inline std::size_t
PointCloudBVH<T, Meta, K, W>::nearestNeighborsBruteForce(std::size_t a_particle, std::size_t a_k, Hit* a_out) const
{
  return this->bruteForceK(m_positions[a_particle], a_k, a_out, a_particle);
}

} // namespace EBGeometry

#endif
