// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file    EBGeometry_DCEL_ReconcileParallel.hpp
 * @brief   DCEL::reconcileParallel(): MeshT::reconcile() run with GPU::parallelFor(), on a device in a
 *          CUDA or HIP translation unit and in host loops otherwise.
 * @details All three parts of the reconcile, since a mesh reconciled in part has its signs wrong near
 *          vertices and edges:
 *          1. the faces: normal, centroid, area and projection axes, each a walk of the face's own
 *             half-edge loop (FaceT::reconcile());
 *          2. the edges: their angle-weighted pseudonormals from the two faces (EdgeT::reconcile());
 *          3. the vertices: their angle-weighted pseudonormals from the faces around them, which needs
 *             to know which faces touch each vertex.
 *          The host builds that vertex-to-face list as a vector of vectors. Here it is a CSR array
 *          built without atomics: every face corner becomes a key (vertex << 32 | face), the keys are
 *          sorted, and each vertex finds where its faces start with a binary search. The sort puts a
 *          vertex's faces in face order, the order the host lists them in, so every sum is taken in the
 *          same order and the result is the host's, bit for bit.
 * @author  Robert Marskar
 */

#ifndef EBGEOMETRY_DCEL_RECONCILEPARALLEL_HPP
#define EBGEOMETRY_DCEL_RECONCILEPARALLEL_HPP

// Std includes
#include <cstdint>

// Our includes
#include "EBGeometry_DCEL_Iterator.hpp"
#include "EBGeometry_DCEL_Mesh.hpp"
#include "EBGeometry_DCEL_Vertex.hpp"
#include "EBGeometry_GPU.hpp"
#include "EBGeometry_GPUAlgorithms.hpp"
#include "EBGeometry_GPUTransform.hpp"
#include "EBGeometry_Macros.hpp"
#include "EBGeometry_PODVector.hpp"
#include "EBGeometry_Pool.hpp"

namespace EBGeometry {

namespace DCEL {

/**
 * @brief The functors of reconcileParallel().
 * @details Internal; not part of the public API.
 */
namespace ReconcileDetail {

/**
 * @brief What is wrong with a face, if anything.
 */
enum class FaceStatus : uint8_t
{
  Ok,             ///< The face can be reconciled.
  NoHalfEdge,     ///< The face has no half-edge.
  OpenLoop,       ///< The face's half-edge loop does not close.
  TooFewVertices, ///< The loop has fewer than three vertices.
  BadVertex,      ///< The loop references a vertex past the end of the vertex array.
};

/**
 * @brief Step 1: check every face, and count its vertices for the corner offsets.
 * @tparam T Precision.
 */
template <class T>
struct CheckFaces
{
  /// @brief The mesh.
  MeshT<T> m_mesh;

  /// @brief Status per face.
  FaceStatus* m_status;

  /// @brief Detail per face: the edge reached (OpenLoop), the vertex count or the bad vertex.
  uint32_t* m_detail;

  /// @brief Steps taken per face (OpenLoop).
  uint32_t* m_steps;

  /// @brief Vertex count per face, zero for a bad face; padded with zeros to a power of two.
  uint32_t* m_counts;

  /**
   * @brief Face @p a_f.
   * @param[in] a_f Face index.
   */
  EBGEOMETRY_HOST_DEVICE
  void
  operator()(const std::size_t a_f) const noexcept
  {
    const auto f = static_cast<uint32_t>(a_f);

    const FaceT<T>& face = m_mesh.getFace(f);

    FaceStatus status = FaceStatus::Ok;
    uint32_t   detail = 0;
    uint32_t   steps  = 0;
    uint32_t   count  = 0;

    if (face.getHalfEdgeIndex() == UINT32_MAX) {
      status = FaceStatus::NoHalfEdge;
    }
    else if (!m_mesh.faceLoopCloses(f, detail, steps)) {
      status = FaceStatus::OpenLoop;
    }
    else {
      for (EdgeIteratorT<T> iter(m_mesh, face); iter.ok(); ++iter) {
        const uint32_t v = m_mesh.getEdge(iter()).getVertexIndex();

        if (v >= m_mesh.numVertices() && status == FaceStatus::Ok) {
          status = FaceStatus::BadVertex;
          detail = v;
        }

        count++;
      }

      if (status == FaceStatus::Ok && count < 3) {
        status = FaceStatus::TooFewVertices;
        detail = count;
      }
    }

    m_status[f] = status;
    m_detail[f] = detail;
    m_steps[f]  = steps;
    m_counts[f] = (status == FaceStatus::Ok) ? count : 0;
  }
};

/**
 * @brief Step 2: reconcile every face.
 * @tparam T Precision.
 */
template <class T>
struct ReconcileFaces
{
  /// @brief The mesh, whose faces are written.
  mutable MeshT<T> m_mesh;

