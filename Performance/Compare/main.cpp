// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

// PerfCompare: compare two sets of performance results, a baseline and a candidate, each the JSON lines
// the probes print with --json (many runs of each case appended to one file). For every case it
// reports the median time of each side, the ratio, a bootstrap 95% confidence interval for the ratio,
// and a verdict:
//
//   major   slower by at least --major percent (default 10), with the interval above 1: red flag
//   minor   slower by at least --minor percent (default 3), with the interval above 1: ask
//   faster  faster by at least --minor percent, with the interval below 1
//   ~       anything else (within the thresholds, or within the noise)
//
// A paired host/device case (side "hostdevice") also shows its host/device time ratio, with the
// baseline's when the two differ by --minor percent or more, and is at least minor when the host and
// device answers disagree anywhere.
//
// Pool bytes are deterministic and judged by the same thresholds without an interval. A checksum that
// differs between the sides is reported, since a change in speed is only comparable for equal answers.
//
//   PerfCompare.ex --base base.jsonl --head head.jsonl [--minor 3] [--major 10] [--markdown report.md]
//                  [--smoke] [--verdict-only | --no-verdict] [--failed-runs <n>]
//
// Every report ends with a one-line VERDICT (green: no regression; yellow: look closer; red: do not
// merge), coloured on a terminal, and report.md starts with it. --verdict-only prints only that line
// (Compare.cmake uses it to end the run with it); --no-verdict leaves it out; --failed-runs counts
// runs that crashed, which make the verdict red.
//
// With --smoke (results of a smoke run) it judges no timings: it reports, per case, whether it ran on
// each side, whether the checksums match and whether a pair's host and device answers agree.
//
// The table goes to the terminal; --markdown also writes it as Markdown. Exit code 2 if any case is a
// major regression, else 0. Usually run by Compare.cmake (see ../README.md).

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <map>
#include <sstream>
#include <string>
#include <unistd.h>
#include <vector>

