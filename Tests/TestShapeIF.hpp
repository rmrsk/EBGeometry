// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
// SPDX-License-Identifier: GPL-3.0-or-later

// Test-only helper: presents an analytic shape (a plain value type with a signedDistance() member)
// as an ImplicitFunction, so the host-side transform and CSG tests can keep using the real shape
// formulas as their inputs. This is deliberately not part of the library: the shapes are not
// ImplicitFunction objects, and composing them in the library returns with the tape.

#ifndef EBGEOMETRY_TESTSHAPEIF_HPP
#define EBGEOMETRY_TESTSHAPEIF_HPP

#include "EBGeometry.hpp"

#include <utility>

namespace EBGeometry::TestUtils {

// An ImplicitFunction<T> evaluating a Shape<T> by value. Constructor arguments are forwarded to the
// shape, so std::make_shared<ShapeIF<T, SphereSDF>>(center, radius) builds the same sphere that
// SphereSDF<T>(center, radius) does.
template <class T, template <class> class Shape>
class ShapeIF : public ImplicitFunction<T>
{
public:
  template <class... Args>
  explicit ShapeIF(Args&&... a_args) noexcept : m_shape(std::forward<Args>(a_args)...)
  {}

  [[nodiscard]] T
  value(const Vec3T<T>& a_point) const noexcept override
  {
    return m_shape.signedDistance(a_point);
  }

  [[nodiscard]] T
  signedDistance(const Vec3T<T>& a_point) const noexcept
  {
    return m_shape.signedDistance(a_point);
  }

  [[nodiscard]] const Shape<T>&
  shape() const noexcept
  {
    return m_shape;
  }

private:
  Shape<T> m_shape;
};

template <class T>
using SphereIF = ShapeIF<T, SphereSDF>;

template <class T>
using BoxIF = ShapeIF<T, BoxSDF>;

} // namespace EBGeometry::TestUtils

#endif
