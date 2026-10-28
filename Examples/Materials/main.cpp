// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

// Which part of an object a query found: the armadillo with a cylinder bored through it, and for a point or a
// ray, which of the two surfaces the answer comes from. That id indexes a table of your own, such as
// materials or boundary conditions; the viewer colours by it. See README.md.

#include <iomanip>
#include <iostream>
#include <string>

#include <EBGeometry.hpp>

// Floating-point precision. Overridable from CMake (-DEBGEOMETRY_PRECISION=float).
#ifndef EBGEOMETRY_PRECISION
#define EBGEOMETRY_PRECISION double
#endif

using T    = EBGEOMETRY_PRECISION;
using Vec3 = EBGeometry::Vec3T<T>;
using Expr = EBGeometry::Expr<T>;
using Tape = EBGeometry::Tape<T>;

// The function-try-block reports a mesh file that cannot be read, rather than letting the exception
// terminate the program.
int
main(int argc, char* argv[])
try {
  // ── Build the object ───────────────────────────────────────────────────────────────────────────────
  //
  // The default mesh is the armadillo from the common-3d-test-models submodule (clone with
  // --recurse-submodules, or run git submodule update --init); pass another on the command line. Arguments
  // starting with -- are options (--no-view keeps the viewer shut).
  std::string file = "../../common-3d-test-models/data/armadillo.obj";

  for (int i = 1; i < argc; i++) {
    if (std::string(argv[i]).rfind("--", 0) != 0) {
      file = argv[i];

      break;
    }
  }

  EBGeometry::Pool pool(EBGeometry::hostMemoryResource());

  const auto mesh = EBGeometry::Parser::readIntoTriMeshSDF<T>(file, pool);

  // Stand the mesh on y = 0 with height 2, turned to face +z, and bore a cylinder through its body along z.
  const Vec3 lo = mesh.computeBoundingVolume().getLowCorner();
  const Vec3 hi = mesh.computeBoundingVolume().getHighCorner();

  const Expr armadillo = EBGeometry::Rotate(
    EBGeometry::Scale(EBGeometry::Translate(mesh, Vec3(T(-0.5) * (lo[0] + hi[0]), -lo[1], T(-0.5) * (lo[2] + hi[2]))),
                      T(2) / (hi[1] - lo[1])),
    T(180),
    1);

  const EBGeometry::CylinderSDF<T> bore(Vec3(T(0), T(1.22), T(-2)), Vec3(T(0), T(1.22), T(2)), T(0.16));

  const Tape tape(pool, EBGeometry::Difference(armadillo, bore));

  // ── Which part: the leaf id ────────────────────────────────────────────────────────────────────────
  //
  // A tape numbers its leaves in expression order: here the armadillo 0 and the cylinder 1. Your own table,
  // indexed by that number, says what each part is. A query can name no leaf (PrimitiveRef::None), so check
  // the number before indexing.
  const char* materials[] = {"skin (the armadillo)", "machined (the cylinder's bore)"};

  const auto material = [&materials](const uint32_t a_leaf) { return a_leaf < 2 ? materials[a_leaf] : "none"; };

  // closestPrimitive(p) names the leaf the value at p comes from.
  std::cout << std::fixed << std::setprecision(3) << "Near a point:\n";

  for (const Vec3& p : {Vec3(T(0), T(1.22), T(-0.2)), Vec3(T(0.25), T(1.22), T(-0.2)), Vec3(T(0), T(0.4), T(0))}) {
    std::cout << "  " << p << "   value " << std::setw(7) << tape.signedDistance(p) << "   "
              << material(tape.closestPrimitive(p).m_leaf) << '\n';
  }

  // firstHit() along a ray reports the leaf it crossed in HitT::m_primitive, with the point and the normal.
  // Three rays: at the chest from in front, straight through the bore, and sideways from inside the bore.
  std::cout << "Along a ray:\n";

  for (const EBGeometry::RayT<T>& ray : {EBGeometry::RayT<T>{Vec3(T(0), T(1.6), T(3)), Vec3(T(0), T(0), T(-1))},
                                         EBGeometry::RayT<T>{Vec3(T(0), T(1.22), T(3)), Vec3(T(0), T(0), T(-1))},
                                         EBGeometry::RayT<T>{Vec3(T(0), T(1.22), T(-0.2)), Vec3(T(1), T(0), T(0))}}) {
    const auto hit = EBGeometry::firstHit(tape, ray, T(0), T(6));

    std::cout << "  from " << ray.m_origin << " towards " << ray.m_direction << ": ";

    if (hit.m_status == EBGeometry::HitStatus::Hit) {
      std::cout << "hit at " << hit.m_point << ", " << material(hit.m_primitive) << '\n';
    }
    else {
      std::cout << "no hit\n";
    }
  }

  // ── Look at it ─────────────────────────────────────────────────────────────────────────────────────
  //
  // ColourBy::Primitive colours each surface by the leaf it belongs to, from the same HitT::m_primitive (built
  // with VIEWER=SDL2; otherwise, with --no-view or without a display, view() prints why and returns). The
  // table m_primitiveColours gives a leaf a colour of its own, here the skin sand and the bore steel grey;
  // a leaf not in it gets the built-in palette's. Given the pool, view() renders on the GPU in a GPU build
  // (G switches to the CPU and back).
  EBGeometry::Render::Settings<T> settings;
  settings.m_colourBy            = EBGeometry::Render::ColourBy::Primitive;
  settings.m_primitiveColours[0] = Vec3(T(0.78), T(0.62), T(0.42));
  settings.m_primitiveColours[1] = Vec3(T(0.55), T(0.57), T(0.60));

  // The camera looks from low in front, where the bore shows.
  auto camera    = EBGeometry::Render::Camera<T>::fit(tape.computeBoundingVolume());
  camera.m_yaw   = T(20);
  camera.m_pitch = T(5);

  EBGeometry::Render::view(tape, pool, camera, settings, argc, argv);

  return 0;
} catch (const EBGeometry::Parser::ParseError& e) {
  std::cerr << "Cannot read the mesh: " << e.what() << '\n'
            << "The default mesh is in the common-3d-test-models submodule: git submodule update --init\n";

  return 1;
}
