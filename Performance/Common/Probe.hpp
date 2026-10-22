// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file   Probe.hpp
 * @brief  What the probes have in common: the signed-distance query, checksums of results, how a host
 * and a device result are judged to agree, and the timing of a query loop and of a build.
 */

#ifndef EBGEOMETRY_PERF_PROBE_HPP
#define EBGEOMETRY_PERF_PROBE_HPP

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <optional>
#include <type_traits>
#include <utility>
#include <vector>

#include <EBGeometry.hpp>

#include "Case.hpp"

namespace Perf {

/**
 * @brief A precision's name in a case name.
 * @tparam T float or double.
 * @return "float" or "double".
 */
template <class T>
constexpr const char*
precisionName() noexcept
{
  static_assert(std::is_same_v<T, float> || std::is_same_v<T, double>, "Perf: precision must be float or double");

  return std::is_same_v<T, float> ? "float" : "double";
}

/**
 * @brief The signed distance to a shape (a mesh SDF, a union): one query per point, on either side.
 * @tparam Shape A trivially copyable type with signedDistance(const Vec3T<T>&).
 * @tparam T     Precision.
 */
template <class Shape, class T>
struct SignedDistance
{
  Shape m_shape; ///< The shape, by value (a descriptor: its pool must outlive the query).

  /**
   * @brief The signed distance from a point to the shape.
   * @param[in] a_point The point.
   * @return The signed distance.
   */
  EBGEOMETRY_HOST_DEVICE
  T
  operator()(const EBGeometry::Vec3T<T>& a_point) const noexcept
  {
    return m_shape.signedDistance(a_point);
  }
};

/**
 * @brief A checksum of signed distances: their sum.
 * @tparam T Precision.
 * @param[in] a_values The distances.
 * @return The checksum.
 */
template <class T>
double
checksumOf(const std::vector<T>& a_values)
{
  static_assert(std::is_floating_point_v<T>, "Perf::checksumOf: a vector of distances");

  double sum = 0.0;

  for (const T v : a_values) {
    sum += static_cast<double>(v);
  }

  return sum;
}

/**
 * @brief A checksum of point-cloud hits' distances: the sum of their squared distances. The points they
 * found are checked separately, and exactly, by IndexChecksum.
 * @tparam T Precision.
 * @param[in] a_hits The hits.
 * @return The sum.
 */
template <class T>
double
checksumOf(const std::vector<EBGeometry::PointCloud::Hit<T>>& a_hits)
{
  double sum = 0.0;

  for (const auto& h : a_hits) {
    sum += static_cast<double>(h.distanceSquared);
  }

  return sum;
}

/**
 * @brief An exact checksum of the points a set of queries found: a 64-bit FNV-style hash of their
 * indices, in query order, so that a different point (at the same distance, or for another query)
 * changes it. A floating-point sum cannot do this: an index small enough not to swamp the distances is
 * below the rounding a comparison has to allow.
 */
class IndexChecksum
{
public:
  /**
   * @brief Add the next index.
   * @param[in] a_index The index.
   */
  void
  add(std::uint32_t a_index) noexcept
  {
    m_hash = (m_hash ^ a_index) * 1099511628211ULL;
  }

