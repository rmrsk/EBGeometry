// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file   PerfHarness.hpp
 * @brief  The shared harness for the programs under Performance/: named cases, repeated timing,
 * checksums and machine-readable output.
 * @details A program builds one Perf::Suite from its command line and adds its cases to it. A case is
 * a name and a function; the function sets up its inputs (untimed), calls Perf::Case::measure() on
 * the part to time, and records a checksum of its results and, where it has one, the pool bytes the
 * structure occupies. Only the cases selected on the command line run, so a program holds many cases
 * and Performance/Compare.cmake runs each one in a fresh process.
 *
 * Command line (every program):
 *   --list           print the case names and exit
 *   --case <glob>    run only the cases matching the glob (* and ?); may be repeated
 *   --side <s>       host, device or all: which side's cases run (default: EBGEOMETRY_PERF_SIDE, else all)
 *   --reps <n>       timed repetitions per case (default 5), after one untimed warm-up
 *   --smoke          small problem sizes and one repetition: checks that a case runs, times nothing
 *   --json           one JSON object per case instead of the table
 *
 * This header is host code for the programs under Performance/, not part of the library.
 */

#ifndef EBGEOMETRY_PERF_HARNESS_HPP
#define EBGEOMETRY_PERF_HARNESS_HPP

#include <algorithm>
#include <chrono>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <exception>
#include <fstream>
#include <functional>
#include <string>
#include <utility>
#include <vector>

#include <EBGeometry.hpp>

// Set by the build (Performance/CMakeLists.txt) to the commit the program was built from.
#ifndef EBGEOMETRY_PERF_GIT_SHA
#define EBGEOMETRY_PERF_GIT_SHA "unknown"
#endif

// Where the meshes are. Set by the build to absolute paths; the defaults work from a program's folder.
#ifndef EBGEOMETRY_PERF_MESH_DIR
#define EBGEOMETRY_PERF_MESH_DIR "../../common-3d-test-models/data"
#endif

#ifndef EBGEOMETRY_PERF_SMOKE_MESH_DIR
#define EBGEOMETRY_PERF_SMOKE_MESH_DIR "../../Tests/data"
#endif

// The side whose cases run when --side is not given: "host", "device" or "all". Set by the build
// (EBGEOMETRY_PERF_SIDE in Performance/CMakeLists.txt).
#ifndef EBGEOMETRY_PERF_SIDE
#define EBGEOMETRY_PERF_SIDE "all"
#endif

