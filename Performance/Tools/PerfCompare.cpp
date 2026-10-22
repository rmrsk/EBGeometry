// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

// PerfCompare: compare two sets of performance results, a baseline and a candidate, each the JSON lines
// the probes write (Common/Schema.hpp; many runs of each case appended to one file).
//
//   PerfCompare.ex --base base.jsonl --head head.jsonl [--minor <percent>] [--major <percent>]
//                  [--markdown <file>] [--smoke] [--verdict-only | --no-verdict] [--failed-runs <n>]
//
// Each JSON line of a case is one run: a fresh process, timing several repetitions. Repetitions in one
// process share its luck (memory placement, clock speed, neighbours on the machine), so they are not
// independent measurements; runs are. Every case is therefore judged by its runs: each run is reduced to
// the median of its repetitions, and for every case it reports the median of those per side, their
// ratio, a bootstrap 95% confidence interval for the ratio (the runs resampled, not the repetitions),
// the change in pool bytes, and a verdict:
//
//   lost     ran in the baseline, but was skipped or is missing in the candidate: a red flag
//   DIFFER   a pair whose host and device answers disagree: a red flag, its times not comparable
//   MAJOR    slower by at least --major percent (default 10), or that much more pool memory: a red flag
//   minor    slower by at least --minor percent (default 3), or that much more pool memory: ask
//   faster   faster by at least --minor percent, and no more pool memory
//   ~        anything else (within the thresholds, or within the noise)
//
// A slowdown or speedup counts only when the interval lies above or below 1, and is judged only with
// at least two runs on each side; pool bytes are deterministic and judged by the thresholds alone. A
// pair also shows the device's speedup over the host, with the baseline's beside it when the two differ
// by --minor percent or more, and a checksum or a point-index checksum that differs between the sides is
// noted: a change in speed is only comparable for equal answers.
//
// With --smoke (results of a smoke run) it judges no timings: per case, whether it ran on each side,
// whether the checksums match, and whether a pair's host and device answers agree, with the times
// shown for information only.
//
// Every report ends with one VERDICT line (green: no regression; yellow: look closer; red: do not
// merge), coloured on a terminal, and --markdown writes the report as Markdown starting with that line.
// --verdict-only prints only the line (Compare.cmake ends a run with it); --no-verdict leaves it out;
// --failed-runs is the number of runs that crashed, which makes the verdict red.
//
// Exit code: 0 for a green or yellow verdict, 2 for a red one, 3 when it cannot judge (a usage error or
// an unreadable file); Compare.cmake fails on anything else too (a crash). Usually run by Compare.cmake,
// through Scripts/run-performance.sh compare (see ../README.md).

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <map>
#include <optional>
#include <string>
#include <vector>

#include "../Common/Format.hpp"
#include "../Common/Schema.hpp"
#include "../Common/Statistics.hpp"

namespace {

using Perf::formatNumber;
using Perf::formatTime;

// ── Reading ─────────────────────────────────────────────────────────────────────────────────────────

// One side's results for one case, gathered over all its runs.
struct Measurement
{
  std::vector<double>   times;         // Every repetition of every run.
  std::vector<double>   runs;          // The median of each run's repetitions.
  std::optional<double> checksum;      // From the first run.
  std::string           indexChecksum; // From the first run; empty: not recorded.
  double                poolBytes = 0; // 0: not recorded.
  bool                  paired    = false;
  std::vector<double>   hostOverDevice; // One per run, for a pair.
  double                mismatches = 0; // The most over the runs.
  double                maxDiff    = 0;
  bool                  skipped    = false;
  std::string           reason; // Why it was skipped.
};

// One side's results file.
struct Results
{
  std::map<std::string, Measurement> cases;
  std::vector<std::string>           order; // Case names in the order they first appear.
  std::map<std::string, std::string> build; // What built and ran the cases (first build line).

  [[nodiscard]] std::string
  fact(const char* a_key) const
  {
    const auto it = build.find(a_key);

    return it == build.end() ? "?" : it->second;
  }

