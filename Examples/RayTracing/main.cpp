// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

// Ray tracing a triangle mesh (a TriMeshSDF): where a ray first crosses its surface, with the face it
// crosses and the normal there (firstHit()), whether a particle's step from one point to the next crosses
// it (intersects() and firstHit() on a segment), and the same first crossing from a walk of the mesh's BVH
// written by hand (PackedBVH::traverse() with a BVH::RayQuery), for when the queries do not fit. The last
// part casts rays on a GPU. See README.md.

#include <cstddef>
#include <cstdint>
#include <iostream>
#include <string>
#include <vector>

#include <EBGeometry.hpp>

// Floating-point precision. Overridable from CMake (-DEBGEOMETRY_PRECISION=float).
#ifndef EBGEOMETRY_PRECISION
#define EBGEOMETRY_PRECISION double
#endif

using T    = EBGEOMETRY_PRECISION;
using Vec3 = EBGeometry::Vec3T<T>;
using Ray  = EBGeometry::RayT<T>;
using Hit  = EBGeometry::HitT<T>;

// The mesh SDF's BVH branching factor and triangles per leaf group, spelled out.
constexpr std::size_t K = 4;
constexpr std::size_t W = 4;

using Mesh = EBGeometry::TriMeshSDF<T, K, W>;

namespace {

// ── The code that runs on either side ────────────────────────────────────────────────────────────────
//
// firstHit() and intersects() are called like signedDistance(): from your own loop on the host, or from
// your own functor in a kernel. Written as trivially copyable functors with EBGEOMETRY_HOST_DEVICE call
// operators, the same queries run on the host and through GPU::transform().

// The first crossing of a ray from a point towards a target. HitT is trivially copyable, so it comes back
// from the device as it is.
struct CastTowards
{
  Mesh m_mesh;
  Vec3 m_target;

  EBGEOMETRY_HOST_DEVICE
  Hit
  operator()(const Vec3& a_origin) const noexcept
  {
    const Vec3 d = m_target - a_origin;

    // The direction must be of unit length: t is then a distance along the ray.
    return EBGeometry::firstHit(m_mesh, Ray{a_origin, d / d.length()}, T(0), EBGeometry::Math::Limits<T>::infinity());
  }
};

// The same first crossing, from a walk of the mesh's BVH written by hand. The walk is the library's
// (traverse()); the state, the leaf callback and the bound are yours, so you can keep what you need (here
// the nearest distance and the face) and stop when you like. The key of a box is the distance along the
// ray at which the ray enters it; the bound is the nearest crossing so far, so boxes beyond it are skipped.
struct HandWritten
{
  Mesh m_mesh;

  struct Nearest
  {
    T             t;
    std::uint32_t face;
  };

  EBGEOMETRY_HOST_DEVICE
  Nearest
  operator()(const Ray& a_ray) const noexcept
  {
    const auto bvh    = m_mesh.getRoot();
    const auto groups = bvh.getPrimitives(); // The leaves: groups of W triangles.

    Nearest nearest{EBGeometry::Math::Limits<T>::infinity(), UINT32_MAX};

    bvh.traverse(
      EBGeometry::BVH::RayQuery<T>(a_ray.m_origin, a_ray.m_direction),
      nearest,
      [&a_ray, &groups](Nearest& a_nearest, std::size_t a_offset, std::size_t a_count) noexcept {
        for (std::size_t i = a_offset; i < a_offset + a_count; i++) {
          std::uint32_t face = UINT32_MAX;
          Vec3          normal;

          // Where the ray crosses the group's triangles, nearest first, up to the nearest crossing so far.
          const T t = groups[static_cast<std::uint32_t>(i)].rayCrossing(
            a_ray.m_origin, a_ray.m_direction, T(0), a_nearest.t, face, normal);

          if (t < a_nearest.t) {
            a_nearest = Nearest{t, face};
          }
        }
      },
      [](const Nearest& a_nearest) noexcept { return a_nearest.t; });

    return nearest;
  }
};

void
print(const Hit& a_hit)
{
  if (a_hit.m_status == EBGeometry::HitStatus::Hit) {
    std::cout << "crosses at t = " << a_hit.m_t << ", point " << a_hit.m_point << ", face " << a_hit.m_primitive
              << ", normal " << a_hit.m_normal << (a_hit.m_entering ? " (entering)" : " (leaving)") << "\n";
  }
  else if (a_hit.m_status == EBGeometry::HitStatus::Miss) {
    std::cout << "misses\n";
  }
  else {
    std::cout << "unresolved\n";
  }
}

} // namespace

