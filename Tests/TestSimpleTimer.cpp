// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
// SPDX-License-Identifier: GPL-3.0-or-later

// Test suite for EBGeometry_SimpleTimer.hpp. Previously untested. Timing-based checks use
// generous, one-sided bounds (a lower bound well below the requested sleep duration, and an upper
// bound far above it) to avoid flakiness on loaded CI runners while still exercising the actual
// start()/stop()/seconds() mechanics rather than just compiling them.

#include "EBGeometry.hpp"

#include <thread>

#include <catch2/catch_test_macros.hpp>

using namespace EBGeometry;

TEST_CASE("SimpleTimer: default construction leaves it in a valid, near-zero-elapsed state", "[SimpleTimer]")
{
  const SimpleTimer timer;

  REQUIRE(timer.seconds() >= 0.0);
  REQUIRE(timer.seconds() < 1.0);
}

TEST_CASE("SimpleTimer: measures at least the requested sleep duration", "[SimpleTimer]")
{
  SimpleTimer timer;

  timer.start();
  std::this_thread::sleep_for(std::chrono::milliseconds(20));
  timer.stop();

  // Sleep guarantees are one-sided (may oversleep, never undersleep by design), so only assert a
  // lower bound close to the requested duration and a generous upper bound.
  REQUIRE(timer.seconds() >= 0.015);
  REQUIRE(timer.seconds() < 5.0);
}

TEST_CASE("SimpleTimer: calling start() again resets the timer", "[SimpleTimer]")
{
  SimpleTimer timer;

  timer.start();
  std::this_thread::sleep_for(std::chrono::milliseconds(20));

  // Restarting must discard the elapsed time accumulated so far.
  timer.start();
  timer.stop();

  REQUIRE(timer.seconds() < 0.015);
}

TEST_CASE("SimpleTimer: a longer sleep yields a proportionally longer measured duration", "[SimpleTimer]")
{
  SimpleTimer shortTimer;
  shortTimer.start();
  std::this_thread::sleep_for(std::chrono::milliseconds(10));
  shortTimer.stop();

  SimpleTimer longTimer;
  longTimer.start();
  std::this_thread::sleep_for(std::chrono::milliseconds(50));
  longTimer.stop();

  REQUIRE(longTimer.seconds() > shortTimer.seconds());
}
