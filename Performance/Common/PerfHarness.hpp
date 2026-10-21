// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file   PerfHarness.hpp
 * @brief  The harness every probe under Performance/ includes: everything in Common/.
 * @details A probe builds one Perf::Suite from its command line, adds its cases, and runs it. A case is
 * a name and a function: the function sets up its inputs (untimed), times the operation with
 * Perf::Case::measure(), and records a checksum, its code-selecting parameters and, where there is one,
 * its pool bytes. The headers, from the bottom up:
 *
 *   Format.hpp       text: units, padding, terminal style, tables         (shared with PerfCompare)
 *   Statistics.hpp   quantiles, summaries, the bootstrap interval          (shared with PerfCompare)
 *   Schema.hpp       the JSON field names, writer and reader               (shared with PerfCompare)
 *   Environment.hpp  the commit, compiler, SIMD level, CPU and host threads
 *   Case.hpp         one case: its name, timing and records
 *   Inputs.hpp       random points with named seeds, Morton order, the mesh files
 *   Report.hpp       the table on the terminal and the JSON lines
 *   Suite.hpp        the command line, which cases a build keeps, and the run loop
 *   Device.hpp       device timing and the host/device pairs
 *   Probe.hpp        what the probes have in common: queries, checksums, agreement, timing helpers
 *
 * This is host code for the probes, not part of the library.
 */

#ifndef EBGEOMETRY_PERF_HARNESS_HPP
#define EBGEOMETRY_PERF_HARNESS_HPP

#include "Case.hpp"
#include "Device.hpp"
#include "Environment.hpp"
#include "Format.hpp"
#include "Inputs.hpp"
#include "Report.hpp"
#include "Schema.hpp"
#include "Statistics.hpp"
#include "Suite.hpp"

#endif
