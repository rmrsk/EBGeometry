// SPDX-FileCopyrightText: 2024 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
  @file   EBGeometry_SimpleTimerImplem.hpp
  @brief  Implementation of EBGeometry_SimpleTimer.hpp
  @author Robert Marskar
*/

#ifndef EBGEOMETRY_SIMPLETIMERIMPLEM_HPP
#define EBGEOMETRY_SIMPLETIMERIMPLEM_HPP

// Std includes
#include <chrono>

// Our includes
#include "EBGeometry_SimpleTimer.hpp"

namespace EBGeometry {

inline SimpleTimer::SimpleTimer() noexcept
{
  this->start();
  this->stop();
}

inline void
SimpleTimer::start() noexcept
{
  m_start = Clock::now();
}

inline void
SimpleTimer::stop() noexcept
{
  m_stop = Clock::now();
}

inline double
SimpleTimer::seconds() const noexcept
{
  return std::chrono::duration_cast<std::chrono::duration<double>>(m_stop - m_start).count();
}

} // namespace EBGeometry

#endif
