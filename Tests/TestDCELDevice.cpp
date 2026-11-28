// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
// SPDX-License-Identifier: BSD-3-Clause

// The DCEL tests that run on a device: the query surface of MeshT, VertexT, EdgeT, FaceT and EdgeIteratorT through
// GPU::transform(), and DCEL::reconcile() of a moved mesh where its pool is. A GPU build compiles this file with the
// device compiler; the host-only DCEL tests are in TestDCEL.

#include "EBGeometry.hpp"
#include "TestDCELHelpers.hpp"
#include "TestDeath.hpp"
#include "TestFloatingPointUtils.hpp"
#include "TestGPU.hpp"
#include "TestUtilities.hpp"

#include <array>
#include <cstdint>
#include <cstring>
#include <initializer_list>
#include <memory>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

#include <catch2/catch_template_test_macros.hpp>
#include <catch2/matchers/catch_matchers_exception.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

using namespace EBGeometry;
using namespace EBGeometry::DCEL;
using namespace EBGeometryTestDCEL;
using Catch::Matchers::WithinRel;

// ─────────────────────────────────────────────────────────────────────────────
// Device: the DCEL query surface (MeshT/VertexT/EdgeT/FaceT/EdgeIteratorT) is
// callable from a kernel and matches the host
// ─────────────────────────────────────────────────────────────────────────────

// Face m_face's signed distance to each query point (FaceT::signedDistance, which runs the face's
// current inside/outside algorithm).
template <class T>
struct FaceSignedDistanceQuery
{
  TestMesh<T> m_mesh;
  uint32_t    m_face;

  EBGEOMETRY_HOST_DEVICE
  T
  operator()(const Vec3T<T>& a_point) const noexcept
  {
    return m_mesh.getFace(m_face).signedDistance(a_point, m_mesh);
  }
};

// The mesh's own signed distance to each query point: MeshT's algorithm-dispatching entry point
// (Direct2 by default), which used to be EBGEOMETRY_HOST-only because of a std::cerr call in its
// switch's defensive default case (see the class-level note in EBGeometry_DCEL_Mesh.hpp).
template <class T>
struct MeshSignedDistanceQuery
{
  TestMesh<T> m_mesh;

  EBGEOMETRY_HOST_DEVICE
  T
  operator()(const Vec3T<T>& a_point) const noexcept
  {
    return m_mesh.signedDistance(a_point);
  }
};

// The mesh's squared unsigned distance to each query point.
template <class T>
struct MeshUnsignedDistance2Query
{
  TestMesh<T> m_mesh;

  EBGEOMETRY_HOST_DEVICE
  T
  operator()(const Vec3T<T>& a_point) const noexcept
  {
    return m_mesh.unsignedDistance2(a_point);
  }
};

// The number of half-edges EdgeIteratorT visits around each queried face.
template <class T>
struct EdgeLoopLengthQuery
{
  TestMesh<T> m_mesh;

  EBGEOMETRY_HOST_DEVICE
  uint32_t
  operator()(const uint32_t& a_face) const noexcept
  {
    uint32_t count = 0;

    for (TestEdgeIterator<T> it(m_mesh, m_mesh.getFace(a_face)); it.ok(); ++it) {
      ++count;
    }

    return count;
  }
};

// The m_rank-th half-edge EdgeIteratorT visits around each queried face (UINT32_MAX past the end).
template <class T>
struct EdgeLoopEdgeQuery
{
  TestMesh<T> m_mesh;
  uint32_t    m_rank;

  EBGEOMETRY_HOST_DEVICE
  uint32_t
  operator()(const uint32_t& a_face) const noexcept
  {
    uint32_t rank = 0;

    for (TestEdgeIterator<T> it(m_mesh, m_mesh.getFace(a_face)); it.ok(); ++it) {
      if (rank == m_rank) {
        return it();
      }

      ++rank;
    }

    return UINT32_MAX;
  }
};

// Component m_axis of each queried face's highest (m_high) or smallest vertex coordinate. These
// reduce componentwise over the half-edge loop without materializing a std::vector, which is what
// keeps them callable from a kernel.
template <class T>
struct FaceExtentQuery
{
  TestMesh<T> m_mesh;
  size_t      m_axis;
  bool        m_high;

