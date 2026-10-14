// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

// A shape of your own, composed with built-in shapes into a Tape, evaluated on the host and, in a CUDA
// or HIP build, in a GPU kernel too. The same source builds as plain C++ (the default) or for a GPU.
//
// The shape is an octahedron, written below as an ordinary struct: trivially copyable, with an
// EBGEOMETRY_HOST_DEVICE signedDistance() and computeBoundingVolume(). Twelve rotated copies on a ring
// around a sphere are compiled into a Tape<T, Octahedron<T>> (the octahedron is listed because it is
// not a built-in type). In a GPU build the tape lives in managed memory, so one handle serves the host
// and the device and nothing is copied:
//
//   Pool pool(managedMemoryResource());                   // hostMemoryResource() in a CPU build
//   const Tape<T, Octahedron<T>> tape(pool, expression);
//   pool.freeze();
//   const auto view = tape.rebasedView(pool);             // pass this to a kernel by value
//
// The program checks the values at points where they are known (the centers of the octahedra and of
// the sphere, and points beyond an octahedron's vertex and face) and, in a GPU build, that the device
// and the host agree on a grid of points. The kernel is written out below, to show what
// EBGeometry::GPU::transform() (which the other examples call) does. See README.md.

#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
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

// ── Running a query in a GPU kernel ─────────────────────────────────────────────────────────────────
//
// A query is a trivially copyable functor whose call operator is EBGEOMETRY_HOST_DEVICE. It holds what
// it evaluates by value (here a tape that resolves against memory the device can read), is passed to
// the kernel by value, and maps one input to one result. This is what EBGeometry::GPU::transform()
// does, written out; the other examples call it instead.
namespace {

#if defined(EBGEOMETRY_CUDA) || defined(EBGEOMETRY_HIP)

// Kernel: a_out[i] = a_query(a_in[i]), one thread per input.
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

// Stop with a message if a GPU runtime call failed.
void
check(const EBGeometry::GPU::Error a_error, const char* a_what)
{
  if (a_error != EBGeometry::GPU::Success) {
    std::cerr << "GPU error in " << a_what << ": " << EBGeometry::GPU::getErrorString(a_error) << '\n';
    std::exit(EXIT_FAILURE);
  }
}

// Copy the inputs to the device, run the kernel, and copy the results back.
template <class Out, class In, class Query>
std::vector<Out>
runOnDevice(const Query& a_query, const std::vector<In>& a_inputs)
{
  static_assert(std::is_trivially_copyable_v<Query>, "a query is passed to the kernel by value");

  const auto n = static_cast<unsigned int>(a_inputs.size());

  In*  in  = nullptr;
  Out* out = nullptr;

  check(EBGeometry::GPU::memAlloc(reinterpret_cast<void**>(&in), sizeof(In) * n), "memAlloc");
  check(EBGeometry::GPU::memAlloc(reinterpret_cast<void**>(&out), sizeof(Out) * n), "memAlloc");
  check(EBGeometry::GPU::memcpy(in, a_inputs.data(), sizeof(In) * n, EBGeometry::GPU::MemcpyHostToDevice), "memcpy");

  constexpr unsigned int threads = 128;

  queryKernel<<<(n + threads - 1) / threads, threads>>>(a_query, in, out, n);

  check(EBGeometry::GPU::getLastError(), "kernel launch");
  check(EBGeometry::GPU::deviceSynchronize(), "kernel execution");

  std::vector<Out> results(n);

  check(EBGeometry::GPU::memcpy(results.data(), out, sizeof(Out) * n, EBGeometry::GPU::MemcpyDeviceToHost), "memcpy");
  check(EBGeometry::GPU::memFree(in), "memFree");
  check(EBGeometry::GPU::memFree(out), "memFree");

  return results;
}

#endif

// ── The query ────────────────────────────────────────────────────────────────────────────────────────

// The composed object's value at one point.
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

bool
matches(const T a_value, const T a_expected, const T a_tolerance)
{
  return std::abs(a_value - a_expected) <= a_tolerance * (T(1) + std::abs(a_expected));
}

// Center of octahedron i on the ring, in the xz-plane.
Vec3
ringCenter(const int a_index)
{
  const T angle = T(2) * std::acos(T(-1)) * T(a_index) / T(numOctahedra);

  return {ringRadius * std::cos(angle), T(0), ringRadius * std::sin(angle)};
}

} // namespace

