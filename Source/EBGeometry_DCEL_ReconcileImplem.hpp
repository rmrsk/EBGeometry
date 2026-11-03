// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file   EBGeometry_DCEL_ReconcileImplem.hpp
 * @brief  Implementation of EBGeometry_DCEL_Reconcile.hpp
 * @author Robert Marskar
 */

#ifndef EBGEOMETRY_DCEL_RECONCILEIMPLEM_HPP
#define EBGEOMETRY_DCEL_RECONCILEIMPLEM_HPP

// Std includes
#include <vector>

// Our includes
#include "EBGeometry_DCEL_Iterator.hpp"
#include "EBGeometry_DCEL_Reconcile.hpp"
#include "EBGeometry_GPURuntime.hpp"

namespace EBGeometry {

namespace DCEL {

template <class T>
EBGEOMETRY_HOST
ReconcileSchedule::ReconcileSchedule(Pool& a_pool, const MeshT<T>& a_mesh)
{
  EBGEOMETRY_REQUIRE(a_mesh.isAttachedTo(a_pool),
                     "DCEL::ReconcileSchedule: the pool must be the one the mesh lives in");
  EBGEOMETRY_REQUIRE(!a_pool.isFrozen(),
                     "DCEL::ReconcileSchedule: the pool is frozen; build the schedule after the mesh and before "
                     "freezing the pool");

  a_mesh.checkFaceLoops();

  const typename MeshT<T>::Layout layout = a_mesh.layout();

  m_rootId       = a_pool.rootId();
  m_vertexOffset = layout.m_vertices.m_offset;
  m_edgeOffset   = layout.m_edges.m_offset;
  m_faceOffset   = layout.m_faces.m_offset;
  m_numVertices  = a_mesh.numVertices();
  m_numEdges     = a_mesh.numEdges();
  m_numFaces     = a_mesh.numFaces();

  // Corners per vertex, then where each vertex's corners start.
  std::vector<uint32_t> cornerBegin(m_numVertices + 1, 0);

  for (uint32_t f = 0; f < m_numFaces; f++) {
    for (EdgeIteratorT<T> iter(a_mesh, a_mesh.getFace(f)); iter.ok(); ++iter) {
      cornerBegin[a_mesh.getEdge(iter()).getVertexIndex() + 1]++;
    }
  }

  for (uint32_t v = 0; v < m_numVertices; v++) {
    cornerBegin[v + 1] += cornerBegin[v];
  }

  // Every corner, a vertex's faces in face order, with the vertex's two neighbors in the order a walk of the
  // face's loop from its half-edge finds them: VertexT::computeVertexNormalAngleWeighted()'s order. Each corner
  // walks its face's loop again, so a face of n vertices costs n^2 steps: nothing for triangles and quads, and
  // paid once, when the schedule is built.
  std::vector<ReconcileDetail::Corner> corners(cornerBegin[m_numVertices]);
  std::vector<uint32_t>                next(cornerBegin.begin(), cornerBegin.end() - 1);

  for (uint32_t f = 0; f < m_numFaces; f++) {
    const FaceT<T>& face = a_mesh.getFace(f);

    for (EdgeIteratorT<T> iter(a_mesh, face); iter.ok(); ++iter) {
      const uint32_t v = a_mesh.getEdge(iter()).getVertexIndex();

      ReconcileDetail::Corner corner{f, UINT32_MAX, UINT32_MAX};

      uint32_t found = 0;

      for (EdgeIteratorT<T> walk(a_mesh, face); walk.ok(); ++walk) {
        const EdgeT<T>& edge = a_mesh.getEdge(walk());
        const uint32_t  v1   = edge.getVertexIndex();
        const uint32_t  v2   = edge.getNextEdge(a_mesh).getVertexIndex();

        if (v1 == v || v2 == v) {
          const uint32_t neighbor = (v1 == v) ? v2 : v1;

          if (found == 0) {
            corner.m_first = neighbor;
          }
          else if (found == 1) {
            corner.m_second = neighbor;
          }

          found++;
        }
      }

      EBGEOMETRY_REQUIRE(found == 2,
                         "DCEL::VertexT::computeVertexNormalAngleWeighted: face %u must visit vertex %u exactly once "
                         "(found %u incident half-edges instead of 2)",
                         unsigned(f),
                         unsigned(v),
                         unsigned(found));

      corners[next[v]++] = corner;
    }
  }

  const uint32_t numCorners = cornerBegin[m_numVertices];
  const uint32_t status     = 0;

  m_cornerBegin.reserveSizedFrom(a_pool, m_numVertices + 1);
  m_cornerBegin.assign(a_pool.base(), cornerBegin.data(), m_numVertices + 1);
  m_corners.reserveSizedFrom(a_pool, numCorners);
  m_corners.assign(a_pool.base(), corners.data(), numCorners);
  m_status.reserveSizedFrom(a_pool, 1);
  m_status.assign(a_pool.base(), &status, 1);
}

namespace ReconcileDetail {

/**
 * @brief Run @p a_f over [0, @p a_n): with GPU::parallelFor() on a device-accessible pool, a host loop
 * otherwise.
 * @tparam F Functor.
 * @param[in] a_device Whether the pool is device-accessible.
 * @param[in] a_n      Number of indices.
 * @param[in] a_f      Functor.
 */
template <class F>
EBGEOMETRY_HOST
inline void
forEach(const bool a_device, const uint32_t a_n, const F& a_f)
{
  if (a_device) {
    GPU::parallelFor(a_n, a_f);

    return;
  }

  for (uint32_t i = 0; i < a_n; i++) {
    a_f(i);
  }
}

/**
 * @brief Copy one word between the host and the pool's memory: through the device runtime in a CUDA or HIP
 * build when the pool is device-accessible, directly otherwise.
 * @param[in]  a_device Whether the pool is device-accessible.
 * @param[out] a_dst    Destination.
 * @param[in]  a_src    Source.
 * @param[in]  a_toPool Whether the copy goes into the pool (otherwise out of it).
 */
EBGEOMETRY_HOST
inline void
copyWord(const bool a_device, uint32_t* a_dst, const uint32_t* a_src, const bool a_toPool)
{
#if defined(EBGEOMETRY_CUDA) || defined(EBGEOMETRY_HIP)
  if (a_device) {
    EBGEOMETRY_GPU_CHECK(
      GPU::memcpy(a_dst, a_src, sizeof(uint32_t), a_toPool ? GPU::MemcpyHostToDevice : GPU::MemcpyDeviceToHost));

    return;
  }
#else
  (void)a_device;
  (void)a_toPool;
#endif

  *a_dst = *a_src;
}

/**
 * @brief Abort, with VertexT::computeVertexNormalAngleWeighted()'s message, on the first vertex that has the same
 * position as one of its two neighbors on a face of non-zero area, or whose two neighbors have the same position:
 * the vertex pass's check, repeated on the host to name the vertex once the pass reported one.
 * @tparam T Floating-point precision.
 * @param[in] a_mesh        The reconciled mesh, readable on the host.
 * @param[in] a_base        The base address the schedule's arrays resolve against.
 * @param[in] a_cornerBegin The schedule's corner starts.
 * @param[in] a_corners     The schedule's corners.
 */
template <class T>
EBGEOMETRY_HOST
inline void
requireDistinct(const MeshT<T>&            a_mesh,
                const void*                a_base,
                const PODVector<uint32_t>& a_cornerBegin,
                const PODVector<Corner>&   a_corners)
{
  for (uint32_t v = 0; v < a_mesh.numVertices(); v++) {
    for (uint32_t c = a_cornerBegin.at(a_base, v); c < a_cornerBegin.at(a_base, v + 1); c++) {
      const Corner& corner = a_corners.at(a_base, c);

      if (a_mesh.getFace(corner.m_face).getNormal().length2() == T(0)) {
        continue;
      }

      const Vec3T<T>& x0 = a_mesh.getVertex(v).getPosition();
      const Vec3T<T>& x1 = a_mesh.getVertex(corner.m_first).getPosition();
      const Vec3T<T>& x2 = a_mesh.getVertex(corner.m_second).getPosition();

      EBGEOMETRY_REQUIRE(x0 != x1 && x0 != x2 && x1 != x2,
                         "DCEL::VertexT::computeVertexNormalAngleWeighted: face %u has coincident positions among "
                         "vertices %u, %u and %u",
                         unsigned(corner.m_face),
                         unsigned(v),
                         unsigned(corner.m_first),
                         unsigned(corner.m_second));
    }
  }
}

} // namespace ReconcileDetail

template <class T>
EBGEOMETRY_HOST
inline void
reconcile(Pool& a_pool, const MeshT<T>& a_mesh, const ReconcileSchedule& a_schedule)
{
  // The counts and offsets are in the layout's descriptors, so this reads no pool memory.
  const typename MeshT<T>::Layout layout = a_mesh.layout();

  EBGEOMETRY_REQUIRE(a_schedule.m_rootId == a_pool.rootId(),
                     "DCEL::reconcile: the schedule was built in a pool of another mirror chain; build it in the "
                     "mesh's pool");
  EBGEOMETRY_REQUIRE(
    a_schedule.m_numVertices == layout.m_vertices.size() && a_schedule.m_numEdges == layout.m_edges.size() &&
      a_schedule.m_numFaces == layout.m_faces.size(),
    "DCEL::reconcile: the schedule covers %u vertices, %u half-edges and %u faces but the mesh has %u, "
    "%u and %u; build the schedule for this mesh",
    unsigned(a_schedule.m_numVertices),
    unsigned(a_schedule.m_numEdges),
    unsigned(a_schedule.m_numFaces),
    unsigned(layout.m_vertices.size()),
    unsigned(layout.m_edges.size()),
    unsigned(layout.m_faces.size()));
  EBGEOMETRY_REQUIRE(a_schedule.m_vertexOffset == layout.m_vertices.m_offset &&
                       a_schedule.m_edgeOffset == layout.m_edges.m_offset &&
                       a_schedule.m_faceOffset == layout.m_faces.m_offset,
                     "DCEL::reconcile: the schedule was built for another mesh in this pool; build the schedule for "
                     "this mesh");

  const MeshT<T> view   = a_mesh.rebasedView(a_pool);
  const bool     device = a_pool.resource().isDeviceAccessible();
  void*          base   = a_pool.base();
  uint32_t*      status = &a_schedule.m_status.at(base, 0);

  const uint32_t clear = 0;

  ReconcileDetail::copyWord(device, status, &clear, true);

  // Faces first: the half-edge and vertex pseudonormals are made of the face normals.
  ReconcileDetail::forEach(device, a_schedule.m_numFaces, ReconcileDetail::Faces<T>{view});
  ReconcileDetail::forEach(device, a_schedule.m_numEdges, ReconcileDetail::Edges<T>{view});
  ReconcileDetail::forEach(
    device,
    a_schedule.m_numVertices,
    ReconcileDetail::Vertices<T>{view, base, a_schedule.m_cornerBegin, a_schedule.m_corners, a_schedule.m_status});

  uint32_t failed = 0;

  ReconcileDetail::copyWord(device, &failed, status, false);

  if (failed == 0) {
    return;
  }

  // Find the first vertex with coincident positions, for MeshT::reconcile()'s message: in place when the host can
  // read the pool, in a host copy otherwise.
  if (a_pool.resource().isHostAccessible()) {
    ReconcileDetail::requireDistinct(view, base, a_schedule.m_cornerBegin, a_schedule.m_corners);

    return;
  }

  const Pool copy = Pool::mirror(a_pool, hostMemoryResource());

  ReconcileDetail::requireDistinct(
    a_mesh.rebasedView(copy), copy.base(), a_schedule.m_cornerBegin, a_schedule.m_corners);
}

} // namespace DCEL

} // namespace EBGeometry

#endif