  EBGEOMETRY_HOST_DEVICE
  T
  operator()(const uint32_t& a_face) const noexcept
  {
    const TestFace<T>& face = m_mesh.getFace(a_face);

    return m_high ? face.getHighestCoordinate(m_mesh)[m_axis] : face.getSmallestCoordinate(m_mesh)[m_axis];
  }
};

// Component m_axis of each queried vertex's position.
template <class T>
struct VertexPositionQuery
{
  TestMesh<T> m_mesh;
  size_t      m_axis;

  EBGEOMETRY_HOST_DEVICE
  T
  operator()(const uint32_t& a_vertex) const noexcept
  {
    return m_mesh.getVertex(a_vertex).getPosition()[m_axis];
  }
};

// Selects m_algorithm on every face, then returns face 0's signed distance to the query point. It
// writes pool-resident face data that every thread would share, so it is run on a single query.
template <class T>
struct SetInsideOutsideAlgorithmQuery
{
  TestMesh<T>            m_mesh;
  InsideOutsideAlgorithm m_algorithm;

  EBGEOMETRY_HOST_DEVICE
  T
  operator()(const Vec3T<T>& a_point) const noexcept
  {
    TestMesh<T> mesh = m_mesh;

    mesh.setInsideOutsideAlgorithm(m_algorithm);

    return mesh.getFace(0).signedDistance(a_point, mesh);
  }
};

// Flips every normal, then returns face 0's signed distance to the query point. Like the one above,
// it writes shared pool-resident data, so it is run on a single query.
template <class T>
struct FlipQuery
{
  TestMesh<T> m_mesh;

  EBGEOMETRY_HOST_DEVICE
  T
  operator()(const Vec3T<T>& a_point) const noexcept
  {
    TestMesh<T> mesh = m_mesh;

    mesh.flip();

    return mesh.getFace(0).signedDistance(a_point, mesh);
  }
};

