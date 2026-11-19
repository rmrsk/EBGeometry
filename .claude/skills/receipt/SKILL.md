---
name: receipt
description: Vet an EBGeometry pull request for performance - the quick look on a draft after a change to Source/ or Performance/, and the full comparison that records the performance receipt (Performance/receipts/<branch>.md) before promotion, with how to read and act on its verdicts and what goes in the PR's Performance section. Use when a PR changes the library or the probes, when asked for a receipt, a performance comparison or a quick look, or when CI's Performance-receipt check fails.
---

# Performance vetting of a pull request

The rules (when each step is due, who may accept a regression) are in `CLAUDE.md`, "Vetting every pull request
for performance". This is how to carry them out.

`Performance/` holds the probes (one program per component, many named cases,
`component/input/operation/precision/side`). `Scripts/run-performance.sh` is the one way in (its logic is
`Performance/RunPerformance.cmake` and `Performance/Compare.cmake`, needing only CMake, git and a compiler;
`--help` lists everything). The full description is `Docs/Sphinx/source/PerformanceTesting.rst`.

## Before any timed run

Timings are only meaningful on a quiet machine. Other agents share it and start timed runs directly.
- Wait for the maintainer's go that the machine is quiet; never use the `perf-slot` (Slurm) wrapper still found on some PATHs: other agents start timed runs outside its queue, so it gives no exclusivity.
- Check `pgrep -fa run-performance` and `nvidia-smi` for other runs, and `uptime` for load.
- Run the comparison directly, in the background.
- On the GPU, the device also drives the desktop: check `nvidia-smi` for compute apps and utilization (a viewer or
  a browser left running skews every device timing). For an ad-hoc A/B outside the comparison, build the variants
  as separate binaries, alternate them over several rounds, and call one slower only when the ranges do not overlap.
- Do not occupy the machine (builds and tests included) while another agent records a receipt.

## The quick look (a draft, after each change to the library or the probes)

One round, only the probes the change can affect (picked by glob), no receipt, reported in a line or two:

```bash
Scripts/run-performance.sh compare origin/<base> 'PointCloud*' --rounds 1
```

It takes a few minutes; one round shows times and judges none. When the PR changes device ray or render code,
the quick look is also where `--test-builds` and timing ptxas on the changed target belong (manual checks, never
part of the receipt).

## The receipt (once the code is final)

The code is final only after the `@claude review` rounds have converged; a fix from a round would make the receipt
stale. No extra rounds or repeated full runs while the code still moves.

1. **Run the comparison against the PR's base**, from the top of the branch's worktree, with the code committed.
   The receipt must have measured the base the branch is built on (its merge-base with `origin/<base>`), so rebase
   the branch onto `origin/<base>` first; a receipt recorded against a newer `origin/<base>` than the branch is
   built on fails CI.
   `--smoke` first gives a one-minute check that both sides build and every case runs with the same answers. The
   defaults, 2 rounds of 3 repetitions, are the least that gives a verdict, and take about eight minutes for every
   case, builds included. A probe that compiled to the same code on both sides is not run (listed as such in the
   report), so a PR touching one component runs only the probes it reaches (`--no-skip` runs them all); a side
   built before with the same code and settings is reused from `build/performance/cache` (`--no-cache` builds
   afresh).

   ```bash
   Scripts/run-performance.sh compare origin/<base> --record                       # host cases, receipt
   Scripts/run-performance.sh compare origin/<base> <pr-branch> --gpu cuda --gpu-arch <local GPU's, e.g. 86> \
                              --cuda-compiler <nvcc> --record                      # + device section
   ```

   - Add the device run whenever the PR touches code a device runs (`EBGEOMETRY_HOST_DEVICE` functions, `GPU::`,
     the pool/`rebasedView` machinery, the packed layouts). It compares the `hostdevice` pairs, which also report
     the host's time on every hardware thread but one, the device's speedup and whether the host and device
     answers agree.
   - The receipt's runs never take `--test-builds`: its build costs come from the probe builds the comparison
     does anyway, while the Debug CUDA builds of the heaviest tests take about 45 minutes a side.
   - Each case runs in a fresh process, alternating between the sides (a host comparison pinned to one core).
   - `--record` writes `Performance/receipts/<branch>.md`, named after the branch checked out (the host section;
     the device run adds a device section). Record it on the PR's branch, in its worktree. It refuses a detached
     HEAD, a HEAD argument other than the checked-out commit, a smoke run, a glob, uncommitted changes to the library or the probes, or a run with skipped cases (a
     fresh worktree needs `git submodule update --init` for the mesh cases).
   - `Scripts/run-performance.sh` with no command runs every case of the current tree, and a glob such as
     `'PointCloudBVH/*'` runs a subset.

2. **Report and commit.**
   - Report the result to the user in the terminal: the table and the "Run it manually" block the comparison
     prints.
   - Commit the receipt to the PR branch. Pushing it, like any push, needs the user's go-ahead.
   - In the PR body's *Performance* section, write a few lines, not the report: each side's verdict, the flagged
     cases with their change, any accepted reason, and a pointer to `Performance/receipts/<branch>.md`. The full report is
     long and is committed in the receipt anyway.
   - A PR that changes neither the library nor the probes keeps the receipt it inherited.

3. **Act on the verdicts.** The comparison ends with one `VERDICT:` line, green, yellow or red; a change counts
   only outside the 95% bootstrap interval.
   - **minor** (3% to 10% slower, or 3% to 10% more pool bytes): tell the user which cases, by how much, and the
     likely cause if the diff shows one, and **ask whether it should be investigated further** before going on.
     A yellow receipt fails CI until fixed or accepted.
   - **MAJOR** (10% or more): **red-flag it**. The PR must not be merged until the regression is fixed, or the
     user explicitly accepts it; record the reason in the PR's *Performance* section and on the receipt's
     `**Accepted:**` line (replacing its `-`).
   - A **differing checksum** (of the values or of the points found) or **host/device disagreements** are
     flagged whatever the timing says: the sides computed different answers.
   - A **lost** case (ran on the base, skipped or missing on the PR) is red: find out why it no longer runs before
     reading the timings.
   - **Build costs** (the report's last section: each probe's compile CPU time and peak memory, and every device
     kernel whose registers, stack or spills changed) are reported, not judged: they do not enter the verdict or
     CI. Report any flag to the user like a minor regression, with the kernels behind it, and say whether it is
     expected (new code) or not.
   - A flagged case is already confirmed: the comparison reruns every case flagged after its two rounds with 8
     more (`--confirm-rounds`) and judges it on all ten, so a flag is not one noisy round. Do not rerun until a
     result passes.

4. **What CI checks.** CI times nothing; it checks the receipt (`Performance-receipt` in
   `.github/workflows/CI.yml`, `Performance/CheckReceipt.cmake`, required by `CI-passed`), on a PR that is ready
   for review (a draft only gets a notice). It fails when:
   - the PR changes the library or the probes and its receipt has no host section;
   - any section present (host, or a device one) measured other code (any later code change makes it stale; a
     later docs-only commit does not);
   - a section measured another base than the one the branch is built on (it was rebased since, or the receipt
     measured a newer base: rebase and record again). The base branch moving on without the PR does not make
     the receipt stale;
   - a section is not green and has no reason on its `**Accepted:**` line under the section's heading (a yellow
     verdict, a minor regression or a changed checksum needs one too). A pass on an accepted verdict shows as a
     warning.

   Write an accepted reason into the receipt only when the user explicitly accepts that verdict, and give it as
   the user did: CI cannot tell who wrote it, and git history shows only that it was pushed from the user's
   account.
