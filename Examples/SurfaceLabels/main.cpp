// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

// Which part of an object a query found: the armadillo with a cylinder bored through it, standing in a ring of
// pebbles, each part labeled, and for a point or a ray, which surface the answer comes from: the armadillo's
// skin, the bore, or one pebble of the gravel. The label indexes a table of your own, such as boundary conditions,
// and a pebble's number one per pebble; the viewer colors by the leaf. For the materials an object is made of, see
// the Materials example. See README.md.

#include <cmath>
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>

#include <EBGeometry.hpp>

// Floating-point precision. Overridable from CMake (-DEBGEOMETRY_PRECISION=float).
#ifndef EBGEOMETRY_PRECISION
#define EBGEOMETRY_PRECISION double
#endif

using T     = EBGEOMETRY_PRECISION;
using Vec3  = EBGeometry::Vec3T<T>;
using Expr  = EBGeometry::Expr<T>;
using Cloud = EBGeometry::SphereCloudBVH<T>;
using Tape  = EBGeometry::Tape<T, Cloud>; // a sphere cloud is not built into the tape, so it is listed

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

  // Forty pebbles in a ring around its feet, on a spiral, as one sphere cloud: one leaf of the tape.
  std::vector<EBGeometry::SphereCloud::Sphere<T>> pebbles;

  for (int i = 0; i < 40; i++) {
    const T angle  = T(2.39996) * T(i);
    const T radius = T(0.75) + T(0.25) * std::sqrt(T(i) / T(40));

    pebbles.push_back(
      {Vec3(radius * std::cos(angle), T(0.04), radius * std::sin(angle)), T(0.04) + T(0.005) * T(i % 5)});
  }

  const Cloud gravel = Cloud::build(pool, pebbles, 8);

  // Label each part: every leaf below a Label gets its name. Labels change nothing in the values.
  const Expr object = EBGeometry::Union(
    EBGeometry::Difference(EBGeometry::SurfaceLabel(armadillo, "skin"), EBGeometry::SurfaceLabel(bore, "bore")),
    EBGeometry::SurfaceLabel(gravel, "gravel"));

  const Tape tape(pool, object);

  // ── Which part: the label ──────────────────────────────────────────────────────────────────────────
  //
  // The tape stores each leaf's label, as a number it can report on the host and on a GPU. ExprIndex, built
  // from the same expression on the host, numbers the labels the same way and holds their names, so a table
  // of your own, indexed by the label's value(), can be filled by name. findSurfaceLabel() returns no label
  // (SurfaceLabelId::none()) for a name the expression does not have, and a query finds none for an unlabeled
  // leaf, so check before indexing.
  const EBGeometry::ExprIndex<T> index(object);

  std::vector<std::string> surfaces(tape.getNumSurfaceLabels());

  const auto describe = [&index, &surfaces](const char* a_name, const char* a_what) {
    const EBGeometry::SurfaceLabelId label = index.findSurfaceLabel(a_name);

    if (label.value() < surfaces.size()) {
      surfaces[label.value()] = a_what;
    }
  };

  describe("skin", "skin (the armadillo)");
  describe("bore", "machined (the cylinder's bore)");
  describe("gravel", "stone (a pebble)");

  const auto surfaceOf = [&surfaces](const EBGeometry::SurfaceLabelId a_label) {
    return a_label.value() < surfaces.size() ? surfaces[a_label.value()] : std::string("none");
  };

  // closestLeaf(p) names the leaf the value at p comes from, and getSurfaceLabel() its label.
  std::cout << std::fixed << std::setprecision(3) << "Near a point:\n";

  for (const Vec3& p : {Vec3(T(0), T(1.22), T(-0.2)), Vec3(T(0.25), T(1.22), T(-0.2)), Vec3(T(0), T(0.4), T(0))}) {
    std::cout << "  " << p << "   value " << std::setw(7) << tape.signedDistance(p) << "   "
              << surfaceOf(tape.getSurfaceLabel(tape.closestLeaf(p))) << '\n';
  }

  // ── Which pebble: the surface ──────────────────────────────────────────────────────────────────────
  //
  // The whole cloud is one leaf, so closestLeaf() names the gravel, not a pebble. closestSurface() goes on
  // into the cloud and names the pebble that decides: surfaces are numbered across the scene, each leaf a
  // range of them starting at getFirstSurface(leaf), a cloud's in its own order. So data of your own per
  // pebble is an array indexed by the surface minus the leaf's first, and no pebble needs a label.
  std::cout << "Which pebble:\n";

  for (const int k : {3, 17, 31}) {
    const Vec3 p = pebbles[k].m_center + Vec3(T(0), T(0.1), T(0)); // just above pebble k

    const EBGeometry::SurfaceRef<T> s = tape.closestSurface(p);

    std::cout << "  above pebble " << std::setw(2) << k << ": surface " << std::setw(2) << s.m_surface << " of "
              << tape.getNumSurfaces() << ", pebble " << std::setw(2) << (s.m_surface - tape.getFirstSurface(s.m_leaf))
              << ", " << surfaceOf(s.m_surfaceLabel) << '\n';
  }

  // firstSurface() along a ray reports the crossing, with the point and the normal, and the surface crossed as
  // closestSurface() names it: its label, and the part of the shape (the face of the mesh, the side of the
  // bore). Three rays: at the chest from in front, straight through the bore, and sideways from inside the bore.
  std::cout << "Along a ray:\n";

  for (const EBGeometry::RayT<T>& ray : {EBGeometry::RayT<T>{Vec3(T(0), T(1.6), T(3)), Vec3(T(0), T(0), T(-1))},
                                         EBGeometry::RayT<T>{Vec3(T(0), T(1.22), T(3)), Vec3(T(0), T(0), T(-1))},
                                         EBGeometry::RayT<T>{Vec3(T(0), T(1.22), T(-0.2)), Vec3(T(1), T(0), T(0))}}) {
    const EBGeometry::SurfaceHitT<T> hit = tape.firstSurface(ray, T(0), T(6));

    std::cout << "  from " << ray.m_origin << " towards " << ray.m_direction << ": ";

    if (hit.m_hit.hit()) {
      std::cout << "hit at " << hit.m_hit.m_point << ", " << surfaceOf(hit.m_surface.m_surfaceLabel) << ", part "
                << hit.m_surface.m_part << '\n';
    }
    else {
      std::cout << "no hit\n";
    }
  }

  // ── Look at it ─────────────────────────────────────────────────────────────────────────────────────
  //
  // ColorBy::Primitive colors each surface by the leaf it belongs to, from the same HitT::m_primitive (built
  // with VIEWER=SDL2; otherwise, with --no-view or without a display, view() prints why and returns). Leaves
  // are numbered in expression order, the armadillo 0, the cylinder 1 and the gravel 2. The table
  // m_primitiveColors gives a leaf a color of its own, here the skin sand, the bore steel gray and the gravel
  // slate;
  // a leaf not in it gets the built-in palette's. Given the pool, view() renders on the GPU in a GPU build
  // (G switches to the CPU and back).
  EBGeometry::Render::Settings<T> settings;
  settings.m_colorBy            = EBGeometry::Render::ColorBy::Primitive;
  settings.m_primitiveColors[0] = Vec3(T(0.78), T(0.62), T(0.42));
  settings.m_primitiveColors[1] = Vec3(T(0.55), T(0.57), T(0.60));
  settings.m_primitiveColors[2] = Vec3(T(0.35), T(0.40), T(0.45));

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
