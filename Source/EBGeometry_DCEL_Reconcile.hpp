// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file   EBGeometry_DCEL_Reconcile.hpp
 * @brief  Reconciling a DCEL mesh whose vertices moved, where its pool is: in parallel on a device, in host
 * loops otherwise.
 * @details MeshT::reconcile() checks the mesh's topology and builds which faces touch each vertex, as vectors,
 * on every call, and runs on the host only. Moving vertices changes neither, so DCEL::ReconcileSchedule checks
 * the topology and stores the vertex-to-face lists once, on the host, and DCEL::reconcile() then reconciles the
 * mesh in three passes: one per face, per half-edge and per vertex.
 * @author Robert Marskar
 */

#ifndef EBGEOMETRY_DCEL_RECONCILE_HPP
#define EBGEOMETRY_DCEL_RECONCILE_HPP

// Std includes
#include <cstddef>
#include <cstdint>

// Our includes
#include "EBGeometry_DCEL_Mesh.hpp"
#include "EBGeometry_DCEL_Vertex.hpp"
#include "EBGeometry_GPUTransform.hpp"
#include "EBGeometry_Macros.hpp"
#include "EBGeometry_PODVector.hpp"
#include "EBGeometry_Pool.hpp"

namespace EBGeometry {

namespace DCEL {

/**
 * @brief Internals of DCEL::ReconcileSchedule and DCEL::reconcile().
 * @details Internal; not part of the public API.
 */
namespace ReconcileDetail {

/**
 * @brief One corner of a face, as seen from the vertex at it: the face, and the vertices before and after
 * the vertex on the face's loop.
 */
struct Corner
{
  /// @brief The face.
  uint32_t m_face;

  /// @brief The first neighbor found walking the face's loop from its half-edge.
  uint32_t m_first;

  /// @brief The second neighbor found.
  uint32_t m_second;
};

} // namespace ReconcileDetail

/**
 * @brief Which faces touch each vertex of a mesh, checked and stored once, for DCEL::reconcile().
 * @details Opt-in: only a mesh that is reconciled with DCEL::reconcile() needs one. Build it on the host after
 * the mesh is built and before its pool is frozen: its arrays are reserved from the mesh's pool, so a mirror of
 * the pool carries them along, and one schedule serves the mesh in its own pool and in every mirror of it.
 * Moving vertices never changes which faces touch a vertex, so a schedule is built once; a mesh whose vertices,
 * half-edges or faces are added, removed or relinked needs a new one. The schedule belongs to the mesh in that pool
 * and its mirrors: a deepCopy() of the mesh into another pool needs its own schedule, and the pool must outlive
 * the schedule as it does the mesh.
 *
 * The constructor checks the topology as MeshT::reconcile() does, with its messages (every face has a
 * half-edge, its loop closes, has at least three vertices, names existing vertices and visits each of them
 * once), so that DCEL::reconcile() has only the geometry left to check.
 *
 * Per vertex, the schedule stores where its corners start (4 bytes), and per face corner, the face and the two
 * vertices next to the corner (12 bytes), in the order MeshT::reconcile() visits them: a vertex's faces in face
 * order, and the neighbors in the order a walk of the face's loop from its half-edge finds them. Storing the
 * neighbors spares the vertex pass a walk of every face's loop: without them (4 bytes per corner instead of 12)
 * the pass measured 2.6 to 4 times slower in float and 10-20% slower in double on a device (100,000 and 250,000
 * faces, RTX 3080). The schedule also holds one status word, which the vertex pass sets on coincident positions.
 */
class ReconcileSchedule
{
public:
  /**
   * @brief Check the topology of @p a_mesh and store which faces touch each vertex.
   * @details Aborts, in every build, if @p a_mesh does not live in @p a_pool, the pool is frozen, or the
   * topology check fails.
   * @tparam T Floating-point precision.
   * @param[in,out] a_pool The pool @p a_mesh lives in, which the schedule's arrays are reserved from. It must
   * not be frozen.
   * @param[in]     a_mesh The mesh, on the host.
   */
  template <class T>
  EBGEOMETRY_HOST
  ReconcileSchedule(Pool& a_pool, const MeshT<T>& a_mesh);

private:
  /**
   * @brief DCEL::reconcile() reads the arrays.
   */
  template <class T>
  friend void
  reconcile(Pool& a_pool, const MeshT<T>& a_mesh, const ReconcileSchedule& a_schedule);

