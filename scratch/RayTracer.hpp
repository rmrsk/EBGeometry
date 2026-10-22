// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

// Scratch: a ray tracer for the Armadillo scenes. Instead of asking "how far is the surface?" many times
// per ray (sphere tracing), every primitive answers "where does this ray next cross your surface, and is
// it going in or out?", and CSG walks those crossings along the ray. The mesh answers through a PackedBVH
// over its triangles, traversed with a ray-box test; the cylinder answers in closed form.
#ifndef SCRATCH_RAYTRACER_HPP
#define SCRATCH_RAYTRACER_HPP

#include "Tracer.hpp"

#include <array>
#include <string>
#include <utility>

namespace RayTracer {

using Tracer::T;
using Tracer::Vec3;

namespace Math = EBGeometry::Math;

using AABB = EBGeometry::BoundingVolumes::AABBT<T>;

// A crossing of a surface by a ray: where (t), the outward normal there, and whether the ray enters.
struct Crossing
{
  T    t = Math::Limits<T>::infinity();
  Vec3 normal;
  bool entering = false;

  EBGEOMETRY_HOST_DEVICE
  bool
  valid() const noexcept
  {
    return t < Math::Limits<T>::infinity();
  }
};

// One triangle with its vertex normals, for Moller-Trumbore and smooth shading.
struct Tri
{
  Vec3 v0, e1, e2;
  Vec3 n0, n1, n2;
};

// A triangle mesh in its own coordinates, placed in the world as world = scale * (mesh + shift), like
// Scale(Translate(mesh, shift), scale) in the expression.
struct Mesh
{
  using BVH = EBGeometry::BVH::PackedBVH<T, Tri, 4>;

  BVH  m_bvh;
  Vec3 m_shift;
  T    m_scale;

  // The next crossing of the surface after t > a_tMin, along the world ray o + t d.
  EBGEOMETRY_HOST_DEVICE
  Crossing
  next(const Vec3& o, const Vec3& d, const T a_tMin) const noexcept
  {
    // The same t parametrises the ray in mesh coordinates when the direction is scaled too.
    const Vec3 om = o / m_scale - m_shift;
    const Vec3 dm = d / m_scale;
    const Vec3 inv(T(1) / dm[0], T(1) / dm[1], T(1) / dm[2]);

    Crossing best;
    T        bestU = 0;
    T        bestV = 0;
    Tri      bestTri;

    // A node's key is where the ray enters its box (infinity if it misses).
    const auto entry = [&](const typename BVH::Node& a_node) noexcept -> T {
      const AABB& box = a_node.getBoundingVolume();

      T t0 = a_tMin;
      T t1 = best.t;

      for (int k = 0; k < 3; k++) {
        T a = (box.getLowCorner()[k] - om[k]) * inv[k];
        T b = (box.getHighCorner()[k] - om[k]) * inv[k];

        if (a > b) {
          const T tmp = a;

          a = b;
          b = tmp;
        }

        t0 = Math::max(t0, a);
        t1 = Math::min(t1, b);
      }

      return (t0 <= t1) ? t0 : Math::Limits<T>::infinity();
    };

    const auto evalLeaf = [&](EBGeometry::PODSpan<const Tri> a_tris, size_t a_offset, size_t a_count) noexcept {
      for (size_t i = a_offset; i < a_offset + a_count; i++) {
        const Tri& tri = a_tris[uint32_t(i)];

        const Vec3 p   = EBGeometry::cross(dm, tri.e2);
        const T    det = dot(tri.e1, p);

        if (std::abs(det) < T(1e-12)) {
          continue;
        }

        const T    invDet = T(1) / det;
        const Vec3 s      = om - tri.v0;
        const T    u      = dot(s, p) * invDet;

        if (u < T(0) || u > T(1)) {
          continue;
        }

        const Vec3 q = EBGeometry::cross(s, tri.e1);
        const T    v = dot(dm, q) * invDet;

        if (v < T(0) || u + v > T(1)) {
          continue;
        }

        const T t = dot(tri.e2, q) * invDet;

        if (t > a_tMin && t < best.t) {
          best.t  = t;
          bestU   = u;
          bestV   = v;
          bestTri = tri;
        }
      }
    };

    // Descend while the box is entered before the closest crossing found so far.
    const auto prune = [&](const typename BVH::Node&, const T& a_key) noexcept { return a_key < best.t; };

    // Visit the nearest child first: traverse() visits the last entry first, so sort by key, descending.
    const auto order = [](EBGeometry::Array<EBGeometry::BVH::NodeAndKey<T>, 4>& a_children) noexcept {
      for (int i = 1; i < 4; i++) {
        const auto c = a_children[size_t(i)];
        int        j = i - 1;

        while (j >= 0 && a_children[size_t(j)].second < c.second) {
          a_children[size_t(j + 1)] = a_children[size_t(j)];
          j--;
        }

        a_children[size_t(j + 1)] = c;
      }
    };

    m_bvh.traverse(evalLeaf, prune, order, entry);

    if (best.valid()) {
      const Vec3 face = EBGeometry::cross(bestTri.e1, bestTri.e2);
      const Vec3 n    = (T(1) - bestU - bestV) * bestTri.n0 + bestU * bestTri.n1 + bestV * bestTri.n2;

      best.entering = dot(face, dm) < T(0);
      best.normal   = Tracer::normalized(n);
    }

    return best;
  }
};

// A capped cylinder, convex, so a ray is inside it over at most one interval.
struct Cylinder
{
  Vec3 m_a, m_axis;
  T    m_length, m_radius;

