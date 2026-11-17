Integrations
============

This folder contains examples of using `EBGeometry` with third-party code:

* `MPI/` — uses `EBGeometry` under MPI: every rank holds the whole geometry, so its queries need no communication,
  and a node can hold one copy of it. **Maintained: built and run in CI.**
* `AMReX/` — couples `EBGeometry` signed distance functions to
  [AMReX](https://amrex-codes.github.io/amrex/)'s embedded-boundary grid generation. Not CI-tested.

MPI
---

| Folder | What it shows |
|---|---|
| `ParticleWalls` | A domain-decomposed particle code: each rank traces its particles' paths against the whole geometry (a `TriMeshSDF`) and reflects them off it; the only communication is the migration of particles between ranks (`MPI_Alltoallv`). |
| `SharedGeometry` | The geometry built once on rank 0 and rebuilt on the other ranks from its pool's bytes and its `Layout`, with one copy per node in an MPI shared-memory window (`MPI_Win_allocate_shared`) behind a `MemoryResource`. |

These are maintained: CI builds both with Open MPI and runs them on four ranks (`mpirun -n 4`), so a change that
breaks them fails the pull request. They need an MPI installation and nothing else; their default mesh is in the
repository (`Examples/data/`). See the README in each folder.

AMReX
-----

Each integration mirrors one or more of the native examples under [`Examples/`](../Examples/README.md) and hands
the object it builds to AMReX's embedded-boundary grid generation:

| Folder | What it hands over |
|---|---|
| `Shapes` | One analytic shape, a torus, or the classic CSG solid, in a `Tape` or fused (`shape = torus`, `csg` or `fused`). |
| `MeshSDF` | A surface mesh as a `TriMeshSDF`. |
| `MeshCSG` | The dragon on a table, blended by a smooth union, in a `Tape`. |
| `SurfaceLabels` | The armadillo with a bored cylinder, in a `Tape`, and each cut cell labeled with its closest leaf and mesh face. |
| `Materials` | One subdomain of a domain (a cable's regions: the air, or the jacket), and each cut cell labeled with its boundary condition, its role (a boundary or an interface) and the region across. |
| `ManyObjects` | Thousands of objects in a BVH union: spheres packed in a cylinder, or a city of boxes (`scene = spheres` or `city`). |

> [!IMPORTANT]
> The AMReX integrations are **not CI-tested**. Unlike everything under `Examples/`, which is compiled and
> run on every pull request, none of them is built as part of `EBGeometry`'s continuous
> integration -- doing so would require installing and maintaining a matching version of AMReX,
> a third-party project with its own release cadence. All of them were last compiled and run in October 2026,
> on CPUs and on an NVIDIA GPU (CUDA), against AMReX's development branch (26.09); they have not been run
> with HIP.
> Treat them as a starting point for your own integration.

Several read a mesh from the `common-3d-test-models` git submodule by default: clone the repository with
`--recurse-submodules`, or run `git submodule update --init`.

Each integration requires AMReX to be installed separately, with the environment variable `AMREX_HOME`
pointing to it; see the README in each example's own folder for exact build and run instructions.