  /**
   * @brief Where each vertex's corners start in m_corners, with the corner count at the end.
   */
  PODVector<uint32_t> m_cornerBegin;

  /**
   * @brief Every face corner, grouped by vertex.
   */
  PODVector<ReconcileDetail::Corner> m_corners;

  /**
   * @brief One word, set by the vertex pass when a vertex and its neighbors on a face have coincident
   * positions.
   */
  PODVector<uint32_t> m_status;

  /**
   * @brief Pool::rootId() of the pool the schedule was built in, so that DCEL::reconcile() can tell a pool
   * of another mirror chain.
   */
  uint64_t m_rootId = 0;

  /**
   * @brief Where the mesh's vertex, half-edge and face arrays start in the pool. Every array of a pool has its
   * own offset and a mirror keeps them, so with m_rootId they name the one mesh the schedule was built for.
   */
  uint64_t m_vertexOffset = 0;

  /// @brief See m_vertexOffset.
  uint64_t m_edgeOffset = 0;

  /// @brief See m_vertexOffset.
  uint64_t m_faceOffset = 0;

  /**
   * @brief The mesh's vertex, half-edge and face counts.
   */
  uint32_t m_numVertices = 0;

  /// @brief See m_numVertices.
  uint32_t m_numEdges = 0;

  /// @brief See m_numVertices.
  uint32_t m_numFaces = 0;
};

/**
 * @brief Reconcile the copy of @p a_mesh that lives in @p a_pool, after its vertices have moved: every face's
 * normal, centroid, area and projection axes, and every half-edge's and vertex's pseudonormal, as
 * MeshT::reconcile() computes them.
 * @details For vertices that move without changing the mesh's topology. Runs where @p a_pool's memory is, in
 * three passes, one per face, per half-edge and per vertex: on a device-accessible pool, each pass is one
 * GPU::parallelFor() (device kernels in a CUDA or HIP build, host loops over emulated device memory in a host
 * build); on a host pool, each is a host loop. @p a_pool is the mesh's own pool or any mirror of it.
 *
 * In host memory the result is MeshT::reconcile()'s, byte for byte. On a device it agrees to rounding: device
 * compilers fuse multiplies and adds by default, and the device's std::acos differs from the host's in the last
 * bits.
 *
 * How the vertices were moved is the caller's business (any kernel, any library, a copy from the host), as long
 * as that work has finished: the passes run on the default stream, and reconcile() returns once they are done.
 * One reconcile at a time per schedule and pool: the passes share the schedule's status word in that pool, so two
 * reconciles of the same pool's mesh running at once (from two host threads, say) would race on it.
 * A mesh SDF over the mesh also needs its BVH refitted (BVH::refit()) before it is queried again.
 *
 * Aborts, in every build, on a schedule built in a pool of another mirror chain or for another mesh, and on the
 * geometry MeshT::reconcile() aborts on, with its message: a vertex and its two neighbors on a face (of
 * non-zero area) with coincident positions. The status word the vertex pass sets is read back after the passes
 * (4 bytes from a device), and only when it is set is the vertex found on the host (in place when the host can
 * read the pool, in a host mirror otherwise). With several such vertices, the message names the first in vertex
 * order, as MeshT::reconcile() does, whatever order the device's threads found them in.
 * @tparam T Floating-point precision.
 * @param[in,out] a_pool     The pool whose copy of the mesh is reconciled: the mesh's own pool or a mirror.
 * @param[in]     a_mesh     The mesh, as built (a handle to its own pool).
 * @param[in]     a_schedule The schedule built for @p a_mesh.
 */
template <class T>
EBGEOMETRY_HOST
inline void
reconcile(Pool& a_pool, const MeshT<T>& a_mesh, const ReconcileSchedule& a_schedule);

namespace ReconcileDetail {

/**
 * @brief The face pass: FaceT::reconcile() on every face.
 * @tparam T Floating-point precision.
 */
template <class T>
struct Faces
{
  /**
   * @brief The mesh, rebased onto the pool being reconciled.
   * @details Mutable because GPU::parallelFor() calls a const operator() and the pass writes the faces:
   * through the pool, which the handle's non-const accessors reach.
   */
  mutable MeshT<T> m_mesh;