  Cylinder() = default;

  Cylinder(const Vec3& a, const Vec3& b, const T r) noexcept
    : m_a(a), m_axis((b - a) / (b - a).length()), m_length((b - a).length()), m_radius(r)
  {}

  // The interval [in.t, out.t] where the ray is inside; both invalid if it misses.
  EBGEOMETRY_HOST_DEVICE
  void
  interval(const Vec3& o, const Vec3& d, Crossing& a_in, Crossing& a_out) const noexcept
  {
    a_in  = Crossing();
    a_out = Crossing();

    const Vec3 oc = o - m_a;
    const T    od = dot(oc, m_axis);
    const T    dd = dot(d, m_axis);

    // Radial part: |(oc + t d) minus its axial component| = r.
    const Vec3 pr = oc - od * m_axis;
    const Vec3 dr = d - dd * m_axis;
    const T    A  = dot(dr, dr);
    const T    B  = dot(pr, dr);
    const T    C  = dot(pr, pr) - m_radius * m_radius;

    T r0, r1;

    if (A < T(1e-12)) {
      if (C > 0) {
        return;
      }

      r0 = -Math::Limits<T>::infinity();
      r1 = Math::Limits<T>::infinity();
    }
    else {
      const T disc = B * B - A * C;

      if (disc < 0) {
        return;
      }

      const T s = std::sqrt(disc);

      r0 = (-B - s) / A;
      r1 = (-B + s) / A;
    }

    // Axial part: 0 <= od + t dd <= length.
    T s0, s1;

    if (std::abs(dd) < T(1e-12)) {
      if (od < 0 || od > m_length) {
        return;
      }

      s0 = -Math::Limits<T>::infinity();
      s1 = Math::Limits<T>::infinity();
    }
    else {
      s0 = (T(0) - od) / dd;
      s1 = (m_length - od) / dd;

      if (s0 > s1) {
        const T tmp = s0;

        s0 = s1;
        s1 = tmp;
      }
    }

    const T t0 = Math::max(r0, s0);
    const T t1 = Math::min(r1, s1);

    if (t0 > t1) {
      return;
    }

    const auto normalAt = [&](const T t, const bool radial) noexcept -> Vec3 {
      if (!radial) {
        return (dd > 0) == (t == t0) ? -m_axis : m_axis;
      }

      const Vec3 p = oc + t * d;

      return Tracer::normalized(p - dot(p, m_axis) * m_axis);
    };

    a_in.t        = t0;
    a_in.normal   = normalAt(t0, r0 >= s0);
    a_in.entering = true;

    a_out.t        = t1;
    a_out.normal   = normalAt(t1, r1 <= s1);
    a_out.entering = false;
  }
};

// The scene: the mesh A and the cylinder B combined as A u B (rod) or A - B (tunnel), on a floor. With
// m_meshOnly, the mesh alone: its first crossing, seen from either side, so open meshes render too.
struct Scene
{
  Mesh             m_mesh;
  Cylinder         m_cyl;
  bool             m_union;
  Tracer::Settings m_settings;
  bool             m_meshOnly = false;