namespace Perf {

/**
 * @brief Whether a name matches a glob of * (any run) and ? (any one character).
 * @param[in] a_glob Pattern.
 * @param[in] a_name Name.
 * @return True on a match.
 */
inline bool
globMatch(const char* a_glob, const char* a_name) noexcept
{
  if (*a_glob == '\0') {
    return *a_name == '\0';
  }

  if (*a_glob == '*') {
    return globMatch(a_glob + 1, a_name) || (*a_name != '\0' && globMatch(a_glob, a_name + 1));
  }

  return *a_name != '\0' && (*a_glob == '?' || *a_glob == *a_name) && globMatch(a_glob + 1, a_name + 1);
}

/**
 * @brief The CPU model, from /proc/cpuinfo where there is one.
 * @return The model name, or "unknown".
 */
inline std::string
cpuModel()
{
  std::ifstream in("/proc/cpuinfo");
  std::string   line;

  while (std::getline(in, line)) {
    if (line.rfind("model name", 0) == 0) {
      const auto colon = line.find(':');

      return colon == std::string::npos ? line : line.substr(colon + 2);
    }
  }

  return "unknown";
}

/**
 * @brief The SIMD level this translation unit was compiled for.
 * @return One of avx512, avx2, avx, sse41, none.
 */
constexpr const char*
simdLevel() noexcept
{
#if defined(__AVX512F__)
  return "avx512";
#elif defined(__AVX2__)
  return "avx2";
#elif defined(__AVX__)
  return "avx";
#elif defined(__SSE4_1__)
  return "sse41";
#else
  return "none";
#endif
}

/**
 * @brief Whether the build defines EBGEOMETRY_HOST_TUNED_DEFAULTS, which changes TriangleSoA::DefaultWidth.
 * @return True if it does.
 */
constexpr bool
hostTunedDefaults() noexcept
{
#ifdef EBGEOMETRY_HOST_TUNED_DEFAULTS
  return true;
#else
  return false;
#endif
}

/**
 * @brief The compiler and its version.
 * @return A short description.
 */
inline std::string
compilerName()
{
#if defined(__clang__)
  return std::string("clang ") + __clang_version__;
#elif defined(__GNUC__)
  return std::string("gcc ") + __VERSION__;
#else
  return "unknown";
#endif
}

/**
 * @brief Write a string as a JSON string literal.
 * @param[in] a_s The string.
 * @return The quoted, escaped string.
 */
inline std::string
jsonString(const std::string& a_s)
{
  std::string out = "\"";

  for (const char c : a_s) {
    if (c == '"' || c == '\\') {
      out += '\\';
    }

    out += c;
  }

  return out + "\"";
}

/**
 * @brief One running case: what it measured and recorded.
 */
class Case
{
public:
  /**
   * @brief Set up a case.
   * @param[in] a_name  Full case name.
   * @param[in] a_reps  Timed repetitions.
   * @param[in] a_smoke Whether this is a smoke run.
   */
  Case(std::string a_name, int a_reps, bool a_smoke) : m_name(std::move(a_name)), m_reps(a_reps), m_smoke(a_smoke)
  {}

  /**
   * @brief Whether this is a smoke run, in which a case should pick its small problem size.
   * @return True for --smoke.
   */
  [[nodiscard]] bool
  smoke() const noexcept
  {
    return m_smoke;
  }

  /**
   * @brief Pick a problem size: the full one, or the smoke one under --smoke.
   * @param[in] a_full  Size for a timing run.
   * @param[in] a_smoke Size for a smoke run.
   * @return The size to use.
   */
  [[nodiscard]] std::size_t
  size(std::size_t a_full, std::size_t a_smoke) const noexcept
  {
    return m_smoke ? a_smoke : a_full;
  }

  /**
   * @brief Time a function: one untimed warm-up, then the configured number of repetitions.
   * @details A device case passes a function that launches its kernel and returns once the kernel has
   * finished (GPU::transform and GPU::parallelFor already return then), so wall time is kernel time.
   * @param[in] a_body       The work to time.
   * @param[in] a_itemsPerRep Items processed per repetition, for a per-item time (0: none).
   */
  void
  measure(const std::function<void()>& a_body, std::size_t a_itemsPerRep = 0)
  {
    m_times = this->repeat(a_body);
    m_items = a_itemsPerRep;
  }

  /**
   * @brief Time a function as measure() does, but return the times instead of recording them as the
   * case's own (for a second side of a paired case).
   * @param[in] a_body The work to time.
   * @return The time of each repetition, in milliseconds.
   */
  [[nodiscard]] std::vector<double>
  repeat(const std::function<void()>& a_body) const
  {
    a_body();

    std::vector<double> times;

    for (int r = 0; r < m_reps; r++) {
      const auto start = std::chrono::steady_clock::now();
      a_body();
      const auto stop = std::chrono::steady_clock::now();

      times.push_back(std::chrono::duration<double, std::milli>(stop - start).count());
    }

    return times;
  }

  /**
   * @brief The median of the case's own times.
   * @return The median, in milliseconds.
   */
  [[nodiscard]] double
  medianTime() const
  {
    return Case::median(m_times);
  }

  /**
   * @brief Record a further named number (a second side's time, a ratio, a count of disagreements).
   * @param[in] a_key   Name.
   * @param[in] a_value Value.
   */
  void
  metric(const std::string& a_key, double a_value)
  {
    m_metrics.emplace_back(a_key, a_value);
  }

