// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

// A shape of your own, composed with built-in shapes into a Tape, and called from a GPU kernel written
// out by hand. The same source builds as plain C++ (the default) or for a GPU.
//
// The shape is an octahedron, written below as an ordinary struct: trivially copyable, with an
// EBGEOMETRY_HOST_DEVICE signedDistance() and computeBoundingVolume(). Twelve rotated copies on a ring
// around a sphere are compiled into a Tape<T, Octahedron<T>> (the octahedron is listed because it is
// not a built-in type). The tape is built directly in memory that the host and the device can both
// read, so one handle serves both and nothing is copied.
//
// The other examples run their queries through EBGeometry::GPU::transform(), which launches a kernel
// for them. This one writes that kernel out, because a kernel of your own is how an application calls
// EBGeometry from inside its own device loops (a Kokkos parallel_for, a CUDA kernel that does
// other work too). See README.md.

#include <cstddef>
#include <iostream>
#include <type_traits>
#include <vector>

#include <EBGeometry.hpp>

// Floating-point precision. Overridable from CMake (-DEBGEOMETRY_PRECISION=float).
#ifndef EBGEOMETRY_PRECISION
#define EBGEOMETRY_PRECISION double
#endif

using T = EBGEOMETRY_PRECISION;

using Vec3 = EBGeometry::Vec3T<T>;

// ── A shape of your own ──────────────────────────────────────────────────────────────────────────────
//
// A regular octahedron centered at the origin, with its vertices at distance s along each axis: the set
// |x| + |y| + |z| <= s. The distance is exact (after Inigo Quilez's formula). Like every EBGeometry
// shape, it is negative inside and positive outside. Inside an EBGEOMETRY_HOST_DEVICE function, use
// EBGeometry::Math::min/max/clamp rather than std::min/max/clamp, which nvcc does not accept in device
// code without extra flags; std::abs and std::sqrt are fine.
template <class P>
struct Octahedron
{
  P m_size = P(1);

  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  P
  signedDistance(const EBGeometry::Vec3T<P>& a_point) const noexcept
  {
    const P x = std::abs(a_point[0]);
    const P y = std::abs(a_point[1]);
    const P z = std::abs(a_point[2]);
    const P m = x + y + z - m_size;

    // Order the coordinates so that the one used below comes first.
    P q0;
    P q1;
    P q2;

    if (P(3) * x < m) {
      q0 = x;
      q1 = y;
      q2 = z;
    }
    else if (P(3) * y < m) {
      q0 = y;
      q1 = z;
      q2 = x;
    }
    else if (P(3) * z < m) {
      q0 = z;
      q1 = x;
      q2 = y;
    }
    else {
      // The closest point is on a face.
      return m / std::sqrt(P(3));
    }

    // The closest point is on an edge or a vertex.
    const P k  = EBGeometry::Math::clamp(P(0.5) * (q2 - q1 + m_size), P(0), m_size);
    const P d0 = q0;
    const P d1 = q1 - m_size + k;
    const P d2 = q2 - k;

    return std::sqrt(d0 * d0 + d1 * d1 + d2 * d2);
  }

  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  EBGeometry::BoundingVolumes::AABBT<P>
  computeBoundingVolume() const noexcept
  {
    return EBGeometry::BoundingVolumes::AABBT<P>(-m_size * EBGeometry::Vec3T<P>::ones(),
                                                 m_size * EBGeometry::Vec3T<P>::ones());
  }

  // The value is the Euclidean distance to the surface, so BVH unions can prune on it.
  static constexpr EBGeometry::DistanceQuality distanceQuality = EBGeometry::DistanceQuality::Exact;
};

static_assert(EBGeometry::isShape<Octahedron<T>>, "Octahedron must satisfy EBGeometry's shape requirements");

// The tape knows the built-in shapes; any other shape type is listed after T.
using Shapes = EBGeometry::Tape<T, Octahedron<T>>;

// Run configuration.
constexpr int numOctahedra  = 12;
constexpr T   ringRadius    = T(1.5);
constexpr T   octahedronR   = T(0.25); // vertex distance of each octahedron
constexpr T   sphereRadius  = T(0.75);
constexpr int gridPerAxis   = 40;
constexpr T   gridHalfWidth = T(2);

