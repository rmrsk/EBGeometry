// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

// BuildCost: runs one compile and records what it cost. Compare.cmake sets it as CMake's compiler
// launcher (CMAKE_<LANG>_COMPILER_LAUNCHER), so every compile of a comparison's builds goes through it:
//
//   BuildCost --record=<file.jsonl> <compiler> <arguments...>
//
// It runs the compiler, passing its output through, and appends one JSON line per compile to the record:
// the source, the object, the CPU time (user and system, of the compiler and every process it ran) and the
// peak memory (the largest such process). CPU time rather than wall time, because the builds run in parallel
// on a machine that may be doing other work, and CPU time barely moves with that. For nvcc it also asks
// ptxas for its statistics (-Xptxas -v, which changes nothing in the code) and records one line per kernel:
// registers, cumulative stack and spill bytes. It exits with the compiler's status. POSIX only (Linux and
// macOS), as is the rest of the comparison.

#include <cerrno>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cxxabi.h>
#include <fcntl.h>
#include <regex>
#include <sstream>
#include <string>
#include <sys/resource.h>
#include <sys/wait.h>
#include <unistd.h>
#include <vector>

namespace {

// A string as a JSON string literal.
std::string
quoted(const std::string& a_text)
{
  std::string out = "\"";

  for (const char c : a_text) {
    switch (c) {
    case '"': {
      out += "\\\"";

      break;
    }
    case '\\': {
      out += "\\\\";

      break;
    }
    case '\n': {
      out += "\\n";

      break;
    }
    case '\t': {
      out += "\\t";

      break;
    }
    default: {
      if (static_cast<unsigned char>(c) < 0x20) {
        char buffer[8];

        std::snprintf(buffer, sizeof(buffer), "\\u%04x", static_cast<unsigned>(c));
        out += buffer;
      }
      else {
        out += c;
      }

      break;
    }
    }
  }

  return out + "\"";
}

// A mangled name demangled, or as it is if it is not one.
std::string
demangled(const std::string& a_name)
{
  int   status = 0;
  char* name   = abi::__cxa_demangle(a_name.c_str(), nullptr, nullptr, &status);

  if (status != 0 || name == nullptr) {
    return a_name;
  }

  std::string result(name);

  std::free(name);

  return result;
}

bool
endsWith(const std::string& a_text, const std::string& a_suffix)
{
  return a_text.size() >= a_suffix.size() &&
         a_text.compare(a_text.size() - a_suffix.size(), a_suffix.size(), a_suffix) == 0;
}

// The kernels in ptxas's -v output: each "Compiling entry function 'K' for 'sm_XX'" is followed by K's
// "Function properties for K" (its stack frame and spills; other functions' properties may come between, and
// are not K's) and its "Used N registers, ... B bytes cumulative stack size" line. A kernel compiled for
// several architectures is recorded once per architecture, its name carrying it.
std::string
kernelRecords(const std::string& a_output, const std::string& a_source)
{
  static const std::regex entry(R"(Compiling entry function '([^']+)'(?: for '([^']+)')?)");
  static const std::regex properties(R"(Function properties for (\S+))");
  static const std::regex spills(R"((\d+) bytes spill stores, (\d+) bytes spill loads)");
  static const std::regex used(R"(Used (\d+) registers.*?(\d+) bytes cumulative stack)");

  std::istringstream lines(a_output);
  std::string        line;
  std::string        kernel;
  std::string        arch;
  std::string        described;
  std::string        stores = "0";
  std::string        loads  = "0";
  std::string        records;
  std::smatch        m;

  while (std::getline(lines, line)) {
    if (std::regex_search(line, m, entry)) {
      kernel = m[1];
      arch   = m[2];
      stores = "0";
      loads  = "0";
      described.clear();
    }
    else if (std::regex_search(line, m, properties)) {
      described = m[1];
    }
    else if (!kernel.empty() && described == kernel && std::regex_search(line, m, spills)) {
      stores = m[1];
      loads  = m[2];
    }
    else if (!kernel.empty() && std::regex_search(line, m, used)) {
      const std::string name = demangled(kernel) + (arch.empty() ? "" : " [" + arch + "]");

      records += "{\"type\":\"kernel\",\"source\":" + quoted(a_source) + ",\"name\":" + quoted(name) +
                 ",\"registers\":" + std::string(m[1]) + ",\"stack\":" + std::string(m[2]) +
                 ",\"spill_stores\":" + stores + ",\"spill_loads\":" + loads + "}\n";
      kernel.clear();
    }
  }

  return records;
}

} // namespace