  /**
   * @brief Face @p a_f.
   * @param[in] a_f Face index.
   */
  EBGEOMETRY_HOST_DEVICE
  void
  operator()(const std::size_t a_f) const noexcept
  {
    m_mesh.getFace(static_cast<uint32_t>(a_f)).reconcile(m_mesh);
  }
};

/**
 * @brief Step 3: reconcile every half-edge, from the reconciled faces.
 * @tparam T Precision.
 */
template <class T>
struct ReconcileEdges
{
  /// @brief The mesh, whose edges are written.
  mutable MeshT<T> m_mesh;

  /**
   * @brief Half-edge @p a_e.
   * @param[in] a_e Half-edge index.
   */
  EBGEOMETRY_HOST_DEVICE
  void
  operator()(const std::size_t a_e) const noexcept
  {
    m_mesh.getEdge(static_cast<uint32_t>(a_e)).reconcile(m_mesh);
  }
};

/**
 * @brief Step 4: one key (vertex << 32 | face) per face corner, at the face's offset.
 * @tparam T Precision.
 */
template <class T>
struct CornerKeys
{
  /// @brief The mesh.
  MeshT<T> m_mesh;

  /// @brief Where each face's corners start.
  const uint32_t* m_offsets;

  /// @brief The keys.
  uint64_t* m_keys;

  /**
   * @brief Face @p a_f.
   * @param[in] a_f Face index.
   */
  EBGEOMETRY_HOST_DEVICE
  void
  operator()(const std::size_t a_f) const noexcept
  {
    const auto f = static_cast<uint32_t>(a_f);

    uint32_t k = m_offsets[f];

    for (EdgeIteratorT<T> iter(m_mesh, m_mesh.getFace(f)); iter.ok(); ++iter) {
      m_keys[k++] = (uint64_t(m_mesh.getEdge(iter()).getVertexIndex()) << 32) | uint64_t(f);
    }
  }
};

/**
 * @brief Step 4b: padding keys after the corners, which sort last.
 */
struct PadKeys
{
  /// @brief The first padding key.
  uint64_t* m_keys;

  /**
   * @brief Padding key @p a_i.
   * @param[in] a_i Index past the corners.
   */
  EBGEOMETRY_HOST_DEVICE
  void
  operator()(const std::size_t a_i) const noexcept
  {
    m_keys[a_i] = UINT64_MAX;
  }
};

/**
 * @brief Step 5: where each vertex's faces start in the sorted keys, and the face of every key.
 */
struct VertexStarts
{
  /// @brief Sorted keys; the first m_numCorners are the corners.
  const uint64_t* m_keys;

  /// @brief Number of corners.
  uint32_t m_numCorners;

  /// @brief Number of vertices.
  uint32_t m_numVertices;

  /// @brief CSR offsets, one per vertex plus one.
  uint32_t* m_starts;

  /// @brief The face of every corner, in sorted order.
  uint32_t* m_faces;

  /**
   * @brief Vertex @p a_i (its start), and corner @p a_i (its face).
   * @param[in] a_i Index, below max(vertex count + 1, corner count).
   */
  EBGEOMETRY_HOST_DEVICE
  void
  operator()(const std::size_t a_i) const noexcept
  {
    const auto i = static_cast<uint32_t>(a_i);

    if (i <= m_numVertices) {
      const uint64_t first = uint64_t(i) << 32;

      uint32_t lo = 0;
      uint32_t hi = m_numCorners;

      while (lo < hi) {
        const uint32_t mid = lo + (hi - lo) / 2;

        if (m_keys[mid] < first) {
          lo = mid + 1;
        }
        else {
          hi = mid;
        }
      }

      m_starts[i] = lo;
    }

    if (i < m_numCorners) {
      m_faces[i] = static_cast<uint32_t>(m_keys[i] & uint64_t{0xffffffff});
    }
  }
};

/**
 * @brief Step 6: every vertex's angle-weighted pseudonormal from the faces around it.
 * @tparam T Precision.
 */
template <class T>
struct ReconcileVertices
{
  /// @brief The mesh, whose vertices are written.
  mutable MeshT<T> m_mesh;

  /// @brief CSR offsets.
  const uint32_t* m_starts;

  /// @brief Faces around each vertex.
  const uint32_t* m_faces;

  /// @brief Status per vertex.
  VertexNormalStatus* m_status;

