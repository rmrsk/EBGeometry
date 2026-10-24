// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

// Constructive solid geometry with the expression builder: combining analytic shapes with Union,
// Intersection and Difference, blending them smoothly, moving them with Translate, Rotate and Scale,
// and compiling the result into a Tape that evaluates it on the host and, through the same query, on
// a GPU. Then the same part as a fused composition (Fused::), which the C++ compiler evaluates as it
// would code written by hand. See README.md.

#include <chrono>
#include <cstddef>
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <string>
#include <type_traits>
#include <vector>

#include <EBGeometry.hpp>

// Floating-point precision. Overridable from CMake (-DEBGEOMETRY_PRECISION=float).
#ifndef EBGEOMETRY_PRECISION
#define EBGEOMETRY_PRECISION double
#endif

using T    = EBGEOMETRY_PRECISION;
using Vec3 = EBGeometry::Vec3T<T>;
using Expr = EBGeometry::Expr<T>;
using Tape = EBGeometry::Tape<T>;

namespace {

// ── The code that runs on either side ────────────────────────────────────────────────────────────────
//
// A query for EBGeometry::GPU::transform(): a trivially copyable functor whose EBGEOMETRY_HOST_DEVICE
// call operator maps one point to one result. It holds a shape by value: a Tape, or a fused composition.
// A Tape owns no memory: it is a descriptor of the compiled program in a Pool, so a copy is cheap and a
// kernel can receive it by value once it describes a pool the device can read (see the last section of
// main()). A fused composition of analytic shapes holds its shapes themselves and needs no pool.
template <class Shape>
struct Distance
{
  Shape m_shape;

  EBGEOMETRY_HOST_DEVICE
  T
  operator()(const Vec3& a_point) const noexcept
  {
    return m_shape.signedDistance(a_point);
  }
};

// Microseconds per evaluation of a shape over a list of points, on the host; the sum keeps the compiler
// from skipping the work.
template <class Shape>
double
timePerPoint(const Shape& a_shape, const std::vector<Vec3>& a_points, T& a_sum)
{
  const auto start = std::chrono::steady_clock::now();

  for (const Vec3& p : a_points) {
    a_sum += a_shape.signedDistance(p);
  }

  const auto stop = std::chrono::steady_clock::now();

  return std::chrono::duration<double, std::micro>(stop - start).count() / double(a_points.size());
}

const char*
qualityName(const EBGeometry::DistanceQuality a_quality)
{
  switch (a_quality) {
  case EBGeometry::DistanceQuality::Exact:
    return "exact";
  case EBGeometry::DistanceQuality::Bound:
    return "bound";
  default:
    return "not a distance";
  }
}

// What the compiler made of an object, and its value at a few points.
void
report(const std::string& a_name, const Tape& a_tape, const std::vector<Vec3>& a_points)
{
  std::cout << a_name << ": " << a_tape.getNumClauses() << " clauses in " << a_tape.getNumPrograms() << " program(s), "
            << a_tape.getNumBVHUnions() << " BVH union(s), " << qualityName(a_tape.getDistanceQuality()) << ", box "
            << a_tape.computeBoundingVolume() << '\n';

  for (const Vec3& p : a_points) {
    std::cout << "  " << p << "   " << std::setw(9) << a_tape.signedDistance(p) << '\n';
  }
}

} // namespace