namespace {

struct Side
{
  std::vector<double> times;
  double              checksum    = 0.0;
  bool                hasChecksum = false;
  double              poolBytes   = 0.0;
  std::vector<double> hostOverDevice;
  double              mismatches = 0.0;
  double              maxDiff    = 0.0;
  bool                paired     = false;
  bool                skipped    = false;
  std::string         reason;
};

struct Run
{
  std::map<std::string, Side>        cases;
  std::vector<std::string>           order;
  std::map<std::string, std::string> meta;
};

// The value of "key": in a flat JSON object the probes wrote (strings, numbers, one number array,
// one nested object of numbers). Not a general JSON parser.
bool
field(const std::string& a_line, const std::string& a_key, std::string& a_value)
{
  const std::string pattern = "\"" + a_key + "\":";
  const auto        pos     = a_line.find(pattern);

  if (pos == std::string::npos) {
    return false;
  }

  std::size_t i = pos + pattern.size();

  if (a_line[i] == '"') {
    std::string out;

    for (i++; i < a_line.size() && a_line[i] != '"'; i++) {
      if (a_line[i] == '\\' && i + 1 < a_line.size()) {
        i++;
      }

      out += a_line[i];
    }

    a_value = out;
  }
  else if (a_line[i] == '[') {
    a_value = a_line.substr(i + 1, a_line.find(']', i) - i - 1);
  }
  else {
    const auto end = a_line.find_first_of(",}", i);
    a_value        = a_line.substr(i, end - i);
  }

  return true;
}

Run
read(const std::string& a_file)
{
  std::ifstream in(a_file);

  if (!in) {
    std::fprintf(stderr, "PerfCompare: cannot read %s\n", a_file.c_str());
    std::exit(1);
  }

  Run         run;
  std::string line;

  while (std::getline(in, line)) {
    std::string name;

    if (!field(line, "case", name)) {
      for (const char* key : {"git", "compiler", "simd", "gpu_backend", "cpu"}) {
        std::string v;

        if (field(line, key, v) && run.meta.count(key) == 0) {
          run.meta[key] = v;
        }
      }

      continue;
    }

    if (run.cases.count(name) == 0) {
      run.order.push_back(name);
    }

    Side&       side = run.cases[name];
    std::string v;

    if (field(line, "skipped", v)) {
      side.skipped = true;
      side.reason  = v;
      continue;
    }

    if (field(line, "times_ms", v)) {
      std::stringstream ss(v);
      std::string       t;

      while (std::getline(ss, t, ',')) {
        side.times.push_back(std::atof(t.c_str()));
      }
    }

    if (field(line, "checksum", v) && !side.hasChecksum) {
      side.checksum    = std::atof(v.c_str());
      side.hasChecksum = true;
    }

    if (field(line, "pool_bytes", v)) {
      side.poolBytes = std::atof(v.c_str());
    }

    // A paired host/device case: its ratio, and how far the two sides' answers were apart.
    if (field(line, "host_over_device", v)) {
      side.paired = true;
      side.hostOverDevice.push_back(std::atof(v.c_str()));

      if (field(line, "mismatches", v)) {
        side.mismatches = std::max(side.mismatches, std::atof(v.c_str()));
      }

      if (field(line, "max_diff", v)) {
        side.maxDiff = std::max(side.maxDiff, std::atof(v.c_str()));
      }
    }
  }

  return run;
}

// A time in milliseconds with a unit that keeps it between 1 and 1000, three significant digits.
std::string
formatTime(double a_ms)
{
  const char* unit  = "ms";
  double      value = a_ms;

  if (a_ms >= 1000.0) {
    unit  = "s";
    value = a_ms / 1000.0;
  }
  else if (a_ms < 1e-3) {
    unit  = "ns";
    value = a_ms * 1e6;
  }
  else if (a_ms < 1.0) {
    unit  = "\u00b5s";
    value = a_ms * 1e3;
  }

  const int decimals = value < 10.0 ? 2 : (value < 100.0 ? 1 : 0);

  char buf[32];
  std::snprintf(buf, sizeof(buf), "%.*f %s", decimals, value, unit);

  return buf;
}

// Pad to a width in characters (UTF-8 code points, so that the unit µs lines up).
std::string
pad(const std::string& a_s, std::size_t a_width, bool a_left = false)
{
  std::size_t chars = 0;

  for (const char c : a_s) {
    chars += (static_cast<unsigned char>(c) & 0xC0) != 0x80 ? 1 : 0;
  }

  const std::string fill(a_width > chars ? a_width - chars : 0, ' ');

  return a_left ? a_s + fill : fill + a_s;
}

double
median(std::vector<double> a_v)
{
  std::sort(a_v.begin(), a_v.end());

  const std::size_t n = a_v.size();

  return n == 0 ? 0.0 : (n % 2 == 1 ? a_v[n / 2] : 0.5 * (a_v[n / 2 - 1] + a_v[n / 2]));
}

// A 95% bootstrap interval for median(head) / median(base), from a fixed seed.
std::pair<double, double>
bootstrap(const std::vector<double>& a_base, const std::vector<double>& a_head)
{
  std::uint64_t state = 88172645463325252ULL;

  const auto next = [&state](std::size_t a_n) {
    state ^= state << 13;
    state ^= state >> 7;
    state ^= state << 17;

    return static_cast<std::size_t>(state % a_n);
  };

  constexpr int       resamples = 2000;
  std::vector<double> ratios(resamples);
  std::vector<double> b(a_base.size());
  std::vector<double> h(a_head.size());

  for (int r = 0; r < resamples; r++) {
    for (auto& x : b) {
      x = a_base[next(a_base.size())];
    }

    for (auto& x : h) {
      x = a_head[next(a_head.size())];
    }

    ratios[r] = median(h) / median(b);
  }

  std::sort(ratios.begin(), ratios.end());

  return {ratios[resamples / 40], ratios[resamples - 1 - resamples / 40]};
}

// Whether a report prints its verdict line: after the table, not at all (Compare.cmake prints it last,
// after its own notes, with --verdict-only), or alone.
constexpr int VerdictAfter = 0;
constexpr int VerdictNone  = 1;
constexpr int VerdictOnly  = 2;

// The one-line answer a comparison ends with: green (no regression), yellow (look closer) or red (do
// not merge). Coloured only on a terminal, unless NO_COLOR is set or TERM is dumb.
struct Verdict
{
  int         level = 0; // 0 green, 1 yellow, 2 red
  std::string text;
};

void
printVerdict(const Verdict& a_verdict)
{
  const char* term  = std::getenv("TERM");
  const bool  color = isatty(fileno(stdout)) != 0 && std::getenv("NO_COLOR") == nullptr &&
                     !(term != nullptr && std::string(term) == "dumb");
  const char* codes[] = {"\033[1;32m", "\033[1;33m", "\033[1;31m"};

  if (color) {
    std::printf("\n%s%s\033[0m\n", codes[a_verdict.level], a_verdict.text.c_str());
  }
  else {
    std::printf("\n%s\n", a_verdict.text.c_str());
  }

  std::fflush(stdout);
}

// The verdict of a smoke comparison.
Verdict
smokeVerdict(int a_failures, int a_disagreements, int a_checksums, std::size_t a_cases, int a_failedRuns)
{
  char buf[256];

  if (a_failedRuns > 0) {
    std::snprintf(
      buf, sizeof(buf), "VERDICT (smoke): FAILED: %d runs crashed or exited with an error (see above)", a_failedRuns);
    return {2, buf};
  }

  if (a_disagreements > 0) {
    std::snprintf(
      buf, sizeof(buf), "VERDICT (smoke): FAILED: host and device answers differ in %d cases", a_disagreements);
    return {2, buf};
  }

  if (a_failures > 0) {
    std::snprintf(buf,
                  sizeof(buf),
                  "VERDICT (smoke): FAILED: %d cases missing or skipped on one side (rows marked FAIL)",
                  a_failures);
    return {2, buf};
  }

  if (a_checksums > 0) {
    std::snprintf(buf,
                  sizeof(buf),
                  "VERDICT (smoke): passed, but %d checksums changed between the commits: check those answers",
                  a_checksums);
    return {1, buf};
  }

  std::snprintf(
    buf, sizeof(buf), "VERDICT (smoke): passed; all %zu cases ran on both sides with the same answers", a_cases);
  return {0, buf};
}

// The verdict of a full comparison.
Verdict
fullVerdict(int a_majors, int a_minors, int a_disagreements, int a_checksums, std::size_t a_cases, int a_failedRuns)
{
  char buf[256];

  if (a_failedRuns > 0) {
    std::snprintf(buf,
                  sizeof(buf),
                  "VERDICT: %d runs crashed or exited with an error (see above); fix them before judging",
                  a_failedRuns);
    return {2, buf};
  }

  if (a_disagreements > 0) {
    std::snprintf(buf,
                  sizeof(buf),
                  "VERDICT: host and device answers differ in %d cases; investigate before merging",
                  a_disagreements);
    return {2, buf};
  }

  if (a_majors > 0) {
    std::snprintf(
      buf,
      sizeof(buf),
      "VERDICT: MAJOR regression in %d case%s; do not merge until it is fixed or accepted (see Flagged above)",
      a_majors,
      a_majors == 1 ? "" : "s");
    return {2, buf};
  }

  if (a_minors > 0) {
    std::snprintf(buf,
                  sizeof(buf),
                  "VERDICT: %d minor regression%s; rerun %s with --rounds 10 before deciding (see Flagged above)",
                  a_minors,
                  a_minors == 1 ? "" : "s",
                  a_minors == 1 ? "it" : "them");
    return {1, buf};
  }

  if (a_checksums > 0) {
    std::snprintf(buf,
                  sizeof(buf),
                  "VERDICT: no regression, but %d checksums changed: check those answers (see Flagged above)",
                  a_checksums);
    return {1, buf};
  }

  std::snprintf(buf, sizeof(buf), "VERDICT: no regression (0 major, 0 minor across %zu cases)", a_cases);
  return {0, buf};
}

// The smoke comparison: a check, not a measurement. For every case, whether it ran on each side, whether
// the two sides' checksums match and, for a paired case, whether its host and device answers agree; no
// timing verdicts, since a smoke run times nothing. Returns 2 if a case is missing or skipped on one
// side or a pair's answers differ, else 0; a differing checksum is reported but does not fail, since a
// change to a kernel may change a result's last bits.
int
smokeReport(const Run& a_base, const Run& a_head, const std::string& a_markdown, int a_verdictMode, int a_failedRuns)
{
  struct SmokeRow
  {
    std::string name;
    std::string base;
    std::string head;
    std::string checksum;
    std::string answers;
    std::string result;
    std::string baseTime;
    std::string headTime;
    std::string note;
  };

  const auto state = [](const Run& a_run, const std::string& a_name) -> std::string {
    const auto it = a_run.cases.find(a_name);

    if (it == a_run.cases.end()) {
      return "missing";
    }

    return it->second.skipped ? "skipped" : (it->second.times.empty() ? "no result" : "ran");
  };

  std::vector<std::string> names = a_head.order;

  for (const auto& n : a_base.order) {
    if (a_head.cases.count(n) == 0) {
      names.push_back(n);
    }
  }

  std::vector<SmokeRow> rows;
  int                   failures  = 0;
  int                   checksums = 0;
  int                   disagree  = 0;

  for (const auto& name : names) {
    SmokeRow row{name, state(a_base, name), state(a_head, name), "-", "-", "ok", "", "", ""};

    const bool bothRan = row.base == "ran" && row.head == "ran";

    if (bothRan) {
      const Side& b = a_base.cases.at(name);
      const Side& h = a_head.cases.at(name);

      // The times, for information only: one repetition at the smoke sizes is no measurement (see the
      // function comment), so they are shown, and a change of more than ten times noted, never judged.
      const double tb = median(b.times);
      const double th = median(h.times);

      row.baseTime = formatTime(tb);
      row.headTime = formatTime(th);

      if (tb > 0.0 && th > 0.0 && (th / tb > 10.0 || tb / th > 10.0)) {
        char buf[64];
        std::snprintf(buf,
                      sizeof(buf),
                      "head %.0fx %s (information only)",
                      th > tb ? th / tb : tb / th,
                      th > tb ? "slower" : "faster");
        row.note = buf;
      }

      if (b.hasChecksum && h.hasChecksum) {
        const double scale = std::max(std::abs(b.checksum), 1.0);

        row.checksum = std::abs(b.checksum - h.checksum) > 1e-6 * scale ? "DIFFERS" : "same";

        if (row.checksum == "DIFFERS") {
          checksums++;
          row.result = "check";
        }
      }

      if (h.paired) {
        char buf[64];
        std::snprintf(buf, sizeof(buf), "%.0f differ", h.mismatches);
        row.answers = h.mismatches > 0.0 ? buf : "agree";

        if (h.mismatches > 0.0) {
          disagree++;
          row.result = "FAIL";
        }
      }
    }
    else {
      row.result = "FAIL";
    }

    if (row.result == "FAIL") {
      failures++;
    }

    rows.push_back(row);
  }

  const auto get = [](const Run& a_run, const char* a_key) {
    const auto it = a_run.meta.find(a_key);
    return it == a_run.meta.end() ? std::string("?") : it->second;
  };

  char summary[256];
  std::snprintf(summary,
                sizeof(summary),
                "%zu cases, %d failed or missing on a side, %d checksum differences, %d host/device disagreements",
                rows.size(),
                failures - disagree,
                checksums,
                disagree);

  std::size_t width = 4;

  for (const auto& r : rows) {
    width = std::max(width, r.name.size());
  }

  const Verdict verdict = smokeVerdict(failures - disagree, disagree, checksums, rows.size(), a_failedRuns);

  if (a_verdictMode == VerdictOnly) {
    printVerdict(verdict);

    return failures > 0 || a_failedRuns > 0 ? 2 : 0;
  }

  std::printf("\nSmoke comparison (a check, not a measurement: no timing verdicts): %s\n", summary);
  std::printf("  base %s, head %s\n\n", get(a_base, "git").c_str(), get(a_head, "git").c_str());

  const auto line = [width](const SmokeRow& a_r) {
    std::string out = "  " + pad(a_r.name, width, true) + "  " + pad(a_r.base, 9, true) + "  " +
                      pad(a_r.head, 9, true) + "  " + pad(a_r.checksum, 8, true) + "  " + pad(a_r.answers, 10, true) +
                      "  " + pad(a_r.result, 6, true) + "  " + pad(a_r.baseTime, 10) + "  " + pad(a_r.headTime, 10) +
                      (a_r.note.empty() ? "" : "  " + a_r.note);

    out.erase(out.find_last_not_of(' ') + 1);

    return out;
  };

  std::printf(
    "%s\n",
    line(SmokeRow{"case", "base", "head", "checksum", "answers", "result", "base time*", "head time*", ""}).c_str());

  for (const auto& r : rows) {
    std::printf("%s\n", line(r).c_str());
  }

  std::printf("\n* Information only: one repetition at the smoke sizes, dominated by start-up and launch overhead\n"
              "  and far from the cache and traversal behaviour of the full sizes; never judged.\n");
  if (a_verdictMode != VerdictNone) {
    printVerdict(verdict);
  }

  if (!a_markdown.empty()) {
    std::ofstream md(a_markdown);

    md << "**" << verdict.text << "**\n\n";
    md << "Smoke comparison (no timing verdicts): " << summary << "\n\n";
    md << "- base: " << get(a_base, "git") << "\n- head: " << get(a_head, "git") << "\n\n";
    md << "| case | base | head | checksum | answers | result | base time* | head time* | note |\n"
       << "|---|---|---|---|---|---|---:|---:|---|\n";

    for (const auto& r : rows) {
      md << "| `" << r.name << "` | " << r.base << " | " << r.head << " | " << r.checksum << " | " << r.answers << " | "
         << (r.result == "FAIL" ? "**FAIL**" : r.result) << " | " << r.baseTime << " | " << r.headTime << " | "
         << r.note << " |\n";
    }

    md << "\n\\* Information only: one repetition at the smoke sizes, never judged.\n";
  }

  return failures > 0 || a_failedRuns > 0 ? 2 : 0;
}

} // namespace

