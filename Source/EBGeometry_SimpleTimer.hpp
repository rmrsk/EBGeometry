/* chombo-discharge
 * Copyright © 2024 Robert Marskar
 * Please refer to Copyright.txt and LICENSE in the chombo-discharge root directory.
 */

/*!
  @file   EBGeometry_SimpleTimer.hpp
  @brief  Header for the SimpleTimer class
  @author Robert Marskar
*/

#ifndef EBGeometry_SimpleTimer
#define EBGeometry_SimpleTimer

// Std includes
#include <chrono>

namespace EBGeometry {

  /*!
    @brief Simple timer class used for local performance profiling. Does not include MPI capabilities and is therefore local to each rank. 
  */
  class SimpleTimer
  {
  public:
    /*!
      @brief Clock alias
    */
    using Clock = std::chrono::steady_clock;

    /*!
      @brief Time point alias
    */
    using TimePoint = Clock::time_point;

    /*!
      @brief Constructor
    */
    inline SimpleTimer() noexcept;

    /*!
      @brief Destructor
    */
    inline ~SimpleTimer() noexcept = default;

    /*!
      @brief Start timing
    */
    inline void
    start() noexcept;

    /*!
      @brief Stop timing
    */
    inline void
    stop() noexcept;

    /*!
      @brief Report result -- prints result in seconds
    */
    inline double
    seconds() const noexcept;

  protected:
    /*!
      @brief Start point
    */
    TimePoint m_start;

    /*!
      @brief Stop point
    */
    TimePoint m_stop;
  };

} // namespace EBGeometry

#include "EBGeometry_SimpleTimerImplem.hpp"

#endif
