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
> and Chombo, which are third-party projects with their own release cadence. All of them except
> the two `Shapes` folders were last compiled and run against AMReX's development branch
> (September 2026) and Chombo 3.2, on CPUs; the AMReX ones were also compiled (not run) for HIP. Treat them
> as a starting point for your own integration.
>
> The two `Shapes` folders are parked: they compose the analytic shapes through the virtual
> transform layer, which the shapes no longer derive from, and do not compile against the current
> API. They return with the tape (see their READMEs).

Each example requires the corresponding third-party library to be installed separately, with an
environment variable (`AMREX_HOME` or `CHOMBO_HOME`) pointing to it; see the README in each
example's own folder for exact build and run instructions.