TEMPLATE_TEST_CASE("MeshT/VertexT/EdgeT/FaceT/EdgeIteratorT: device query surface matches the host",
                   "[DCEL][gpu]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  using namespace EBGeometryTestGPU;

  if (!deviceAvailable()) {
    SKIP("no GPU device available");
  }

  Pool hostPool(hostMemoryResource());
  auto mesh = buildTetrahedron<T>(hostPool);

  hostPool.freeze();

  Pool              devicePool = Pool::mirror(hostPool, deviceTestResource());
  const TestMesh<T> device     = mesh->rebasedView(devicePool);
  const TestMesh<T> host       = *mesh;

  // The tetrahedron spans [0, 1] on each axis, so this grid covers points inside, outside, and near
  // every face.
  const auto points = queryGrid<T>(Vec3T<T>(T(-0.5), T(-0.5), T(-0.5)), Vec3T<T>(T(1.5), T(1.5), T(1.5)), 10);

  // The mutating functors run on this one point only.
  const std::vector<Vec3T<T>> mutationPoint = {Vec3T<T>(T(2), T(2), T(2))};

  std::vector<uint32_t> faces;
  std::vector<uint32_t> vertices;

  for (uint32_t i = 0; i < mesh->numFaces(); i++) {
    faces.push_back(i);
  }

  for (uint32_t i = 0; i < mesh->numVertices(); i++) {
    vertices.push_back(i);
  }

  // Integer results must match exactly.
  const auto requireSameIndices = [](const std::vector<uint32_t>& a_device, const std::vector<uint32_t>& a_host) {
    REQUIRE(a_device.size() == a_host.size());

    for (size_t i = 0; i < a_host.size(); i++) {
      INFO("query " << i);
      REQUIRE(a_device[i] == a_host[i]);
    }
  };

  // Every face's signed distance, and the mesh's own. Rerun after each mutation below, since the
  // mutations change what these return.
  const auto requireSameDistances = [&]() {
    for (const uint32_t f : faces) {
      INFO("face " << f);
      requireSameResults(evaluateOnDevice<T>(FaceSignedDistanceQuery<T>{device, f}, points),
                         evaluateOnHost<T>(FaceSignedDistanceQuery<T>{host, f}, points));
    }

    requireSameResults(evaluateOnDevice<T>(MeshSignedDistanceQuery<T>{device}, points),
                       evaluateOnHost<T>(MeshSignedDistanceQuery<T>{host}, points));
  };

  // The unmutated mesh: buildTetrahedron leaves every face at the default CrossingNumber.
  requireSameDistances();
  requireSameResults(evaluateOnDevice<T>(MeshUnsignedDistance2Query<T>{device}, points),
                     evaluateOnHost<T>(MeshUnsignedDistance2Query<T>{host}, points));

  // The half-edge loops, walked with EdgeIteratorT (one past the triangle's three edges too, which
  // must come back as UINT32_MAX on both sides).
  requireSameIndices(evaluateOnDevice<uint32_t>(EdgeLoopLengthQuery<T>{device}, faces),
                     evaluateOnHost<uint32_t>(EdgeLoopLengthQuery<T>{host}, faces));

  for (uint32_t rank = 0; rank < 4; rank++) {
    INFO("rank " << rank);
    requireSameIndices(evaluateOnDevice<uint32_t>(EdgeLoopEdgeQuery<T>{device, rank}, faces),
                       evaluateOnHost<uint32_t>(EdgeLoopEdgeQuery<T>{host, rank}, faces));
  }

  for (size_t axis = 0; axis < 3; axis++) {
    INFO("axis " << axis);
    requireSameResults(evaluateOnDevice<T>(FaceExtentQuery<T>{device, axis, true}, faces),
                       evaluateOnHost<T>(FaceExtentQuery<T>{host, axis, true}, faces));
    requireSameResults(evaluateOnDevice<T>(FaceExtentQuery<T>{device, axis, false}, faces),
                       evaluateOnHost<T>(FaceExtentQuery<T>{host, axis, false}, faces));
    requireSameResults(evaluateOnDevice<T>(VertexPositionQuery<T>{device, axis}, vertices),
                       evaluateOnHost<T>(VertexPositionQuery<T>{host, axis}, vertices));
  }

  // setInsideOutsideAlgorithm() and the WindingNumber/SubtendedAngle branches of
  // FaceT::isPointInsideFace, applied on the device to the device pool and on the host to the host
  // pool by the same functor.
  for (const auto algorithm : {InsideOutsideAlgorithm::WindingNumber, InsideOutsideAlgorithm::SubtendedAngle}) {
    INFO("algorithm " << static_cast<int>(algorithm));
    requireSameResults(evaluateOnDevice<T>(SetInsideOutsideAlgorithmQuery<T>{device, algorithm}, mutationPoint),
                       evaluateOnHost<T>(SetInsideOutsideAlgorithmQuery<T>{host, algorithm}, mutationPoint));
    requireSameDistances();
  }

  // flip(): negating every normal must negate every face's signed distance, on the device as on the
  // host.
  std::vector<T> flipped;

  for (const uint32_t f : faces) {
    for (const T d : evaluateOnDevice<T>(FaceSignedDistanceQuery<T>{device, f}, points)) {
      flipped.push_back(-d);
    }
  }

  requireSameResults(evaluateOnDevice<T>(FlipQuery<T>{device}, mutationPoint),
                     evaluateOnHost<T>(FlipQuery<T>{host}, mutationPoint));
  requireSameDistances();

  std::vector<T> afterFlip;

  for (const uint32_t f : faces) {
    for (const T d : evaluateOnDevice<T>(FaceSignedDistanceQuery<T>{device, f}, points)) {
      afterFlip.push_back(d);
    }
  }

  requireSameResults(afterFlip, flipped);
}

// ─────────────────────────────────────────────────────────────────────────────
// DCEL::reconcile: a mesh whose vertices moved, reconciled where its pool is
// ─────────────────────────────────────────────────────────────────────────────

namespace {

// Moves every vertex: a rotation about z, a stretch and a shift, so every normal, centroid and area changes.
template <class T>
struct MoveVertices
{
  mutable TestMesh<T> m_mesh;

