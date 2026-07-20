// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
// SPDX-License-Identifier: GPL-3.0-or-later

// Shared helper for assert-death tests: run an operation in a forked child and check that it
// aborts (SIGABRT), which is how EBGEOMETRY_EXPECT signals a violated invariant. Not part of the
// library itself -- test infrastructure only.
//
// The whole facility is compiled only when EBGEOMETRY_ENABLE_ASSERTIONS is defined (the debug and
// debug-san presets); with assertions off EBGEOMETRY_EXPECT is a no-op, so there is nothing to
// test and callers guard their death checks on the same macro.

#ifndef EBGEOMETRY_TEST_DEATH_HPP
#define EBGEOMETRY_TEST_DEATH_HPP

#if defined(EBGEOMETRY_ENABLE_ASSERTIONS)

#include <csignal>
#include <cstdio>

#include <sys/wait.h>
#include <unistd.h>

// Run a_fn in a forked child and return true iff the child terminated via SIGABRT (i.e. an
// EBGEOMETRY_EXPECT fired). If the operation wrongly returns without aborting, the child calls
// _exit(0) -- bypassing atexit handlers so a sanitizer leak check on the deliberately-abandoned
// child state is not run -- and this returns false.
template <class F>
static bool
abortsUnderAssertions(F&& a_fn)
{
  const pid_t pid = fork();

  if (pid == 0) {
    // Silence the child's stderr: the expected EBGEOMETRY_EXPECT diagnostic and Catch2's own
    // SIGABRT-handler summary would otherwise clutter the parent test's output.
    (void)std::freopen("/dev/null", "w", stderr);

    a_fn();

    _exit(0);
  }

  int status = 0;

  (void)waitpid(pid, &status, 0);

  return (WIFSIGNALED(status) != 0) && (WTERMSIG(status) == SIGABRT);
}

#endif // EBGEOMETRY_ENABLE_ASSERTIONS

#endif // EBGEOMETRY_TEST_DEATH_HPP