  /**
   * @brief Reconcile face @p a_i.
   * @param[in] a_i Face index.
   */
  EBGEOMETRY_HOST_DEVICE
  inline void
  operator()(const std::size_t a_i) const
  {
    m_mesh.getFace(static_cast<uint32_t>(a_i)).reconcile(m_mesh);
  }
};

/**
 * @brief The half-edge pass: EdgeT::reconcile() on every half-edge.
 * @tparam T Floating-point precision.
 */
template <class T>
struct Edges
{
  /**
   * @brief The mesh, rebased onto the pool being reconciled (mutable as in Faces).
   */
  mutable MeshT<T> m_mesh;

  /**
   * @brief Reconcile half-edge @p a_i.
   * @param[in] a_i Half-edge index.
   */
  EBGEOMETRY_HOST_DEVICE
  inline void
  operator()(const std::size_t a_i) const noexcept
  {
    m_mesh.getEdge(static_cast<uint32_t>(a_i)).reconcile(m_mesh);
  }
};

/**
 * @brief The vertex pass: every vertex's angle-weighted pseudonormal, from the schedule's corners.
 * @details The terms are those of VertexT::computeVertexNormalAngleWeighted(), summed in its order.
 * @tparam T Floating-point precision.
 */
template <class T>
struct Vertices
{
  /**
   * @brief The mesh, rebased onto the pool being reconciled (mutable as in Faces).
   */
  mutable MeshT<T> m_mesh;

  /**
   * @brief The pool's base address, which the schedule's arrays resolve against.
   */
  void* m_base;

  /**
   * @brief The schedule's corner starts.
   */
  PODVector<uint32_t> m_cornerBegin;

  /**
   * @brief The schedule's corners.
   */
  PODVector<Corner> m_corners;

  /**
   * @brief The schedule's status word.
   */
  PODVector<uint32_t> m_status;

  /**
   * @brief Compute vertex @p a_i's pseudonormal.
   * @param[in] a_i Vertex index.
   */
  EBGEOMETRY_HOST_DEVICE
  inline void
  operator()(const std::size_t a_i) const noexcept
  {
    const auto     v     = static_cast<uint32_t>(a_i);
    const uint32_t begin = m_cornerBegin.at(m_base, v);
    const uint32_t end   = m_cornerBegin.at(m_base, v + 1);

    VertexT<T>&    vertex = m_mesh.getVertex(v);
    const Vec3T<T> x0     = vertex.getPosition();

    // A vertex no face uses gets a zero normal, as in MeshT::reconcile().
    Vec3T<T> normal = Vec3T<T>::zeros();

    for (uint32_t c = begin; c < end; c++) {
      const Corner&   corner = m_corners.at(m_base, c);
      const FaceT<T>& face   = m_mesh.getFace(corner.m_face);

      // A zero-area face contributes nothing, as in VertexT::computeVertexNormalAngleWeighted().
      if (face.getNormal().length2() == T(0)) {
        continue;
      }

      const Vec3T<T>& x1 = m_mesh.getVertex(corner.m_first).getPosition();
      const Vec3T<T>& x2 = m_mesh.getVertex(corner.m_second).getPosition();

      // Every thread that finds a coincidence writes the same value, so the order does not matter.
      if (x0 == x1 || x0 == x2 || x1 == x2) {
        m_status.at(m_base, 0) = 1;

        continue;
      }

      normal += VertexDetail::angleWeightedTerm(x0, x1, x2, face.getNormal());
    }

    vertex.setNormal(normal);

    if (end > begin) {
      vertex.normalizeNormalVector();
    }
  }
};

} // namespace ReconcileDetail

} // namespace DCEL

} // namespace EBGeometry

#include "EBGeometry_DCEL_ReconcileImplem.hpp"

#endif