  EBGEOMETRY_HOST_DEVICE
  void
  operator()(const std::size_t a_i) const noexcept
  {
    auto&          vertex = m_mesh.getVertex(static_cast<uint32_t>(a_i));
    const Vec3T<T> x      = vertex.getPosition();

    vertex.setPosition(
      Vec3T<T>(T(1.3) * x[0] - T(0.4) * x[1] + T(0.2), T(0.4) * x[0] + T(0.9) * x[1], T(0.7) * x[2] - T(1)));
  }
};

template <class T>
void
moveOnHost(const TestMesh<T>& a_mesh)
{
  const MoveVertices<T> move{a_mesh};

  for (uint32_t v = 0; v < a_mesh.numVertices(); v++) {
    move(v);
  }
}

// Every quantity a reconcile writes, compared exactly.
template <class T>
void
requireSameReconcile(const TestMesh<T>& a_got, const TestMesh<T>& a_want)
{
  for (uint32_t f = 0; f < a_want.numFaces(); f++) {
    INFO("face " << f);

    REQUIRE(a_got.getFace(f).getNormal() == a_want.getFace(f).getNormal());
    REQUIRE(a_got.getFace(f).getCentroid() == a_want.getFace(f).getCentroid());
    REQUIRE(a_got.getFace(f).getArea() == a_want.getFace(f).getArea());
  }

  for (uint32_t e = 0; e < a_want.numEdges(); e++) {
    INFO("edge " << e);

    REQUIRE(a_got.getEdge(e).getNormal() == a_want.getEdge(e).getNormal());
  }

  for (uint32_t v = 0; v < a_want.numVertices(); v++) {
    INFO("vertex " << v);

    REQUIRE(a_got.getVertex(v).getNormal() == a_want.getVertex(v).getNormal());
  }
}

// The same, to within a margin: a device fuses multiplies and adds, and its std::acos differs in the last bits.
template <class T>
void
requireCloseReconcile(const TestMesh<T>& a_got, const TestMesh<T>& a_want)
{
  const auto requireClose = [](const Vec3T<T>& a_a, const Vec3T<T>& a_b) {
    for (size_t d = 0; d < 3; d++) {
      REQUIRE_THAT(a_a[d], withinAbsT(a_b[d], tightMargin<T>()));
    }
  };

  for (uint32_t f = 0; f < a_want.numFaces(); f++) {
    INFO("face " << f);

    requireClose(a_got.getFace(f).getNormal(), a_want.getFace(f).getNormal());
    requireClose(a_got.getFace(f).getCentroid(), a_want.getFace(f).getCentroid());
    REQUIRE_THAT(a_got.getFace(f).getArea(), withinAbsT(a_want.getFace(f).getArea(), tightMargin<T>()));
  }

  for (uint32_t e = 0; e < a_want.numEdges(); e++) {
    INFO("edge " << e);

    requireClose(a_got.getEdge(e).getNormal(), a_want.getEdge(e).getNormal());
  }

  for (uint32_t v = 0; v < a_want.numVertices(); v++) {
    INFO("vertex " << v);

    requireClose(a_got.getVertex(v).getNormal(), a_want.getVertex(v).getNormal());
  }
}

} // namespace

TEMPLATE_TEST_CASE("DCEL::reconcile: a host pool's mesh, after its vertices moved, gets MeshT::reconcile()'s values "
                   "byte for byte",
                   "[DCEL][Mesh][reconcile]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  // Triangles and quads, convex and concave, closed and open, and a sliver repaired by the reader.
  for (const char* name : {"dodecahedron.stl",
                           "dodecahedron.obj",
                           "dodecahedron.ply",
                           "dodecahedron.vtk",
                           "cube_quads.obj",
                           "cube_open_top.obj",
                           "lblock.stl",
                           "notch.stl",
                           "tetrahedron.stl",
                           "tetrahedron_sliver.stl"}) {
    INFO(name);

    Pool pool(hostMemoryResource());

    const TestMesh<T>             mesh = Parser::readIntoDCEL<T>(dataPath(name), pool);
    const DCEL::ReconcileSchedule schedule(pool, mesh);

    pool.freeze();

    Pool reference = Pool::mirror(pool, hostMemoryResource());

    TestMesh<T> want = mesh.rebasedView(reference);

    moveOnHost(want);
    want.reconcile();

    moveOnHost(mesh);
    DCEL::reconcile(pool, mesh, schedule);

    requireSameReconcile(mesh, want);
  }
}

