# Dev/ — DELETE THIS ENTIRE FOLDER BEFORE MERGING

**Reviewer: this whole `Dev/` directory must be removed before the PR is merged.**

It holds the *old*, verbose point-cloud example programs that were superseded by the turnkey
`PointCloudBVH` class (`Source/EBGeometry_PointCloudBVH.hpp`) and its two replacement examples,
`Examples/ClosestPoint` and `Examples/NearestNeighbor`.

## What's here and why it's kept (temporarily)

| Old example | Replaced by |
|-------------|-------------|
| `ClosestPointSFCPacked`   | `Examples/ClosestPoint` (uses `PointCloudBVH::closestPoint` / `closestPoints`) |
| `ClosestPointTreePacked`  | `Examples/ClosestPoint` |
| `NearestNeighborSFCPacked`  | `Examples/NearestNeighbor` (uses `PointCloudBVH::allNearestNeighbors`) |
| `NearestNeighborTreePacked` | `Examples/NearestNeighbor` |

These four hand-rolled the whole pipeline: SFC-grouping points into `PointAoSoA` leaves (or forming
them from a `TreeBVH` via `packWith()`), building a `PackedBVH` five ways, and driving
`pruneTraverse()` directly with a bespoke traversal state. `PointCloudBVH` now hides all of that:
the construction path (so the **SFC vs Tree** split no longer exists) *and* the `pruneTraverse`
guts, behind a few high-level query methods.

They are retained only so the two paths can be compared side by side during review (build strategy
sweeps, reciprocal-culling demonstration, the SFC-vs-Tree construction contrast). None of them is
registered in `Examples/CMakeLists.txt`, so they are **not** built or run by the top-level project
or `ctest`; they only build via their own standalone `CMakeLists.txt` / `GNUmakefile`.

## Before merge

1. `git rm -r Dev/`
2. Remove the `Dev/**` annotation block from `REUSE.toml`.
3. Confirm nothing references `Dev/` (`grep -rn "Dev/" --include='*.rst' --include='*.md' Docs README.md`).
