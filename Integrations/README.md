Integrations
============

This folder contains examples of coupling `EBGeometry` to third-party application codes:

* `AMReX/` — couples `EBGeometry` signed distance functions to
  [AMReX](https://amrex-codes.github.io/amrex/)'s embedded-boundary grid generation.
* `Chombo/` — couples `EBGeometry` signed distance functions to
  [Chombo](https://commons.lbl.gov/display/chombo/)'s embedded-boundary grid generation.

Each integration mirrors one of the native examples under [`Examples/`](../Examples/README.md) (`Shapes`,
`BasicCSG`, `MeshSDF`, `MeshCSG`, `PackedSpheres`, `RandomCity`, and for AMReX `PaintEB`) and hands the
object it builds to the platform's embedded-boundary grid generation.

> [!IMPORTANT]
> These integrations are **not CI-tested**. Unlike everything under `Examples/`, which is compiled and
> run on every pull request, nothing here is built as part of `EBGeometry`'s continuous
> integration -- doing so would require installing and maintaining a matching version of AMReX
> and Chombo, which are third-party projects with their own release cadence. All of them were last
> compiled and run in October 2026, on CPUs, against AMReX's development branch (26.09) and Chombo 3.3.
> Treat them as a starting point for your own integration.

Several read a mesh from the `common-3d-test-models` git submodule by default: clone the repository with
`--recurse-submodules`, or run `git submodule update --init`.

Each integration requires the corresponding third-party library to be installed separately, with an
environment variable (`AMREX_HOME` or `CHOMBO_HOME`) pointing to it; see the README in each
example's own folder for exact build and run instructions.