TEMPLATE_TEST_CASE("DCEL::reconcile: a stretched cube gets its face areas, axis normals and corner normals",
                   "[DCEL][Mesh][reconcile]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  Pool pool(hostMemoryResource());

  TestMesh<T>                   mesh = Parser::readIntoDCEL<T>(dataPath("cube_quads.obj"), pool);
  const DCEL::ReconcileSchedule schedule(pool, mesh);

  // The unit cube stretched to 2 x 3 x 4: every angle stays a right angle.
  for (uint32_t v = 0; v < mesh.numVertices(); v++) {
    const Vec3T<T> x = mesh.getVertex(v).getPosition();

    mesh.getVertex(v).setPosition(Vec3T<T>(T(2) * x[0], T(3) * x[1], T(4) * x[2]));
  }

  DCEL::reconcile(pool, mesh, schedule);

  for (uint32_t f = 0; f < mesh.numFaces(); f++) {
    const auto& face = mesh.getFace(f);
    const auto& n    = face.getNormal();

    // The axis the normal points along, and the area of the face across it.
    size_t axis = 0;

    for (size_t d = 1; d < 3; d++) {
      axis = (std::abs(n[d]) > std::abs(n[axis])) ? d : axis;
    }

    const T areas[3] = {T(12), T(8), T(6)};

    REQUIRE_THAT(std::abs(n[axis]), withinAbsT(T(1), tightMargin<T>()));
    REQUIRE_THAT(face.getArea(), withinAbsT(areas[axis], tightMargin<T>()));
  }

  // Each corner's normal points out along the diagonal, whatever the stretch.
  const T s = T(1) / std::sqrt(T(3));

  for (uint32_t v = 0; v < mesh.numVertices(); v++) {
    const Vec3T<T>& x = mesh.getVertex(v).getPosition();
    const Vec3T<T>& n = mesh.getVertex(v).getNormal();

    for (size_t d = 0; d < 3; d++) {
      REQUIRE_THAT(n[d], withinAbsT((x[d] > T(0)) ? s : -s, tightMargin<T>()));
    }
  }
}

TEMPLATE_TEST_CASE("DCEL::reconcile: a mirrored mesh, after the caller's own kernel moved its vertices, is reconciled "
                   "on the device to the host's values",
                   "[DCEL][Mesh][reconcile][gpu]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  using namespace EBGeometryTestGPU;

  if (!deviceAvailable()) {
    SKIP("no GPU device available");
  }

  for (const char* name : {"dodecahedron.stl", "cube_quads.obj", "cube_open_top.obj", "notch.stl"}) {
    INFO(name);

    Pool pool(hostMemoryResource());

    TestMesh<T>                   mesh = Parser::readIntoDCEL<T>(dataPath(name), pool);
    const DCEL::ReconcileSchedule schedule(pool, mesh);

    pool.freeze();

    Pool devicePool = Pool::mirror(pool, deviceTestResource());

    // The caller's kernel moves the vertices in device memory; DCEL::reconcile() then reads them there.
    GPU::parallelFor(mesh.numVertices(), MoveVertices<T>{mesh.rebasedView(devicePool)});

    DCEL::reconcile(devicePool, mesh, schedule);

    moveOnHost(mesh);
    mesh.reconcile();

    const Pool back = Pool::mirror(devicePool, hostMemoryResource());

    requireCloseReconcile(mesh.rebasedView(back), mesh);
  }
}