int
main(int argc, char* argv[])
{
  // ── Build it ───────────────────────────────────────────────────────────────────────────────────────
  //
  // The mesh is read into a TriMeshSDF in a Pool, which must outlive it (and every copy of it).
  const std::string file = argc >= 2 ? std::string(argv[1]) : "../../common-3d-test-models/data/armadillo.obj";

  EBGeometry::Pool pool(EBGeometry::hostMemoryResource());

  const Mesh mesh = EBGeometry::Parser::readIntoTriMeshSDF<T, K, W>(file, pool);

  const auto box    = mesh.computeBoundingVolume();
  const Vec3 lo     = box.getLowCorner();
  const Vec3 hi     = box.getHighCorner();
  const Vec3 centre = T(0.5) * (lo + hi);
  const Vec3 size   = hi - lo;

  std::cout << "Read " << file << ": box " << lo << " to " << hi << "\n\n";

  // ── Use it on the host ─────────────────────────────────────────────────────────────────────────────
  //
  // A ray from in front of the mesh towards its centre. firstHit() searches [tMin, tMax] along the ray and
  // returns the first crossing, entering or leaving: its distance, point, the face it crosses (the face's
  // index in the file), the face's outward normal, and whether the ray enters the object there.
  const Vec3 eye = centre - Vec3(T(0), T(0), size[2]);
  const Ray  ray{eye, Vec3(T(0), T(0), T(1))};

  std::cout << "A ray from " << eye << " along +z: ";
  print(EBGeometry::firstHit(mesh, ray, T(0), EBGeometry::Math::Limits<T>::infinity()));

  // A particle stepping from one point to the next, here straight through the mesh: does the step cross
  // the surface, and where? On a segment, t is the distance from its start. intersects() is the cheaper
  // question: it stops at the first crossing it finds.
  const Vec3 from = eye;
  const Vec3 to   = centre + Vec3(T(0), T(0), size[2]);

  std::cout << "The step from " << from << " to " << to << ": "
            << (EBGeometry::intersects(mesh, from, to) == EBGeometry::HitStatus::Hit ? "crosses the surface"
                                                                                     : "stays on one side")
            << "\n  firstHit: ";

  const Hit wall = EBGeometry::firstHit(mesh, from, to);

  print(wall);

  // A particle that stopped on the wall and moves on: a crossing at the start of a segment counts, so it
  // starts the next search a little past the wall, with tMin. It now finds where it leaves the mesh.
  if (wall.hit()) {
    const T stepOff = T(1e-4) * size.length();

    std::cout << "Moving on from the wall to " << to << ", searching from t = " << stepOff << ": ";
    print(EBGeometry::firstHit(mesh, wall.m_point, to, stepOff));
  }

  // The same ray through the hand-written walk: the same crossing and face.
  const HandWritten::Nearest byHand = HandWritten{mesh}(ray);

  std::cout << "\nThe same ray, walking the BVH by hand: t = " << byHand.t << ", face " << byHand.face << "\n\n";

  // ── The same query on a GPU ────────────────────────────────────────────────────────────────────────
  //
  // Freeze the pool, mirror it in one piece into GPU::memoryResource() (device memory in a CUDA or HIP
  // build), and take a rebasedView() of the mesh that resolves against the copy. GPU::transform() runs
  // the functor once per input in a kernel, one ray per thread. In a plain C++ build the same lines run on
  // the host.
  if (EBGeometry::GPU::enabled && !EBGeometry::GPU::deviceAvailable()) {
    std::cout << "On the device: skipped, no GPU is visible\n";

    return 0;
  }

  pool.freeze();

  const EBGeometry::Pool devicePool = EBGeometry::Pool::mirror(pool, EBGeometry::GPU::memoryResource());

  // Five rays towards the centre, from points spread across the front of the box.
  std::vector<Vec3> origins;

  for (int i = -2; i <= 2; i++) {
    origins.push_back(eye + T(0.2) * T(i) * Vec3(size[0], T(0), T(0)));
  }

  std::vector<Hit> hits(origins.size());

  EBGeometry::GPU::transform(CastTowards{mesh.rebasedView(devicePool), centre}, origins, hits);

  std::cout << "Rays towards the centre through GPU::transform (backend: " << EBGeometry::GPU::backendName << "):\n";

  for (std::size_t i = 0; i < origins.size(); i++) {
    std::cout << "  from " << origins[i] << ": ";
    print(hits[i]);
  }

  return 0;
}