  // The first point along the ray, after a_tMin, where the ray enters the CSG object: walk the
  // crossings of A and B in order and track whether the ray is inside each.
  EBGEOMETRY_HOST_DEVICE
  Crossing
  firstHit(const Vec3& o, const Vec3& d, const T a_tMin) const noexcept
  {
    if (m_meshOnly) {
      Crossing c = m_mesh.next(o, d, a_tMin);

      if (c.valid() && dot(c.normal, d) > T(0)) {
        c.normal = -c.normal;
      }

      return c;
    }

    Crossing bIn, bOut;

    m_cyl.interval(o, d, bIn, bOut);

    Crossing a = m_mesh.next(o, d, a_tMin);

    // Inside A at the start iff its next crossing leaves it; inside B iff tMin lies in its interval.
    bool inA = a.valid() && !a.entering;
    bool inB = bIn.valid() && bIn.t <= a_tMin && a_tMin < bOut.t;

    const auto inside = [this](const bool x, const bool y) noexcept { return m_union ? (x || y) : (x && !y); };

    T t = a_tMin;

    for (int step = 0; step < 64; step++) {
      // The next B boundary after t.
      Crossing b;

      if (bIn.valid()) {
        if (bIn.t > t) {
          b = bIn;
        }
        else if (bOut.t > t) {
          b = bOut;
        }
      }

      if (!a.valid() && !b.valid()) {
        return Crossing();
      }

      const bool before = inside(inA, inB);
      const bool fromA  = a.t <= b.t;

      Crossing c = fromA ? a : b;

      if (fromA) {
        inA = a.entering;
      }
      else {
        inB = b.entering;
      }

      if (!before && inside(inA, inB)) {
        // Entering A - B through B's surface: the object's outward normal is B's inward one.
        if (!fromA && !m_union) {
          c.normal = -c.normal;
        }

        c.entering = true;

        return c;
      }

      t = c.t;

      if (fromA) {
        a = m_mesh.next(o, d, t);
      }
    }

    return Crossing();
  }

  // Whether anything blocks the ray before a_tMax (the floor casts no shadows on the object).
  EBGEOMETRY_HOST_DEVICE
  bool
  occluded(const Vec3& o, const Vec3& d, const T a_tMax) const noexcept
  {
    return this->firstHit(o, d, T(0)).t < a_tMax;
  }

  EBGEOMETRY_HOST_DEVICE
  Vec3
  shade(const Tracer::Orbit& a_cam, const T u, const T v, const Vec3& a_light) const noexcept
  {
    const Vec3 dir = Tracer::normalized(u * a_cam.right + v * a_cam.up + a_cam.focal * a_cam.fwd);

    Vec3 c = (T(1) - T(0.5) * (v + 1)) * Vec3(0.80, 0.82, 0.86) + T(0.5) * (v + 1) * Vec3(0.30, 0.42, 0.62);

    Crossing hit     = this->firstHit(a_cam.eye, dir, T(0));
    bool     onFloor = false;

    // The floor plane y = floorY, if it is nearer.
    if (m_settings.floor && dir[1] < 0) {
      const T tf = (m_settings.floorY - a_cam.eye[1]) / dir[1];

      if (tf > 0 && tf < hit.t) {
        hit.t      = tf;
        hit.normal = Vec3(0, 1, 0);
        onFloor    = true;
      }
    }

    if (!hit.valid()) {
      return c;
    }

    const Vec3 p = a_cam.eye + hit.t * dir;
    const Vec3 n = hit.normal;
    const Vec3 h = Tracer::normalized(a_light - dir);
    const Vec3 q = p + T(1e-3) * n;

    const T lit  = (m_settings.shadows && this->occluded(q, a_light, T(1e30))) ? T(0) : T(1);
    const T diff = Math::max(T(0), dot(n, a_light)) * lit;
    const T spec = std::pow(Math::max(T(0), dot(n, h)), T(48)) * diff;
    const T sky  = T(0.5) + T(0.5) * n[1];

    // Ambient occlusion from four short rays around the normal.
    T ao = 1;

    if (m_settings.occlusion) {
      const Vec3 t1 = Tracer::normalized(EBGeometry::cross(std::abs(n[0]) < T(0.9) ? Vec3(1, 0, 0) : Vec3(0, 1, 0), n));
      const Vec3 t2 = EBGeometry::cross(n, t1);

      int blocked = 0;

      for (int k = 0; k < 4; k++) {
        const T    a = T(1.5707963) * T(k);
        const Vec3 r = Tracer::normalized(n + T(0.8) * (std::cos(a) * t1 + std::sin(a) * t2));

        blocked += this->occluded(q, r, T(0.25)) ? 1 : 0;
      }

      ao = T(1) - T(0.2) * T(blocked);
    }

    const Vec3 albedo = onFloor ? Vec3(0.55, 0.56, 0.58) : Vec3(0.62, 0.20, 0.06);

    return albedo * (T(1.1) * diff + T(0.35) * sky * ao) + T(0.45) * spec * Vec3::ones();
  }
};

// One pixel, as RGBA8888, for GPU::transform() (or a host loop). The light is jittered per subsample, a
// small area light, so 2x2 supersampling also softens the shadow edges a little.
template <class SceneT>
struct Pixel
{
  SceneT        m_scene;
  Tracer::Orbit m_cam;
  int           m_w;
  int           m_h;
  int           m_ss;