  /**
   * @brief The median of a set of times.
   * @param[in] a_times Times.
   * @return Their median.
   */
  [[nodiscard]] static double
  median(std::vector<double> a_times)
  {
    std::sort(a_times.begin(), a_times.end());

    const std::size_t n = a_times.size();

    return n == 0 ? 0.0 : (n % 2 == 1 ? a_times[n / 2] : 0.5 * (a_times[n / 2 - 1] + a_times[n / 2]));
  }

  /**
   * @brief Time something that cannot run twice on the same state (a build): a_setup prepares fresh
   * state untimed, a_body is timed. One untimed warm-up, then the configured repetitions.
   * @param[in] a_setup Untimed preparation before each repetition.
   * @param[in] a_body  The work to time.
   */
  void
  measureFresh(const std::function<void()>& a_setup, const std::function<void()>& a_body)
  {
    a_setup();
    a_body();

    m_times.clear();

    for (int r = 0; r < m_reps; r++) {
      a_setup();

      const auto start = std::chrono::steady_clock::now();
      a_body();
      const auto stop = std::chrono::steady_clock::now();

      m_times.push_back(std::chrono::duration<double, std::milli>(stop - start).count());
    }
  }

  /**
   * @brief Record a checksum of the case's results. Before and after a change, equal inputs should give
   * equal checksums; the comparison reports any difference.
   * @param[in] a_value The checksum.
   */
  void
  checksum(double a_value) noexcept
  {
    m_checksum    = a_value;
    m_hasChecksum = true;
  }

  /**
   * @brief Record the bytes the measured structure occupies in its pool (Pool::usedBytes()).
   * @param[in] a_bytes Bytes.
   */
  void
  poolBytes(std::size_t a_bytes) noexcept
  {
    m_poolBytes = a_bytes;
  }

  /**
   * @brief Record a parameter of the case (a branching factor, a width, a point count).
   * @param[in] a_key   Name.
   * @param[in] a_value Value.
   */
  void
  param(const std::string& a_key, std::size_t a_value)
  {
    m_params.emplace_back(a_key, a_value);
  }

  /**
   * @brief Print the case: a table row, or one JSON object.
   * @param[in] a_json Whether to print JSON.
   */
  void
  report(bool a_json) const
  {
    std::vector<double> t = m_times;
    std::sort(t.begin(), t.end());

    const auto quantile = [&t](double a_q) {
      if (t.empty()) {
        return 0.0;
      }

      const double pos = a_q * double(t.size() - 1);
      const auto   lo  = static_cast<std::size_t>(pos);
      const auto   hi  = std::min(lo + 1, t.size() - 1);

      return t[lo] + (pos - double(lo)) * (t[hi] - t[lo]);
    };

    const double median = quantile(0.5);
    const double iqr    = quantile(0.75) - quantile(0.25);

    if (a_json) {
      std::string s = "{\"case\":" + jsonString(m_name) + ",\"median_ms\":" + num(median) +
                      ",\"min_ms\":" + num(t.empty() ? 0.0 : t.front()) + ",\"iqr_ms\":" + num(iqr) + ",\"times_ms\":[";

      for (std::size_t i = 0; i < m_times.size(); i++) {
        s += (i > 0 ? "," : "") + num(m_times[i]);
      }

      s += "]";

      if (m_items > 0) {
        s += ",\"items\":" + std::to_string(m_items) + ",\"us_per_item\":" + num(1e3 * median / double(m_items));
      }

      if (m_hasChecksum) {
        s += ",\"checksum\":" + num(m_checksum);
      }

      if (m_poolBytes > 0) {
        s += ",\"pool_bytes\":" + std::to_string(m_poolBytes);
      }

      if (!m_metrics.empty()) {
        s += ",\"metrics\":{";

        for (std::size_t i = 0; i < m_metrics.size(); i++) {
          s += (i > 0 ? "," : "") + jsonString(m_metrics[i].first) + ":" + num(m_metrics[i].second);
        }

        s += "}";
      }

      s += ",\"params\":{";

      for (std::size_t i = 0; i < m_params.size(); i++) {
        s += (i > 0 ? "," : "") + jsonString(m_params[i].first) + ":" + std::to_string(m_params[i].second);
      }

      std::printf("%s}}\n", s.c_str());
    }
    else {
      std::printf("  %-52s %10.3f ms  (IQR %6.3f)", m_name.c_str(), median, iqr);

      if (m_items > 0) {
        std::printf("  %9.4f us/item", 1e3 * median / double(m_items));
      }

      if (m_poolBytes > 0) {
        std::printf("  %8.2f MB", double(m_poolBytes) / 1e6);
      }

      if (m_hasChecksum) {
        std::printf("  sum %.10g", m_checksum);
      }

      for (const auto& m : m_metrics) {
        std::printf("  %s %.6g", m.first.c_str(), m.second);
      }

      std::printf("\n");
    }

    std::fflush(stdout);
  }

private:
  static std::string
  num(double a_v)
  {
    char buf[64];
    std::snprintf(buf, sizeof(buf), "%.17g", a_v);

    return buf;
  }

