// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
// SPDX-License-Identifier: GPL-3.0-or-later

// Shared helpers for death tests: run an operation in a forked child and check that it aborts
// (SIGABRT), which is how EBGEOMETRY_REQUIRE and EBGEOMETRY_EXPECT signal a failed check. Not part of
// the library itself -- test infrastructure only.
//
// EBGEOMETRY_REQUIRE is always on, so its death tests run in every build. EBGEOMETRY_EXPECT is a no-op
// unless EBGEOMETRY_ENABLE_ASSERTIONS is defined, so callers guard those death tests on that macro.

#ifndef EBGEOMETRY_TEST_DEATH_HPP
#define EBGEOMETRY_TEST_DEATH_HPP

#include <csignal>
#include <cstdio>
#include <optional>
#include <string>
#include <utility>

#include <sys/wait.h>
#include <unistd.h>

// Run a_fn in a forked child. Returns what the child wrote to stderr if it terminated via SIGABRT,
// and std::nullopt otherwise. If the operation wrongly returns without aborting, the child calls
// _exit(0) -- bypassing atexit handlers so a sanitizer leak check on the deliberately-abandoned child
// state is not run.
template <class F>
static std::optional<std::string>
abortMessage(F&& a_fn)
{
  std::FILE* captured = std::tmpfile();

  if (captured == nullptr) {
    return std::nullopt;
  }

  const pid_t pid = fork();

  if (pid == 0) {
    // Silence the child: the expected diagnostic goes to the capture file, and Catch2's own
    // SIGABRT-handler summary (stdout, reported as a failure of the child's copy of the test) would
    // otherwise clutter the parent test's output and read like a real failure.
    // (Release builds with glibc's fortified headers reject discarding these results via a cast.)
    if (std::freopen("/dev/null", "w", stdout) == nullptr || dup2(fileno(captured), STDERR_FILENO) < 0) {
      _exit(1);
    }

    a_fn();

    _exit(0);
  }

  int status = 0;

  (void)waitpid(pid, &status, 0);

  std::string message;

  std::rewind(captured);

  for (int c = std::fgetc(captured); c != EOF; c = std::fgetc(captured)) {
    message.push_back(static_cast<char>(c));
  }

  std::fclose(captured);

  if ((WIFSIGNALED(status) != 0) && (WTERMSIG(status) == SIGABRT)) {
    return message;
  }

  return std::nullopt;
}

// True iff a_fn aborts (SIGABRT) when run in a forked child.
template <class F>
static bool
aborts(F&& a_fn)
{
  return abortMessage(std::forward<F>(a_fn)).has_value();
}

// True iff a_fn aborts and its stderr output contains a_expected.
template <class F>
static bool
abortsWith(F&& a_fn, const std::string& a_expected)
{
  const auto message = abortMessage(std::forward<F>(a_fn));

  return message.has_value() && message->find(a_expected) != std::string::npos;
}

// The name the EBGEOMETRY_EXPECT death tests use.
template <class F>
static bool
abortsUnderAssertions(F&& a_fn)
{
  return aborts(std::forward<F>(a_fn));
}

#endif // EBGEOMETRY_TEST_DEATH_HPP
