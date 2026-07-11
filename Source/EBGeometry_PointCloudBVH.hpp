// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

/*!
  @file    EBGeometry_PointCloudBVH.hpp
  @brief   A specialized PackedBVH for point clouds: fast index-based build and
           high-level nearest-neighbor / closest-point queries.
  @author  Robert Marskar
*/

#ifndef EBGeometry_PointCloudBVH
#define EBGeometry_PointCloudBVH

// Std includes
#include <cstddef>
#include <cstdint>
#include <limits>
#include <vector>

// Our includes
#include "EBGeometry_BVH.hpp"
#include "EBGeometry_BoundingVolumes.hpp"
#include "EBGeometry_PointAoSoA.hpp"
#include "EBGeometry_PointSoA.hpp"
#include "EBGeometry_Vec.hpp"

namespace EBGeometry {

/*!
  @brief A PackedBVH specialized for point clouds, with a fast build and turnkey queries.
  @details Unlike the general PackedBVH constructors (which take a list of pre-made primitives and
  partition it via SAH/Midpoint/SFC), PointCloudBVH is built directly from a raw particle cloud --
  positions plus a parallel array of user metadata. It uses an index-based, copy-free top-down
  build (partition an index permutation in place by longest-axis midpoint, pack leaves into
  PointAoSoA<T, size_t, W> groups inline), which is far cheaper than the general path and produces
  a tree just as tight for near-uniform clouds.

  Every leaf carries the particle's **cloud index** (its position in the input arrays) as metadata,
  so queries return that index; the user's own metadata is stored alongside and reachable via
  metadata(). The class hides all of pruneTraverse()/the SoA leaf kernel/the seed-from-own-leaf
  optimization behind a few high-level query methods.

  @note Queries come in two flavours. *External* queries (closestPoint / closestPoints) take an
  arbitrary point and traverse top-down. *Self* queries (nearestNeighbor / nearestNeighbors), and
  the batch allNearestNeighbors(), take a particle already in the cloud and additionally
  **seed the search bound from the leaf that particle lives in** (skipping that leaf during
  traversal) -- a strictly cheaper search that an external point cannot use.

  @tparam T    Floating-point precision.
  @tparam Meta User metadata type stored per particle and returned via metadata(). Defaults to the
               cloud index itself (std::size_t).
  @tparam K    BVH branching factor. Defaults to the SIMD-optimal value for T.
  @tparam W    Particles per SoA leaf lane group. Defaults to the SIMD-optimal width for T.
*/
template <class T,
          class Meta = std::size_t,
          size_t K   = BVH::DefaultBranchingRatio<T>(),
          size_t W   = PointSoA::DefaultWidth<T>()>
class PointCloudBVH
  : public BVH::PackedBVH<T, PointAoSoA<T, std::size_t, W>, K, BVH::ValueStorage<PointAoSoA<T, std::size_t, W>>>
{
public:
  /*! @brief The SoA leaf primitive: a group of up to W particles carrying their cloud indices. */
  using ParticleGroup = PointAoSoA<T, std::size_t, W>;

  /*! @brief The general packed BVH this specializes. */
  using Base = BVH::PackedBVH<T, ParticleGroup, K, BVH::ValueStorage<ParticleGroup>>;

  /*! @brief Flat node type inherited from Base. */
  using Node = typename Base::Node;

  /*! @brief Axis-aligned bounding box type. */
  using AABB = BoundingVolumes::AABBT<T>;

  /*!
    @brief One query result: the cloud index of a matched particle and its squared distance.
    @details @c index is the particle's position in the input @c positions / @c metadata arrays;
    use position()/metadata() to recover its data. @c distanceSquared avoids a sqrt on the hot path.
    @note A "no match" result (an empty cloud, or a self-query on a cloud with no other particle) is
    signalled by @c distanceSquared == std::numeric_limits<T>::max(); test that rather than @c index,
    since the default @c index of 0 is indistinguishable from a genuine match on particle 0. The
    multi-result queries instead report the count found via their return value.
  */
  struct Hit
  {
    std::size_t index           = 0;                             //!< Cloud index of the matched particle.
    T           distanceSquared = std::numeric_limits<T>::max(); //!< Squared distance from the query to it.
  };

  /*! @brief No default construction -- a point cloud is required. */
  PointCloudBVH() = delete;

  /*!
    @brief Build a BVH over a particle cloud.
    @param[in] a_positions      Particle positions.
    @param[in] a_metadata       Per-particle user metadata (same length/order as a_positions).
    @param[in] a_targetLeafSize Target particles per leaf (the build stops splitting at or below it).
  */
  inline PointCloudBVH(const std::vector<Vec3T<T>>& a_positions,
                       const std::vector<Meta>&     a_metadata,
                       std::size_t                  a_targetLeafSize = 16 * W);

  /*!
    @brief Closest cloud particle to an arbitrary query point.
    @param[in] a_query Query point (need not be in the cloud).
    @return The nearest particle and its squared distance.
  */
  [[nodiscard]] inline Hit
  closestPoint(const Vec3T<T>& a_query) const noexcept;

  /*!
    @brief The a_k closest cloud particles to an arbitrary query point, nearest first.
    @param[in]  a_query Query point (need not be in the cloud).
    @param[in]  a_k     Number of neighbors requested.
    @param[out] a_out   Buffer of at least a_k Hits; filled [0, returned count), ascending by distance.
    @return The number of neighbors found (min(a_k, cloud size)).
  */
  inline std::size_t
  closestPoints(const Vec3T<T>& a_query, std::size_t a_k, Hit* a_out) const noexcept;

  /*!
    @brief Nearest *other* particle to a particle already in the cloud (seed-from-own-leaf search).
    @param[in] a_particle Cloud index of the query particle; excluded from its own result.
    @return The nearest other particle and its squared distance.
  */
  [[nodiscard]] inline Hit
  nearestNeighbor(std::size_t a_particle) const noexcept;

  /*!
    @brief The a_k nearest *other* particles to a particle already in the cloud, nearest first.
    @param[in]  a_particle Cloud index of the query particle; excluded from its own result.
    @param[in]  a_k        Number of neighbors requested.
    @param[out] a_out      Buffer of at least a_k Hits; filled [0, returned count), ascending.
    @return The number of neighbors found (min(a_k, cloud size - 1)).
  */
  inline std::size_t
  nearestNeighbors(std::size_t a_particle, std::size_t a_k, Hit* a_out) const noexcept;

  /*!
    @brief For every particle, its a_k nearest *other* particles (the k-nearest-neighbor graph).
    @details Processes particles in leaf (build) order -- already spatially coherent, so consecutive
    queries touch nearby leaves and stay hot in cache, with no per-call sort -- and seeds each from its
    own leaf, so the whole batch is cheaper than the sum of independent queries. Result is flattened
    row-major:
    entry [i*a_k + j] is the j-th nearest neighbor of particle i (ascending by distance).
    @param[in] a_k Number of neighbors per particle.
    @return A vector of size numParticles()*a_k of Hits.
  */
  [[nodiscard]] inline std::vector<Hit>
  allNearestNeighbors(std::size_t a_k = 1) const;

  // ── Brute-force reference queries ────────────────────────────────────────────────────────────
  // O(N) full-scan counterparts of the accelerated queries above. They exist so tests, examples,
  // and downstream debugging can check a BVH result against ground truth without re-implementing a
  // scan, and so a suspected tree/traversal bug can be A/B-tested against an unaccelerated path.
  // They are NOT for production queries -- each is linear in the cloud size; use closestPoint() etc.

  /*!
    @brief Brute-force closest particle to an arbitrary point (O(N) reference for closestPoint()).
    @details Full linear scan; for testing/debugging only, never a hot path. Same result contract as
    closestPoint().
    @param[in] a_query Query point (need not be in the cloud).
    @return The nearest particle and its squared distance.
  */
  [[nodiscard]] inline Hit
  closestPointBruteForce(const Vec3T<T>& a_query) const noexcept;

  /*!
    @brief Brute-force a_k closest particles to an arbitrary point (O(N) reference for closestPoints()).
    @details Full linear scan; for testing/debugging only. Same result contract as closestPoints().
    @param[in]  a_query Query point (need not be in the cloud).
    @param[in]  a_k     Number of neighbors requested.
    @param[out] a_out   Buffer of at least a_k Hits; filled [0, returned count), ascending by distance.
    @return The number of neighbors found (min(a_k, cloud size)).
  */
  inline std::size_t
  closestPointsBruteForce(const Vec3T<T>& a_query, std::size_t a_k, Hit* a_out) const;

  /*!
    @brief Brute-force nearest *other* particle to a cloud particle (O(N) reference for nearestNeighbor()).
    @details Full linear scan excluding the particle itself; for testing/debugging only. Same result
    contract as nearestNeighbor().
    @param[in] a_particle Cloud index of the query particle; excluded from its own result.
    @return The nearest other particle and its squared distance.
  */
  [[nodiscard]] inline Hit
  nearestNeighborBruteForce(std::size_t a_particle) const noexcept;

  /*!
    @brief Brute-force a_k nearest *other* particles to a cloud particle (O(N) reference for
    nearestNeighbors()).
    @details Full linear scan excluding the particle itself; for testing/debugging only. Same result
    contract as nearestNeighbors().
    @param[in]  a_particle Cloud index of the query particle; excluded from its own result.
    @param[in]  a_k        Number of neighbors requested.
    @param[out] a_out      Buffer of at least a_k Hits; filled [0, returned count), ascending.
    @return The number of neighbors found (min(a_k, cloud size - 1)).
  */
  inline std::size_t
  nearestNeighborsBruteForce(std::size_t a_particle, std::size_t a_k, Hit* a_out) const;

  /*! @brief Number of particles in the cloud. @return The particle count. */
  [[nodiscard]] inline std::size_t
  numParticles() const noexcept
  {
    return m_positions.size();
  }

  /*!
    @brief Position of the particle with the given cloud index.
    @param[in] a_index Cloud index.
    @return The particle's position.
  */
  [[nodiscard]] inline const Vec3T<T>&
  position(std::size_t a_index) const noexcept
  {
    return m_positions[a_index];
  }

  /*!
    @brief User metadata of the particle with the given cloud index.
    @param[in] a_index Cloud index.
    @return The particle's user metadata.
  */
  [[nodiscard]] inline const Meta&
  metadata(std::size_t a_index) const noexcept
  {
    return m_metadata[a_index];
  }

private:
  /*! @brief The arrays produced by the index-based build, moved into the base and this class. */
  struct BuildResult
  {
    std::vector<Node>          nodes;
    std::vector<ParticleGroup> primitives;
    std::vector<std::uint32_t> leafOff;
    std::vector<std::uint32_t> leafCnt;
    std::vector<std::uint32_t> order; //!< Particle indices in leaf (build) order -- spatially coherent.
  };

  /*! @brief Delegated-to constructor: adopt the build result and keep the cloud data. */
  inline PointCloudBVH(BuildResult&&                a_build,
                       const std::vector<Vec3T<T>>& a_positions,
                       const std::vector<Meta>&     a_metadata);

  /*! @brief The index-based top-down midpoint build (static so it can run before base construction). */
  static inline BuildResult
  buildTree(const std::vector<Vec3T<T>>& a_positions, std::size_t a_leafSize);

  /*!
    @brief The shared query core: fill the a_k nearest to a_query into a_out.
    @param[in]  a_query   Query point.
    @param[in]  a_k       Neighbors requested.
    @param[out] a_out     Buffer of at least a_k Hits.
    @param[out] a_found   Number of neighbors found.
    @param[in]  a_exclude Cloud index to exclude (self-queries); s_none to exclude nothing.
    @param[in]  a_seedOff Group offset of the own leaf to seed from (self-queries; ignored if a_seedCnt == 0).
    @param[in]  a_seedCnt Group count of the own leaf; 0 means "no seed" (external queries).
  */
  inline void
  query(const Vec3T<T>& a_query,
        std::size_t     a_k,
        Hit*            a_out,
        std::size_t&    a_found,
        std::size_t     a_exclude,
        std::uint32_t   a_seedOff,
        std::uint32_t   a_seedCnt) const noexcept;

  /*!
    @brief Brute-force single nearest by full scan (shared by closestPointBruteForce / nearestNeighborBruteForce).
    @param[in] a_query   Query point.
    @param[in] a_exclude Cloud index to exclude, or s_none to exclude nothing.
    @return The nearest (non-excluded) particle and its squared distance; a default Hit if none.
  */
  [[nodiscard]] inline Hit
  bruteForceOne(const Vec3T<T>& a_query, std::size_t a_exclude) const noexcept;

  /*!
    @brief Brute-force a_k nearest by full scan (shared by closestPointsBruteForce / nearestNeighborsBruteForce).
    @param[in]  a_query   Query point.
    @param[in]  a_k       Neighbors requested.
    @param[out] a_out     Buffer of at least a_k Hits; filled ascending by distance.
    @param[in]  a_exclude Cloud index to exclude, or s_none to exclude nothing.
    @return The number of neighbors found.
  */
  inline std::size_t
  bruteForceK(const Vec3T<T>& a_query, std::size_t a_k, Hit* a_out, std::size_t a_exclude) const;

  /*! @brief Sentinel meaning "exclude no particle". */
  static constexpr std::size_t s_none = std::numeric_limits<std::size_t>::max();

  std::vector<Vec3T<T>>      m_positions; //!< Kept for self-query points and spatial ordering.
  std::vector<Meta>          m_metadata;  //!< User metadata, indexed by cloud index.
  std::vector<std::uint32_t> m_leafOff;   //!< Per-particle own-leaf group offset (for seeding).
  std::vector<std::uint32_t> m_leafCnt;   //!< Per-particle own-leaf group count (for seeding).
  std::vector<std::uint32_t> m_order;     //!< Particle indices in leaf (build) order; the spatially
                                          //!< coherent batch-query order, reused so allNearestNeighbors
                                          //!< needn't recompute a space-filling-curve sort per call.
};

} // namespace EBGeometry

#include "EBGeometry_PointCloudBVHImplem.hpp"

#endif
