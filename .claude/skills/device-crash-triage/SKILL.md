---
name: device-crash-triage
description: Find the cause of an EBGeometry GPU failure - an illegal memory access, a misaligned address, a wrong answer only on the device, a crash only in Release - from a [gpu] test or a GPU::transform() run. Covers compute-sanitizer, cuda-gdb on a -lineinfo build, narrowing the inputs to a few lanes of one warp, ruling out our own code on the host, and testing another ptxas. Use when a device run fails or disagrees with the host and the cause is not obvious from the diff.
---

# Triage of a device failure

Aim: decide whether the bug is ours (a lifetime, a bound, undefined behavior) or the toolchain's, with evidence
someone else can rerun. Keep every build and repro in the scratchpad, not in the repository.

**When not to use it:** a failure the diff explains (an index you just changed, a missing `rebasedView()`), or a
host failure (run the debug preset and the sanitizer preset first).

## 1. Reproduce small

- Rebuild only the failing target, in the configuration that fails (Release, the precision, `-arch` of the local
  GPU). Note which variants pass: Debug, assertions on (`-DEBGEOMETRY_ENABLE_ASSERTIONS`), `-G`, the other
  precision. A crash only without assertions or only in Release points at code generation or undefined behavior.
- Cut the test down to a standalone `.cu` that calls the same functor through `GPU::transform()`, with the inputs
  dumped from the test. Make the input set selectable from the environment (a first index and a count, or a list of
  indices), so it can be bisected without recompiling.

## 2. Ask the tools

- `compute-sanitizer --tool memcheck` names the faulting kernel, thread and address; `initcheck`, `racecheck` and
  `synccheck` rule out uninitialized reads and races.
- Build with `-lineinfo` and disassemble (`cuobjdump -sass`) to tie the faulting PC to a source line.
- `cuda-gdb`: `set breakpoint pending on` before breaking in a kernel; `info cuda kernels`, `cuda block (b) thread
  (t)` to focus a lane, `info registers` (uniform registers `UR*` too) and `x/i $pc`. Compare the faulting warp with
  a healthy one: a value that differs only on one warp, written by no instruction on its path, is not our code.

## 3. Narrow the inputs

Bisect the input list until the failure needs the fewest points. Then test what depends on lanes: each point alone,
the same points in another order, another point placed in front. If a set fails only in some lane assignments and no
code on the path lets threads interact, the failure comes from warp execution (the compiler or the driver), not from
per-thread logic.

## 4. Rule out our own code

- Check every local array on the path against its bound (`EBGEOMETRY_REQUIRE` at build time, or a guard).
- Run the same functor on the host, in a loop over the same inputs, built `-O3` with
  `-fsanitize=address,undefined` (and `ASAN_OPTIONS=detect_stack_use_after_return=1`), and under valgrind memcheck.
- Run the device build with assertions on.

Clean everywhere, with a lane-dependent repro, means a toolchain suspect.

## 5. Test the toolchain

- Another ptxas on the same PTX: get it from NVIDIA's PyPI wheels (`nvidia-cuda-nvcc-cu12` and the like; running a
  downloaded binary needs the user's approval; extract each wheel into its own new, empty directory in the
  scratchpad), take nvcc's command list with `--dryrun`, and swap the ptxas line.
- A newer driver or toolkit, where the user can provide one (CUDA 13 needs driver 580 or newer).
- Search NVIDIA's CUDA Toolkit release notes ("resolved issues", compiler) for the symptom.

## 6. Report

State what was ruled out and how, the smallest repro and how to run it, the versions (toolkit, ptxas, driver, GPU,
`sm_XX`), and whether a workaround is reliable (check it on a variant of the code: a workaround that only moves the
code can make the crash move too). If the PR waits on it, put the handoff in the PR as a comment.