  /**
   * @brief Vertex @p a_v.
   * @param[in] a_v Vertex index.
   */
  EBGEOMETRY_HOST_DEVICE
  void
  operator()(const std::size_t a_v) const noexcept
  {
    const auto v = static_cast<uint32_t>(a_v);

    const uint32_t begin = m_starts[v];
    const uint32_t count = m_starts[v + 1] - begin;

    VertexT<T>& vertex = m_mesh.getVertex(v);

    // A vertex no face uses (an OBJ file may list one) is never the closest feature of any face, so
    // its normal is never read; it gets a zero normal instead of one computed from nothing.
    if (count == 0) {
      vertex.setNormal(Vec3T<T>::zeros());

      m_status[v] = VertexNormalStatus{};
    }
    else {
      m_status[v] = vertex.computeVertexNormalAngleWeighted(v, m_faces + begin, count, m_mesh);
    }
  }
};

/**
 * @brief The smallest power of two at least @p a_n (1 for 0).
 * @param[in] a_n Count.
 * @return The power of two.
 */
[[nodiscard]] EBGEOMETRY_HOST
inline uint32_t
powerOfTwoAtLeast(const uint32_t a_n) noexcept
{
  uint32_t p = 1;

  while (p < a_n) {
    p *= 2;
  }

  return p;
}

} // namespace ReconcileDetail

/**
 * @brief MeshT::reconcile() with GPU::parallelFor(): the same faces, edges and vertex normals, bit for
 * bit, with every face, edge and vertex computed by its own thread.
 * @details On a device in a CUDA or HIP translation unit, and in host loops otherwise. The mesh must
 * live in @p a_pool, which must be host-accessible, and device-accessible as well in a CUDA or HIP
 * build (managed memory): the kernels write the mesh in place, and the host reads back one status per
 * face and per vertex to report a malformed mesh. The scratch arrays come from pools on the same
 * memory resource and are released on return. A mesh MeshT::reconcile() rejects is rejected here too,
 * with the same messages: a face with no half-edge, a half-edge loop that does not close, a face with
 * fewer than three vertices or a vertex index past the end of the vertex array, a face that visits a
 * vertex twice, and a face with coincident vertex positions. It also rejects a mesh with more than
 * 2^31 faces, 2^31 - 1 vertices or 2^31 face corners, the most its uint32_t scans and sort can count.
 * Every check aborts in every build, after the faces and edges are rewritten when a vertex check
 * fails, as with MeshT::reconcile().
 * @tparam T Precision.
 * @param[in,out] a_mesh The mesh; its faces, edges and vertices are rewritten.
 * @param[in]     a_pool The pool the mesh lives in.
 */
template <class T>
EBGEOMETRY_HOST
inline void
reconcileParallel(MeshT<T>& a_mesh, Pool& a_pool)
{
  using namespace ReconcileDetail;

  EBGEOMETRY_REQUIRE(a_mesh.isAttachedTo(a_pool),
                     "DCEL::reconcileParallel: the pool must be the one the mesh lives in");
  EBGEOMETRY_REQUIRE(a_pool.resource().isHostAccessible() && (!GPU::enabled || a_pool.resource().isDeviceAccessible()),
                     "DCEL::reconcileParallel: the pool must be host-accessible, and device-accessible as well in a "
                     "%s build (managed memory)",
                     GPU::backendName);

  const uint32_t numFaces    = a_mesh.numFaces();
  const uint32_t numEdges    = a_mesh.numEdges();
  const uint32_t numVertices = a_mesh.numVertices();

  // The scans and the sort run over power-of-two counts of uint32_t indices.
  constexpr uint32_t maxCount = uint32_t(1) << 31;

  EBGEOMETRY_REQUIRE(numFaces <= maxCount && numVertices < maxCount,
                     "DCEL::reconcileParallel: at most 2^31 faces and 2^31 - 1 vertices (%u faces, %u vertices)",
                     unsigned(numFaces),
                     unsigned(numVertices));

  // Step 1: check the faces and count their corners. Every reservation comes before the base is
  // resolved, since a reserve can move the block.
  Pool faceScratch(a_pool.resource());

  const uint32_t paddedFaces = powerOfTwoAtLeast(numFaces);

  PODVector<FaceStatus> faceStatus;
  PODVector<uint32_t>   faceDetail;
  PODVector<uint32_t>   faceSteps;
  PODVector<uint32_t>   offsets;
  PODVector<uint32_t>   total;

  faceStatus.reserveSizedFrom(faceScratch, numFaces);
  faceDetail.reserveSizedFrom(faceScratch, numFaces);
  faceSteps.reserveSizedFrom(faceScratch, numFaces);
  offsets.reserveSizedFrom(faceScratch, paddedFaces);
  total.reserveSizedFrom(faceScratch, 1);

  void* faceBase = faceScratch.base();

  // The padding counts are zeros, so the scan's total is the number of corners.
  for (uint32_t f = numFaces; f < paddedFaces; f++) {
    offsets.at(faceBase, f) = 0;
  }

  GPU::parallelFor(
    numFaces,
    CheckFaces<T>{
      a_mesh, faceStatus.data(faceBase), faceDetail.data(faceBase), faceSteps.data(faceBase), offsets.data(faceBase)});

  // Summed here in 64 bits, since the scan's uint32_t total would wrap on a mesh whose faces share loops.
  uint64_t cornerSum = 0;

  for (uint32_t f = 0; f < numFaces; f++) {
    const FaceStatus status = faceStatus.at(faceBase, f);
    const uint32_t   detail = faceDetail.at(faceBase, f);

    cornerSum += offsets.at(faceBase, f);

    EBGEOMETRY_REQUIRE(
      status != FaceStatus::NoHalfEdge, "DCEL::reconcileParallel: face %u has no half-edge", unsigned(f));
    EBGEOMETRY_REQUIRE(status != FaceStatus::OpenLoop,
                       "DCEL::reconcileParallel: the half-edge loop of face %u does not close (it reaches edge %u "
                       "after %u steps, and the mesh has %u half-edges)",
                       unsigned(f),
                       unsigned(detail),
                       unsigned(faceSteps.at(faceBase, f)),
                       unsigned(numEdges));
    EBGEOMETRY_REQUIRE(status != FaceStatus::TooFewVertices,
                       "DCEL::reconcileParallel: face %u needs at least 3 vertices (%u)",
                       unsigned(f),
                       unsigned(detail));
    EBGEOMETRY_REQUIRE(status != FaceStatus::BadVertex,
                       "DCEL::reconcileParallel: face %u references vertex %u, but the mesh has %u vertices",
                       unsigned(f),
                       unsigned(detail),
                       unsigned(numVertices));
  }

  EBGEOMETRY_REQUIRE(cornerSum <= maxCount,
                     "DCEL::reconcileParallel: at most 2^31 face corners in all (%llu)",
                     static_cast<unsigned long long>(cornerSum));

  GPU::exclusiveScan(offsets.data(faceBase), paddedFaces, total.data(faceBase));

  const uint32_t numCorners = total.at(faceBase, 0);

  // Steps 2 and 3: the faces, then the edges, whose pseudonormals read the face normals.
  GPU::parallelFor(numFaces, ReconcileFaces<T>{a_mesh});
  GPU::parallelFor(numEdges, ReconcileEdges<T>{a_mesh});

  // Steps 4 and 5: the vertex-to-face lists, as sorted corner keys.
  Pool vertexScratch(a_pool.resource());

  const uint32_t paddedCorners = powerOfTwoAtLeast(numCorners);

  PODVector<uint64_t>           keys;
  PODVector<uint32_t>           starts;
  PODVector<uint32_t>           faces;
  PODVector<VertexNormalStatus> vertexStatus;

  keys.reserveSizedFrom(vertexScratch, paddedCorners);
  starts.reserveSizedFrom(vertexScratch, numVertices + 1);
  faces.reserveSizedFrom(vertexScratch, numCorners);
  vertexStatus.reserveSizedFrom(vertexScratch, numVertices);

  void* vertexBase = vertexScratch.base();

  GPU::parallelFor(numFaces,
                   CornerKeys<T>{a_mesh, offsets.data(static_cast<const void*>(faceBase)), keys.data(vertexBase)});
  GPU::parallelFor(paddedCorners - numCorners, PadKeys{keys.data(vertexBase) + numCorners});
  GPU::bitonicSort(keys.data(vertexBase), paddedCorners);
  GPU::parallelFor(Math::max(numVertices + 1, numCorners),
                   VertexStarts{keys.data(static_cast<const void*>(vertexBase)),
                                numCorners,
                                numVertices,
                                starts.data(vertexBase),
                                faces.data(vertexBase)});

  // Step 6: the vertices.
  GPU::parallelFor(numVertices,
                   ReconcileVertices<T>{a_mesh,
                                        starts.data(static_cast<const void*>(vertexBase)),
                                        faces.data(static_cast<const void*>(vertexBase)),
                                        vertexStatus.data(vertexBase)});

  for (uint32_t v = 0; v < numVertices; v++) {
    const VertexNormalStatus& status = vertexStatus.at(vertexBase, v);

    EBGEOMETRY_REQUIRE(status.m_code != VertexNormalStatus::Code::NotVisitedOnce,
                       "DCEL::reconcileParallel: face %u must visit vertex %u exactly once (found %u incident "
                       "half-edges instead of 2)",
                       unsigned(status.m_face),
                       unsigned(v),
                       unsigned(status.m_found));
    EBGEOMETRY_REQUIRE(status.m_code != VertexNormalStatus::Code::CoincidentPositions,
                       "DCEL::reconcileParallel: face %u has coincident positions among vertices %u, %u and %u",
                       unsigned(status.m_face),
                       unsigned(v),
                       unsigned(status.m_neighbours[0]),
                       unsigned(status.m_neighbours[1]));
  }
}

} // namespace DCEL

} // namespace EBGeometry

#endif
