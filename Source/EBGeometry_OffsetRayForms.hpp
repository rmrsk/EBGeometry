// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file   EBGeometry_OffsetRayForms.hpp
 * @brief  Ray forms of offsets and annuli: the level sets of their operands' values.
 * @details An offset by r is the level set {f <= r} of its operand's value f, and an annulus of half-thickness
 * d is {f <= d} less {f < -d}. Where the operand's level sets have a closed form, a ray traces them exactly
 * instead of marching: a sphere's, a capsule's and an infinite cylinder's radius grows by the level, a plane
 * moves, a box shrinks or grows into a rounded box, and a mesh's surface grown by d is the union of its faces
 * offset by d (each face's slab over the face, with a capsule around each edge). The level passes through an
 * affine map divided by the map's value scale. RayDetail::LevelView is such a level set of a shape. The fused
 * Offset and Annular trace through it always; a tape's ray program traces its Annular and Offset clauses
 * through it only when the tape lists TapeFeature::ExactOffsets, and marches them otherwise. An offset or
 * annulus of anything else (a composition, CylinderSDF, whose grown rim is a torus, TorusSDF, or another
 * offset or annulus) is marched.
 * @author Robert Marskar
 */

#ifndef EBGEOMETRY_OFFSETRAYFORMS_HPP
#define EBGEOMETRY_OFFSETRAYFORMS_HPP

// Std includes
#include <cmath>
#include <cstdint>
#include <type_traits>

// Our includes
#include "EBGeometry_AnalyticDistanceFunctions.hpp"
#include "EBGeometry_AnalyticRayForms.hpp"
#include "EBGeometry_BVHQuery.hpp"
#include "EBGeometry_CSGRayForms.hpp"
#include "EBGeometry_DCEL_Iterator.hpp"
#include "EBGeometry_Fused.hpp"
#include "EBGeometry_Math.hpp"
#include "EBGeometry_MeshDistanceFunctions.hpp"
#include "EBGeometry_MeshRayForms.hpp"
#include "EBGeometry_Ray.hpp"
#include "EBGeometry_RaySpans.hpp"

namespace EBGeometry {

namespace RayDetail {

/**
 * @brief Where a ray is inside a convex piece: [m_in, m_out], with the outward normals there; empty if
 * m_in > m_out.
 * @details Internal helper; not part of the public API.
 * @tparam T Floating-point precision.
 */
template <class T>
struct Piece
{
  /**
   * @brief Where the ray enters.
   */
  T m_in = -Math::Limits<T>::infinity();

  /**
   * @brief Where it leaves.
   */
  T m_out = Math::Limits<T>::infinity();

  /**
   * @brief Outward normal where it enters.
   */
  Vec3T<T> m_nIn = Vec3T<T>::zeros();

  /**
   * @brief Outward normal where it leaves.
   */
  Vec3T<T> m_nOut = Vec3T<T>::zeros();

  /**
   * @brief Whether the ray meets the piece.
   * @return m_in <= m_out.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline bool
  hit() const noexcept
  {
    return m_in <= m_out;
  }

  /**
   * @brief An empty piece.
   * @return The piece.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  static inline Piece
  empty() noexcept
  {
    Piece p;

    p.m_in  = Math::Limits<T>::infinity();
    p.m_out = -Math::Limits<T>::infinity();

    return p;
  }

  /**
   * @brief Clip to the half-space n . x <= d.
   * @param[in] a_ray The ray.
   * @param[in] a_n   Outward normal, unit length.
   * @param[in] a_d   Offset.
   */
  EBGEOMETRY_HOST_DEVICE
  inline void
  clip(const RayT<T>& a_ray, const Vec3T<T>& a_n, const T a_d) noexcept
  {
    const T denom = dot(a_n, a_ray.m_direction);
    const T num   = a_d - dot(a_n, a_ray.m_origin);

    if (denom == T(0)) {
      if (num < T(0)) {
        *this = Piece::empty();
      }

      return;
    }

    const T t = num / denom;

    if (denom < T(0)) {
      if (t > m_in) {
        m_in  = t;
        m_nIn = a_n;
      }
    }
    else if (t < m_out) {
      m_out  = t;
      m_nOut = a_n;
    }
  }

  /**
   * @brief Unite with another piece, the union being convex: the nearer entry and the farther exit.
   * @param[in] a_other The other piece.
   */
  EBGEOMETRY_HOST_DEVICE
  inline void
  unite(const Piece& a_other) noexcept
  {
    if (!a_other.hit()) {
      return;
    }

    if (!this->hit()) {
      *this = a_other;

      return;
    }

    if (a_other.m_in < m_in) {
      m_in  = a_other.m_in;
      m_nIn = a_other.m_nIn;
    }

    if (a_other.m_out > m_out) {
      m_out  = a_other.m_out;
      m_nOut = a_other.m_nOut;
    }
  }
};

/**
 * @brief A ray through a ball.
 * @details Internal helper; not part of the public API. The discriminant is r^2 less the squared distance
 * from the center to the ray, from the ray's nearest point: b^2 - |oc|^2 would lose the digits of a far origin.
 * @tparam T Floating-point precision.
 * @param[in] a_ray    The ray, with a unit direction.
 * @param[in] a_center Center.
 * @param[in] a_radius Radius.
 * @return The piece.
 */
template <class T>
[[nodiscard]] EBGEOMETRY_HOST_DEVICE
inline Piece<T>
ballPiece(const RayT<T>& a_ray, const Vec3T<T>& a_center, const T a_radius) noexcept
{
  const Vec3T<T> oc   = a_ray.m_origin - a_center;
  const T        b    = dot(oc, a_ray.m_direction);
  const Vec3T<T> perp = oc - b * a_ray.m_direction;
  const T        h    = a_radius * a_radius - dot(perp, perp);

  if (!(a_radius > T(0)) || h < T(0)) {
    return Piece<T>::empty();
  }

  const T s = std::sqrt(h);

  Piece<T> p;

  p.m_in   = -b - s;
  p.m_out  = -b + s;
  p.m_nIn  = (a_ray.at(p.m_in) - a_center) / a_radius;
  p.m_nOut = (a_ray.at(p.m_out) - a_center) / a_radius;

  return p;
}

/**
 * @brief A ray through an infinite cylinder: the points within a radius of a line.
 * @details Internal helper; not part of the public API. The discriminant is taken as for a ball.
 * @tparam T Floating-point precision.
 * @param[in] a_ray    The ray, with a unit direction.
 * @param[in] a_point  A point on the line.
 * @param[in] a_axis   The line's direction, unit length.
 * @param[in] a_radius Radius.
 * @return The piece.
 */
template <class T>
[[nodiscard]] EBGEOMETRY_HOST_DEVICE
inline Piece<T>
tubePiece(const RayT<T>& a_ray, const Vec3T<T>& a_point, const Vec3T<T>& a_axis, const T a_radius) noexcept
{
  if (!(a_radius > T(0))) {
    return Piece<T>::empty();
  }

  const Vec3T<T> oa = a_ray.m_origin - a_point;
  const Vec3T<T> dp = a_ray.m_direction - dot(a_ray.m_direction, a_axis) * a_axis;
  const Vec3T<T> op = oa - dot(oa, a_axis) * a_axis;

  const T qa = dot(dp, dp);
  const T qb = dot(dp, op);

  Piece<T> p;

  if (qa > T(0)) {
    const Vec3T<T> perp = op - (qb / qa) * dp;
    const T        h    = qa * (a_radius * a_radius - dot(perp, perp));

    if (h < T(0)) {
      return Piece<T>::empty();
    }

    const T s = std::sqrt(h);

    p.m_in   = (-qb - s) / qa;
    p.m_out  = (-qb + s) / qa;
    p.m_nIn  = (op + p.m_in * dp) / a_radius;
    p.m_nOut = (op + p.m_out * dp) / a_radius;
  }
  else if (dot(op, op) > a_radius * a_radius) {
    return Piece<T>::empty();
  }

  return p;
}

/**
 * @brief A ray through a capsule: the segment [a, b] grown by a radius, the cylinder between the end planes
 * united with the balls at the ends, which hold the end discs.
 * @details Internal helper; not part of the public API. Out of line on a device, as are roundedBoxPiece() and
 * offsetFacePiece(): a rounded box tests twelve capsules and a face one per edge, and inlined in every tape
 * kernel (through its leaf dispatch) they took ptxas several times as long.
 * @tparam T Floating-point precision.
 * @param[in] a_ray    The ray, with a unit direction.
 * @param[in] a_a      One end.
 * @param[in] a_b      The other end.
 * @param[in] a_radius Radius.
 * @return The piece.
 */
template <class T>
[[nodiscard]] EBGEOMETRY_HOST_DEVICE
EBGEOMETRY_DEVICE_NOINLINE
inline Piece<T>
capsulePiece(const RayT<T>& a_ray, const Vec3T<T>& a_a, const Vec3T<T>& a_b, const T a_radius) noexcept
{
  Piece<T> p = RayDetail::ballPiece(a_ray, a_a, a_radius);

  p.unite(RayDetail::ballPiece(a_ray, a_b, a_radius));

  const Vec3T<T> axis   = a_b - a_a;
  const T        length = axis.length();

  if (length > T(0)) {
    const Vec3T<T> u = axis / length;

    Piece<T> tube = RayDetail::tubePiece(a_ray, a_a, u, a_radius);

    if (tube.hit()) {
      tube.clip(a_ray, -u, -dot(u, a_a));
      tube.clip(a_ray, u, dot(u, a_b));

      p.unite(tube);
    }
  }

  return p;
}

/**
 * @brief A ray through an axis-aligned box.
 * @details Internal helper; not part of the public API.
 * @tparam T Floating-point precision.
 * @param[in] a_ray The ray.
 * @param[in] a_lo  Low corner.
 * @param[in] a_hi  High corner.
 * @return The piece, empty if the box is.
 */
template <class T>
[[nodiscard]] EBGEOMETRY_HOST_DEVICE
inline Piece<T>
boxPiece(const RayT<T>& a_ray, const Vec3T<T>& a_lo, const Vec3T<T>& a_hi) noexcept
{
  Piece<T> p;

  for (size_t dir = 0; dir < 3; dir++) {
    if (!(a_lo[dir] <= a_hi[dir])) {
      return Piece<T>::empty();
    }

    p.clip(a_ray, -Vec3T<T>::unit(dir), -a_lo[dir]);
    p.clip(a_ray, Vec3T<T>::unit(dir), a_hi[dir]);
  }

  return p;
}

/**
 * @brief A ray through a box grown by a radius: three boxes each grown along one axis, and a capsule along
 * each of the twelve edges.
 * @details Internal helper; not part of the public API.
 * @tparam T Floating-point precision.
 * @param[in] a_ray    The ray.
 * @param[in] a_lo     Low corner.
 * @param[in] a_hi     High corner.
 * @param[in] a_radius Radius, positive.
 * @return The piece.
 */
template <class T>
[[nodiscard]] EBGEOMETRY_HOST_DEVICE
EBGEOMETRY_DEVICE_NOINLINE
inline Piece<T>
roundedBoxPiece(const RayT<T>& a_ray, const Vec3T<T>& a_lo, const Vec3T<T>& a_hi, const T a_radius) noexcept
{
  // It lies in the box grown by the radius along every axis.
  const Vec3T<T> grow = a_radius * Vec3T<T>::ones();

  if (!RayDetail::boxPiece(a_ray, a_lo - grow, a_hi + grow).hit()) {
    return Piece<T>::empty();
  }

  // The three boxes grown along one axis each. Where the ray enters their union through a face across the axis
  // its box grows along, that face is the rounded box's, and the entry is the rounded box's (it is convex); so
  // for the exit. Near an edge it is a capsule's.
  Piece<T> p = Piece<T>::empty();

  size_t dirIn  = 0;
  size_t dirOut = 0;

  for (size_t dir = 0; dir < 3; dir++) {
    const Vec3T<T> along = a_radius * Vec3T<T>::unit(dir);
    const Piece<T> box   = RayDetail::boxPiece(a_ray, a_lo - along, a_hi + along);

    if (box.hit()) {
      if (!p.hit() || box.m_in < p.m_in) {
        dirIn = dir;
      }

      if (!p.hit() || box.m_out > p.m_out) {
        dirOut = dir;
      }

      p.unite(box);
    }
  }

  if (p.hit() && p.m_nIn[dirIn] != T(0) && p.m_nOut[dirOut] != T(0)) {
    return p;
  }

  for (size_t dir = 0; dir < 3; dir++) {
    const size_t d1 = (dir + 1) % 3;
    const size_t d2 = (dir + 2) % 3;

    for (int corner = 0; corner < 4; corner++) {
      Vec3T<T> a = a_lo;

      a[d1] = ((corner & 1) != 0) ? a_hi[d1] : a_lo[d1];
      a[d2] = ((corner & 2) != 0) ? a_hi[d2] : a_lo[d2];

      Vec3T<T> b = a;

      b[dir] = a_hi[dir];

      p.unite(RayDetail::capsulePiece(a_ray, a, b, a_radius));
    }
  }

  return p;
}

/**
 * @brief Twice a planar polygon's signed area along its normal.
 * @details Internal helper; not part of the public API. Its sign says whether the edges run counter-clockwise
 * about the normal (positive), which sets which side of each edge is inside.
 * @tparam T     Floating-point precision.
 * @tparam Edges Calls its argument with each edge's ends, in order around the polygon.
 * @param[in] a_n     Unit normal.
 * @param[in] a_edges The edges.
 * @return The area.
 */
template <class T, class Edges>
[[nodiscard]] EBGEOMETRY_HOST_DEVICE
inline T
twiceArea(const Vec3T<T>& a_n, const Edges& a_edges) noexcept
{
  T        area  = T(0);
  bool     start = true;
  Vec3T<T> first = Vec3T<T>::zeros();

  a_edges([&](const Vec3T<T>& a_a, const Vec3T<T>& a_b) noexcept {
    if (start) {
      first = a_a;
      start = false;
    }

    area += dot(cross(a_a - first, a_b - a_a), a_n);
  });

  return area;
}

/**
 * @brief Where a ray first enters a planar convex face offset by a radius, if before a bound: the offset
 * face's flat side towards the ray first (where the ray enters the face's slab over the face), its rim (a
 * capsule along each edge) only if that misses.
 * @details Internal helper; not part of the public API. The offset face lies in the slab |n . (x - p0)| <=
 * radius, so a ray outside the slab over [a_tMin, a_best] does not enter it there, and nothing of it is
 * entered before the slab. A ray whose part in the slab lies further than the radius outside one edge's line
 * passes the face by more than the radius. The rest test each edge's capsule if the ray passes within the
 * radius of the edge's line.
 * @tparam T     Floating-point precision.
 * @tparam Edges Calls its argument with each edge's ends, in order around the face.
 * @param[in]  a_ray    The ray.
 * @param[in]  a_n      Unit normal, zero for a face of no area.
 * @param[in]  a_p0     A point in the face's plane.
 * @param[in]  a_edges  The edges.
 * @param[in]  a_radius Radius, positive.
 * @param[in]  a_tMin   Start.
 * @param[in]  a_best   Entries at or beyond it are not wanted.
 * @param[out] a_t      The entry.
 * @param[out] a_normal Its outward normal.
 * @param[out] a_inSlab Whether a_tMin is in the slab (the ray may start inside the offset face).
 * @return Whether an entry in [a_tMin, a_best) was found.
 */
template <class T, class Edges>
EBGEOMETRY_HOST_DEVICE
inline bool
offsetFaceEntry(const RayT<T>&  a_ray,
                const Vec3T<T>& a_n,
                const Vec3T<T>& a_p0,
                const Edges&    a_edges,
                const T         a_radius,
                const T         a_tMin,
                const T         a_best,
                T&              a_t,
                Vec3T<T>&       a_normal,
                bool&           a_inSlab) noexcept
{
  const T dn = dot(a_n, a_ray.m_direction);
  const T h0 = dot(a_n, a_ray.m_origin - a_p0);

  T s0 = -Math::Limits<T>::infinity();
  T s1 = Math::Limits<T>::infinity();

  if (dn != T(0)) {
    const T a = (-a_radius - h0) / dn;
    const T b = (a_radius - h0) / dn;

    s0 = Math::min(a, b);
    s1 = Math::max(a, b);
  }
  else if (std::abs(h0) > a_radius) {
    a_inSlab = false;

    return false;
  }

  a_inSlab = s0 < a_tMin && s1 >= a_tMin;

  if (s1 < a_tMin || !(s0 < a_best)) {
    return false;
  }

  const T area = (dn != T(0)) ? RayDetail::twiceArea(a_n, a_edges) : T(0);

  if (area != T(0)) {
    // Edge values (edge length times the in-plane distance from the edge's line, positive inside) where the
    // ray's part in the slab begins and ends, projected onto the plane.
    const T        sign = (area > T(0)) ? T(1) : T(-1);
    const T        tIn  = Math::max(s0, a_tMin);
    const Vec3T<T> qIn  = a_ray.at(tIn) - dot(a_n, a_ray.at(tIn) - a_p0) * a_n;
    const Vec3T<T> qOut = a_ray.at(s1) - dot(a_n, a_ray.at(s1) - a_p0) * a_n;

    bool over      = true;
    bool separated = false;

    a_edges([&](const Vec3T<T>& a_a, const Vec3T<T>& a_b) noexcept {
      const Vec3T<T> e = a_b - a_a;

      const T in  = sign * dot(cross(e, qIn - a_a), a_n);
      const T out = sign * dot(cross(e, qOut - a_a), a_n);
      const T r   = a_radius * e.length();

      over      = over && in >= T(0);
      separated = separated || (in < -r && out < -r);
    });

    if (s0 >= a_tMin && over) {
      a_t      = s0;
      a_normal = ((dn < T(0)) ? T(1) : T(-1)) * a_n;

      return true;
    }

    if (separated) {
      return false;
    }
  }

  // The rim: each edge's capsule, if the ray passes within the radius of the edge's line.
  T    best  = a_best;
  bool found = false;

  a_edges([&](const Vec3T<T>& a_a, const Vec3T<T>& a_b) noexcept {
    const Vec3T<T> m     = cross(a_ray.m_direction, a_b - a_a);
    const T        m2    = dot(m, m);
    const T        along = dot(a_ray.m_origin - a_a, m);

    if (m2 > T(0) && along * along > a_radius * a_radius * m2) {
      return;
    }

    const Piece<T> capsule = RayDetail::capsulePiece(a_ray, a_a, a_b, a_radius);

    if (capsule.hit() && capsule.m_in >= a_tMin && capsule.m_in < best) {
      best     = capsule.m_in;
      a_t      = capsule.m_in;
      a_normal = capsule.m_nIn;
      found    = true;
    }
  });

  return found;
}

/**
 * @brief Where a ray is inside a planar convex face offset by a radius: its slab over the face, united with a
 * capsule around each edge.
 * @details Internal helper; not part of the public API.
 * @tparam T     Floating-point precision.
 * @tparam Edges Calls its argument with each edge's ends, in order around the face.
 * @param[in] a_ray    The ray.
 * @param[in] a_n      Unit normal, zero for a face of no area.
 * @param[in] a_p0     A point in the face's plane.
 * @param[in] a_edges  The edges.
 * @param[in] a_radius Radius, positive.
 * @return The piece.
 */
template <class T, class Edges>
[[nodiscard]] EBGEOMETRY_HOST_DEVICE
EBGEOMETRY_DEVICE_NOINLINE
inline Piece<T>
offsetFacePiece(
  const RayT<T>& a_ray, const Vec3T<T>& a_n, const Vec3T<T>& a_p0, const Edges& a_edges, const T a_radius) noexcept
{
  Piece<T> p = Piece<T>::empty();

  const T area = RayDetail::twiceArea(a_n, a_edges);

  if (area != T(0)) {
    const T sign = (area > T(0)) ? T(1) : T(-1);

    Piece<T> prism;

    prism.clip(a_ray, a_n, dot(a_n, a_p0) + a_radius);
    prism.clip(a_ray, -a_n, -dot(a_n, a_p0) + a_radius);

    a_edges([&](const Vec3T<T>& a_a, const Vec3T<T>& a_b) noexcept {
      // Outward of the face, in its plane.
      const Vec3T<T> side = sign * cross(a_b - a_a, a_n);
      const T        len  = side.length();

      if (len > T(0)) {
        prism.clip(a_ray, side / len, dot(side / len, a_a));
      }
    });

    p = prism;
  }

  a_edges([&](const Vec3T<T>& a_a, const Vec3T<T>& a_b) noexcept {
    p.unite(RayDetail::capsulePiece(a_ray, a_a, a_b, a_radius));
  });

  return p;
}

/**
 * @brief A BVH ray query whose boxes are grown by a radius: what a mesh's offset faces are walked with.
 * @details Internal helper; not part of the public API. Growing [lo, hi] by g moves each axis's slab
 * distances by g / |direction component| outward.
 * @tparam T Floating-point precision.
 */
template <class T>
struct GrownRayQuery
{
  /**
   * @brief Floating-point precision.
   */
  using Scalar = T;

  /**
   * @brief Key type: a distance along the ray.
   */
  using Key = T;

  /**
   * @brief Ray origin.
   */
  Vec3T<T> m_origin;

  /**
   * @brief 1 / direction, per component.
   */
  Vec3T<T> m_inverseDirection;

  /**
   * @brief Start of the ray.
   */
  T m_tMin;

  /**
   * @brief How much each box is grown.
   */
  T m_grow;

  /**
   * @brief The query along a ray.
   * @param[in] a_ray  The ray.
   * @param[in] a_tMin Start.
   * @param[in] a_grow How much each box is grown.
   */
  EBGEOMETRY_HOST_DEVICE
  inline GrownRayQuery(const RayT<T>& a_ray, const T a_tMin, const T a_grow) noexcept
    : m_origin(a_ray.m_origin),
      m_inverseDirection(T(1) / a_ray.m_direction[0], T(1) / a_ray.m_direction[1], T(1) / a_ray.m_direction[2]),
      m_tMin(a_tMin),
      m_grow(a_grow)
  {}

  /**
   * @brief Where the ray enters a grown box.
   * @param[in] a_lo Low corner.
   * @param[in] a_hi High corner.
   * @return The entry, or +infinity.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline T
  entry(const T (&a_lo)[3], const T (&a_hi)[3]) const noexcept
  {
    const T o[3]   = {m_origin[0], m_origin[1], m_origin[2]};
    const T inv[3] = {m_inverseDirection[0], m_inverseDirection[1], m_inverseDirection[2]};
    const T lo[3]  = {a_lo[0] - m_grow, a_lo[1] - m_grow, a_lo[2] - m_grow};
    const T hi[3]  = {a_hi[0] + m_grow, a_hi[1] + m_grow, a_hi[2] + m_grow};

    return BVH::QueryDetail::slabEntry<T>(lo, hi, o, inv, m_tMin);
  }

  /**
   * @brief Where the ray enters the K grown child boxes.
   * @tparam K Branching factor.
   * @param[in] a_row Child boxes.
   * @return The entries.
   */
  template <size_t K>
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline Array<T, K>
  keys(const BVH::ChildBoxes<T, K>& a_row) const noexcept
  {
    Array<T, K> keys;

    for (size_t k = 0; k < K; k++) {
      keys[k] = this->key(a_row, k);
    }

    return keys;
  }

  /**
   * @brief Where the ray enters one grown child box.
   * @tparam K Branching factor.
   * @param[in] a_row Child boxes.
   * @param[in] a_k   Child slot.
   * @return The entry.
   */
  template <size_t K>
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline T
  key(const BVH::ChildBoxes<T, K>& a_row, const size_t a_k) const noexcept
  {
    const T lo[3] = {a_row.m_lo[0][a_k], a_row.m_lo[1][a_k], a_row.m_lo[2][a_k]};
    const T hi[3] = {a_row.m_hi[0][a_k], a_row.m_hi[1][a_k], a_row.m_hi[2][a_k]};

    return this->entry(lo, hi);
  }

  /**
   * @brief Where the ray enters a grown box.
   * @param[in] a_box Box.
   * @return The entry.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline T
  key(const BoundingVolumes::AABBT<T>& a_box) const noexcept
  {
    const Vec3T<T>& l = a_box.getLowCorner();
    const Vec3T<T>& h = a_box.getHighCorner();

    const T lo[3] = {l[0], l[1], l[2]};
    const T hi[3] = {h[0], h[1], h[2]};

    return this->entry(lo, hi);
  }
};

/**
 * @brief The union of the offset faces a ray meets, as disjoint spans merged as they come, at most Capacity:
 * an offset face that would begin a span beyond them is dropped and lowers the horizon to its entry, which is
 * also the walk's bound, so that the boxes beyond are left out.
 * @details Internal helper; not part of the public API.
 * @tparam T Floating-point precision.
 */
template <class T>
struct OffsetFaceUnion
{
  /**
   * @brief Most spans held.
   */
  static constexpr uint32_t Capacity = 4;

  /**
   * @brief Entries.
   */
  T m_in[Capacity];

  /**
   * @brief Exits.
   */
  T m_out[Capacity];

  /**
   * @brief Outward normals at the exits, of the piece each exit is on.
   */
  Vec3T<T> m_nOut[Capacity];

  /**
   * @brief How many are held.
   */
  uint32_t m_count = 0;

  /**
   * @brief The union is known on [t0, m_horizon].
   */
  T m_horizon = Math::Limits<T>::infinity();

  /**
   * @brief Take a piece.
   * @param[in] a_in   Entry.
   * @param[in] a_out  Exit.
   * @param[in] a_nOut Outward normal at the exit.
   */
  EBGEOMETRY_HOST_DEVICE
  inline void
  insert(T a_in, T a_out, Vec3T<T> a_nOut) noexcept
  {
    if (a_in > m_horizon) {
      return;
    }

    // The spans it meets, [first, last), merged into one.
    uint32_t first = 0;

    while (first < m_count && m_out[first] < a_in) {
      first++;
    }

    uint32_t last = first;

    while (last < m_count && m_in[last] <= a_out) {
      a_in = Math::min(a_in, m_in[last]);

      if (m_out[last] > a_out) {
        a_out  = m_out[last];
        a_nOut = m_nOut[last];
      }

      last++;
    }

    if (last > first) {
      m_in[first]   = a_in;
      m_out[first]  = a_out;
      m_nOut[first] = a_nOut;

      const uint32_t removed = last - first - 1U;

      for (uint32_t k = first + 1U; k + removed < m_count; k++) {
        m_in[k]   = m_in[k + removed];
        m_out[k]  = m_out[k + removed];
        m_nOut[k] = m_nOut[k + removed];
      }

      m_count -= removed;

      return;
    }

    // A new span at position first.
    if (m_count == Capacity) {
      if (first == m_count) {
        m_horizon = a_in;

        return;
      }

      m_horizon = m_in[Capacity - 1U];

      m_count--;
    }

    for (uint32_t k = m_count; k > first; k--) {
      m_in[k]   = m_in[k - 1U];
      m_out[k]  = m_out[k - 1U];
      m_nOut[k] = m_nOut[k - 1U];
    }

    m_in[first]   = a_in;
    m_out[first]  = a_out;
    m_nOut[first] = a_nOut;

    m_count++;
  }

  /**
   * @brief The union as a list over [a_t0, horizon].
   * @details A span entered exactly at a_t0 keeps its crossing: a trace starting again from a horizon starts
   * at the entry of the span it dropped there.
   * @param[in]  a_t0    Start of the interval.
   * @param[out] a_spans The list.
   */
  EBGEOMETRY_HOST_DEVICE
  inline void
  toSpans(const T a_t0, RaySpansT<T>& a_spans) const noexcept
  {
    a_spans.reset(m_horizon);

    for (uint32_t i = 0; i < m_count && m_in[i] <= m_horizon; i++) {
      const SpanEndT<T> a = (m_in[i] < a_t0) ? RayDetail::openAt(a_t0) : SpanEndT<T>{m_in[i], 0U, SpanEndT<T>::Exact};
      const SpanEndT<T> b =
        (m_out[i] >= m_horizon) ? RayDetail::openAt(m_horizon) : SpanEndT<T>{m_out[i], 0U, SpanEndT<T>::Exact};

      a_spans.add(a, b);
    }
  }
};

/**
 * @brief How a mesh's faces are walked along a ray, each given as its normal, a point in its plane, its edges,
 * its squared distance from a point and its id.
 * @details Internal helper; not part of the public API. Specialised for TriMeshSDF, MeshSDF and FlatMeshSDF.
 * A_face(state, normal, p0, edges, distance2, id) is called for each face the walk reaches; a_bound(state)
 * is the walk's bound.
 * @tparam S Mesh type.
 */
template <class S>
struct MeshFaces
{
};

/**
 * @brief TriMeshSDF: its BVH, a triangle per SoA lane.
 * @tparam T Floating-point precision.
 * @tparam K BVH branching factor.
 * @tparam W SIMD width.
 */
template <class T, size_t K, size_t W>
struct MeshFaces<TriMeshSDF<T, K, W>>
{
  /**
   * @brief Walk the faces whose boxes, grown by a_grow, the ray enters before the bound.
   * @tparam State State type.
   * @tparam Face  Face callback.
   * @tparam Bound Bound callback.
   * @param[in]     a_mesh  The mesh.
   * @param[in]     a_ray   The ray.
   * @param[in]     a_tMin  Start.
   * @param[in]     a_grow  How much the boxes grow.
   * @param[in,out] a_state State.
   * @param[in]     a_face  Called for each face.
   * @param[in]     a_bound The bound.
   */
  template <class State, class Face, class Bound>
  EBGEOMETRY_HOST_DEVICE
  static inline void
  walk(const TriMeshSDF<T, K, W>& a_mesh,
       const RayT<T>&             a_ray,
       const T                    a_tMin,
       const T                    a_grow,
       State&                     a_state,
       const Face&                a_face,
       const Bound&               a_bound) noexcept
  {
    const auto bvh    = a_mesh.getRoot();
    const auto groups = bvh.getPrimitives();

    const auto leaf = [&groups, &a_face](State& a_s, size_t a_offset, size_t a_count) noexcept {
      for (size_t i = a_offset; i < a_offset + a_count; i++) {
        const auto& group = groups[static_cast<uint32_t>(i)];

        for (size_t lane = 0; lane < W; lane++) {
          const Vec3T<T> v[3] = {group.getVertex(lane, 0), group.getVertex(lane, 1), group.getVertex(lane, 2)};

          const auto edges = [&v](const auto& a_f) noexcept {
            a_f(v[0], v[1]);
            a_f(v[1], v[2]);
            a_f(v[2], v[0]);
          };

          const auto distance2 = [&group, lane](const Vec3T<T>& a_point) noexcept {
            const T d = group.signedDistanceAt(static_cast<uint32_t>(lane), a_point);

            return d * d;
          };

          a_face(a_s, group.getNormal(lane), v[0], edges, distance2, group.getFaceId(lane));
        }
      }
    };

    bvh.traverse(GrownRayQuery<T>(a_ray, a_tMin, a_grow), a_state, leaf, a_bound);
  }
};

/**
 * @brief MeshSDF: its BVH, a polygon face per leaf entry.
 * @tparam T Floating-point precision.
 * @tparam K BVH branching factor.
 */
template <class T, size_t K>
struct MeshFaces<MeshSDF<T, K>>
{
  /**
   * @brief Walk the faces whose boxes, grown by a_grow, the ray enters before the bound.
   * @tparam State State type.
   * @tparam Face  Face callback.
   * @tparam Bound Bound callback.
   * @param[in]     a_mesh  The mesh.
   * @param[in]     a_ray   The ray.
   * @param[in]     a_tMin  Start.
   * @param[in]     a_grow  How much the boxes grow.
   * @param[in,out] a_state State.
   * @param[in]     a_face  Called for each face.
   * @param[in]     a_bound The bound.
   */
  template <class State, class Face, class Bound>
  EBGEOMETRY_HOST_DEVICE
  static inline void
  walk(const MeshSDF<T, K>& a_mesh,
       const RayT<T>&       a_ray,
       const T              a_tMin,
       const T              a_grow,
       State&               a_state,
       const Face&          a_face,
       const Bound&         a_bound) noexcept
  {
    const auto bvh     = a_mesh.getRoot();
    const auto mesh    = a_mesh.getMesh();
    const auto faceIds = bvh.getPrimitives();

    const auto leaf = [&faceIds, &mesh, &a_face](State& a_s, size_t a_offset, size_t a_count) noexcept {
      for (size_t i = a_offset; i < a_offset + a_count; i++) {
        const uint32_t id = faceIds[static_cast<uint32_t>(i)];

        MeshFaces::visit(mesh, id, a_s, a_face);
      }
    };

    bvh.traverse(GrownRayQuery<T>(a_ray, a_tMin, a_grow), a_state, leaf, a_bound);
  }

  /**
   * @brief Call a_face for one DCEL face.
   * @tparam Mesh  DCEL mesh type.
   * @tparam State State type.
   * @tparam Face  Face callback.
   * @param[in]     a_mesh  The mesh.
   * @param[in]     a_id    The face.
   * @param[in,out] a_state State.
   * @param[in]     a_face  The callback.
   */
  template <class Mesh, class State, class Face>
  EBGEOMETRY_HOST_DEVICE
  static inline void
  visit(const Mesh& a_mesh, const uint32_t a_id, State& a_state, const Face& a_face) noexcept
  {
    const auto& face = a_mesh.getFace(a_id);

    const auto edges = [&a_mesh, &face](const auto& a_f) noexcept {
      for (DCEL::EdgeIteratorT<T> iter(a_mesh, face); iter.ok(); ++iter) {
        const auto& edge = a_mesh.getEdge(iter());

        a_f(edge.getVertex(a_mesh).getPosition(), edge.getNextEdge(a_mesh).getVertex(a_mesh).getPosition());
      }
    };

    const auto distance2 = [&a_mesh, &face](const Vec3T<T>& a_point) noexcept {
      return face.unsignedDistance2(a_point, a_mesh);
    };

    a_face(a_state, face.getNormal(), face.getCentroid(), edges, distance2, a_id);
  }
};

/**
 * @brief FlatMeshSDF: every face, in turn.
 * @tparam T Floating-point precision.
 */
template <class T>
struct MeshFaces<FlatMeshSDF<T>>
{
  /**
   * @brief Walk every face.
   * @tparam State State type.
   * @tparam Face  Face callback.
   * @tparam Bound Bound callback (unused: there is no hierarchy to prune).
   * @param[in]     a_mesh  The mesh.
   * @param[in,out] a_state State.
   * @param[in]     a_face  Called for each face.
   */
  template <class State, class Face, class Bound>
  EBGEOMETRY_HOST_DEVICE
  static inline void
  walk(const FlatMeshSDF<T>& a_mesh,
       const RayT<T>& /*a_ray*/,
       const T /*a_tMin*/,
       const T /*a_grow*/,
       State&      a_state,
       const Face& a_face,
       const Bound& /*a_bound*/) noexcept
  {
    const auto mesh = a_mesh.getMesh();

    for (uint32_t i = 0; i < mesh.numFaces(); i++) {
      MeshFaces<MeshSDF<T, 4>>::visit(mesh, i, a_state, a_face);
    }
  }
};

/**
 * @brief What a mesh's nearest offset-face walk finds.
 * @details Internal helper; not part of the public API.
 * @tparam T Floating-point precision.
 */
template <class T>
struct NearestOffsetFace
{
  /**
   * @brief The nearest entry at or after the start, or the end of the interval.
   */
  T m_t;

  /**
   * @brief Its face, or UINT32_MAX.
   */
  uint32_t m_face;

  /**
   * @brief Its outward normal.
   */
  Vec3T<T> m_normal;

  /**
   * @brief Whether an offset face holds the start: the ray starts inside the grown surface.
   */
  bool m_inside;
};

/**
 * @brief The nearest entry after a_tMin of a mesh's faces offset by a radius, and whether one holds a_tMin.
 * @details Internal helper; not part of the public API. Out of line on a device, like every mesh trace.
 * @tparam T Floating-point precision.
 * @tparam S Mesh type.
 * @param[in] a_mesh   The mesh.
 * @param[in] a_ray    The ray.
 * @param[in] a_tMin   Start.
 * @param[in] a_tMax   End.
 * @param[in] a_radius Radius, positive.
 * @return What the walk found.
 */
template <class T, class S>
[[nodiscard]] EBGEOMETRY_HOST_DEVICE
EBGEOMETRY_DEVICE_NOINLINE
inline NearestOffsetFace<T>
nearestOffsetFace(const S& a_mesh, const RayT<T>& a_ray, const T a_tMin, const T a_tMax, const T a_radius) noexcept
{
  NearestOffsetFace<T> found{a_tMax, UINT32_MAX, Vec3T<T>::zeros(), false};

  const auto face = [&a_ray, a_tMin, a_radius](NearestOffsetFace<T>& a_state,
                                               const Vec3T<T>&       a_n,
                                               const Vec3T<T>&       a_p0,
                                               const auto&           a_edges,
                                               const auto&           a_distance2,
                                               const uint32_t        a_id) noexcept {
    if (a_state.m_inside) {
      return;
    }

    T        t      = T(0);
    Vec3T<T> normal = Vec3T<T>::zeros();
    bool     inSlab = false;

    const bool entered =
      RayDetail::offsetFaceEntry(a_ray, a_n, a_p0, a_edges, a_radius, a_tMin, a_state.m_t, t, normal, inSlab);

    // A start within the radius of the face is inside the grown surface.
    if (inSlab && a_distance2(a_ray.at(a_tMin)) <= a_radius * a_radius) {
      a_state.m_inside = true;
      a_state.m_t      = a_tMin;

      return;
    }

    if (entered) {
      a_state.m_t      = t;
      a_state.m_face   = a_id;
      a_state.m_normal = normal;
    }
  };

  const auto bound = [](const NearestOffsetFace<T>& a_state) noexcept -> T { return a_state.m_t; };

  MeshFaces<S>::walk(a_mesh, a_ray, a_tMin, a_radius, found, face, bound);

  return found;
}

/**
 * @brief The union of a mesh's faces offset by a radius along a ray, merged span by span.
 * @details Internal helper; not part of the public API. Out of line on a device, like every mesh trace.
 * @tparam T Floating-point precision.
 * @tparam S Mesh type.
 * @param[in]  a_mesh   The mesh.
 * @param[in]  a_ray    The ray.
 * @param[in]  a_tMin   Start.
 * @param[in]  a_tMax   End.
 * @param[in]  a_radius Radius, positive.
 * @param[out] a_union  The merged spans, with the normals at their exits.
 */
template <class T, class S>
EBGEOMETRY_HOST_DEVICE
EBGEOMETRY_DEVICE_NOINLINE
inline void
offsetFaceUnion(const S&            a_mesh,
                const RayT<T>&      a_ray,
                const T             a_tMin,
                const T             a_tMax,
                const T             a_radius,
                OffsetFaceUnion<T>& a_union) noexcept
{
  a_union.m_count   = 0;
  a_union.m_horizon = a_tMax;

  const auto face = [&a_ray, a_tMin, a_radius](OffsetFaceUnion<T>& a_state,
                                               const Vec3T<T>&     a_n,
                                               const Vec3T<T>&     a_p0,
                                               const auto&         a_edges,
                                               const auto& /*a_distance2*/,
                                               const uint32_t /*a_id*/) noexcept {
    const Piece<T> p = RayDetail::offsetFacePiece(a_ray, a_n, a_p0, a_edges, a_radius);

    if (p.hit() && p.m_out >= a_tMin && p.m_in <= a_state.m_horizon) {
      a_state.insert(p.m_in, p.m_out, p.m_nOut);
    }
  };

  const auto bound = [](const OffsetFaceUnion<T>& a_state) noexcept -> T { return a_state.m_horizon; };

  MeshFaces<S>::walk(a_mesh, a_ray, a_tMin, a_radius, a_union, face, bound);
}

/**
 * @brief A mesh's own spans (RayDetail::shapeSpans()), out of line on a device, where its level sets need them:
 * one copy of its walk per kernel.
 * @details Internal helper; not part of the public API.
 * @tparam T Floating-point precision.
 * @tparam S Mesh type.
 * @param[in]  a_mesh      The mesh.
 * @param[in]  a_ray       The ray.
 * @param[in]  a_tMin      Start.
 * @param[in]  a_tMax      End.
 * @param[in]  a_options   How to march.
 * @param[in]  a_firstOnly Whether only the first crossing is needed.
 * @param[out] a_spans     The list.
 */
template <class T, class S>
EBGEOMETRY_HOST_DEVICE
EBGEOMETRY_DEVICE_NOINLINE
inline void
meshSpans(const S&             a_mesh,
          const RayT<T>&       a_ray,
          const T              a_tMin,
          const T              a_tMax,
          const RayOptions<T>& a_options,
          const bool           a_firstOnly,
          RaySpansT<T>&        a_spans) noexcept
{
  RayDetail::shapeSpans(a_mesh, a_ray, a_tMin, a_tMax, a_options, a_firstOnly, a_spans);
}

/**
 * @brief Whether a shape's level sets are a mesh's (its faces offset), or a fused affine map of one: costly to
 * trace over a whole ray, cheap up to a first crossing.
 * @details Internal helper; not part of the public API.
 * @tparam S Shape type.
 */
template <class S>
struct IsMeshLevel : std::false_type
{
};

/**
 * @brief TriMeshSDF.
 * @tparam T Floating-point precision.
 * @tparam K BVH branching factor.
 * @tparam W SIMD width.
 */
template <class T, size_t K, size_t W>
struct IsMeshLevel<TriMeshSDF<T, K, W>> : std::true_type
{
};

/**
 * @brief MeshSDF.
 * @tparam T Floating-point precision.
 * @tparam K BVH branching factor.
 */
template <class T, size_t K>
struct IsMeshLevel<MeshSDF<T, K>> : std::true_type
{
};

/**
 * @brief FlatMeshSDF.
 * @tparam T Floating-point precision.
 */
template <class T>
struct IsMeshLevel<FlatMeshSDF<T>> : std::true_type
{
};

/**
 * @brief A fused affine map of one.
 * @tparam T Floating-point precision.
 * @tparam C Operand type.
 */
template <class T, class C>
struct IsMeshLevel<Fused::Node<Fused::AffineOp<T>, C>> : IsMeshLevel<C>
{
};

/**
 * @brief An intersection traces a fused offset or annulus of a mesh only within its other operands' spans,
 * where it needs only first crossings: the nearest offset face, not the union of all along the ray.
 * @tparam T         Floating-point precision.
 * @tparam IsAnnulus Whether this is an annulus.
 * @tparam C         Operand type.
 */
template <class T, bool IsAnnulus, class C>
struct DeferredOperand<Fused::Node<Fused::ShiftOp<T, IsAnnulus>, C>, std::enable_if_t<IsMeshLevel<C>::value>>
  : std::true_type
{
};

/**
 * @brief A level set {f <= c} of a shape's value f along a ray.
 * @details Internal helper; not part of the public API. Specialised for the shapes whose level sets have a
 * closed form: Convex (a sphere, capsule, infinite cylinder, plane or box: piece() gives the one convex
 * piece), or a mesh (its surface grown or shrunk by its faces offset by |c|), or a fused affine map of one.
 * @tparam S Shape type.
 */
template <class S, class = void>
struct LevelForm
{
};

/**
 * @brief A sphere's level set: the sphere of radius r + c.
 * @tparam T Floating-point precision.
 */
template <class T>
struct LevelForm<SphereSDF<T>>
{
  /**
   * @brief A convex piece.
   */
  static constexpr bool Convex = true;

  /**
   * @brief The piece.
   * @param[in] a_shape The sphere.
   * @param[in] a_level The level c.
   * @param[in] a_ray   The ray.
   * @return The piece.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  static inline Piece<T>
  piece(const SphereSDF<T>& a_shape, const T a_level, const RayT<T>& a_ray) noexcept
  {
    return RayDetail::ballPiece(a_ray, a_shape.getCenter(), a_shape.getRadius() + a_level);
  }
};

/**
 * @brief A capsule's level set: the capsule of radius r + c.
 * @tparam T Floating-point precision.
 */
template <class T>
struct LevelForm<CapsuleSDF<T>>
{
  /**
   * @brief A convex piece.
   */
  static constexpr bool Convex = true;

  /**
   * @brief The piece.
   * @param[in] a_shape The capsule.
   * @param[in] a_level The level c.
   * @param[in] a_ray   The ray.
   * @return The piece.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  static inline Piece<T>
  piece(const CapsuleSDF<T>& a_shape, const T a_level, const RayT<T>& a_ray) noexcept
  {
    return RayDetail::capsulePiece(a_ray, a_shape.getCenter1(), a_shape.getCenter2(), a_shape.getRadius() + a_level);
  }
};

/**
 * @brief An infinite cylinder's level set: the cylinder of radius r + c.
 * @tparam T Floating-point precision.
 */
template <class T>
struct LevelForm<InfiniteCylinderSDF<T>>
{
  /**
   * @brief A convex piece.
   */
  static constexpr bool Convex = true;

  /**
   * @brief The piece.
   * @param[in] a_shape The cylinder.
   * @param[in] a_level The level c.
   * @param[in] a_ray   The ray.
   * @return The piece.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  static inline Piece<T>
  piece(const InfiniteCylinderSDF<T>& a_shape, const T a_level, const RayT<T>& a_ray) noexcept
  {
    return RayDetail::tubePiece(
      a_ray, a_shape.getCenter(), Vec3T<T>::unit(a_shape.getAxis()), a_shape.getRadius() + a_level);
  }
};

/**
 * @brief A plane's level set: the half-space moved by c along the normal.
 * @tparam T Floating-point precision.
 */
template <class T>
struct LevelForm<PlaneSDF<T>>
{
  /**
   * @brief A convex piece.
   */
  static constexpr bool Convex = true;

  /**
   * @brief The piece.
   * @param[in] a_shape The plane.
   * @param[in] a_level The level c.
   * @param[in] a_ray   The ray.
   * @return The piece.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  static inline Piece<T>
  piece(const PlaneSDF<T>& a_shape, const T a_level, const RayT<T>& a_ray) noexcept
  {
    Piece<T> p;

    p.clip(a_ray, a_shape.getNormal(), dot(a_shape.getNormal(), a_shape.getPoint()) + a_level);

    return p;
  }
};

/**
 * @brief A box's level set: the box shrunk by -c, or grown by c into a rounded box.
 * @tparam T Floating-point precision.
 */
template <class T>
struct LevelForm<BoxSDF<T>>
{
  /**
   * @brief A convex piece.
   */
  static constexpr bool Convex = true;

  /**
   * @brief The piece.
   * @param[in] a_shape The box.
   * @param[in] a_level The level c.
   * @param[in] a_ray   The ray.
   * @return The piece.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  static inline Piece<T>
  piece(const BoxSDF<T>& a_shape, const T a_level, const RayT<T>& a_ray) noexcept
  {
    const Vec3T<T>& lo = a_shape.getLowCorner();
    const Vec3T<T>& hi = a_shape.getHighCorner();

    if (a_level > T(0)) {
      return RayDetail::roundedBoxPiece(a_ray, lo, hi, a_level);
    }

    const Vec3T<T> shrink = -a_level * Vec3T<T>::ones();

    return RayDetail::boxPiece(a_ray, lo + shrink, hi - shrink);
  }
};

/**
 * @brief A mesh's level set: the mesh united with its faces offset by c, or less its faces offset by -c.
 * @tparam S Mesh type.
 */
template <class S>
struct MeshLevelForm
{
  /**
   * @brief Not a convex piece.
   */
  static constexpr bool Convex = false;

  /**
   * @brief A mesh.
   */
  static constexpr bool Mesh = true;
};

/**
 * @brief TriMeshSDF's level sets.
 * @tparam T Floating-point precision.
 * @tparam K BVH branching factor.
 * @tparam W SIMD width.
 */
template <class T, size_t K, size_t W>
struct LevelForm<TriMeshSDF<T, K, W>> : MeshLevelForm<TriMeshSDF<T, K, W>>
{
};

/**
 * @brief MeshSDF's level sets.
 * @tparam T Floating-point precision.
 * @tparam K BVH branching factor.
 */
template <class T, size_t K>
struct LevelForm<MeshSDF<T, K>> : MeshLevelForm<MeshSDF<T, K>>
{
};

/**
 * @brief FlatMeshSDF's level sets.
 * @tparam T Floating-point precision.
 */
template <class T>
struct LevelForm<FlatMeshSDF<T>> : MeshLevelForm<FlatMeshSDF<T>>
{
};

/**
 * @brief Whether a shape's level sets have a closed form (a LevelForm specialisation).
 * @details Internal helper; not part of the public API.
 * @tparam S Shape type.
 */
template <class S, class = void>
struct HasLevelForm : std::false_type
{
};

/**
 * @brief A shape with a LevelForm.
 * @tparam S Shape type.
 */
template <class S>
struct HasLevelForm<S, std::void_t<decltype(LevelForm<S>::Convex)>> : std::true_type
{
};

/**
 * @brief A fused affine map's level sets: its operand's, at the level divided by the map's value scale, along
 * the mapped ray.
 * @tparam T Floating-point precision.
 * @tparam C Operand type.
 */
template <class T, class C>
struct LevelForm<Fused::Node<Fused::AffineOp<T>, C>, std::enable_if_t<HasLevelForm<C>::value>>
{
  /**
   * @brief Not a convex piece of its own.
   */
  static constexpr bool Convex = false;

  /**
   * @brief Not a mesh.
   */
  static constexpr bool Mesh = false;
};

/**
 * @brief A level set of a shape, {f <= c}, or an annulus of it, {f <= c} less {f < -c}: a shape whose ray
 * form traces it from the shape's closed-form level sets.
 * @details Internal helper; not part of the public API. What a fused Offset or Annular, and a tape's Leaf
 * clause of one, trace. Its value is the operand's shifted (or folded, for an annulus) as the operation's.
 * @tparam S Shape type, with a LevelForm.
 */
template <class S>
struct LevelView
{
  /**
   * @brief Floating-point precision.
   */
  using T = ShapePrecision<S>;

  /**
   * @brief The quality of its value: the operand's.
   */
  static constexpr DistanceQuality distanceQuality = distanceQualityOf<S>;

  /**
   * @brief The shape.
   */
  S m_shape;

  /**
   * @brief The level c: the offset, or the annulus's half-thickness.
   */
  T m_level;

  /**
   * @brief Whether this is an annulus.
   */
  bool m_annulus;

  /**
   * @brief The value: f - c, or |f| - c.
   * @param[in] a_point The point.
   * @return The value.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline T
  signedDistance(const Vec3T<T>& a_point) const noexcept
  {
    const T f = m_shape.signedDistance(a_point);

    return (m_annulus ? std::abs(f) : f) - m_level;
  }

  /**
   * @brief The shape's box, grown by the level.
   * @return The box.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  inline BoundingVolumes::AABBT<T>
  computeBoundingVolume() const noexcept
  {
    const BoundingVolumes::AABBT<T> box  = m_shape.computeBoundingVolume();
    const Vec3T<T>                  grow = Math::max(m_level, T(0)) * Vec3T<T>::ones();

    return BoundingVolumes::AABBT<T>(box.getLowCorner() - grow, box.getHighCorner() + grow);
  }
};

/**
 * @brief A level view is called out of line on a device where its shape is.
 * @tparam S Shape type.
 */
template <class S>
struct IsComposition<LevelView<S>> : std::bool_constant<CalledOutOfLine<S>>
{
};

/**
 * @brief The spans of a convex level form's offset (one piece) or annulus (the outer piece less the inner,
 * at most two), with the outward normals at their ends.
 * @details Internal helper; not part of the public API.
 * @tparam T Floating-point precision.
 * @tparam S Shape type.
 * @param[in]  a_view  The view.
 * @param[in]  a_ray   The ray.
 * @param[out] a_spans The spans.
 * @return How many.
 */
template <class T, class S>
EBGEOMETRY_HOST_DEVICE
inline int
convexLevelSpans(const LevelView<S>& a_view, const RayT<T>& a_ray, Piece<T> (&a_spans)[2]) noexcept
{
  if (a_view.m_annulus && !(a_view.m_level > T(0))) {
    return 0;
  }

  const Piece<T> outer = LevelForm<S>::piece(a_view.m_shape, a_view.m_level, a_ray);

  if (!outer.hit()) {
    return 0;
  }

  const Piece<T> inner =
    a_view.m_annulus ? LevelForm<S>::piece(a_view.m_shape, -a_view.m_level, a_ray) : Piece<T>::empty();

  if (!inner.hit()) {
    a_spans[0] = outer;

    return 1;
  }

  int n = 0;

  if (inner.m_in > outer.m_in) {
    a_spans[n++] = Piece<T>{outer.m_in, inner.m_in, outer.m_nIn, -inner.m_nIn};
  }

  if (outer.m_out > inner.m_out) {
    a_spans[n++] = Piece<T>{inner.m_out, outer.m_out, -inner.m_nOut, outer.m_nOut};
  }

  return n;
}

/**
 * @brief A crossing at a distance, with its normal, as a hit.
 * @details Internal helper; not part of the public API.
 * @tparam T Floating-point precision.
 * @param[in] a_ray       The ray.
 * @param[in] a_t         Distance.
 * @param[in] a_normal    Outward normal.
 * @param[in] a_entering  Whether the ray enters there.
 * @param[in] a_primitive The primitive.
 * @return The hit.
 */
template <class T>
[[nodiscard]] EBGEOMETRY_HOST_DEVICE
inline HitT<T>
levelHitAt(
  const RayT<T>& a_ray, const T a_t, const Vec3T<T>& a_normal, const bool a_entering, const uint32_t a_primitive)
{
  HitT<T> hit;

  hit.m_status    = HitStatus::Hit;
  hit.m_t         = a_t;
  hit.m_point     = a_ray.at(a_t);
  hit.m_normal    = a_normal;
  hit.m_entering  = a_entering;
  hit.m_exact     = true;
  hit.m_primitive = a_primitive;

  return hit;
}

/**
 * @brief A level view's outward normal at a point, from its shape's gradient (out of line already): reversed on
 * an annulus's inner side, where the shape's value is negative.
 * @details Internal helper; not part of the public API. Where a crossing's own normal is not found (see
 * levelLeafHit()); the view's own value would bring another copy of a mesh's distance into a kernel.
 * @tparam T Floating-point precision.
 * @tparam S Shape type.
 * @param[in] a_view  The view.
 * @param[in] a_point The point.
 * @param[in] a_h     Finite-difference step.
 * @return The normal.
 */
template <class T, class S>
[[nodiscard]] EBGEOMETRY_HOST_DEVICE
inline Vec3T<T>
levelNormal(const LevelView<S>& a_view, const Vec3T<T>& a_point, const T a_h) noexcept
{
  const Vec3T<T> n = RayDetail::gradientNormal(a_view.m_shape, a_point, a_h);

  return (a_view.m_annulus && RayDetail::valueAt(a_view.m_shape, a_point) < T(0)) ? -n : n;
}

// A level view's spans, out of line on a device: declared here for its ray form, defined after it.
template <class T, class S>
EBGEOMETRY_HOST_DEVICE
EBGEOMETRY_DEVICE_NOINLINE
inline void
levelSpans(const LevelView<S>&  a_view,
           const RayT<T>&       a_ray,
           T                    a_tMin,
           T                    a_tMax,
           const RayOptions<T>& a_options,
           bool                 a_firstOnly,
           RaySpansT<T>&        a_spans) noexcept;

} // namespace RayDetail

/**
 * @brief The ray form of a level view (RayDetail::LevelView): an offset or an annulus of a shape traced from
 * its closed-form level sets, exactly and callable on a device.
 * @details A convex shape's level set is one convex piece and its annulus at most two. A mesh's annulus is
 * the union of its faces offset by the half-thickness, its offset by r > 0 the mesh united with its faces
 * offset by r, and by r < 0 the mesh less its faces offset by -r. A fused affine map passes the level to its
 * operand divided by its value scale, along the mapped ray.
 * @tparam S Shape type.
 */
template <class S>
struct RayForm<RayDetail::LevelView<S>, std::enable_if_t<RayDetail::HasLevelForm<S>::value>>
{
  /**
   * @brief The view type.
   */
  using View = RayDetail::LevelView<S>;

  /**
   * @brief Floating-point precision.
   */
  using T = typename View::T;

  /**
   * @brief The first crossing along a ray.
   * @param[in] a_view    The view.
   * @param[in] a_ray     The ray, with a unit direction.
   * @param[in] a_tMin    Start.
   * @param[in] a_tMax    End.
   * @param[in] a_options How to march (passed on to a mesh's own crossings).
   * @return The crossing, or a miss.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  static inline HitT<T>
  firstHit(const View& a_view, const RayT<T>& a_ray, T a_tMin, T a_tMax, const RayOptions<T>& a_options) noexcept
  {
    using Form = RayDetail::LevelForm<S>;

    if constexpr (Form::Convex) {
      RayDetail::Piece<T> spans[2];

      const int n = RayDetail::convexLevelSpans(a_view, a_ray, spans);

      for (int i = 0; i < n; i++) {
        const RayDetail::Piece<T>& s = spans[i];

        if (s.m_in >= a_tMin && s.m_in <= a_tMax) {
          return RayDetail::levelHitAt(a_ray, s.m_in, s.m_nIn, true, 0U);
        }

        if (s.m_in < a_tMin && s.m_out >= a_tMin && s.m_out <= a_tMax) {
          return RayDetail::levelHitAt(a_ray, s.m_out, s.m_nOut, false, 0U);
        }
      }

      return HitT<T>();
    }
    else if constexpr (Form::Mesh) {
      if (a_view.m_annulus || a_view.m_level == T(0)) {
        if (a_view.m_level == T(0) && !a_view.m_annulus) {
          return RayDetail::shapeHit(a_view.m_shape, a_ray, a_tMin, a_tMax, a_options);
        }

        return annulusHit(a_view, a_ray, a_tMin, a_tMax);
      }

      // An offset of a mesh: from its spans, the crossing's normal from the mesh (leaf 0) or its offset faces
      // (leaf 1).
      const auto trace =
        [&a_view, &a_ray, &a_options](const T a_t0, const T a_t1, const bool a_all, RaySpansT<T>& a_spans) noexcept {
          RayDetail::levelSpans(a_view, a_ray, a_t0, a_t1, a_options, !a_all, a_spans);
        };

      const auto resolve = [&a_view, &a_ray, &a_options](const T a_t, const uint32_t a_leaf) noexcept {
        if (a_leaf == 0U) {
          return RayDetail::leafHit(a_view.m_shape, a_ray, a_t, a_options);
        }

        return annulusLeafHit(View{a_view.m_shape, std::abs(a_view.m_level), true}, a_ray, a_t, a_options);
      };

      return RayDetail::traceSpans(trace, resolve, a_ray, a_tMin, a_tMax);
    }
    else {
      const auto& map = a_view.m_shape.layout().m_op;

      const CSGOps::AffineCoefficients<T> m      = CSGOps::affineCoefficients(map.m_map);
      const RayDetail::MappedRay<T>       mapped = RayDetail::mapRay(m.m_values, a_ray, a_tMin, a_tMax);

      const auto inner = mappedView(a_view);

      HitT<T> hit = EBGeometry::firstHit(inner, mapped.m_ray, mapped.m_t0, mapped.m_t1, a_options);

      if (hit.m_status != HitStatus::Miss) {
        hit.m_t = Math::clamp(hit.m_t / mapped.m_scale, a_tMin, a_tMax);
      }

      if (hit.hit()) {
        hit.m_point  = a_ray.at(hit.m_t);
        hit.m_normal = RayDetail::mapNormalBack(m.m_values, hit.m_normal);
      }

      return hit;
    }
  }

  /**
   * @brief Where a ray is inside over an interval.
   * @param[in]  a_view      The view.
   * @param[in]  a_ray       The ray, with a unit direction.
   * @param[in]  a_tMin      Start.
   * @param[in]  a_tMax      End.
   * @param[in]  a_options   How to march (passed on to a mesh's own spans).
   * @param[in]  a_firstOnly Whether only the first crossing is needed.
   * @param[out] a_spans     The list.
   */
  EBGEOMETRY_HOST_DEVICE
  static inline void
  spans(const View&          a_view,
        const RayT<T>&       a_ray,
        T                    a_tMin,
        T                    a_tMax,
        const RayOptions<T>& a_options,
        bool                 a_firstOnly,
        RaySpansT<T>&        a_spans) noexcept
  {
    using Form = RayDetail::LevelForm<S>;

    if constexpr (Form::Convex) {
      RayDetail::Piece<T> spans[2];

      const int n = RayDetail::convexLevelSpans(a_view, a_ray, spans);

      a_spans.reset(a_tMax);

      for (int i = 0; i < n; i++) {
        a_spans.addClipped(spans[i].m_in, spans[i].m_out, a_tMin, a_tMax);
      }
    }
    else if constexpr (Form::Mesh) {
      const T level = a_view.m_level;

      if (!a_view.m_annulus && level == T(0)) {
        RayDetail::meshSpans(a_view.m_shape, a_ray, a_tMin, a_tMax, a_options, a_firstOnly, a_spans);

        return;
      }

      if (a_view.m_annulus) {
        annulusSpans(a_view, a_ray, a_tMin, a_tMax, a_firstOnly, a_spans);

        return;
      }

      // An offset: the mesh (leaf 0) united with its faces offset by the level (leaf 1), or less those offset
      // by its opposite.
      const View   faces{a_view.m_shape, std::abs(level), true};
      RaySpansT<T> grown;
      RaySpansT<T> scratch;

      if (level > T(0)) {
        RayDetail::meshSpans(a_view.m_shape, a_ray, a_tMin, a_tMax, a_options, a_firstOnly, a_spans);
        annulusSpans(faces, a_ray, a_tMin, a_spans.m_horizon, a_firstOnly, grown);
        RayDetail::nameLeaf(1U, grown);
        RayDetail::uniteInto(a_spans, grown);

        if (a_firstOnly) {
          RayDetail::keepFirst(a_spans);
        }
      }
      else {
        RayDetail::meshSpans(a_view.m_shape, a_ray, a_tMin, a_tMax, a_options, false, a_spans);
        annulusSpans(faces, a_ray, a_tMin, a_spans.m_horizon, false, grown);
        RayDetail::nameLeaf(1U, grown);
        RayDetail::complement(grown, a_tMin, scratch);
        RayDetail::intersectInto(a_spans, scratch, grown);
      }
    }
    else {
      const auto& map = a_view.m_shape.layout().m_op;

      const CSGOps::AffineCoefficients<T> m      = CSGOps::affineCoefficients(map.m_map);
      const RayDetail::MappedRay<T>       mapped = RayDetail::mapRay(m.m_values, a_ray, a_tMin, a_tMax);

      RayDetail::shapeSpans(
        mappedView(a_view), mapped.m_ray, mapped.m_t0, mapped.m_t1, a_options, a_firstOnly, a_spans);
      RayDetail::mapBack(mapped, a_tMin, a_tMax, a_spans);
    }
  }

private:
  /**
   * @brief A fused affine map's view: of its operand, at the level divided by its value scale.
   * @param[in] a_view The view of the map.
   * @return The operand's view.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  static inline auto
  mappedView(const View& a_view) noexcept
  {
    const auto& op      = a_view.m_shape.layout().m_op;
    const auto  operand = RayDetail::fusedOperand<0>(a_view.m_shape);

    return RayDetail::LevelView<std::decay_t<decltype(operand)>>{
      operand, a_view.m_level / op.m_valueScale, a_view.m_annulus};
  }

  /**
   * @brief The first crossing of a mesh's annulus: the nearest entry of its offset faces, or, for a ray that
   * starts inside, where the merged span holding the start ends.
   * @param[in] a_view The view, an annulus.
   * @param[in] a_ray  The ray.
   * @param[in] a_tMin Start.
   * @param[in] a_tMax End.
   * @return The crossing, a miss, or an unresolved query (a start inside more spans than are merged at once).
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  static inline HitT<T>
  annulusHit(const View& a_view, const RayT<T>& a_ray, const T a_tMin, const T a_tMax) noexcept
  {
    if (!(a_view.m_level > T(0))) {
      return HitT<T>();
    }

    const RayDetail::NearestOffsetFace<T> found =
      RayDetail::nearestOffsetFace(a_view.m_shape, a_ray, a_tMin, a_tMax, a_view.m_level);

    if (!found.m_inside) {
      return (found.m_face == UINT32_MAX) ? HitT<T>()
                                          : RayDetail::levelHitAt(a_ray, found.m_t, found.m_normal, true, found.m_face);
    }

    RayDetail::OffsetFaceUnion<T> merged;

    RayDetail::offsetFaceUnion(a_view.m_shape, a_ray, a_tMin, a_tMax, a_view.m_level, merged);

    if (merged.m_count > 0U && merged.m_out[0] < merged.m_horizon) {
      return RayDetail::levelHitAt(a_ray, merged.m_out[0], merged.m_nOut[0], false, 0U);
    }

    HitT<T> hit;

    if (merged.m_horizon < a_tMax) {
      hit.m_status = HitStatus::Unresolved;
      hit.m_t      = merged.m_horizon;
    }

    return hit;
  }

  /**
   * @brief The crossing of a mesh's annulus at a distance along a ray: its normal and face.
   * @details As RayDetail::leafHit(), through annulusHit() rather than firstHit(), which would reach this ray
   * form again: a cycle a device compiler cannot size the stack of.
   * @param[in] a_view    The view, an annulus.
   * @param[in] a_ray     The ray.
   * @param[in] a_t       The crossing's distance.
   * @param[in] a_options For the window's width (RayOptions::m_tolerance).
   * @return The crossing.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  static inline HitT<T>
  annulusLeafHit(const View& a_view, const RayT<T>& a_ray, const T a_t, const RayOptions<T>& a_options) noexcept
  {
    const auto box   = a_view.computeBoundingVolume();
    const T    width = a_options.m_tolerance * RayDetail::scaleOf(box.getLowCorner(), box.getHighCorner(), a_options);

    HitT<T> hit = annulusHit(a_view, a_ray, a_t - width, a_t + width);

    if (!hit.hit()) {
      hit.m_normal = RayDetail::levelNormal(a_view, a_ray.at(a_t), width);
    }

    return hit;
  }

  /**
   * @brief A mesh annulus's spans: up to the nearest entry for a ray outside at the start that needs only its
   * first crossing, and merged span by span otherwise.
   * @param[in]  a_view      The view, an annulus.
   * @param[in]  a_ray       The ray.
   * @param[in]  a_tMin      Start.
   * @param[in]  a_tMax      End.
   * @param[in]  a_firstOnly Whether only the first crossing is needed.
   * @param[out] a_spans     The list.
   */
  EBGEOMETRY_HOST_DEVICE
  static inline void
  annulusSpans(const View&    a_view,
               const RayT<T>& a_ray,
               const T        a_tMin,
               const T        a_tMax,
               const bool     a_firstOnly,
               RaySpansT<T>&  a_spans) noexcept
  {
    a_spans.reset(a_tMax);

    if (!(a_view.m_level > T(0))) {
      return;
    }

    if (a_firstOnly) {
      const RayDetail::NearestOffsetFace<T> found =
        RayDetail::nearestOffsetFace(a_view.m_shape, a_ray, a_tMin, a_tMax, a_view.m_level);

      if (!found.m_inside) {
        if (found.m_face != UINT32_MAX) {
          a_spans.add(SpanEndT<T>{found.m_t, 0U, SpanEndT<T>::Exact}, RayDetail::openAt(found.m_t));

          a_spans.m_horizon = found.m_t;
        }

        return;
      }
    }

    RayDetail::OffsetFaceUnion<T> merged;

    RayDetail::offsetFaceUnion(a_view.m_shape, a_ray, a_tMin, a_tMax, a_view.m_level, merged);

    merged.toSpans(a_tMin, a_spans);
  }
};

namespace RayDetail {

/**
 * @brief A level view's spans (its ray form's spans()), out of line on a device.
 * @details Internal helper; not part of the public API. What a tape's leaf dispatch and a fused offset or
 * annulus call: inlined, every call site copied a mesh's offset-face walks and a box's rounded edges, and ptxas
 * took several times as long and as much memory on a tape kernel.
 * @tparam T Floating-point precision.
 * @tparam S Shape type.
 * @param[in]  a_view      The view.
 * @param[in]  a_ray       The ray.
 * @param[in]  a_tMin      Start.
 * @param[in]  a_tMax      End.
 * @param[in]  a_options   How to march.
 * @param[in]  a_firstOnly Whether only the first crossing is needed.
 * @param[out] a_spans     The list.
 */
template <class T, class S>
EBGEOMETRY_HOST_DEVICE
EBGEOMETRY_DEVICE_NOINLINE
inline void
levelSpans(const LevelView<S>&  a_view,
           const RayT<T>&       a_ray,
           const T              a_tMin,
           const T              a_tMax,
           const RayOptions<T>& a_options,
           const bool           a_firstOnly,
           RaySpansT<T>&        a_spans) noexcept
{
  RayForm<LevelView<S>>::spans(a_view, a_ray, a_tMin, a_tMax, a_options, a_firstOnly, a_spans);
}

/**
 * @brief A level view's spans within another list's spans (as RayDetail::spansWithin()), the other list's
 * distances scaled into the view's frame: what a tape's intersection asks of a deferred operand (a mesh's offset
 * or annulus, see DeferredOperand) once its other operands are known. Out of line on a device.
 * @details Internal helper; not part of the public API.
 * @tparam T Floating-point precision.
 * @tparam S Shape type.
 * @param[in]  a_view      The view.
 * @param[in]  a_ray       The ray, in the view's frame.
 * @param[in]  a_within    The other list, in its own frame.
 * @param[in]  a_scale     Distance along a_ray per distance in a_within's frame.
 * @param[in]  a_options   How to march.
 * @param[in]  a_firstOnly Whether only the intersection's first crossing is needed.
 * @param[out] a_spans     The list.
 */
template <class T, class S>
EBGEOMETRY_HOST_DEVICE
EBGEOMETRY_DEVICE_NOINLINE
inline void
levelSpansWithin(const LevelView<S>&  a_view,
                 const RayT<T>&       a_ray,
                 const RaySpansT<T>&  a_within,
                 const T              a_scale,
                 const RayOptions<T>& a_options,
                 const bool           a_firstOnly,
                 RaySpansT<T>&        a_spans) noexcept
{
  const T horizon = a_within.m_horizon * a_scale;

  a_spans.reset(horizon);

  RaySpansT<T> piece;

  for (uint32_t i = 0; i < a_within.m_count && a_within.m_spans[i].m_in.m_t * a_scale <= horizon; i++) {
    const T t0 = a_within.m_spans[i].m_in.m_t * a_scale;
    const T t1 = Math::min(a_within.m_spans[i].m_out.m_t * a_scale, horizon);

    RayDetail::levelSpans(a_view, a_ray, t0, t1, a_options, a_firstOnly, piece);

    for (uint32_t k = 0; k < piece.m_count && piece.m_spans[k].m_in.m_t <= piece.m_horizon; k++) {
      const typename RaySpansT<T>::Span span = RayDetail::cut(piece.m_spans[k], piece.m_horizon);

      a_spans.add(span.m_in, span.m_out);
    }

    if (piece.m_horizon < t1 || (a_firstOnly && piece.m_count > 0U)) {
      a_spans.m_horizon = Math::min(a_spans.m_horizon, piece.m_horizon);
    }

    // Cut back to this span or before it: the list is known no further, so no later span can add to it.
    if (a_spans.m_horizon <= t1) {
      return;
    }
  }
}

/**
 * @brief A level view's first crossing (its ray form's firstHit()), out of line on a device (see levelSpans()).
 * @details Internal helper; not part of the public API.
 * @tparam T Floating-point precision.
 * @tparam S Shape type.
 * @param[in] a_view    The view.
 * @param[in] a_ray     The ray.
 * @param[in] a_tMin    Start.
 * @param[in] a_tMax    End.
 * @param[in] a_options How to march.
 * @return The crossing.
 */
template <class T, class S>
[[nodiscard]] EBGEOMETRY_HOST_DEVICE
EBGEOMETRY_DEVICE_NOINLINE
inline HitT<T>
levelHit(const LevelView<S>&  a_view,
         const RayT<T>&       a_ray,
         const T              a_tMin,
         const T              a_tMax,
         const RayOptions<T>& a_options) noexcept
{
  return RayForm<LevelView<S>>::firstHit(a_view, a_ray, a_tMin, a_tMax, a_options);
}

/**
 * @brief A level view's crossing at a distance along a ray: its normal and primitive (RayDetail::leafHit() for a
 * level view), out of line on a device.
 * @details Internal helper; not part of the public API. Searches a window of the march's tolerance about the
 * crossing through levelHit(), and takes the normal from levelNormal() where that finds nothing: the generic
 * leafHit() would bring another copy of the view's trace and a mesh's distance into a tape kernel.
 * @tparam T Floating-point precision.
 * @tparam S Shape type.
 * @param[in] a_view    The view.
 * @param[in] a_ray     The ray.
 * @param[in] a_t       The crossing's distance.
 * @param[in] a_options How to march (its tolerance sets the window).
 * @return The crossing.
 */
template <class T, class S>
[[nodiscard]] EBGEOMETRY_HOST_DEVICE
EBGEOMETRY_DEVICE_NOINLINE
inline HitT<T>
levelLeafHit(const LevelView<S>& a_view, const RayT<T>& a_ray, const T a_t, const RayOptions<T>& a_options) noexcept
{
  const auto box   = a_view.computeBoundingVolume();
  const T    width = a_options.m_tolerance * RayDetail::scaleOf(box.getLowCorner(), box.getHighCorner(), a_options);

  HitT<T> nearest;

  T from = a_t - width;

  // As leafHit(): at most three crossings in the window.
  for (int k = 0; k < 3; k++) {
    const HitT<T> hit = RayDetail::levelHit(a_view, a_ray, from, a_t + width, a_options);

    if (!hit.hit()) {
      break;
    }

    if (!nearest.hit() || std::abs(hit.m_t - a_t) < std::abs(nearest.m_t - a_t)) {
      nearest = hit;
    }

    if (hit.m_t >= a_t) {
      break;
    }

    from = hit.m_t + T(1e-3) * width;
  }

  if (!nearest.hit()) {
    nearest.m_normal = RayDetail::levelNormal(a_view, a_ray.at(a_t), width);
  }

  return nearest;
}

} // namespace RayDetail

/**
 * @brief The ray form of a fused offset or annulus of a shape whose level sets have a closed form (see
 * RayDetail::LevelForm): traced through its level view.
 * @tparam T         Floating-point precision.
 * @tparam IsAnnulus Whether this is an annulus.
 * @tparam C         Operand type.
 */
template <class T, bool IsAnnulus, class C>
struct RayForm<Fused::Node<Fused::ShiftOp<T, IsAnnulus>, C>, std::enable_if_t<RayDetail::HasLevelForm<C>::value>>
{
  /**
   * @brief The node type.
   */
  using Node = Fused::Node<Fused::ShiftOp<T, IsAnnulus>, C>;

  /**
   * @brief The first crossing along a ray.
   * @param[in] a_shape   The node.
   * @param[in] a_ray     The ray, with a unit direction.
   * @param[in] a_tMin    Start.
   * @param[in] a_tMax    End.
   * @param[in] a_options How to march.
   * @return The crossing, or a miss.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  static inline HitT<T>
  firstHit(const Node& a_shape, const RayT<T>& a_ray, T a_tMin, T a_tMax, const RayOptions<T>& a_options) noexcept
  {
    return RayDetail::levelHit(view(a_shape), a_ray, a_tMin, a_tMax, a_options);
  }

  /**
   * @brief Where a ray is inside over an interval.
   * @param[in]  a_shape     The node.
   * @param[in]  a_ray       The ray, with a unit direction.
   * @param[in]  a_tMin      Start.
   * @param[in]  a_tMax      End.
   * @param[in]  a_options   How to march.
   * @param[in]  a_firstOnly Whether only the first crossing is needed.
   * @param[out] a_spans     The list.
   */
  EBGEOMETRY_HOST_DEVICE
  static inline void
  spans(const Node&          a_shape,
        const RayT<T>&       a_ray,
        T                    a_tMin,
        T                    a_tMax,
        const RayOptions<T>& a_options,
        bool                 a_firstOnly,
        RaySpansT<T>&        a_spans) noexcept
  {
    RayDetail::levelSpans(view(a_shape), a_ray, a_tMin, a_tMax, a_options, a_firstOnly, a_spans);
  }

private:
  /**
   * @brief The node's level view.
   * @param[in] a_shape The node.
   * @return The view.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  static inline RayDetail::LevelView<C>
  view(const Node& a_shape) noexcept
  {
    return RayDetail::LevelView<C>{RayDetail::fusedOperand<0>(a_shape), a_shape.layout().m_op.m_amount, IsAnnulus};
  }
};

} // namespace EBGeometry

// The ray forms of the smooth unions and intersections, which build on these: whoever has the offsets' ray forms
// has theirs too. Included last, since that header includes this one.
#include "EBGeometry_SmoothRayForms.hpp"

#endif