TEMPLATE_TEST_CASE("DCEL::reconcile: a small face far from the origin keeps its normal",
                   "[DCEL][Mesh][regression]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  // A right triangle with legs of 0.1, a thousand units from the origin. With the Newell sums taken over absolute
  // positions, each term was of the order of 1e6 while the sum was of the order of 1e-2: in single precision the
  // normal came out wrong in its first digit.
  const Vec3T<T> origin(T(1000), T(-700), T(1200));

  Pool        pool(hostMemoryResource());
  TestMesh<T> mesh;

  mesh.reserveVertices(pool, 3);
  mesh.reserveEdges(pool, 3);
  mesh.reserveFaces(pool, 1);

  (void)mesh.addVertex(pool, TestVertex<T>(origin));
  (void)mesh.addVertex(pool, TestVertex<T>(origin + Vec3T<T>(T(0.1), T(0), T(0))));
  (void)mesh.addVertex(pool, TestVertex<T>(origin + Vec3T<T>(T(0), T(0.1), T(0))));

  for (uint32_t e = 0; e < 3; e++) {
    TestEdge<T> edge(e);

    edge.setNextEdge((e + 1) % 3);
    edge.setFace(0);

    (void)mesh.addEdge(pool, edge);
  }

  (void)mesh.addFace(pool, TestFace<T>(0u));

  mesh.reconcile();

  const auto& face = mesh.getFace(0);

  // The triangle lies in a plane z = const, so its normal is +z exactly; its legs are rounded where they were stored
  // (to about 6e-5 in single precision), which the area's margin allows for.
  REQUIRE_THAT(face.getNormal()[0], withinAbsT(T(0), looseMargin<T>()));
  REQUIRE_THAT(face.getNormal()[1], withinAbsT(T(0), looseMargin<T>()));
  REQUIRE_THAT(face.getNormal()[2], withinAbsT(T(1), looseMargin<T>()));
  REQUIRE_THAT(face.getArea(), withinAbsT(T(0.005), 2e-5));
}

