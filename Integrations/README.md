Integrations
============

This folder contains examples of coupling `EBGeometry` to third-party application codes:

* `AMReX/` — couples `EBGeometry` signed distance functions to
  [AMReX](https://amrex-codes.github.io/amrex/)'s embedded-boundary grid generation.
* `Chombo/` — couples `EBGeometry` signed distance functions to
  [Chombo](https://commons.lbl.gov/display/chombo/)'s embedded-boundary grid generation.

> [!IMPORTANT]
> These examples are **not CI-tested**. Unlike everything under `Examples/`, which is compiled and
> run on every pull request, nothing here is built as part of `EBGeometry`'s continuous
> integration -- doing so would require installing and maintaining a matching version of AMReX
> and Chombo, which are third-party projects with their own release cadence. All of them were last
> compiled and run against AMReX's development branch and Chombo 3.2, on CPUs (the two `Shapes`
> folders in October 2026, the others in September 2026); the AMReX ones were also compiled (not run) for HIP. Treat them
> as a starting point for your own integration.

Each example requires the corresponding third-party library to be installed separately, with an
environment variable (`AMREX_HOME` or `CHOMBO_HOME`) pointing to it; see the README in each
example's own folder for exact build and run instructions.
