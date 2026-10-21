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

} // namespace

int
main(int argc, char** argv)
{
  std::string baseFile;
  std::string headFile;
  std::string markdownFile;
  double      minor = 3.0;
  double      major = 10.0;

  for (int i = 1; i < argc; i++) {
    const std::string arg = argv[i];

    if (arg == "--base" && i + 1 < argc) {
      baseFile = argv[++i];
    }
    else if (arg == "--head" && i + 1 < argc) {
      headFile = argv[++i];
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
      std::fprintf(stderr,
                   "usage: PerfCompare.ex --base <file> --head <file> [--minor %%] [--major %%] [--markdown <file>]\n");
      return 1;
    }
  }

  if (baseFile.empty() || headFile.empty()) {
    std::fprintf(stderr, "PerfCompare: --base and --head are required\n");
    return 1;
  }

  const Run base = read(baseFile);
  const Run head = read(headFile);

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

  if (!markdownFile.empty()) {
    std::ofstream md(markdownFile);

    md << "**Performance comparison: " << summary << "**\n\n";
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

  return majors > 0 ? 2 : 0;
}