TEMPLATE_TEST_CASE("DCEL::ReconcileSchedule and DCEL::reconcile reject a frozen or wrong pool, another mesh, bad "
                   "topology and coincident positions",
                   "[DCEL][Mesh][reconcile][death]",
                   EBGEOMETRY_TEST_PRECISIONS)
{
  using T = TestType;

  const auto tetrahedron = [](Pool& a_pool) { return Parser::readIntoDCEL<T>(dataPath("tetrahedron.stl"), a_pool); };

  REQUIRE(abortsWith(
    [&] {
      Pool pool(hostMemoryResource());

      const auto mesh = tetrahedron(pool);

      pool.freeze();

      (void)DCEL::ReconcileSchedule(pool, mesh);
    },
    "DCEL::ReconcileSchedule: the pool is frozen; build the schedule after the mesh and before freezing the pool"));

  REQUIRE(abortsWith(
    [&] {
      Pool pool(hostMemoryResource());
      Pool other(hostMemoryResource());

      const auto mesh = tetrahedron(pool);

      (void)DCEL::ReconcileSchedule(other, mesh);
    },
    "DCEL::ReconcileSchedule: the pool must be the one the mesh lives in"));

  // The topology checks, with MeshT::reconcile()'s messages.
  REQUIRE(abortsWith(
    [] {
      Pool        pool(hostMemoryResource());
      TestMesh<T> mesh;

      buildLinkedEdges(mesh, pool, {1, 2, 1});

      (void)DCEL::ReconcileSchedule(pool, mesh);
    },
    "DCEL::MeshT::reconcile: the half-edge loop of face 0 does not close (it reaches edge 1 after 3 steps"));

  // A loop 0 -> 1 -> 2 -> 0 -> 3 visits vertex 0 twice.
  REQUIRE(abortsWith(
    [] {
      Pool        pool(hostMemoryResource());
      TestMesh<T> mesh;

      const std::vector<Vec3T<T>> positions = {{0, 0, 0}, {1, 0, 0}, {0, 1, 0}, {0, -1, 0}};
      const std::vector<uint32_t> loop      = {0, 1, 2, 0, 3};

      mesh.reserveVertices(pool, static_cast<uint32_t>(positions.size()));
      mesh.reserveEdges(pool, static_cast<uint32_t>(loop.size()));
      mesh.reserveFaces(pool, 1);

      for (const auto& x : positions) {
        (void)mesh.addVertex(pool, TestVertex<T>(x));
      }

      for (uint32_t e = 0; e < loop.size(); e++) {
        TestEdge<T> edge(loop[e]);

        edge.setNextEdge((e + 1) % static_cast<uint32_t>(loop.size()));
        edge.setFace(0);

        (void)mesh.addEdge(pool, edge);
      }

      (void)mesh.addFace(pool, TestFace<T>(0u));

      (void)DCEL::ReconcileSchedule(pool, mesh);
    },
    "DCEL::VertexT::computeVertexNormalAngleWeighted: face 0 must visit vertex 0 exactly once (found 4 incident "
    "half-edges instead of 2)"));

  // A schedule from another mirror chain, of another size, or of another mesh of the same size in the pool.
  REQUIRE(abortsWith(
    [&] {
      Pool pool(hostMemoryResource());
      Pool other(hostMemoryResource());

      const auto mesh      = tetrahedron(pool);
      const auto otherMesh = tetrahedron(other);

      const DCEL::ReconcileSchedule schedule(other, otherMesh);

      DCEL::reconcile(pool, mesh, schedule);
    },
    "DCEL::reconcile: the schedule was built in a pool of another mirror chain; build it in the mesh's pool"));

  REQUIRE(abortsWith(
    [&] {
      Pool pool(hostMemoryResource());

      const auto mesh = tetrahedron(pool);
      const auto cube = Parser::readIntoDCEL<T>(dataPath("cube_quads.obj"), pool);

      const DCEL::ReconcileSchedule schedule(pool, cube);

      DCEL::reconcile(pool, mesh, schedule);
    },
    "DCEL::reconcile: the schedule covers 8 vertices, 24 half-edges and 6 faces but the mesh has 4, 12 and 4; build "
    "the schedule for this mesh"));

  REQUIRE(abortsWith(
    [&] {
      Pool pool(hostMemoryResource());

      const auto mesh = tetrahedron(pool);
      const auto twin = tetrahedron(pool);

      const DCEL::ReconcileSchedule schedule(pool, twin);

      DCEL::reconcile(pool, mesh, schedule);
    },
    "DCEL::reconcile: the schedule was built for another mesh in this pool; build the schedule for this mesh"));

  // Coincident positions in the mesh's own pool, not frozen: found in place.
  REQUIRE(abortsWith(
    [&] {
      Pool        pool(hostMemoryResource());
      TestMesh<T> mesh = Parser::readIntoDCEL<T>(dataPath("cube_quads.obj"), pool);

      const DCEL::ReconcileSchedule schedule(pool, mesh);

      mesh.getVertex(1).setPosition(mesh.getVertex(0).getPosition());

      DCEL::reconcile(pool, mesh, schedule);
    },
    "DCEL::VertexT::computeVertexNormalAngleWeighted: face 2 has coincident positions among vertices 0, 4 and 1"));

  // Coincident positions after a move, on a host pool and on a device-accessible one. The death tests run in a
  // forked child, where a CUDA or HIP context does not survive, so the device-accessible pool is the emulated one of
  // a host build.
  for (const bool device : {false, true}) {
    if (device && GPU::enabled) {
      continue;
    }

    REQUIRE(abortsWith(
      [&] {
        Pool pool(hostMemoryResource());

        const auto mesh = Parser::readIntoDCEL<T>(dataPath("cube_quads.obj"), pool);

        const DCEL::ReconcileSchedule schedule(pool, mesh);

        pool.freeze();

        // Vertex 1 onto vertex 0, in a host copy, which the pool reconciled is mirrored from. MeshT::reconcile()
        // reports the same face and vertices.
        Pool        moved = Pool::mirror(pool, hostMemoryResource());
        TestMesh<T> view  = mesh.rebasedView(moved);

        view.getVertex(1).setPosition(view.getVertex(0).getPosition());

        Pool target = Pool::mirror(moved, device ? EBGeometryTestGPU::deviceTestResource() : hostMemoryResource());

        DCEL::reconcile(target, mesh, schedule);
      },
      "DCEL::VertexT::computeVertexNormalAngleWeighted: face 2 has coincident positions among vertices 0, 4 and 1"));
  }
}