  std::string                                      m_name;
  int                                              m_reps;
  bool                                             m_smoke;
  std::vector<double>                              m_times;
  std::size_t                                      m_items       = 0;
  double                                           m_checksum    = 0.0;
  bool                                             m_hasChecksum = false;
  std::size_t                                      m_poolBytes   = 0;
  std::vector<std::pair<std::string, std::size_t>> m_params;
  std::vector<std::pair<std::string, double>>      m_metrics;
};

/**
 * @brief A program's set of cases and its command line.
 */
class Suite
{
public:
  /**
   * @brief Parse the command line (see the file comment).
   * @param[in] a_argc Argument count.
   * @param[in] a_argv Arguments.
   * @param[in] a_name The program's name, printed in the header.
   */
  Suite(int a_argc, char** a_argv, std::string a_name) : m_name(std::move(a_name))
  {
    for (int i = 1; i < a_argc; i++) {
      const std::string arg = a_argv[i];

      if (arg == "--list") {
        m_list = true;
      }
      else if (arg == "--json") {
        m_json = true;
      }
      else if (arg == "--smoke") {
        m_smoke = true;
        m_reps  = 1;
      }
      else if (arg == "--case" && i + 1 < a_argc) {
        m_globs.emplace_back(a_argv[++i]);
      }
      else if (arg == "--side" && i + 1 < a_argc) {
        m_side = a_argv[++i];
      }
      else if (arg == "--reps" && i + 1 < a_argc) {
        m_reps = std::max(1, std::atoi(a_argv[++i]));
      }
      else {
        std::fprintf(stderr, "%s: unknown argument '%s'\n", m_name.c_str(), arg.c_str());
        std::exit(2);
      }
    }

    if (m_side != "host" && m_side != "device" && m_side != "all") {
      std::fprintf(stderr, "%s: --side must be host, device or all (got '%s')\n", m_name.c_str(), m_side.c_str());
      std::exit(2);
    }

    if (m_side == "device" && !deviceRuns()) {
      std::fprintf(stderr,
                   "%s: no device cases: %s\n",
                   m_name.c_str(),
                   EBGeometry::GPU::enabled ? "no GPU is visible" : "this is not a CUDA or HIP build");
    }
  }

  /**
   * @brief Add a case. Its function runs only if the case is selected.
   * @param[in] a_name Case name, '/'-separated from general to specific (component/input/query/precision/side).
   * @param[in] a_run  The case.
   */
  void
  add(const std::string& a_name, std::function<void(Case&)> a_run)
  {
    m_cases.emplace_back(a_name, std::move(a_run));
  }