  EBGEOMETRY_HOST_DEVICE
  uint32_t
  operator()(const uint32_t& a_index) const noexcept
  {
    const int i = int(a_index % uint32_t(m_w));
    const int j = int(a_index / uint32_t(m_w));

    Vec3 color = Vec3::zeros();

    for (int s = 0; s < m_ss * m_ss; s++) {
      const T di = (T(s % m_ss) + T(0.5)) / T(m_ss);
      const T dj = (T(s / m_ss) + T(0.5)) / T(m_ss);
      const T u  = (T(2) * (T(i) + di) - T(m_w)) / T(m_h);
      const T v  = (T(m_h) - T(2) * (T(j) + dj)) / T(m_h);

      const Vec3 light =
        Tracer::normalized(Vec3(T(0.7) + T(0.04) * (di - T(0.5)), T(0.8), T(0.3) + T(0.04) * (dj - T(0.5))));

      color += m_scene.shade(m_cam, u, v, light);
    }

    color = color / T(m_ss * m_ss);

    uint32_t rgba = 0xFF;

    for (int k = 0; k < 3; k++) {
      const T g = std::pow(Math::clamp(color[k], T(0), T(1)), T(1) / T(2.2));

      rgba |= uint32_t(T(255) * g + T(0.5)) << (24 - 8 * k);
    }

    return rgba;
  }
};

// ── The hybrid: rays find boxes, sphere tracing finds the surface inside them ─────────────────────────
//
// A BVH over boxes that together cover the object's surface: for sharp CSG the surface lies on the
// primitives' surfaces, so the mesh's leaf boxes and the cylinder's box cover it. A ray walks the boxes
// front to back with a ray-box test and sphere traces the full tape only inside the boxes it enters, where
// distance queries are cheap. Far from the surface it never asks for a distance at all.
template <class Shape>
struct Hybrid
{
  using BoxBVH = EBGeometry::BVH::PackedBVH<T, AABB, 4>;

  BoxBVH               m_boxes;
  Tracer::Scene<Shape> m_sdf;      // The shape alone: no floor, no box bound.
  Tracer::Settings     m_settings; // Floor, shadows and AO, for shading.

  // The ray's entry into a box after a_tMin and before a_tMax, or infinity.
  EBGEOMETRY_HOST_DEVICE
  static void
  slab(const AABB& a_box, const Vec3& o, const Vec3& inv, T& a_t0, T& a_t1) noexcept
  {
    for (int k = 0; k < 3; k++) {
      T a = (a_box.getLowCorner()[k] - o[k]) * inv[k];
      T b = (a_box.getHighCorner()[k] - o[k]) * inv[k];

      if (a > b) {
        const T tmp = a;

        a = b;
        b = tmp;
      }

      a_t0 = Math::max(a_t0, a);
      a_t1 = Math::min(a_t1, b);
    }
  }

