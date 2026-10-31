Integrations
============

This folder contains examples of coupling `EBGeometry` to a third-party application code:

* `AMReX/` — couples `EBGeometry` signed distance functions to
  [AMReX](https://amrex-codes.github.io/amrex/)'s embedded-boundary grid generation.

Each integration mirrors one or more of the native examples under [`Examples/`](../Examples/README.md) and hands
the object it builds to AMReX's embedded-boundary grid generation:

| Folder | What it hands over |
|---|---|
| `Shapes` | One analytic shape, a torus, or the classic CSG solid, in a `Tape` or fused (`shape = torus`, `csg` or `fused`). |
| `MeshSDF` | A surface mesh as a `TriMeshSDF`. |
| `MeshCSG` | The dragon on a table, blended by a smooth union, in a `Tape`. |
| `Materials` | The armadillo with a bored cylinder, in a `Tape`, and each cut cell labeled with its closest leaf and mesh face. |
| `ManyObjects` | Thousands of objects in a BVH union: spheres packed in a cylinder, or a city of boxes (`scene = spheres` or `city`). |

> [!IMPORTANT]
> These integrations are **not CI-tested**. Unlike everything under `Examples/`, which is compiled and
> run on every pull request, nothing here is built as part of `EBGeometry`'s continuous
> integration -- doing so would require installing and maintaining a matching version of AMReX,
> a third-party project with its own release cadence. All of them were last compiled and run in October 2026,
> on CPUs and on an NVIDIA GPU (CUDA), against AMReX's development branch (26.09); they have not been run
> with HIP.
> Treat them as a starting point for your own integration.

Several read a mesh from the `common-3d-test-models` git submodule by default: clone the repository with
`--recurse-submodules`, or run `git submodule update --init`.

Each integration requires AMReX to be installed separately, with the environment variable `AMREX_HOME`
pointing to it; see the README in each example's own folder for exact build and run instructions.
