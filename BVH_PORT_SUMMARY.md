# BVH GPU port — work summary

Branch `bvh_port_PR1`, three commits on top of `dev` (`e6914d4`). This covers steps A–C of the BVH
plan in `PLAN.md`; step D (the mesh-SDF wrappers on device) is not started.

| Commit | Subject |
|---|---|
| `8c87fc8` | Factor `pruneTraverse` into one loop; give the scalar path a real implementation |
| `4a97678` | Purge `SharedPtrStorage`, add `IndexStorage`, compile out the BVH CSG unions |
| `8eafaa5` | Move `PackedBVH`'s storage onto `Pool`/`PODVector`; add the device view |

## What changed

### 1. One traversal loop instead of seven

`pruneTraverse` held seven copies of the same branch-and-bound loop: six `if constexpr` SIMD
specialisations plus a scalar fallback that had no implementation of its own and instead built four
`std::function`s and delegated to `traverse()` on a heap `std::vector` stack. Comparing the six SIMD
blocks line by line, the only thing that differed was the computation of the K per-child squared
distances.

That one expression is now `computeChildDistances2()`, and there is a single traversal loop. The
scalar case is an ordinary path through it — fixed stack, hand-rolled insertion sort over the K
children, no `std::function`, no heap — rather than a different algorithm reached by a different
entry point. Every path computes `max(0, max(lo-p, p-hi))` per axis and combines as
`dx*dx + (dy*dy + dz*dz)`, so all of them agree bit-for-bit.

### 2. Storage policies reduced to two POD policies

`SharedPtrStorage` is gone: a `shared_ptr` is not trivially copyable, so a `PackedBVH` holding one
could never be mirrored, and keeping it would have forced a second, permanently host-only storage
backend. `ValueStorage` is the default everywhere. `IndexStorage` is new — a `uint32_t` resolved
against a caller-owned array through `get(stored, base)`, recovering what the shared-pointer policy
was actually wanted for (one primitive set shared by many BVHs) at four bytes instead of sixteen plus
a control block, minus automatic lifetime.

### 3. Pool-backed storage and the device view

All three arrays are `PODVector`s reserved from a caller-supplied `Pool`. Every construction entry
point takes a `Pool&`. `m_control`/`m_base`/`base()`/`rebasedView()`/`deepCopy()` duplicate
`DCEL::MeshT`'s pattern. `PackedBVH` is `static_assert`-ed trivially copyable at both precisions and
under both policies, and `pruneTraverse` is `EBGEOMETRY_HOST_DEVICE` with a stack depth of 256 on
host and 64 on device.

## Things found along the way that were not in the plan

**A latent bug that made whole instantiations uncompilable.** `ChildAABBSoA` asked for
`alignas(sizeof(T) * K)` unconditionally, which is not a legal alignment unless that product is a
power of two — so `PackedBVH<double, P, 3>` and its odd-K siblings had never compiled at all
(*"requested alignment 24 is not a positive power of 2"*). Nothing caught it because nothing
instantiated those K values. Only the `(T, K)` pairs with a SIMD path need the row aligned to its own
width, and those widths are all powers of two; everything else now takes `alignof(T)`.

**A stale claim in three places.** `PackedBVH`'s copy-constructor comment and two sections of
`ImplemBVH.rst` asserted that `MeshSDF` *could not* use `ValueStorage`, because `DCEL::FaceT`'s copy
constructor deliberately did not copy its cached 2D embedding. That stopped being true in #137, which
removed `Polygon2D` and left `FaceT` a plain trivially-copyable value. All three corrected.

**Polymorphic primitives block the purge.** `BVHUnionIF`/`BVHSmoothUnionIF` keep their primitives
alive through the primitive array of the `PackedBVH` they own, storing them as `shared_ptr<const P>`
where `P` is the abstract `ImplicitFunction<T>` in every real use. Neither remaining policy can hold
that. See *What is disabled* below.