  // The first surface point after a_tMin and before a_tMax, or infinity.
  EBGEOMETRY_HOST_DEVICE
  T
  firstHit(const Vec3& o, const Vec3& d, const T a_tMin, const T a_tMax) const noexcept
  {
    const Vec3 inv(T(1) / d[0], T(1) / d[1], T(1) / d[2]);

    T best = a_tMax;

    const auto entry = [&](const typename BoxBVH::Node& a_node) noexcept -> T {
      T t0 = a_tMin;
      T t1 = best;

      slab(a_node.getBoundingVolume(), o, inv, t0, t1);

      return (t0 <= t1) ? t0 : Math::Limits<T>::infinity();
    };

    // Sphere trace inside each box the ray enters, up to the closest hit so far.
    const auto evalLeaf = [&](EBGeometry::PODSpan<const AABB> a_boxes, size_t a_offset, size_t a_count) noexcept {
      for (size_t i = a_offset; i < a_offset + a_count; i++) {
        T t0 = a_tMin;
        T t1 = best;

        slab(a_boxes[uint32_t(i)], o, inv, t0, t1);

        T t = t0;

        for (int step = 0; step < 96 && t <= t1; step++) {
          const T h = m_sdf.m_shape.signedDistance(o + t * d);

          if (h < T(1e-4) * Math::max(t, T(1))) {
            best = t;

            break;
          }

          t += h;
        }
      }
    };

    const auto prune = [&](const typename BoxBVH::Node&, const T& a_key) noexcept { return a_key < best; };

    const auto order = [](EBGeometry::Array<EBGeometry::BVH::NodeAndKey<T>, 4>& a_children) noexcept {
      for (int i = 1; i < 4; i++) {
        const auto c = a_children[size_t(i)];
        int        j = i - 1;

        while (j >= 0 && a_children[size_t(j)].second < c.second) {
          a_children[size_t(j + 1)] = a_children[size_t(j)];
          j--;
        }

        a_children[size_t(j + 1)] = c;
      }
    };

    m_boxes.traverse(evalLeaf, prune, order, entry);

    return best < a_tMax ? best : Math::Limits<T>::infinity();
  }