int
main(int argc, char** argv)
{
  std::string baseFile;
  std::string headFile;
  std::string markdownFile;
  double      minor       = 3.0;
  double      major       = 10.0;
  bool        smoke       = false;
  int         verdictMode = VerdictAfter;
  int         failedRuns  = 0;

  for (int i = 1; i < argc; i++) {
    const std::string arg = argv[i];

    if (arg == "--base" && i + 1 < argc) {
      baseFile = argv[++i];
    }
    else if (arg == "--head" && i + 1 < argc) {
      headFile = argv[++i];
    }
    else if (arg == "--smoke") {
      smoke = true;
    }
    else if (arg == "--verdict-only") {
      verdictMode = VerdictOnly;
    }
    else if (arg == "--no-verdict") {
      verdictMode = VerdictNone;
    }
    else if (arg == "--failed-runs" && i + 1 < argc) {
      failedRuns = std::atoi(argv[++i]);
    }
    else if (arg == "--markdown" && i + 1 < argc) {
      markdownFile = argv[++i];
    }
    else if (arg == "--minor" && i + 1 < argc) {
      minor = std::atof(argv[++i]);
    }
    else if (arg == "--major" && i + 1 < argc) {
      major = std::atof(argv[++i]);
    }
    else {
      std::fprintf(
        stderr,
        "usage: PerfCompare.ex --base <file> --head <file> [--minor %%] [--major %%] [--markdown <file>] [--smoke]\n");
      return 1;
    }
  }

  if (baseFile.empty() || headFile.empty()) {
    std::fprintf(stderr, "PerfCompare: --base and --head are required\n");
    return 1;
  }

  const Run base = read(baseFile);
  const Run head = read(headFile);

  if (smoke) {
    return smokeReport(base, head, markdownFile, verdictMode, failedRuns);
  }

  struct Row
  {
    std::string name;
    std::string baseMs;
    std::string headMs;
    std::string change;
    std::string interval;
    std::string memory;
    std::string verdict;
    std::string note;
  };

  std::vector<Row> rows;
  int              majors = 0;
  int              minors = 0;
  int              faster = 0;

  const auto fmt = [](const char* a_format, double a_v) {
    char buf[64];
    std::snprintf(buf, sizeof(buf), a_format, a_v);

    return std::string(buf);
  };

  std::vector<std::string> names = head.order;

  for (const auto& n : base.order) {
    if (head.cases.count(n) == 0) {
      names.push_back(n);
    }
  }

  for (const auto& name : names) {
    Row row{name, "-", "-", "-", "-", "-", "~", ""};

    const auto b = base.cases.find(name);
    const auto h = head.cases.find(name);

    if (b == base.cases.end() || h == head.cases.end()) {
      row.verdict = b == base.cases.end() ? "new" : "removed";
      rows.push_back(row);
      continue;
    }

    if (b->second.skipped || h->second.skipped) {
      row.verdict = "skipped";
      row.note    = b->second.skipped ? b->second.reason : h->second.reason;
      rows.push_back(row);
      continue;
    }

    const double mb    = median(b->second.times);
    const double mh    = median(h->second.times);
    const double ratio = mh / mb;
    const auto   ci    = bootstrap(b->second.times, h->second.times);

    row.baseMs   = formatTime(mb);
    row.headMs   = formatTime(mh);
    row.change   = fmt("%+.1f%%", 100.0 * (ratio - 1.0));
    row.interval = fmt("[%+.1f%%, ", 100.0 * (ci.first - 1.0)) + fmt("%+.1f%%]", 100.0 * (ci.second - 1.0));

    int severity = 0;

    if (ci.first > 1.0 && ratio >= 1.0 + major / 100.0) {
      severity = 2;
    }
    else if (ci.first > 1.0 && ratio >= 1.0 + minor / 100.0) {
      severity = 1;
    }
    else if (ci.second < 1.0 && ratio <= 1.0 - minor / 100.0) {
      row.verdict = "faster";
      faster++;
    }

    if (b->second.poolBytes > 0.0 && h->second.poolBytes > 0.0) {
      const double mem = h->second.poolBytes / b->second.poolBytes;

      row.memory = fmt("%+.1f%%", 100.0 * (mem - 1.0));

      if (mem >= 1.0 + major / 100.0) {
        severity = 2;
        row.note += "pool bytes up; ";
      }
      else if (mem >= 1.0 + minor / 100.0) {
        severity = std::max(severity, 1);
        row.note += "pool bytes up; ";
      }
    }

    if (h->second.paired) {
      const double rh = median(h->second.hostOverDevice);

      row.note += "device " + fmt("%.1f", rh) + "x faster than the host";

      if (b->second.paired) {
        const double rb = median(b->second.hostOverDevice);

        if (std::abs(rh / rb - 1.0) >= minor / 100.0) {
          row.note += " (was " + fmt("%.1f", rb) + "x)";
        }
      }

      row.note += "; ";

      if (h->second.mismatches > 0.0) {
        severity = std::max(severity, 1);
        row.note += fmt("%.0f", h->second.mismatches) + " host and device answers DIFFER (max diff " +
                    fmt("%.3g", h->second.maxDiff) + "); ";
      }
    }

    if (severity == 2) {
      row.verdict = "MAJOR";
      majors++;
    }
    else if (severity == 1) {
      row.verdict = "minor";
      minors++;
    }

    if (b->second.hasChecksum && h->second.hasChecksum) {
      const double scale = std::max(std::abs(b->second.checksum), 1.0);

      if (std::abs(b->second.checksum - h->second.checksum) > 1e-6 * scale) {
        row.note +=
          "checksum differs (" + fmt("%.10g", b->second.checksum) + " -> " + fmt("%.10g", h->second.checksum) + ")";
      }
    }

    // Notes are joined with "; "; drop the last separator.
    while (!row.note.empty() && (row.note.back() == ' ' || row.note.back() == ';')) {
      row.note.pop_back();
    }

    rows.push_back(row);
  }

  const auto metaLine = [](const char* a_label, const Run& a_run) {
    const auto get = [&a_run](const char* a_key) {
      const auto it = a_run.meta.find(a_key);
      return it == a_run.meta.end() ? std::string("?") : it->second;
    };

    return std::string(a_label) + get("git") + " (" + get("compiler") + ", simd " + get("simd") + ", gpu " +
           get("gpu_backend") + ")";
  };

  std::string summary = std::to_string(majors) + " major, " + std::to_string(minors) + " minor, " +
                        std::to_string(faster) + " faster, " + std::to_string(rows.size()) + " cases";

  int disagreements = 0;
  int checksums     = 0;

  for (const auto& r : rows) {
    disagreements += r.note.find("DIFFER") != std::string::npos ? 1 : 0;
    checksums += r.note.find("checksum differs") != std::string::npos ? 1 : 0;
  }

  const Verdict verdict = fullVerdict(majors, minors, disagreements, checksums, rows.size(), failedRuns);
  const int     code    = majors > 0 || disagreements > 0 || failedRuns > 0 ? 2 : 0;

  if (verdictMode == VerdictOnly) {
    printVerdict(verdict);

    return code;
  }

  // Terminal.
  std::printf("\nPerformance comparison: %s\n", summary.c_str());
  std::printf("  %s\n  %s\n", metaLine("base ", base).c_str(), metaLine("head ", head).c_str());
  std::printf("  thresholds: minor >= %.0f%%, major >= %.0f%% slower, and outside the 95%% interval\n\n", minor, major);
  std::size_t nameWidth = 4;

  for (const auto& r : rows) {
    nameWidth = std::max(nameWidth, r.name.size());
  }

  const auto line = [nameWidth](const Row& a_r) {
    std::string out = "  " + pad(a_r.name, nameWidth, true) + "  " + pad(a_r.baseMs, 10) + "  " + pad(a_r.headMs, 10) +
                      "  " + pad(a_r.change, 7) + "  " + pad(a_r.interval, 17) + "  " + pad(a_r.memory, 6) + "  " +
                      pad(a_r.verdict, 7, true) + (a_r.note.empty() ? "" : "  " + a_r.note);

    out.erase(out.find_last_not_of(' ') + 1);

    return out;
  };

  std::printf("%s\n", line(Row{"case", "base", "head", "change", "95% interval", "pool", "verdict", ""}).c_str());

  for (const auto& r : rows) {
    std::printf("%s\n", line(r).c_str());
  }

  // The flagged cases again, so they are not lost in a long table.
  bool flagged = false;

  for (const auto& r : rows) {
    if (r.verdict == "MAJOR" || r.verdict == "minor" || r.note.find("DIFFER") != std::string::npos ||
        r.note.find("checksum") != std::string::npos) {
      if (!flagged) {
        std::printf("\nFlagged:\n");
        flagged = true;
      }

      std::printf("  %-7s %s  %s  %s\n", r.verdict.c_str(), r.name.c_str(), r.change.c_str(), r.note.c_str());
    }
  }

  if (!flagged) {
    std::printf("\nNothing flagged.\n");
  }

  if (verdictMode != VerdictNone) {
    printVerdict(verdict);
  }

  if (!markdownFile.empty()) {
    std::ofstream md(markdownFile);

    md << "**" << verdict.text << "**\n\n";
    md << "Performance comparison: " << summary << "\n\n";
    md << "- " << metaLine("base: ", base) << "\n- " << metaLine("head: ", head) << "\n";
    md << "- CPU: " << (head.meta.count("cpu") ? head.meta.at("cpu") : "?") << "\n";
    md << "- Thresholds: minor >= " << minor << "%, major >= " << major
       << "% slower, and outside the 95% bootstrap interval\n\n";
    md << "| case | base | head | change | 95% interval | pool | verdict | note |\n";
    md << "|---|---:|---:|---:|---:|---:|---|---|\n";

    for (const auto& r : rows) {
      md << "| `" << r.name << "` | " << r.baseMs << " | " << r.headMs << " | " << r.change << " | " << r.interval
         << " | " << r.memory << " | " << (r.verdict == "MAJOR" ? "**MAJOR**" : r.verdict) << " | " << r.note << " |\n";
    }
  }

  return code;
}