  /**
   * @brief Run the selected cases (or list them).
   * @return The program's exit code.
   */
  int
  run() const
  {
    if (m_list) {
      for (const auto& c : m_cases) {
        if (selected(c.first)) {
          std::printf("%s\n", c.first.c_str());
        }
      }

      return 0;
    }

    if (m_json) {
      std::printf("{\"suite\":%s,\"git\":%s,\"compiler\":%s,\"simd\":\"%s\",\"host_tuned_defaults\":%s,"
                  "\"gpu_backend\":\"%s\",\"side\":\"%s\",\"cpu\":%s,\"reps\":%d,\"smoke\":%s}\n",
                  jsonString(m_name).c_str(),
                  jsonString(EBGEOMETRY_PERF_GIT_SHA).c_str(),
                  jsonString(compilerName()).c_str(),
                  simdLevel(),
                  hostTunedDefaults() ? "true" : "false",
                  EBGeometry::GPU::backendName,
                  m_side.c_str(),
                  jsonString(cpuModel()).c_str(),
                  m_reps,
                  m_smoke ? "true" : "false");
    }
    else {
      std::printf("%s  (git %s, %s, simd %s, gpu %s, %d reps%s)\n",
                  m_name.c_str(),
                  EBGEOMETRY_PERF_GIT_SHA,
                  compilerName().c_str(),
                  simdLevel(),
                  EBGeometry::GPU::backendName,
                  m_reps,
                  m_smoke ? ", smoke" : "");
    }

    for (const auto& c : m_cases) {
      if (!selected(c.first)) {
        continue;
      }

      // A case whose input is missing (the mesh submodule not checked out) is reported as skipped, so
      // the other cases still run.
      Case running(c.first, m_reps, m_smoke);

      try {
        c.second(running);
      } catch (const std::exception& e) {
        if (m_json) {
          std::printf("{\"case\":%s,\"skipped\":%s}\n", jsonString(c.first).c_str(), jsonString(e.what()).c_str());
        }
        else {
          std::printf("  %-52s skipped: %s\n", c.first.c_str(), e.what());
        }

        continue;
      }

      running.report(m_json);
    }

    return 0;
  }

  /**
   * @brief Whether device cases can run: a CUDA or HIP build with a visible device.
   * @return True if they can.
   */
  [[nodiscard]] static bool
  deviceRuns()
  {
    return EBGeometry::GPU::enabled && EBGeometry::GPU::deviceAvailable();
  }

private:
  [[nodiscard]] bool
  selected(const std::string& a_name) const
  {
    const auto endsWith = [&a_name](const std::string& a_suffix) {
      return a_name.size() >= a_suffix.size() &&
             a_name.compare(a_name.size() - a_suffix.size(), a_suffix.size(), a_suffix) == 0;
    };

    // A paired host/device case needs a device, so --side device selects it too.
    if (m_side == "host" && !endsWith("/host")) {
      return false;
    }

    if (m_side == "device" && !endsWith("/device") && !endsWith("/hostdevice")) {
      return false;
    }

    if (m_globs.empty()) {
      return true;
    }

    return std::any_of(
      m_globs.begin(), m_globs.end(), [&a_name](const std::string& g) { return globMatch(g.c_str(), a_name.c_str()); });
  }