int
main()
{
  std::cout << "CustomShape: a user-written octahedron in a tape\n";
  std::cout << "  Precision T = " << (std::is_same_v<T, float> ? "float" : "double") << "\n\n";

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

  // In a GPU build the tape goes in managed memory, which the host and the device can both read; that
  // suits a small object like this one (a few KB). For large geometry, build in host memory and mirror
  // the pool to device memory instead, as the other examples do. The pool must outlive every
  // evaluation.
#if defined(EBGEOMETRY_CUDA) || defined(EBGEOMETRY_HIP)
  // Managed memory needs a device; without one, the host part still runs from host memory.
  EBGeometry::MemoryResource& resource =
    EBGeometry::GPU::deviceAvailable() ? static_cast<EBGeometry::MemoryResource&>(EBGeometry::managedMemoryResource())
                                       : EBGeometry::hostMemoryResource();
#else
  EBGeometry::MemoryResource& resource = EBGeometry::hostMemoryResource();
#endif

  EBGeometry::Pool pool(resource);
  const Shapes     tape(pool, scene);

  // Freezing fixes the pool's base, and a view onto the pool's own memory holds that base by value, so
  // a kernel can use it as well as the host. (The handle the constructor returned finds the pool
  // through a host-side control block, which a kernel cannot read.)
  pool.freeze();

  const Shapes view = tape.rebasedView(pool);

  std::cout << "  Objects     : " << objects.size() << " (" << numOctahedra << " octahedra and a sphere)\n";
  std::cout << "  BVH unions  : " << view.getNumBVHUnions() << "\n";
  std::cout << "  Clauses     : " << view.getNumClauses() << "\n";

  // Points where the value is known, then a grid over the whole scene.
  struct Known
  {
    Vec3        point;
    T           value;
    const char* where;
  };

  const T sqrt3 = std::sqrt(T(3));

  std::vector<Known> known;

  known.push_back({Vec3::zeros(), -sphereRadius, "the center of the sphere"});

  for (int i = 0; i < numOctahedra; i++) {
    known.push_back({ringCenter(i), -octahedronR / sqrt3, "the center of an octahedron"});
  }

  // Straight above octahedron 0 (along its unrotated y axis): beyond its top vertex.
  known.push_back({ringCenter(0) + Vec3(T(0), T(2) * octahedronR, T(0)), octahedronR, "above a vertex"});

  // Octahedron 0 is not rotated, so the direction (1, 1, 1) from its center meets a face head on.
  const T t = T(0.2);

  known.push_back({ringCenter(0) + t * Vec3::ones(), (T(3) * t - octahedronR) / sqrt3, "beyond a face"});

  std::vector<Vec3> points;

  points.reserve(known.size() + static_cast<std::size_t>(gridPerAxis * gridPerAxis * gridPerAxis));

  for (const Known& k : known) {
    points.push_back(k.point);
  }

  const T step = T(2) * gridHalfWidth / T(gridPerAxis);

  for (int i = 0; i < gridPerAxis; i++) {
    for (int j = 0; j < gridPerAxis; j++) {
      for (int k = 0; k < gridPerAxis; k++) {
        // Offsets that are not round numbers keep grid points off the octahedra's faces.
        points.emplace_back(-gridHalfWidth + (T(i) + T(0.37)) * step,
                            -gridHalfWidth + (T(j) + T(0.53)) * step,
                            -gridHalfWidth + (T(k) + T(0.61)) * step);
      }
    }
  }

  // On the host: the same query, called in a loop.
  const DistanceQuery query{view};

  std::vector<T> values;

  values.reserve(points.size());

  for (const Vec3& p : points) {
    values.push_back(query(p));
  }

  std::size_t failures = 0;

  const T tight = std::is_same_v<T, float> ? T(1.0e-5) : T(1.0e-12);

  for (std::size_t i = 0; i < known.size(); i++) {
    if (!matches(values[i], known[i].value, tight)) {
      std::cout << "  at " << known[i].where << ": " << values[i] << ", expected " << known[i].value << '\n';
      failures++;
    }
  }

  std::size_t inside = 0;

  for (std::size_t i = known.size(); i < points.size(); i++) {
    if (values[i] < T(0)) {
      inside++;
    }
  }

  std::cout << "  Grid points : " << points.size() - known.size() << " (" << inside << " inside)\n";
  std::cout << "  Mismatches  : " << failures << " at the known points\n";

  // On the device, in a CUDA or HIP build: the same query in the kernel above, compared with the host.
  // They agree to rounding rather than bit for bit, since a device compiler may fuse a multiply and an
  // add where the host compiler does not.
#if defined(EBGEOMETRY_CUDA) || defined(EBGEOMETRY_HIP)
  if (!EBGeometry::GPU::deviceAvailable()) {
    std::cout << "  On the device: skipped, no GPU is visible\n";
  }
  else {
    const std::vector<T> onDevice = runOnDevice<T>(query, points);

    const T rounding = std::is_same_v<T, float> ? T(1.0e-4) : T(1.0e-9);

    std::size_t differ = 0;

    for (std::size_t i = 0; i < points.size(); i++) {
      if (!matches(onDevice[i], values[i], rounding)) {
        differ++;
      }
    }

    std::cout << "  On the device (" << EBGeometry::GPU::backendName << "): " << points.size() << " points, " << differ
              << " differ from the host\n";

    failures += differ;
  }
#else
  std::cout << "  On the device: not built (plain C++); compile with GPU=HIP or GPU=CUDA to run it\n";
#endif

  return failures == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