**Value semantics broke `refit`.** Under `ValueStorage` the packed BVH owns its primitives, so
mutating the source geometry no longer reaches it — the refit test failed on exactly this. That
needed real API rather than a doc note: a mutable `getPrimitives()` so a packed geometry can be moved
in place, and a base parameter on `refit()` for `IndexStorage`.

**A copy is no longer a deep copy.** A pool-resident `PackedBVH` is descriptors plus two address
fields, so a copy aliases the original's pool memory. That shallowness is what makes the type
trivially copyable and therefore mirrorable, but it inverts what the old copy constructor meant.
`deepCopy(Pool&)` is the replacement.

## What is disabled

The BVH-accelerated CSG unions are **compiled out**, not deleted, behind one
`EBGEOMETRY_ENABLE_BVH_CSG_UNION` guard in `EBGeometry_CSG.hpp`. Restoring is a one-line change plus
a storage policy that can hold a polymorphic hierarchy — which is the index-based redesign of the
implicit-function layer (`PORTING.md` roadmap steps 4–5), not another policy.

* `BVHUnionIF`, `BVHSmoothUnionIF`, `BVHUnion`, `BVHSmoothUnion`, `CSGDetail::buildBVH`
* `Tests/TestCSG.cpp`'s BVH-union section (6 cases); `Tests/TestBVH.cpp`'s nested-BVH case
* `Examples/{CSGUnion,NestedBVH,PackedSpheres,RandomCity}` — the guard wraps each `main()` body and
  the disabled branch prints why and exits 0, so they still build and `ctest` still runs them
* `Integrations/{AMReX,Chombo}/{PackedSpheres,RandomCity}` — header note only. These are illustrative,
  are not built or tested by CI, and could not be compile-tested here, so the code is left intact.

## Verification

| Check | Result |
|---|---|
| `ctest --preset debug` | 305/305 |
| `ctest --preset debug-san` (ASan/UBSan) | 305/305 |
| `ctest --preset release-test` | 311/311 |
| `ctest --preset examples` | 11/11 |
| `TestBVH` at `EBGEOMETRY_SIMD=sse41` / `avx` | 1650 assertions, 30 cases each |
| `EBGEOMETRY_SIMD=avx512` | compiles only — no AVX-512 on this CPU |
| clang-format, clang-tidy, codespell, reuse, doxygen-check | pass |
| Sphinx HTML | builds clean |

New tests worth naming:

* **Scalar/SIMD agreement.** `pruneTraverse` dispatches on `(K, T)` and the compiled ISA, so sweeping
  K over values that do and do not have a vector path exercises both within one binary. The sweep
  requires *exact* equality, not a tolerance. This is what surfaced the `ChildAABBSoA` alignment bug.
* **`ValueStorage` vs `IndexStorage`** over one primitive set, requiring bit-identical results.
* **Host-to-host `rebasedView`**, which exercises the rebase invariant without a GPU — the only way
  it runs in CI at all.

### The gap

**The `[gpu]` case has never been compiled.** This machine has an NVIDIA RTX A4000 but no CUDA
toolkit (`nvcc` is not installed), so neither the `cuda` nor the `hip` preset can be configured here.
`TestBVH` is registered in `EBGEOMETRY_GPU_TESTS` and the case is written, but it is unverified.

Mitigation: the functors it hands to `pruneTraverse` are hoisted out of the device-only guard and
used by the host rebase test, so the callable path — the part most likely to be wrong, since the
CUDA/HIP builds do not pass `--extended-lambda` and device callers must pass functors rather than
lambdas — is compiled and run on every build. Only the kernel launch itself is untested. This needs a
machine with a toolkit before it is trusted; a green CI GPU lane would only mean "it compiles".

## Not done

* **Step D**, the mesh-SDF wrappers on device (`TriMeshSDF` then `MeshSDF`).
* **The DCEL reconcile chain** (`PORTING.md` roadmap step 1), deliberately deferred. It is
  independent of the BVH and has no device caller today: `MeshT::reconcile()`'s only production call
  site is `Soup`, which is host-only by design. `PORTING.md` now records that ordering decision.