  EBGEOMETRY_HOST_DEVICE
  Vec3
  shade(const Tracer::Orbit& a_cam, const T u, const T v, const Vec3& a_light) const noexcept
  {
    const Vec3 dir = Tracer::normalized(u * a_cam.right + v * a_cam.up + a_cam.focal * a_cam.fwd);

    Vec3 c = (T(1) - T(0.5) * (v + 1)) * Vec3(0.80, 0.82, 0.86) + T(0.5) * (v + 1) * Vec3(0.30, 0.42, 0.62);

    T    t       = this->firstHit(a_cam.eye, dir, T(0), T(1e30));
    bool onFloor = false;

    if (m_settings.floor && dir[1] < 0) {
      const T tf = (m_settings.floorY - a_cam.eye[1]) / dir[1];

      if (tf > 0 && tf < t) {
        t       = tf;
        onFloor = true;
      }
    }

    if (!(t < Math::Limits<T>::infinity())) {
      return c;
    }

    const Vec3 p = a_cam.eye + t * dir;
    const Vec3 n = onFloor ? Vec3(0, 1, 0) : m_sdf.normal(p);
    const Vec3 h = Tracer::normalized(a_light - dir);
    const Vec3 q = p + T(2e-3) * n;

    const T lit =
      (m_settings.shadows && this->firstHit(q, a_light, T(0), T(1e30)) < Math::Limits<T>::infinity()) ? T(0) : T(1);
    const T diff = Math::max(T(0), dot(n, a_light)) * lit;
    const T spec = std::pow(Math::max(T(0), dot(n, h)), T(48)) * diff;
    const T sky  = T(0.5) + T(0.5) * n[1];
    const T ao   = (m_settings.occlusion && !onFloor) ? m_sdf.occlusion(p, n) : T(1);

    const Vec3 albedo = onFloor ? Vec3(0.55, 0.56, 0.58) : Vec3(0.62, 0.20, 0.06);

    return albedo * (T(1.1) * diff + T(0.35) * sky * ao) + T(0.45) * spec * Vec3::ones();
  }
};

// Boxes covering a mesh's surface: the leaves of a BVH over its triangles with fewer than a_leafSize
// triangles each, mapped to the world as world = scale * (mesh + shift) and grown by a_margin.
inline std::vector<AABB>
meshLeafBoxes(const EBGeometry::PolygonSoup<T>& soup,
              const size_t                      a_leafSize,
              const Vec3&                       a_shift,
              const T                           a_scale,
              const T                           a_margin)
{
  const auto& verts = soup.getVertexCoordinates();

  std::vector<std::pair<uint32_t, AABB>> prims;

  for (const auto& f : soup.getFacets()) {
    Vec3 lo = verts[f[0]];
    Vec3 hi = verts[f[0]];

    for (const size_t v : f) {
      lo = min(lo, verts[v]);
      hi = max(hi, verts[v]);
    }

    prims.emplace_back(uint32_t(prims.size()), AABB(lo, hi));
  }

  EBGeometry::Pool scratch(EBGeometry::hostMemoryResource());

  const EBGeometry::BVH::PackedBVH<T, uint32_t, 4> bvh(
    scratch,
    std::move(prims),
    EBGeometry::BVH::BinnedSAHPartitioner<T, uint32_t, AABB, 4>,
    [a_leafSize](const EBGeometry::BVH::TreeBVH<T, uint32_t, AABB, 4>& a_node) {
      return a_node.getPrimitives().size() < a_leafSize;
    });

  std::vector<AABB> boxes;

  const auto nodes = bvh.getNodes();

  for (uint32_t i = 0; i < nodes.size(); i++) {
    if (nodes[i].isLeaf()) {
      const AABB& b = nodes[i].getBoundingVolume();

      boxes.emplace_back(a_scale * (b.getLowCorner() + a_shift) - a_margin * Vec3::ones(),
                         a_scale * (b.getHighCorner() + a_shift) + a_margin * Vec3::ones());
    }
  }

  return boxes;
}

inline std::vector<AABB>
meshLeafBoxes(
  const std::string& a_file, const size_t a_leafSize, const Vec3& a_shift, const T a_scale, const T a_margin)
{
  return meshLeafBoxes(EBGeometry::Parser::readIntoPolygonSoup<T>(a_file), a_leafSize, a_shift, a_scale, a_margin);
}

// A box enclosing a capped cylinder, grown by a_margin.
inline AABB
cylinderBox(const Vec3& a, const Vec3& b, const T r, const T a_margin)
{
  const Vec3 axis = (b - a) / (b - a).length();
  Vec3       e;

  for (int k = 0; k < 3; k++) {
    e[k] = r * std::sqrt(Math::max(T(0), T(1) - axis[k] * axis[k])) + a_margin;
  }

  return AABB(min(a, b) - e, max(a, b) + e);
}

// The triangles of a mesh file with area-weighted vertex normals, in a PackedBVH built by binned SAH.
// Outward orientation is checked by the sign of the enclosed volume.
inline Mesh::BVH
buildMesh(const EBGeometry::PolygonSoup<T>& soup, EBGeometry::Pool& a_pool)
{
  const auto& verts = soup.getVertexCoordinates();

  std::vector<std::array<size_t, 3>> tris;

  for (const auto& f : soup.getFacets()) {
    for (size_t k = 1; k + 1 < f.size(); k++) {
      tris.push_back({f[0], f[k], f[k + 1]});
    }
  }

  T volume = 0;

  for (const auto& t : tris) {
    volume += dot(verts[t[0]], EBGeometry::cross(verts[t[1]], verts[t[2]]));
  }

  if (volume < 0) {
    for (auto& t : tris) {
      std::swap(t[1], t[2]);
    }
  }

  std::vector<Vec3> normals(verts.size(), Vec3::zeros());

  for (const auto& t : tris) {
    const Vec3 n = EBGeometry::cross(verts[t[1]] - verts[t[0]], verts[t[2]] - verts[t[0]]);

    for (const size_t v : t) {
      normals[v] += n;
    }
  }

  std::vector<std::pair<Tri, AABB>> prims;

  prims.reserve(tris.size());

  for (const auto& t : tris) {
    const Vec3& a = verts[t[0]];
    const Vec3& b = verts[t[1]];
    const Vec3& c = verts[t[2]];

    const Tri tri{a, b - a, c - a, normals[t[0]], normals[t[1]], normals[t[2]]};

    prims.emplace_back(tri, AABB(min(min(a, b), c), max(max(a, b), c)));
  }

  return Mesh::BVH(a_pool, std::move(prims), EBGeometry::BVH::BinnedSAHPartitioner<T, Tri, AABB, 4>);
}

inline Mesh::BVH
buildMesh(const std::string& a_file, EBGeometry::Pool& a_pool)
{
  return buildMesh(EBGeometry::Parser::readIntoPolygonSoup<T>(a_file), a_pool);
}

} // namespace RayTracer

#endif
