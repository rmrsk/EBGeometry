// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
// SPDX-License-Identifier: BSD-3-Clause

// Test helpers for composed objects: a Tape compiled into its own pool, so a test can build an Expr and
// evaluate it in one line, a constant-valued shape for checking that weighted averages (Blur,
// Mollify) are partitions of unity, and a row of labeled spheres.

#ifndef EBGEOMETRY_TESTTAPEUTILS_HPP
#define EBGEOMETRY_TESTTAPEUTILS_HPP

#include "EBGeometry.hpp"

#include <vector>

namespace EBGeometry::TestUtils {

// A Tape together with the pool it is compiled into. Not copyable: the tape refers to the pool.
template <class T, class... UserPrimitives>
class Compiled
{
public:
  explicit Compiled(const Expr<T>& a_expr, const TapeOptions& a_options = TapeOptions())
    : m_pool(hostMemoryResource()), m_tape(m_pool, a_expr, a_options)
  {}

  Compiled(const Compiled&) = delete;
  Compiled&
  operator=(const Compiled&) = delete;

  [[nodiscard]] T
  operator()(const Vec3T<T>& a_point) const noexcept
  {
    return m_tape.signedDistance(a_point);
  }

  [[nodiscard]] T
  signedDistance(const Vec3T<T>& a_point) const noexcept
  {
    return m_tape.signedDistance(a_point);
  }

  [[nodiscard]] const Tape<T, UserPrimitives...>&
  tape() const noexcept
  {
    return m_tape;
  }

private:
  Pool                       m_pool;
  Tape<T, UserPrimitives...> m_tape;
};

// A shape whose value is the same everywhere; it fills all of space if the value is negative and is
// empty otherwise, so its box is unbounded.
template <class T>
struct ConstantSDF
{
  T m_value = T(0);

  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  T
  signedDistance(const Vec3T<T>& /*a_point*/) const noexcept
  {
    return m_value;
  }

  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  BoundingVolumes::AABBT<T>
  computeBoundingVolume() const noexcept
  {
    return BoundingVolumes::AABBT<T>(-Vec3T<T>::max(), Vec3T<T>::max());
  }

  static constexpr DistanceQuality distanceQuality = DistanceQuality::NotADistance;
};

// Five unit spheres three apart along x, labeled left, right, right, none and left: two names, one shared by
// separate objects, one given to a union.
template <class T>
Expr<T>
labeledRow()
{
  std::vector<SphereSDF<T>> s;

  for (int i = 0; i < 5; i++) {
    s.emplace_back(Vec3T<T>(T(3 * i), T(0), T(0)), T(1));
  }

  return Union(std::vector<Expr<T>>{
    SurfaceLabel(s[0], "left"), SurfaceLabel(Union(s[1], s[2]), "right"), s[3], SurfaceLabel(s[4], "left")});
}

} // namespace EBGeometry::TestUtils

#endif