  [[nodiscard]] const Measurement*
  find(const std::string& a_name) const
  {
    const auto it = cases.find(a_name);

    return it == cases.end() ? nullptr : &it->second;
  }
};

Results
read(const std::string& a_file)
{
  std::ifstream in(a_file);

  if (!in) {
    std::fprintf(stderr, "PerfCompare: cannot read %s\n", a_file.c_str());
    std::exit(3);
  }

  using namespace Perf;

  Results     results;
  std::string text;

  while (std::getline(in, text)) {
    const JsonLine line(text);

    if (!line.has(Key::Case)) {
      for (const char* key : {Key::Git, Key::Compiler, Key::Simd, Key::GpuBackend, Key::Cpu}) {
        if (line.has(key) && results.build.count(key) == 0) {
          results.build[key] = line.text(key);
        }
      }

      continue;
    }

    const std::string name = line.text(Key::Case);

    if (results.cases.count(name) == 0) {
      results.order.push_back(name);
    }

    Measurement& m = results.cases[name];

    if (line.has(Key::Skipped)) {
      m.skipped = true;
      m.reason  = line.text(Key::Skipped);
      continue;
    }

    const std::vector<double> times = line.numbers(Key::TimesMs);
    m.times.insert(m.times.end(), times.begin(), times.end());

    if (!times.empty()) {
      m.runs.push_back(Perf::median(times));
    }

    if (line.has(Key::Checksum) && !m.checksum) {
      m.checksum = line.number(Key::Checksum);
    }

    if (line.has(Key::IndexChecksum) && m.indexChecksum.empty()) {
      m.indexChecksum = line.text(Key::IndexChecksum);
    }

    m.poolBytes = line.number(Key::PoolBytes, m.poolBytes);

    if (line.has(Key::HostOverDevice)) {
      m.paired = true;
      m.hostOverDevice.push_back(line.number(Key::HostOverDevice));
      m.mismatches = std::max(m.mismatches, line.number(Key::Mismatches));
      m.maxDiff    = std::max(m.maxDiff, line.number(Key::MaxDiff));
    }
  }

  return results;
}

// Every case name: the candidate's in its order, then those only the baseline has.
std::vector<std::string>
caseNames(const Results& a_base, const Results& a_head)
{
  std::vector<std::string> names = a_head.order;

  for (const auto& n : a_base.order) {
    if (a_head.cases.count(n) == 0) {
      names.push_back(n);
    }
  }

  return names;
}

// Whether two checksums differ beyond rounding.
bool
checksumsDiffer(double a_base, double a_head)
{
  return std::abs(a_base - a_head) > 1e-6 * std::max(std::abs(a_base), 1.0);
}

// Whether two sides' point-index checksums, recorded on both, differ (exactly: they are hashes).
bool
indicesDiffer(const Measurement& a_base, const Measurement& a_head)
{
  return !a_base.indexChecksum.empty() && !a_head.indexChecksum.empty() && a_base.indexChecksum != a_head.indexChecksum;
}

// Whether a case ran on a side: it has a result there, and was not skipped.
bool
ran(const Measurement* a_m)
{
  return a_m != nullptr && !a_m->skipped && !a_m->times.empty();
}

// ── The verdict line ────────────────────────────────────────────────────────────────────────────────

// What a comparison ends with: level 0 (green), 1 (yellow) or 2 (red), and the line.
struct Verdict
{
  int         level = 0;
  std::string text;
};

// Whether a report prints its verdict line after the table, not at all, or alone.
enum class VerdictLine
{
  After,
  None,
  Only
};

void
printVerdict(const Verdict& a_verdict)
{
  std::printf("\n%s\n", Perf::Style().verdict(a_verdict.level, a_verdict.text).c_str());
  std::fflush(stdout);
}

// ── The full comparison ─────────────────────────────────────────────────────────────────────────────

struct Thresholds
{
  double minor = 3.0;  // Percent.
  double major = 10.0; // Percent.
};

enum class Outcome
{
  Unchanged,
  Faster,
  Minor,
  Major,
  Differ,
  Lost,
  New,
  Removed,
  Skipped
};

const char*
label(Outcome a_outcome)
{
  switch (a_outcome) {
  case Outcome::Lost:
    return "lost";
  case Outcome::Faster:
    return "faster";
  case Outcome::Minor:
    return "minor";
  case Outcome::Major:
    return "MAJOR";
  case Outcome::Differ:
    return "DIFFER";
  case Outcome::New:
    return "new";
  case Outcome::Removed:
    return "removed";
  case Outcome::Skipped:
    return "skipped";
  default:
    return "~";
  }
}

// One case, compared.
struct Comparison
{
  std::string           name;
  Outcome               outcome = Outcome::Unchanged;
  bool                  timed   = false; // Whether both sides have times.
  bool                  judged  = false; // Whether both sides have at least two runs, so the time is judged.
  double                baseMs  = 0;
  double                headMs  = 0;
  Perf::Interval        interval;
  std::optional<double> poolRatio;
  bool                  poolUp   = false; // More pool bytes, beyond --minor.
  bool                  poolDown = false; // Fewer pool bytes, beyond --minor (noted, never a verdict).
  std::optional<double> baseChecksum;     // Set, with headChecksum, when the two differ.
  std::optional<double> headChecksum;
  bool                  indicesDiffer = false; // The point-index checksums differ.
  std::optional<double> speedup;               // A pair: the candidate's device speedup.
  std::optional<double> baseSpeedup;           // ... and the baseline's, when it differs by --minor or more.
  double                mismatches = 0;
  double                maxDiff    = 0;
  std::string           reason; // Why a side was skipped, or the case is lost.

