// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file   Statistics.hpp
 * @brief  The statistics the probes and PerfCompare use: quantiles, the summary of a case's
 * repetitions, and the bootstrap interval PerfCompare judges a change by.
 * @details Plain C++17 with no dependency on EBGeometry, so that PerfCompare can use it.
 */

#ifndef EBGEOMETRY_PERF_STATISTICS_HPP
#define EBGEOMETRY_PERF_STATISTICS_HPP

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <vector>

namespace Perf {

/**
 * @brief A quantile of a set of values, interpolated linearly between the two nearest ranks.
 * @param[in] a_values The values.
 * @param[in] a_q      The quantile, in [0, 1] (0.5 is the median).
 * @return The quantile, or 0 for no values.
 */
inline double
quantile(std::vector<double> a_values, double a_q)
{
  if (a_values.empty()) {
    return 0.0;
  }

  std::sort(a_values.begin(), a_values.end());

  const double      pos = a_q * static_cast<double>(a_values.size() - 1);
  const std::size_t lo  = static_cast<std::size_t>(pos);
  const std::size_t hi  = std::min(lo + 1, a_values.size() - 1);

  return a_values[lo] + (pos - static_cast<double>(lo)) * (a_values[hi] - a_values[lo]);
}

/**
 * @brief The median of a set of values.
 * @param[in] a_values The values.
 * @return The median, or 0 for no values.
 */
inline double
median(const std::vector<double>& a_values)
{
  return quantile(a_values, 0.5);
}

/**
 * @brief What the report says about a case's repetitions.
 */
struct Summary
{
  double median  = 0.0; ///< The median.
  double iqr     = 0.0; ///< The interquartile range.
  double minimum = 0.0; ///< The smallest value.
};

/**
 * @brief Summarize a set of repetitions.
 * @param[in] a_values The values.
 * @return Their median, interquartile range and minimum (all 0 for no values).
 */
inline Summary
summarize(const std::vector<double>& a_values)
{
  Summary s;

  if (!a_values.empty()) {
    s.median  = median(a_values);
    s.iqr     = quantile(a_values, 0.75) - quantile(a_values, 0.25);
    s.minimum = *std::min_element(a_values.begin(), a_values.end());
  }

  return s;
}

/**
 * @brief An interval [low, high].
 */
struct Interval
{
  double low  = 0.0; ///< Lower end.
  double high = 0.0; ///< Upper end.
};

/**
 * @brief A bootstrap 95% confidence interval for median(head) / median(base): both samples resampled
 * with replacement 2000 times, from a fixed seed so that the same results always give the same
 * interval.
 * @param[in] a_base The baseline's values; not empty.
 * @param[in] a_head The candidate's values; not empty.
 * @return The 2.5th and 97.5th percentiles of the resampled ratio.
 */
inline Interval
bootstrapRatio(const std::vector<double>& a_base, const std::vector<double>& a_head)
{
  constexpr std::size_t resamples = 2000;

  std::uint64_t state = 88172645463325252ULL;

  const auto pick = [&state](std::size_t a_n) -> std::size_t {
    state ^= state << 13;
    state ^= state >> 7;
    state ^= state << 17;

    return state % a_n;
  };

  std::vector<double> ratios(resamples);
  std::vector<double> b(a_base.size());
  std::vector<double> h(a_head.size());

  for (std::size_t r = 0; r < resamples; r++) {
    for (auto& x : b) {
      x = a_base[pick(a_base.size())];
    }

    for (auto& x : h) {
      x = a_head[pick(a_head.size())];
    }

    ratios[r] = median(h) / median(b);
  }

  std::sort(ratios.begin(), ratios.end());

  return {ratios[resamples / 40], ratios[resamples - 1 - resamples / 40]};
}

} // namespace Perf

#endif