  std::string                                                     m_name;
  std::vector<std::pair<std::string, std::function<void(Case&)>>> m_cases;
  std::vector<std::string>                                        m_globs;
  std::string                                                     m_side  = EBGEOMETRY_PERF_SIDE;
  int                                                             m_reps  = 5;
  bool                                                            m_smoke = false;
  bool                                                            m_list  = false;
  bool                                                            m_json  = false;
};

/**
 * @brief Points uniformly at random in a box, from a fixed seed.
 * @tparam T Precision.
 * @param[in] a_box  The box.
 * @param[in] a_n    Number of points.
 * @param[in] a_seed Seed.
 * @return The points.
 */
template <class T>
std::vector<EBGeometry::Vec3T<T>>
pointsInBox(const EBGeometry::BoundingVolumes::AABBT<T>& a_box, std::size_t a_n, std::uint64_t a_seed)
{
  // A fixed generator, so every build and platform draws the same points.
  std::uint64_t state = a_seed * 6364136223846793005ULL + 1442695040888963407ULL;

  const auto next = [&state]() {
    state ^= state >> 12;
    state ^= state << 25;
    state ^= state >> 27;

    return double((state * 2685821657736338717ULL) >> 11) * (1.0 / 9007199254740992.0);
  };

  const auto lo = a_box.getLowCorner();
  const auto d  = a_box.getHighCorner() - lo;

  std::vector<EBGeometry::Vec3T<T>> points(a_n);

  for (auto& p : points) {
    const T x = T(next());
    const T y = T(next());
    const T z = T(next());

    p = lo + EBGeometry::Vec3T<T>(x * d[0], y * d[1], z * d[2]);
  }

  return points;
}

/**
 * @brief The armadillo mesh from the common-3d-test-models submodule, or the dodecahedron from Tests/data
 * under --smoke.
 * @param[in] a_smoke Whether this is a smoke run.
 * @return Path to the mesh.
 */
inline std::string
meshFile(bool a_smoke)
{
  return a_smoke ? std::string(EBGEOMETRY_PERF_SMOKE_MESH_DIR) + "/dodecahedron.obj"
                 : std::string(EBGEOMETRY_PERF_MESH_DIR) + "/armadillo.obj";
}

/**
 * @brief Time a query on the device over a set of inputs, the kernel alone.
 * @details The structure's pool is frozen and mirrored into GPU::memoryResource() once; the inputs and
 * room for the results go into a pool of their own, mirrored the same way. a_makeQuery receives the
 * device copy of the structure's pool and returns the query (holding the structure's rebasedView()).
 * The timed call is GPU::transform() over pool-resident arrays, which allocates and copies nothing, so
 * the time is the kernel's. The results are copied back to the host, untimed, for the checksum.
 * @tparam Out       Result type.
 * @tparam In        Input type.
 * @tparam MakeQuery Callable taking const EBGeometry::Pool& and returning the query.
 * @param[in,out] a_case          The running case.
 * @param[in,out] a_structurePool The pool the queried structure lives in; frozen here.
 * @param[in]     a_in            The inputs.
 * @param[in]     a_makeQuery     Makes the query against the device pool.
 * @return The results, on the host.
 */
template <class Out, class In, class MakeQuery>
std::vector<Out>
measureOnDevice(Case& a_case, EBGeometry::Pool& a_structurePool, const std::vector<In>& a_in, MakeQuery&& a_makeQuery)
{
  using namespace EBGeometry;

  const auto n = static_cast<uint32_t>(a_in.size());

  Pool             ioPool(hostMemoryResource());
  PODVector<In>    in;
  PODVector<Out>   out;
  std::vector<Out> zeros(n, Out{});

  in.reserveFrom(ioPool, n);
  out.reserveFrom(ioPool, n);
  in.assign(ioPool.base(), a_in.data(), n);
  out.assign(ioPool.base(), zeros.data(), n);
  ioPool.freeze();

  if (!a_structurePool.isFrozen()) {
    a_structurePool.freeze();
  }

  const Pool deviceStructure = Pool::mirror(a_structurePool, GPU::memoryResource());
  const Pool deviceIO        = Pool::mirror(ioPool, GPU::memoryResource());
  const auto query           = a_makeQuery(deviceStructure);

  a_case.measure([&] { GPU::transform(query, deviceIO, in, out); }, n);

  const Pool       back = Pool::mirror(deviceIO, hostMemoryResource());
  std::vector<Out> results(n);

  for (uint32_t i = 0; i < n; i++) {
    results[i] = out.at(back.base(), i);
  }

  return results;
}

/**
 * @brief Points reordered along a Morton curve, the order a caller querying a grid or a box of cells
 * presents them in, which keeps neighbouring queries (a GPU warp, consecutive host iterations) in the
 * same part of a tree.
 * @tparam T Precision.
 * @param[in] a_points The points.
 * @return The points in Morton order.
 */
template <class T>
std::vector<EBGeometry::Vec3T<T>>
mortonOrdered(const std::vector<EBGeometry::Vec3T<T>>& a_points)
{
  const std::vector<uint32_t>       order = EBGeometry::SFC::order<EBGeometry::SFC::Morton>(a_points);
  std::vector<EBGeometry::Vec3T<T>> sorted(a_points.size());

  for (std::size_t i = 0; i < order.size(); i++) {
    sorted[i] = a_points[order[i]];
  }

  return sorted;
}

/**
 * @brief A paired case: the same query over the same inputs on the host and on the device.
 * @details The inputs are divided into chunks of GPU::DefaultBlockSize consecutive inputs: on the device
 * each chunk is one block of threads, and on the host the chunks are visited in turn, so both sides walk
 * the inputs in the same order. The case's own time is the device's (the kernel alone, as for
 * measureOnDevice()); it also records
 *
 *   host_ms        the host's median time
 *   host_over_device  host time / device time, on this machine
 *   max_diff       the largest difference between a host and a device result (a_agree's first value)
 *   mismatches     the number of inputs where the two disagree (a_agree's second value)
 *
 * so that a divergence between the sides, in speed or in answers, shows in one place.
 * @tparam Out       Result type.
 * @tparam In        Input type.
 * @tparam MakeQuery Callable taking const EBGeometry::Pool& and returning the query against that pool.
 * @tparam Agree     Callable (const Out& host, const Out& device) -> std::pair<double, bool>: the size
 * of the difference, and whether it is a disagreement.
 * @param[in,out] a_case          The running case.
 * @param[in,out] a_structurePool The pool the queried structure lives in; frozen here.
 * @param[in]     a_in            The inputs.
 * @param[in]     a_makeQuery     Makes the query against a pool: the host pool, or the device copy.
 * @param[in]     a_agree         Compares a host and a device result.
 * @return The host results.
 */
template <class Out, class In, class MakeQuery, class Agree>
std::vector<Out>
measureHostAndDevice(Case&                  a_case,
                     EBGeometry::Pool&      a_structurePool,
                     const std::vector<In>& a_in,
                     MakeQuery&&            a_makeQuery,
                     Agree&&                a_agree)
{
  const std::vector<Out> device = measureOnDevice<Out>(a_case, a_structurePool, a_in, a_makeQuery);

  const auto       query = a_makeQuery(a_structurePool);
  const auto       n     = a_in.size();
  std::vector<Out> host(n);

  const std::vector<double> hostTimes = a_case.repeat([&] {
    constexpr std::size_t chunk = EBGeometry::GPU::DefaultBlockSize;

    for (std::size_t begin = 0; begin < n; begin += chunk) {
      const std::size_t end = std::min(begin + chunk, n);

      for (std::size_t i = begin; i < end; i++) {
        host[i] = query(a_in[i]);
      }
    }
  });

  double      maxDiff    = 0.0;
  std::size_t mismatches = 0;

  for (std::size_t i = 0; i < n; i++) {
    const auto [diff, mismatch] = a_agree(host[i], device[i]);

    maxDiff = std::max(maxDiff, diff);
    mismatches += mismatch ? 1 : 0;
  }

  const double hostMs   = Case::median(hostTimes);
  const double deviceMs = a_case.medianTime();

  a_case.metric("host_ms", hostMs);
  a_case.metric("host_over_device", deviceMs > 0.0 ? hostMs / deviceMs : 0.0);
  a_case.metric("max_diff", maxDiff);
  a_case.metric("mismatches", double(mismatches));

  return host;
}

} // namespace Perf

#endif