  // The change in time, as a percentage, or "-" without times on both sides.
  [[nodiscard]] std::string
  change() const
  {
    return timed ? formatNumber("%+.1f%%", 100.0 * (headMs / baseMs - 1.0)) : "-";
  }

  [[nodiscard]] bool
  flagged() const
  {
    return outcome == Outcome::Major || outcome == Outcome::Minor || outcome == Outcome::Differ ||
           outcome == Outcome::Lost || this->answersChanged();
  }

  // Whether a checksum, of the values or of the points found, changed between the sides.
  [[nodiscard]] bool
  answersChanged() const
  {
    return baseChecksum.has_value() || indicesDiffer;
  }

  // Whether the verdict is red.
  [[nodiscard]] bool
  red() const
  {
    return outcome == Outcome::Major || outcome == Outcome::Differ || outcome == Outcome::Lost;
  }

  [[nodiscard]] std::string
  note() const
  {
    std::vector<std::string> parts;

    if (!reason.empty()) {
      parts.push_back(reason);
    }

    if (timed && !judged) {
      parts.emplace_back("fewer than two runs on a side: time not judged");
    }

    if (poolUp) {
      parts.emplace_back("pool bytes up");
    }

    if (poolDown) {
      parts.emplace_back("pool bytes down");
    }

    if (speedup) {
      std::string s = "device " + formatNumber("%.1f", *speedup) + "x faster than the host";

      if (baseSpeedup) {
        s += " (was " + formatNumber("%.1f", *baseSpeedup) + "x)";
      }

      parts.push_back(s);
    }

    if (mismatches > 0) {
      parts.push_back(formatNumber("%.0f", mismatches) + " host and device answers DIFFER (max diff " +
                      formatNumber("%.3g", maxDiff) + ")");
    }

    if (baseChecksum && headChecksum) {
      parts.push_back("checksum differs (" + formatNumber("%.10g", *baseChecksum) + " -> " +
                      formatNumber("%.10g", *headChecksum) + ")");
    }

    if (indicesDiffer) {
      parts.emplace_back("the points found differ (index checksum)");
    }

    std::string out;

    for (const auto& p : parts) {
      out += (out.empty() ? "" : "; ") + p;
    }

    return out;
  }
};

Comparison
compare(const std::string& a_name, const Measurement* a_base, const Measurement* a_head, const Thresholds& a_thresholds)
{
  Comparison c;
  c.name = a_name;

  // A case that ran in the baseline but not in the candidate is lost: whatever broke it (a missing
  // input, an exception, a removed case) must be looked at before the comparison means anything.
  if (ran(a_base) && !ran(a_head)) {
    c.outcome = Outcome::Lost;
    c.reason  = a_head == nullptr ? "missing in the candidate"
                : a_head->skipped ? "skipped in the candidate: " + a_head->reason
                                  : "no result in the candidate";
    return c;
  }

  if (a_base == nullptr || a_head == nullptr) {
    c.outcome = a_base == nullptr ? Outcome::New : Outcome::Removed;
    return c;
  }

  if (a_base->skipped || a_head->skipped) {
    c.outcome = Outcome::Skipped;
    c.reason  = a_base->skipped ? a_base->reason : a_head->reason;
    return c;
  }

  const double minor = 1.0 + a_thresholds.minor / 100.0;
  const double major = 1.0 + a_thresholds.major / 100.0;

  // The time: the change, and its interval, both over the runs (see the file comment). With one run on a
  // side there is nothing to tell a change from the noise between processes, so the time is not judged.
  int  severity = 0;
  bool faster   = false;

  if (!a_base->runs.empty() && !a_head->runs.empty()) {
    c.timed  = true;
    c.judged = a_base->runs.size() >= 2 && a_head->runs.size() >= 2;
    c.baseMs = Perf::median(a_base->runs);
    c.headMs = Perf::median(a_head->runs);

    if (c.judged) {
      c.interval = Perf::bootstrapRatio(a_base->runs, a_head->runs);

      const double ratio = c.headMs / c.baseMs;

      if (c.interval.low > 1.0 && ratio >= major) {
        severity = 2;
      }
      else if (c.interval.low > 1.0 && ratio >= minor) {
        severity = 1;
      }
      else if (c.interval.high < 1.0 && ratio <= 2.0 - minor) {
        faster = true;
      }
    }
  }

  // The pool bytes: deterministic, so the thresholds alone.
  if (a_base->poolBytes > 0 && a_head->poolBytes > 0) {
    c.poolRatio = a_head->poolBytes / a_base->poolBytes;

    if (*c.poolRatio >= minor) {
      c.poolUp = true;
      severity = std::max(severity, *c.poolRatio >= major ? 2 : 1);
    }

    c.poolDown = *c.poolRatio <= 2.0 - minor;
  }

  // A pair: the device's speedup, and whether host and device agree.
  if (a_head->paired) {
    c.speedup    = Perf::median(a_head->hostOverDevice);
    c.mismatches = a_head->mismatches;
    c.maxDiff    = a_head->maxDiff;

    if (a_base->paired) {
      const double was = Perf::median(a_base->hostOverDevice);

      if (std::abs(*c.speedup / was - 1.0) >= a_thresholds.minor / 100.0) {
        c.baseSpeedup = was;
      }
    }
  }

  if (a_base->checksum && a_head->checksum && checksumsDiffer(*a_base->checksum, *a_head->checksum)) {
    c.baseChecksum = a_base->checksum;
    c.headChecksum = a_head->checksum;
  }

  c.indicesDiffer = indicesDiffer(*a_base, *a_head);

  // One outcome, decided once: different answers first (they make the times incomparable), then the
  // worse of the time and the pool bytes, then faster.
  if (c.mismatches > 0) {
    c.outcome = Outcome::Differ;
  }
  else if (severity == 2) {
    c.outcome = Outcome::Major;
  }
  else if (severity == 1) {
    c.outcome = Outcome::Minor;
  }
  else if (faster) {
    c.outcome = Outcome::Faster;
  }

  return c;
}

Verdict
fullVerdict(const std::vector<Comparison>& a_cases, int a_failedRuns)
{
  const auto count = [&a_cases](auto a_pred) {
    return static_cast<std::size_t>(std::count_if(a_cases.begin(), a_cases.end(), a_pred));
  };

  const std::size_t major     = count([](const Comparison& c) { return c.outcome == Outcome::Major; });
  const std::size_t minor     = count([](const Comparison& c) { return c.outcome == Outcome::Minor; });
  const std::size_t differ    = count([](const Comparison& c) { return c.outcome == Outcome::Differ; });
  const std::size_t lost      = count([](const Comparison& c) { return c.outcome == Outcome::Lost; });
  const std::size_t checksums = count([](const Comparison& c) { return c.answersChanged(); });

  using Perf::counted;

  if (a_failedRuns > 0) {
    return {2,
            "VERDICT: " + counted(static_cast<std::size_t>(a_failedRuns), "run") +
              " crashed or exited with an error (see above); fix them before judging"};
  }

  if (lost > 0) {
    return {2,
            "VERDICT: " + counted(lost, "case") +
              " ran in the baseline but not in the candidate; fix them before judging (see Flagged above)"};
  }

  if (differ > 0) {
    return {2,
            "VERDICT: host and device answers differ in " + counted(differ, "case") +
              "; investigate before merging (see Flagged above)"};
  }

  if (major > 0) {
    return {2,
            "VERDICT: MAJOR regression in " + counted(major, "case") +
              "; do not merge until it is fixed or accepted (see Flagged above)"};
  }

  if (minor > 0) {
    return {1,
            "VERDICT: " + counted(minor, "minor regression") + "; rerun " + (minor == 1 ? "it" : "them") +
              " with --rounds 10 before deciding (see Flagged above)"};
  }

  if (checksums > 0) {
    return {1,
            "VERDICT: no regression, but " + counted(checksums, "checksum") +
              " changed: check those answers (see Flagged above)"};
  }

  return {0, "VERDICT: no regression (0 major, 0 minor across " + counted(a_cases.size(), "case") + ")"};
}

int
fullReport(const Results&     a_base,
           const Results&     a_head,
           const Thresholds&  a_thresholds,
           const std::string& a_markdown,
           VerdictLine        a_line,
           int                a_failedRuns)
{
  std::vector<Comparison> cases;

  for (const auto& name : caseNames(a_base, a_head)) {
    cases.push_back(compare(name, a_base.find(name), a_head.find(name), a_thresholds));
  }

  const Verdict verdict = fullVerdict(cases, a_failedRuns);
  const int     code    = verdict.level == 2 ? 2 : 0;

  if (a_line == VerdictLine::Only) {
    printVerdict(verdict);

    return code;
  }

  const auto count = [&cases](Outcome a_outcome) {
    return static_cast<std::size_t>(
      std::count_if(cases.begin(), cases.end(), [a_outcome](const Comparison& c) { return c.outcome == a_outcome; }));
  };

  const std::string summary =
    std::to_string(count(Outcome::Major)) + " major, " + std::to_string(count(Outcome::Minor)) + " minor, " +
    std::to_string(count(Outcome::Faster)) + " faster, " + std::to_string(count(Outcome::Differ)) + " differ, " +
    std::to_string(count(Outcome::Lost)) + " lost, " + Perf::counted(cases.size(), "case");

  const auto side = [](const char* a_label, const Results& a_r) {
    return std::string(a_label) + a_r.fact(Perf::Key::Git) + " (" + a_r.fact(Perf::Key::Compiler) + ", simd " +
           a_r.fact(Perf::Key::Simd) + ", gpu " + a_r.fact(Perf::Key::GpuBackend) + ")";
  };

  const std::string thresholds = "minor >= " + formatNumber("%g", a_thresholds.minor) +
                                 "%, major >= " + formatNumber("%g", a_thresholds.major) +
                                 "% slower, and outside the 95% bootstrap interval over the runs";

  // The table, for the terminal and for Markdown alike.
  using Perf::Align;

  Perf::Table table({{"case", Align::Left},
                     {"base", Align::Right},
                     {"head", Align::Right},
                     {"change", Align::Right},
                     {"95% interval", Align::Right},
                     {"pool", Align::Right},
                     {"verdict", Align::Left},
                     {"note", Align::Left}});
  Perf::Table markdown = table;

  for (const auto& c : cases) {
    const std::string change   = c.change();
    const std::string interval = c.judged ? "[" + formatNumber("%+.1f%%", 100.0 * (c.interval.low - 1.0)) + ", " +
                                              formatNumber("%+.1f%%", 100.0 * (c.interval.high - 1.0)) + "]"
                                          : "-";
    const std::string pool     = c.poolRatio ? formatNumber("%+.1f%%", 100.0 * (*c.poolRatio - 1.0)) : "-";
    const std::string base     = c.timed ? formatTime(c.baseMs) : "-";
    const std::string head     = c.timed ? formatTime(c.headMs) : "-";
    const std::string outcome  = label(c.outcome);

    table.add({c.name, base, head, change, interval, pool, outcome, c.note()});
    markdown.add(
      {"`" + c.name + "`", base, head, change, interval, pool, c.red() ? "**" + outcome + "**" : outcome, c.note()});
  }

  std::printf("\nPerformance comparison: %s\n", summary.c_str());
  std::printf("  %s\n  %s\n", side("base ", a_base).c_str(), side("head ", a_head).c_str());
  std::printf("  thresholds: %s\n\n%s", thresholds.c_str(), table.terminal().c_str());

  // The flagged cases again, so that they are not lost in a long table.
  Perf::Table flagged(
    {{"verdict", Align::Left}, {"case", Align::Left}, {"change", Align::Right}, {"note", Align::Left}});
  bool any = false;

  for (const auto& c : cases) {
    if (c.flagged()) {
      any = true;
      flagged.add({label(c.outcome), c.name, c.change(), c.note()});
    }
  }

  std::printf("%s", any ? ("\nFlagged:\n" + flagged.terminal()).c_str() : "\nNothing flagged.\n");

  if (a_line == VerdictLine::After) {
    printVerdict(verdict);
  }

  if (!a_markdown.empty()) {
    std::ofstream md(a_markdown);

    md << "**" << verdict.text << "**\n\n"
       << "Performance comparison: " << summary << "\n\n"
       << "- " << side("base: ", a_base) << "\n- " << side("head: ", a_head) << "\n"
       << "- CPU: " << a_head.fact(Perf::Key::Cpu) << "\n"
       << "- Thresholds: " << thresholds << "\n\n"
       << markdown.markdown();
  }

  return code;
}

// ── The smoke comparison ────────────────────────────────────────────────────────────────────────────

// Whether a case ran on one side.
std::string
state(const Measurement* a_m)
{
  if (a_m == nullptr) {
    return "missing";
  }

  return a_m->skipped ? "skipped" : (a_m->times.empty() ? "no result" : "ran");
}

int
smokeReport(
  const Results& a_base, const Results& a_head, const std::string& a_markdown, VerdictLine a_line, int a_failedRuns)
{
  using Perf::Align;

  Perf::Table table({{"case", Align::Left},
                     {"base", Align::Left},
                     {"head", Align::Left},
                     {"checksum", Align::Left},
                     {"answers", Align::Left},
                     {"result", Align::Left},
                     {"base time*", Align::Right},
                     {"head time*", Align::Right},
                     {"note", Align::Left}});
  Perf::Table markdown = table;

  std::size_t cases     = 0;
  std::size_t failed    = 0; // Missing, skipped or without a result on a side.
  std::size_t differ    = 0; // Pairs whose host and device answers disagree.
  std::size_t checksums = 0; // Checksums changed between the commits.

  for (const auto& name : caseNames(a_base, a_head)) {
    const Measurement* b = a_base.find(name);
    const Measurement* h = a_head.find(name);

    std::string checksum = "-";
    std::string answers  = "-";
    std::string result   = "ok";
    std::string baseTime;
    std::string headTime;
    std::string note;

    cases++;

    if (state(b) == "ran" && state(h) == "ran") {
      // The times, for information only: one repetition at the smoke sizes is no measurement, so they
      // are shown, and a change of more than ten times noted (a case that hangs or degenerates).
      const double tb = Perf::median(b->times);
      const double th = Perf::median(h->times);

      baseTime = formatTime(tb);
      headTime = formatTime(th);

      if (tb > 0 && th > 0 && (th / tb > 10.0 || tb / th > 10.0)) {
        note = "head " + formatNumber("%.0f", th > tb ? th / tb : tb / th) + "x " + (th > tb ? "slower" : "faster") +
               " (information only)";
      }

      if (b->checksum && h->checksum) {
        checksum = checksumsDiffer(*b->checksum, *h->checksum) || indicesDiffer(*b, *h) ? "DIFFERS" : "same";

        if (checksum == "DIFFERS") {
          checksums++;
          result = "check";
        }
      }

      if (h->paired) {
        answers = h->mismatches > 0 ? formatNumber("%.0f differ", h->mismatches) : "agree";

        if (h->mismatches > 0) {
          differ++;
          result = "FAIL";
        }
      }
    }
    else {
      failed++;
      result = "FAIL";
    }

    table.add({name, state(b), state(h), checksum, answers, result, baseTime, headTime, note});
    markdown.add({"`" + name + "`",
                  state(b),
                  state(h),
                  checksum,
                  answers,
                  result == "FAIL" ? "**FAIL**" : result,
                  baseTime,
                  headTime,
                  note});
  }

  using Perf::counted;

  Verdict verdict;

  if (a_failedRuns > 0) {
    verdict = {2,
               "VERDICT (smoke): FAILED: " + counted(static_cast<std::size_t>(a_failedRuns), "run") +
                 " crashed or exited with an error (see above)"};
  }
  else if (differ > 0) {
    verdict = {2, "VERDICT (smoke): FAILED: host and device answers differ in " + counted(differ, "case")};
  }
  else if (failed > 0) {
    verdict = {
      2, "VERDICT (smoke): FAILED: " + counted(failed, "case") + " missing or skipped on one side (rows marked FAIL)"};
  }
  else if (checksums > 0) {
    verdict = {1,
               "VERDICT (smoke): passed, but " + counted(checksums, "checksum") +
                 " changed between the commits: check those answers"};
  }
  else {
    verdict = {0,
               "VERDICT (smoke): passed; all " + counted(cases, "case") + " ran on both sides with the same answers"};
  }

  const int code = verdict.level == 2 ? 2 : 0;

  if (a_line == VerdictLine::Only) {
    printVerdict(verdict);

    return code;
  }

  const std::string summary = counted(cases, "case") + ", " + std::to_string(failed) +
                              " failed or missing on a side, " + counted(checksums, "checksum difference") + ", " +
                              counted(differ, "host/device disagreement");
  const char* const footnote = "* Information only: one repetition at the smoke sizes, dominated by start-up and "
                               "launch overhead\n  and far from the cache and traversal behaviour of the full sizes; "
                               "never judged.\n";

  std::printf("\nSmoke comparison (a check, not a measurement: no timing verdicts): %s\n", summary.c_str());
  std::printf("  base %s, head %s\n\n%s\n%s",
              a_base.fact(Perf::Key::Git).c_str(),
              a_head.fact(Perf::Key::Git).c_str(),
              table.terminal().c_str(),
              footnote);

  if (a_line == VerdictLine::After) {
    printVerdict(verdict);
  }

  if (!a_markdown.empty()) {
    std::ofstream md(a_markdown);

    md << "**" << verdict.text << "**\n\n"
       << "Smoke comparison (no timing verdicts): " << summary << "\n\n"
       << "- base: " << a_base.fact(Perf::Key::Git) << "\n- head: " << a_head.fact(Perf::Key::Git) << "\n\n"
       << markdown.markdown() << "\n\\* Information only: one repetition at the smoke sizes, never judged.\n";
  }

  return code;
}

// ── The command line ────────────────────────────────────────────────────────────────────────────────

[[noreturn]] void
usage(const std::string& a_problem)
{
  std::fprintf(stderr,
               "PerfCompare: %s\n"
               "usage: PerfCompare.ex --base <file> --head <file> [--minor <percent>] [--major <percent>]\n"
               "                      [--markdown <file>] [--smoke] [--verdict-only | --no-verdict]\n"
               "                      [--failed-runs <n>]\n",
               a_problem.c_str());
  std::exit(3);
}

// A non-negative number, or a usage error.
double
nonNegative(const std::string& a_flag, const std::string& a_text)
{
  char*        end = nullptr;
  const double v   = std::strtod(a_text.c_str(), &end);

  if (end == a_text.c_str() || *end != '\0' || !(v >= 0.0)) {
    usage(a_flag + " needs a non-negative number (got '" + a_text + "')");
  }

  return v;
}

} // namespace

