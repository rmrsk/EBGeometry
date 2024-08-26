/* chombo-discharge
 * Copyright © 2024 Robert Marskar
 * Please refer to Copyright.txt and LICENSE in the chombo-discharge root directory.
 */

/*!
  @file   EBGeometry_SimpleTimerImplem.hpp
  @brief  Implementation of EBGeometry_SimpleTimer.hpp
  @author Robert Marskar
*/

#ifndef EBGeometry_SimpleTimerImplem
#define EBGeometry_SimpleTimerImplem

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