namespace {

// ── The code that runs on either side ────────────────────────────────────────────────────────────────
//
// The composed object's value at one point. A query is a trivially copyable functor whose call
// operator is EBGEOMETRY_HOST_DEVICE; it holds the tape by value, and a kernel receives it by value.
struct DistanceQuery
{
  Shapes m_shapes;

  EBGEOMETRY_HOST_DEVICE
  T
  operator()(const Vec3& a_point) const noexcept
  {
    return m_shapes.signedDistance(a_point);
  }
};

// ── A kernel of your own ─────────────────────────────────────────────────────────────────────────────
//
// One thread per point: a_out[i] = a_query(a_in[i]). This is the kernel GPU::parallelFor() and
// GPU::transform() launch for the other examples. A kernel exists only when the file is compiled as
// CUDA or HIP; in a plain C++ build, runQuery() below does what each thread would do, in a loop.
#if defined(EBGEOMETRY_CUDA) || defined(EBGEOMETRY_HIP)
template <class Query, class In, class Out>
EBGEOMETRY_GLOBAL
void
queryKernel(const Query a_query, const In* a_in, Out* a_out, const unsigned int a_n)
{
  const unsigned int i = blockIdx.x * blockDim.x + threadIdx.x;

  if (i < a_n) {
    a_out[i] = a_query(a_in[i]);
  }
}
#endif

// Run the query on every point. The points and results are in managed memory, which the kernel reads
// and writes directly; EBGEOMETRY_GPU_CHECK aborts with the failing call on a runtime error.
template <class Query>
void
runQuery(const Query& a_query, const Vec3* a_points, T* a_results, const unsigned int a_n)
{
  static_assert(std::is_trivially_copyable_v<Query>, "a query is passed to the kernel by value");

#if defined(EBGEOMETRY_CUDA) || defined(EBGEOMETRY_HIP)
  constexpr unsigned int threads = 128;

  queryKernel<<<(a_n + threads - 1) / threads, threads>>>(a_query, a_points, a_results, a_n);

  EBGEOMETRY_GPU_CHECK(EBGeometry::GPU::getLastError());
  EBGEOMETRY_GPU_CHECK(EBGeometry::GPU::deviceSynchronize());
#else
  for (unsigned int i = 0; i < a_n; i++) {
    a_results[i] = a_query(a_points[i]);
  }
#endif
}

// Center of octahedron i on the ring, in the xz-plane.
Vec3
ringCenter(const int a_index)
{
  const T angle = T(2) * EBGeometry::pi<T> * T(a_index) / T(numOctahedra);

  return {ringRadius * std::cos(angle), T(0), ringRadius * std::sin(angle)};
}

} // namespace

