// SPDX-FileCopyrightText: 2024 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file   EBGeometry_SimpleTimer.hpp
 * @brief  Header for the SimpleTimer class
 * @author Robert Marskar
 */

#ifndef EBGEOMETRY_SIMPLETIMER_HPP
#define EBGEOMETRY_SIMPLETIMER_HPP

// Std includes
#include <chrono>

namespace EBGeometry {

/**
 * @brief Simple timer class used for local performance profiling. Does not include MPI capabilities and is therefore local to each rank.
 */
class SimpleTimer
{
public:
  /**
   * @brief Clock alias
   */
  using Clock = std::chrono::steady_clock;

  /**
   * @brief Time point alias
   */
  using TimePoint = Clock::time_point;

  /**
   * @brief Constructor. Immediately calls start() followed by stop() to initialise both
   * time points to the current instant, leaving the timer in a valid state with
   * approximately zero elapsed time.
   */
  inline SimpleTimer() noexcept;

  /**
   * @brief Destructor
   */
  inline ~SimpleTimer() noexcept = default;

  /**
   * @brief Start timing
   */
  inline void
  start() noexcept;

  /**
   * @brief Stop timing
   */
  inline void
  stop() noexcept;

  /**
   * @brief Compute the elapsed time between the last start() and stop() calls.
   * @return Elapsed time in seconds as a double.
   */
  inline double
  seconds() const noexcept;

protected:
  /**
   * @brief Start point
   */
  TimePoint m_start;

  /**
   * @brief Stop point
   */
  TimePoint m_stop;
};

} // namespace EBGeometry

#include "EBGeometry_SimpleTimerImplem.hpp"

#endif