  /**
   * @brief The hash.
   * @return Its value.
   */
  [[nodiscard]] std::uint64_t
  value() const noexcept
  {
    return m_hash;
  }

private:
  std::uint64_t m_hash = 14695981039346656037ULL;
};

/**
 * @brief The index checksum of point-cloud hits.
 * @tparam T Precision.
 * @param[in] a_hits The hits, in query order.
 * @return The hash of their indices.
 */
template <class T>
std::uint64_t
indexChecksumOf(const std::vector<EBGeometry::PointCloud::Hit<T>>& a_hits)
{
  IndexChecksum sum;

  for (const auto& h : a_hits) {
    sum.add(h.index);
  }

  return sum.value();
}

/**
 * @brief How far a host and a device result may differ before they disagree: a thousand units in the
 * last place of a value of size a_scale (at least 1).
 * @tparam T Precision.
 * @param[in] a_scale The size of the value.
 * @return The tolerance.
 */
template <class T>
double
agreementTolerance(double a_scale)
{
  return 1000.0 * static_cast<double>(EBGeometry::Math::Limits<T>::epsilon()) * std::max(1.0, a_scale);
}

/**
 * @brief Whether a host and a device signed distance agree: the same sign, and within
 * agreementTolerance().
 * @tparam T Precision.
 * @param[in] a_host   The host's distance.
 * @param[in] a_device The device's distance.
 * @return The difference, and whether it is a disagreement.
 */
template <class T>
std::pair<double, bool>
agreeDistance(T a_host, T a_device)
{
  const double diff = std::abs(static_cast<double>(a_host) - static_cast<double>(a_device));

  return {diff,
          (a_host < T(0)) != (a_device < T(0)) || diff > agreementTolerance<T>(std::abs(static_cast<double>(a_host)))};
}

/**
 * @brief Whether a host and a device point-cloud hit agree: their squared distances are within
 * agreementTolerance(), whichever points they found (equal distances to different points are ties, not
 * disagreements).
 * @tparam T Precision.
 * @param[in] a_host   The host's hit.
 * @param[in] a_device The device's hit.
 * @return The difference of the squared distances, and whether it is a disagreement.
 */
template <class T>
std::pair<double, bool>
agreeHit(const EBGeometry::PointCloud::Hit<T>& a_host, const EBGeometry::PointCloud::Hit<T>& a_device)
{
  const double dh   = static_cast<double>(a_host.distanceSquared);
  const double diff = std::abs(dh - static_cast<double>(a_device.distanceSquared));

  return {diff, diff > agreementTolerance<T>(dh)};
}

/**
 * @brief The number of points at which a build or refit case queries what it built, untimed, so that its
 * checksum depends on the structure itself rather than on its size alone (at Perf::Seed::BuildChecks).
 */
constexpr std::size_t BuildCheckPoints = 1000;

/**
 * @brief A query's results over a set of inputs, untimed (a build case's check of what it built).
 * @tparam Query A callable taking const In&.
 * @tparam In    Input type.
 * @param[in] a_query The query.
 * @param[in] a_in    The inputs.
 * @return The results, one per input.
 */
template <class Query, class In>
auto
queryAll(const Query& a_query, const std::vector<In>& a_in)
{
  std::vector<std::decay_t<decltype(a_query(a_in[0]))>> out;
  out.reserve(a_in.size());

  for (const auto& x : a_in) {
    out.push_back(a_query(x));
  }

  return out;
}

/**
 * @brief Time a query over a set of inputs on the host, as the case's own time.
 * @tparam Query A callable taking const In&.
 * @tparam In    Input type.
 * @tparam Out   Result type.
 * @param[in,out] a_case  The running case.
 * @param[in]     a_query The query.
 * @param[in]     a_in    The inputs, in the order they are queried.
 * @param[out]    a_out   The results, one per input.
 */
template <class Query, class In, class Out>
void
measureHostQuery(Case& a_case, const Query& a_query, const std::vector<In>& a_in, std::vector<Out>& a_out)
{
  a_out.resize(a_in.size());

  a_case.measure(
    [&] {
      for (std::size_t i = 0; i < a_in.size(); i++) {
        a_out[i] = a_query(a_in[i]);
      }
    },
    a_in.size());
}

/**
 * @brief A structure built by measureBuild(), with the pool it was built in.
 * @tparam Object The structure.
 */
template <class Object>
struct Built
{
  std::unique_ptr<EBGeometry::Pool> pool;   ///< The pool; it must outlive the object.
  std::unique_ptr<Object>           object; ///< The structure, from the last repetition.
};

/**
 * @brief Time building a structure, as the case's own time: every repetition builds it afresh in a new
 * pool, after an untimed preparation in that pool. The pool's memory is fresh pages every time (see
 * fixAllocatorPolicy()), so every repetition pays the page faults of a first build.
 * @tparam Object  The structure.
 * @tparam Prepare Callable taking EBGeometry::Pool& and returning what the build needs (untimed: reading
 * a mesh into the pool, say).
 * @tparam Build   Callable taking EBGeometry::Pool& and the prepared value, returning the structure.
 * @param[in,out] a_case    The running case.
 * @param[in]     a_prepare The untimed preparation.
 * @param[in]     a_build   The timed build.
 * @return The last structure built, and its pool (for its pool bytes and a checksum).
 */
template <class Object, class Prepare, class Build>
Built<Object>
measureBuild(Case& a_case, Prepare&& a_prepare, Build&& a_build)
{
  using State = std::decay_t<decltype(a_prepare(std::declval<EBGeometry::Pool&>()))>;

  Built<Object>        built;
  std::optional<State> state;

  a_case.measureFresh(
    [&] {
      built.object.reset();
      state.reset();
      built.pool = std::make_unique<EBGeometry::Pool>(EBGeometry::hostMemoryResource());
      state.emplace(a_prepare(*built.pool));
    },
    [&] { built.object = std::make_unique<Object>(a_build(*built.pool, *state)); });

  return built;
}

/**
 * @brief Time building a structure that needs no preparation (see the overload with one).
 * @tparam Object The structure.
 * @tparam Build  Callable taking EBGeometry::Pool& and returning the structure.
 * @param[in,out] a_case  The running case.
 * @param[in]     a_build The timed build.
 * @return The last structure built, and its pool.
 */
template <class Object, class Build>
Built<Object>
measureBuild(Case& a_case, Build&& a_build)
{
  return measureBuild<Object>(
    a_case, [](EBGeometry::Pool&) { return 0; }, [&](EBGeometry::Pool& a_pool, int) { return a_build(a_pool); });
}

} // namespace Perf

#endif
