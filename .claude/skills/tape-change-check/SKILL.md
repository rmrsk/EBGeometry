---
name: tape-change-check
description: Check that a change on EBGeometry's tape path did not move the code a plain tape compiles - the ptxas registers, stack and spills of the float tape kernels on the device, the host evaluator's out-of-line functions (nm -C -S), and the device compile cost - against the base. Use after editing code a Tape's evaluator or ray code reaches (EBGeometry_Tape*.hpp, CSGOps, the built-in shapes, the ray helpers), after adding a TapeFeature, a built-in primitive type or a CSG operation, and when a receipt shows the Tape cases moved.
---

# Checking a change on the tape's path

`CLAUDE.md` ("Composition goes through the tape...", "Tape code that only some expressions need is a tape feature")
says why: on a device, ptxas allocates registers over every function a kernel can reach, and on the host GCC's
inlining of the evaluator depends on its callees' sizes. Both move on changes that look unrelated (an equivalent
rewrite of a loop, compile-side code added to the tape's headers), and the receipt only shows it after a long run.
These checks predict it in minutes.

**When not to use it:** a change to comments or documentation only, to tests only, or to code no tape reaches.
Code the tape's compiler (`TapeDetail::Compiler`, the expression builder) adds to the tape's headers *does* count
on the host: it has shifted GCC's inlining of the evaluator before.

## The base

Export the base and the head side by side, outside the worktree (the scratchpad), so that both compile the same
way:

```bash
git archive origin/<base> | tar -x -C <scratch>/base
git archive HEAD          | tar -x -C <scratch>/head     # commit first, or copy the working tree
```

## 1. Device: the float tape kernels

Two small kernels per side (and `firstHit()` when ray code changed), compiled straight to SASS:

```cpp
// kreg.cu (kreg16.cu: the loop runs 16 times, adding up the values at shifted points)
#include "EBGeometry.hpp"
using namespace EBGeometry;
__global__ void __launch_bounds__(128) k(__grid_constant__ const Tape<float> t, const Vec3T<float>* p, float* d, int n)
{
  const int i = blockIdx.x * blockDim.x + threadIdx.x;
  if (i < n) d[i] = t.signedDistance(p[i]);
}
```

```bash
nvcc -std=c++17 -O3 -arch=sm_<local GPU> -cubin -Xptxas -v -I<side> kreg.cu -o /dev/null 2>&1 | grep -E "registers|stack|spill"
```

The registers, stack frame and spills must match the base's for both kernels: the base's figures are the reference,
whatever they are (168 registers each on sm_86 when this was written). The sixteen-evaluation kernel is the most sensitive: the single one can stay put while it jumps to 255.
Add the `firstHit()` kernel when the change reaches ray code, and a kernel of a tape listing the feature when you add
a `TapeFeature` (a plain `Tape<float>` must be unchanged; the feature's tape may grow).

## 2. Host: the evaluator's out-of-line functions

Build the `Tape` and `ClosestPrimitive` probes (`PerfTape`, `PerfClosestPrimitive`) of both sides as the
comparison does (`cmake --preset perf` in each exported tree, then `cmake --build --preset perf --target PerfTape
PerfClosestPrimitive`), and diff:

```bash
nm -C -S --size-sort <side>/build/perf/Performance/PerfTape | grep -E "Tape<|CSGOps" > <side>.nm
diff base.nm head.nm
```

Diff every `Tape<` symbol, user-primitive tapes included, and the tagged `evaluateBody` (the `closestLeaf()`
evaluator): a check limited to `Tape<float>`/`Tape<double>` has missed a moved split before. A function that newly
appears out of line, disappears, or changes size predicts a moved receipt. Typical fixes that held: keep the plain-`T`
code of a `CSGOps` function verbatim and put the tagged handling in `if constexpr` branches or a `Tagged<T>`
overload; never force `always_inline` on the dispatch (it pushes the shapes' distances out instead).

Read the plain tape's hot loop first, `Tape<float>::evaluateBody<0u, float>` and its double twin:
- **Same size on both sides:** the plain tape's evaluator compiled the same way (to be sure, diff
  `objdump -d --no-show-raw-insn` of it on both sides). A moved Tape case is then the change's own cost, or noise
  (about 1% on one pinned core).
- **Changed size:** the evaluator was laid out afresh, and the Tape cases can move by several percent either way with
  no cost of the change behind it (-5% to +6% for changes no probe tape executes; the measurements are in the comment
  at `evaluateBody` in `EBGeometry_Tape.hpp`). Say so in the report, with the sizes (e.g. "evaluateBody<0>
  recompiled, 1845 -> 2120 B: a layout draw"), and look further only at a move beyond -5%/+6%, or one the other Tape
  cases do not share.

This changes no verdict rule: a minor regression still goes to the maintainer, and a MAJOR one is still fixed or
accepted; it settles what they are told. Do not restructure the dispatch to win a draw back: every restructuring
tried (a cold path for the rare clauses, a `noipa` depth-0 entry) moved the plain tapes by -17% to +24%.

## 3. Device compile cost (new built-in types or heavy device code)

The receipt does not measure compile time. Time a Debug CUDA proxy (four kernels: `Tape<float>` and `Tape<double>`,
`signedDistance()` and `firstHit()`) on both sides:

```bash
/usr/bin/time -f "%e s %M KB" nvcc -g -std=c++17 -arch=sm_<local GPU> -DEBGEOMETRY_ENABLE_ASSERTIONS -I<side> proxy.cu -c -o /dev/null
```

A large jump means a new distance or loop is being inlined into every call level of every tape kernel. Bisect by
trimming the type list. The real build to confirm against is a Debug CUDA `TestRay`.

## 4. When a check moved

- Bisect the host timings with `Scripts/run-performance.sh compare <a> <b> 'Tape/*/float/*' --rounds 3`.
- Report the moved figures to the user with the change that caused them, and whether it is expected (new code the
  plain tape now reaches) or not. Do not wait for the receipt to find it.