int
main(int argc, char* argv[])
{
  std::cout << "CustomShape: a user-written octahedron in a tape\n\n";

  // ── Build the scene ────────────────────────────────────────────────────────────────────────────────
  //
  // Twelve octahedra on a ring around a sphere, each turned about its own vertical axis before it is
  // moved into place. A union of eight or more operands is evaluated with a BVH over their boxes.
  const Octahedron<T> octahedron{octahedronR};

  std::vector<EBGeometry::Expr<T>> objects;

  for (int i = 0; i < numOctahedra; i++) {
    const T degrees = T(360) * T(i) / T(numOctahedra);

    objects.push_back(EBGeometry::Translate(EBGeometry::Rotate(octahedron, degrees, 1), ringCenter(i)));
  }

  objects.emplace_back(EBGeometry::SphereSDF<T>(Vec3::zeros(), sphereRadius));

  const EBGeometry::Expr<T> scene = EBGeometry::Union(objects);

  // The tape is compiled straight into memory the device can read. In a GPU build that is managed
  // memory, which migrates between host and device as it is touched and suits a small object like this
  // one (a few KB); for large geometry, build in host memory and mirror the pool to device memory, as
  // the other examples do. In a plain C++ build, or a GPU build that finds no device, it is host memory
  // that stands in for a device, and the same lines run. The pool must outlive every evaluation.
#if defined(EBGEOMETRY_CUDA) || defined(EBGEOMETRY_HIP)
  EBGeometry::MemoryResource& resource =
    EBGeometry::GPU::deviceAvailable()
      ? static_cast<EBGeometry::MemoryResource&>(EBGeometry::managedMemoryResource())
      : static_cast<EBGeometry::MemoryResource&>(EBGeometry::emulatedDeviceMemoryResource());
#else
  EBGeometry::MemoryResource& resource = EBGeometry::emulatedDeviceMemoryResource();
#endif

  EBGeometry::Pool pool(resource);
  const Shapes     tape(pool, scene);

  // Freezing fixes the pool's base, and a view onto the pool's own memory holds that base by value, so
  // a kernel can use it as well as the host. (The handle the constructor returned finds the pool
  // through a host-side control block, which a kernel cannot read.)
  pool.freeze();

  const Shapes view = tape.rebasedView(pool);

  std::cout << "Objects    : " << objects.size() << " (" << numOctahedra << " octahedra and a sphere)\n";
  std::cout << "BVH unions : " << view.getNumBVHUnions() << "\n";
  std::cout << "Clauses    : " << view.getNumClauses() << "\n\n";

  // ── Use it on the host ─────────────────────────────────────────────────────────────────────────────
  //
  // The view is an ordinary shape on the host: negative inside, positive outside.
  const DistanceQuery query{view};

  std::cout << "On the host:\n";
  std::cout << "  center of the sphere       : " << query(Vec3::zeros()) << '\n';
  std::cout << "  center of an octahedron    : " << query(ringCenter(0)) << '\n';
  std::cout << "  above an octahedron's tip  : " << query(ringCenter(0) + Vec3(T(0), T(2) * octahedronR, T(0))) << '\n';
  std::cout << "  between two octahedra      : " << query(T(0.5) * (ringCenter(0) + ringCenter(1))) << "\n\n";

  // ── The same query in your own kernel ──────────────────────────────────────────────────────────────
  //
  // The points and the results are in managed memory too, so the kernel reads and writes them in place.
  if (EBGeometry::GPU::enabled && !EBGeometry::GPU::deviceAvailable()) {
    std::cout << "On the device: skipped, no GPU is visible\n";
  }
  else {
    const unsigned int n = gridPerAxis * gridPerAxis * gridPerAxis;

    EBGeometry::Pool            queryPool(resource);
    EBGeometry::PODVector<Vec3> points;
    EBGeometry::PODVector<T>    results;

    points.reserveFrom(queryPool, n);
    results.reserveFrom(queryPool, n);

    const T step = T(2) * gridHalfWidth / T(gridPerAxis);

    // Offsets that are not round numbers keep grid points off the octahedra's faces.
    for (int i = 0; i < gridPerAxis; i++) {
      for (int j = 0; j < gridPerAxis; j++) {
        for (int k = 0; k < gridPerAxis; k++) {
          points.push_back(queryPool.base(),
                           Vec3(-gridHalfWidth + (T(i) + T(0.37)) * step,
                                -gridHalfWidth + (T(j) + T(0.53)) * step,
                                -gridHalfWidth + (T(k) + T(0.61)) * step));
          results.push_back(queryPool.base(), T(0));
        }
      }
    }

    queryPool.freeze();

    runQuery(query, points.data(queryPool.base()), results.data(queryPool.base()), n);

    std::size_t inside = 0;

    for (unsigned int i = 0; i < n; i++) {
      inside += results.at(queryPool.base(), i) < T(0) ? 1 : 0;
    }

    std::cout << (EBGeometry::GPU::enabled ? "In our own kernel" : "In a host loop (plain C++ build)") << ": " << n
              << " grid points, " << inside << " inside the scene\n";
  }

  // ── Look at it ─────────────────────────────────────────────────────────────────────────────────────
  //
  // A window on the octahedra around the sphere, a tape with a shape of your own (built with VIEWER=SDL2;
  // otherwise, with --no-view or without a display, view() prints why and returns). Drag to orbit, scroll to
  // zoom, X cuts it open, P cycles the colouring.
  // Given the pool, view() renders on the GPU in a GPU build (G switches to the CPU and back).
  EBGeometry::Render::view(tape, pool, argc, argv);

  return 0;
}