int
main()
{
  // ── Describe the objects ───────────────────────────────────────────────────────────────────────────
  //
  // An Expr describes an object without evaluating it. Any shape converts to one, and the builder
  // functions combine and transform them; nothing is computed until a Tape compiles the description.

  const EBGeometry::BoxSDF<T>    cube(-Vec3::ones(), Vec3::ones());
  const EBGeometry::SphereSDF<T> ball(Vec3::zeros(), T(1.35));

  // Three cylinders through the center, one along each axis.
  const T length = T(1.5);
  const T radius = T(0.55);

  const EBGeometry::CylinderSDF<T> drillX(-length * Vec3(1, 0, 0), length * Vec3(1, 0, 0), radius);
  const EBGeometry::CylinderSDF<T> drillY(-length * Vec3(0, 1, 0), length * Vec3(0, 1, 0), radius);
  const EBGeometry::CylinderSDF<T> drillZ(-length * Vec3(0, 0, 1), length * Vec3(0, 0, 1), radius);

  const std::vector<Expr> drills{drillX, drillY, drillZ};

  // The classic CSG tree: a cube with its corners rounded off by a ball (the intersection), and three
  // holes drilled through it (the difference). Union, Intersection and Difference also take a vector.
  const Expr part = EBGeometry::Difference(EBGeometry::Intersection(cube, ball), EBGeometry::Union(drills));

  // The smooth variants blend over a length instead of meeting at a sharp edge.
  const Expr blob = EBGeometry::SmoothUnion(EBGeometry::SphereSDF<T>(Vec3(T(-0.6), T(0), T(0)), T(0.6)),
                                            EBGeometry::SphereSDF<T>(Vec3(T(0.6), T(0), T(0)), T(0.6)),
                                            T(0.3));

  // Transforms apply to whole expressions: here eight copies of the part, each scaled down, turned about
  // the y axis and moved onto a ring. A union of this many operands is evaluated through a BVH over
  // their bounding boxes, so a query only evaluates the copies near it.
  std::vector<Expr> copies;

  for (int i = 0; i < 8; i++) {
    const T degrees = T(45) * T(i);

    const Expr turned = EBGeometry::Rotate(EBGeometry::Scale(part, T(0.4)), degrees, 1);

    copies.push_back(EBGeometry::Translate(turned,
                                           T(2) * Vec3(std::cos(degrees * EBGeometry::pi<T> / T(180)),
                                                       T(0),
                                                       std::sin(degrees * EBGeometry::pi<T> / T(180)))));
  }

  const Expr ring = EBGeometry::Union(copies);

  // ── Compile them and use them on the host ──────────────────────────────────────────────────────────
  //
  // A Tape compiles an expression into a Pool: one array per primitive type and a list of clauses that
  // refer to them. Each Tape is a descriptor into the pool, which must outlive it and every copy of it.
  EBGeometry::Pool pool(EBGeometry::hostMemoryResource());

  const Tape partTape(pool, part);
  const Tape blobTape(pool, blob);
  const Tape ringTape(pool, ring);

  // signedDistance(point) is negative inside and positive outside. getDistanceQuality() says how far
  // the value can be trusted as a distance: an intersection or a difference of exact shapes is a lower
  // bound on the distance, not the distance itself.
  std::cout << std::fixed << std::setprecision(4);

  report("part", partTape, {Vec3::zeros(), Vec3(T(0.8), T(0.8), T(0)), Vec3(T(0.9), T(0.9), T(0.9)), Vec3(2, 0, 0)});
  report("blob", blobTape, {Vec3::zeros(), Vec3(T(0), T(0.55), T(0)), Vec3(T(1.5), T(0), T(0))});
  report("ring", ringTape, {Vec3::zeros(), Vec3(T(2), T(0.3), T(0)), Vec3(T(2.4), T(0.3), T(0))});

  // ── The same part, fused ───────────────────────────────────────────────────────────────────────────
  //
  // When the structure is known as the program is written, the functions in namespace Fused build it as
  // types instead: the result is a shape whose type spells out the whole tree, and the C++ compiler
  // evaluates it as it would the same solid written out by hand. The functions are the builder's, called
  // qualified (Fused::Union, not Union, which is the builder's and takes shapes too); where the builder
  // takes a vector, they take the operands as arguments. A fused shape's values are the tape's.
  const auto fusedPart =
    EBGeometry::Fused::Difference(EBGeometry::Fused::Intersection(cube, ball), drillX, drillY, drillZ);

  std::cout << "\nThe part through its tape and fused:\n";

  for (const Vec3& p : {Vec3::zeros(), Vec3(T(0.8), T(0.8), T(0)), Vec3(T(0.9), T(0.9), T(0.9)), Vec3(2, 0, 0)}) {
    std::cout << "  " << p << "   tape " << std::setw(9) << partTape.signedDistance(p) << "   fused " << std::setw(9)
              << fusedPart.signedDistance(p) << '\n';
  }

  // The tape interprets its clauses one by one; the fused part has nothing to interpret. The gap is
  // largest on a GPU, where interpreting also costs the device its parallelism.
  std::vector<Vec3> grid;

  for (int i = 0; i < 40; i++) {
    for (int j = 0; j < 40; j++) {
      for (int k = 0; k < 10; k++) {
        grid.emplace_back(T(-1.5) + T(0.075) * T(i), T(-1.5) + T(0.075) * T(j), T(-1.5) + T(0.3) * T(k));
      }
    }
  }

  T sum = T(0);

  const double tapeTime  = timePerPoint(partTape, grid, sum);
  const double fusedTime = timePerPoint(fusedPart, grid, sum);

  std::cout << "  on the host, per point: tape " << std::setprecision(3) << tapeTime << " us, fused " << fusedTime
            << " us (sum " << sum << ")\n"
            << std::setprecision(4);

  // ── Which primitive decides the value ──────────────────────────────────────────────────────────────
  //
  // A tape's closestPrimitive(p) names the leaf whose surface the value comes from at p: a union passes on
  // the leaf of the operand it takes, a difference the leaf of whichever object bounds the result. The
  // leaves are numbered in expression order: the cube 0, the ball 1, the drills along x, y and z 2, 3 and 4.
  // m_localPoint is the point in the leaf's own frame, where the leaf itself (primitive<P>(ref),
  // visitPrimitive()) can be queried. A leaf number indexes the caller's own table, so check it first: a
  // reference may name no leaf (m_leaf == PrimitiveRef::None).
  const char* leafNames[] = {"cube", "ball", "drill along x", "drill along y", "drill along z"};

  const auto leafName = [&leafNames](const uint32_t a_leaf) { return a_leaf < 5 ? leafNames[a_leaf] : "no leaf"; };

  std::cout << "\nThe primitive that decides the part's value, through its tape:\n";

  for (const Vec3& p : {Vec3(T(0.95), T(0.2), T(0.1)),
                        Vec3(T(0.8), T(0.8), T(0.8)),
                        Vec3(T(0.3), T(0.2), T(0.9)),
                        Vec3(T(0.2), T(0.9), T(0.3))}) {
    std::cout << "  " << p << "   " << leafName(partTape.closestPrimitive(p).m_leaf) << '\n';
  }

  // A fused part is one shape with one surface, however it is built: in a tape it is one leaf. A scene is
  // therefore a tape whose leaves are parts, each evaluated as compiled code; the tape names the part.
  using FusedPart = std::decay_t<decltype(fusedPart)>;

  const EBGeometry::Tape<T, FusedPart> scene(
    pool, EBGeometry::Union(EBGeometry::Expr<T>(fusedPart), EBGeometry::Translate(fusedPart, Vec3(T(3), T(0), T(0)))));

  std::cout << "\nThe part that decides a scene of two fused parts, 3 apart along x:\n";

  for (const Vec3& p : {Vec3(T(0.95), T(0.2), T(0.1)), Vec3(T(3.2), T(0.2), T(0.9))}) {
    std::cout << "  " << p << "   part " << scene.closestPrimitive(p).m_leaf << '\n';
  }

  // ── The same query on a GPU ────────────────────────────────────────────────────────────────────────
  //
  // A kernel cannot read host memory, so the pool goes to the device first:
  //   1. freeze() the pool, so that nothing can be added to it and its block stays where it is;
  //   2. Pool::mirror() copies the whole block, every tape in it, in one piece into
  //      GPU::memoryResource() (device memory in a CUDA or HIP build);
  //   3. rebasedView() gives a descriptor of the same tape that resolves against the copy.
  // GPU::transform() then runs the query once per point in a kernel and writes the results into a host
  // vector. In a plain C++ build, memoryResource() is host memory standing in for a device and transform()
  // is a host loop, so these lines also run there.
  if (EBGeometry::GPU::enabled && !EBGeometry::GPU::deviceAvailable()) {
    std::cout << "\nOn the device: skipped, no GPU is visible\n";
  }
  else {
    pool.freeze();

    const EBGeometry::Pool devicePool = EBGeometry::Pool::mirror(pool, EBGeometry::GPU::memoryResource());

    const Distance<Tape> query{ringTape.rebasedView(devicePool)};

    std::vector<Vec3> points;

    for (int i = 0; i <= 8; i++) {
      points.emplace_back(T(1.5) + T(0.125) * T(i), T(0.3), T(0));
    }

    std::vector<T> onDevice(points.size());

    EBGeometry::GPU::transform(query, points, onDevice);

    std::cout << "\nThe ring along a line at height y = 0.3, through GPU::transform (backend: "
              << EBGeometry::GPU::backendName << "):\n";

    for (std::size_t i = 0; i < points.size(); i++) {
      std::cout << "  x = " << points[i][0] << "   value = " << std::setw(8) << onDevice[i] << '\n';
    }

    // The fused part goes to the device as it is: it holds its shapes, not offsets into a pool. Along a
    // line through it at height y = 0.3: through a hole near the middle, and through the cube's walls.
    std::vector<Vec3> line;

    for (int i = 0; i <= 8; i++) {
      line.emplace_back(T(-1.2) + T(0.3) * T(i), T(0.3), T(0.7));
    }

    const Distance<std::decay_t<decltype(fusedPart)>> fusedQuery{fusedPart};

    EBGeometry::GPU::transform(fusedQuery, line, onDevice);

    std::cout << "\nThe fused part along a line at y = 0.3, z = 0.7, through GPU::transform:\n";

    for (std::size_t i = 0; i < line.size(); i++) {
      std::cout << "  x = " << line[i][0] << "   value = " << std::setw(8) << onDevice[i] << '\n';
    }
  }

  return 0;
}