int
main(int argc, char** argv)
{
  std::string baseFile;
  std::string headFile;
  std::string markdownFile;
  Thresholds  thresholds;
  bool        smoke      = false;
  VerdictLine line       = VerdictLine::After;
  int         failedRuns = 0;

  // A flag's value, or a usage error when it is missing.
  const auto value = [&](int& a_i, const std::string& a_flag) -> std::string {
    if (a_i + 1 >= argc) {
      usage(a_flag + " needs a value");
    }

    return argv[++a_i];
  };

  for (int i = 1; i < argc; i++) {
    const std::string arg = argv[i];

    if (arg == "--base") {
      baseFile = value(i, arg);
    }
    else if (arg == "--head") {
      headFile = value(i, arg);
    }
    else if (arg == "--markdown") {
      markdownFile = value(i, arg);
    }
    else if (arg == "--minor") {
      thresholds.minor = nonNegative(arg, value(i, arg));
    }
    else if (arg == "--major") {
      thresholds.major = nonNegative(arg, value(i, arg));
    }
    else if (arg == "--failed-runs") {
      failedRuns = static_cast<int>(nonNegative(arg, value(i, arg)));
    }
    else if (arg == "--smoke") {
      smoke = true;
    }
    else if (arg == "--verdict-only") {
      line = VerdictLine::Only;
    }
    else if (arg == "--no-verdict") {
      line = VerdictLine::None;
    }
    else {
      usage("unknown argument '" + arg + "'");
    }
  }

  if (baseFile.empty() || headFile.empty()) {
    usage("--base and --head are required");
  }

  const Results base = read(baseFile);
  const Results head = read(headFile);

  return smoke ? smokeReport(base, head, markdownFile, line, failedRuns)
               : fullReport(base, head, thresholds, markdownFile, line, failedRuns);
}