int
main(int argc, char** argv)
{
  if (argc < 3 || std::strncmp(argv[1], "--record=", 9) != 0) {
    std::fprintf(stderr, "usage: BuildCost --record=<file.jsonl> <compiler> <arguments...>\n");

    return 3;
  }

  const std::string record = argv[1] + 9;

  std::vector<std::string> command(argv + 2, argv + argc);
  std::string              source;
  std::string              object;
  bool                     compiles = false;

  for (size_t i = 1; i < command.size(); i++) {
    const std::string& a = command[i];

    if (a == "-c") {
      compiles = true;
    }
    else if (a == "-o" && i + 1 < command.size()) {
      object = command[i + 1];
    }
    else if (a[0] != '-' && (endsWith(a, ".cpp") || endsWith(a, ".cc") || endsWith(a, ".cxx") || endsWith(a, ".cu") ||
                             endsWith(a, ".c") || endsWith(a, ".hip"))) {
      source = a;
    }
  }

  const std::string compiler = command[0].substr(command[0].find_last_of('/') + 1);
  const bool        nvcc     = compiler.find("nvcc") != std::string::npos;

  if (nvcc && compiles) {
    command.emplace_back("-Xptxas");
    command.emplace_back("-v");
  }

  // The compiler's error output goes through a pipe, so that ptxas's statistics can be read from it.
  int channel[2];

  if (pipe(channel) != 0) {
    std::perror("BuildCost: pipe");

    return 3;
  }

  const auto start = std::chrono::steady_clock::now();
  const auto pid   = fork();

  if (pid < 0) {
    std::perror("BuildCost: fork");
    close(channel[0]);
    close(channel[1]);

    return 3;
  }

  if (pid == 0) {
    std::vector<char*> args;

    for (auto& a : command) {
      args.push_back(a.data());
    }

    args.push_back(nullptr);

    dup2(channel[1], STDERR_FILENO);
    close(channel[0]);
    close(channel[1]);
    execvp(args[0], args.data());
    std::perror("BuildCost: exec");
    _exit(127);
  }

  close(channel[1]);

  std::string output;
  char        buffer[4096];

  for (;;) {
    const ssize_t n = read(channel[0], buffer, sizeof(buffer));

    if (n < 0 && errno == EINTR) {
      continue;
    }
    if (n <= 0) {
      break;
    }

    output.append(buffer, size_t(n));

    // ptxas's statistics are wanted in the record, not on the screen.
    if (!(nvcc && compiles)) {
      ssize_t written = 0;

      while (written < n) {
        const ssize_t w = write(STDERR_FILENO, buffer + written, size_t(n - written));

        if (w <= 0) {
          break;
        }

        written += w;
      }
    }
  }

  close(channel[0]);

  int status = 0;

  while (waitpid(pid, &status, 0) < 0 && errno == EINTR) {
  }

  const double wall = std::chrono::duration<double>(std::chrono::steady_clock::now() - start).count();

  // Every process the compiler ran was waited for, by it or by us, so its usage is in our children's.
  rusage usage{};

  getrusage(RUSAGE_CHILDREN, &usage);

  const double cpu = double(usage.ru_utime.tv_sec) + 1e-6 * double(usage.ru_utime.tv_usec) +
                     double(usage.ru_stime.tv_sec) + 1e-6 * double(usage.ru_stime.tv_usec);

#if defined(__APPLE__)
  const double peakMB = double(usage.ru_maxrss) / (1024.0 * 1024.0);
#else
  const double peakMB = double(usage.ru_maxrss) / 1024.0;
#endif

  const int exitCode = WIFEXITED(status) ? WEXITSTATUS(status) : 128 + WTERMSIG(status);

  // With ptxas's statistics taken out of the screen output, what remains (warnings, errors) is shown.
  if (nvcc && compiles) {
    std::istringstream lines(output);
    std::string        line;

    while (std::getline(lines, line)) {
      if (line.find("ptxas info") == std::string::npos && line.find("bytes stack frame") == std::string::npos) {
        std::fprintf(stderr, "%s\n", line.c_str());
      }
    }
  }

  if (compiles) {
    char numbers[160];

    std::snprintf(numbers,
                  sizeof(numbers),
                  ",\"cpu_s\":%.3f,\"wall_s\":%.3f,\"peak_mb\":%.1f,\"exit\":%d}\n",
                  cpu,
                  wall,
                  peakMB,
                  exitCode);

    std::string text = "{\"type\":\"compile\",\"source\":" + quoted(source) + ",\"object\":" + quoted(object) +
                       ",\"compiler\":" + quoted(compiler) + numbers;

    if (nvcc) {
      text += kernelRecords(output, source);
    }

    // One write per compile, appended: compiles run in parallel and share the record.
    const int fd = open(record.c_str(), O_WRONLY | O_CREAT | O_APPEND, 0644);

    if (fd >= 0) {
      const ssize_t w = write(fd, text.data(), text.size());

      close(fd);

      if (w != ssize_t(text.size())) {
        std::fprintf(stderr, "BuildCost: could not write %s\n", record.c_str());
      }
    }
    else {
      std::fprintf(stderr, "BuildCost: could not open %s\n", record.c_str());
    }
  }

  return exitCode;
}
