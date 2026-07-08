ThirdParty
============

This folder contains examples of coupling `EBGeometry` to third-party application codes:

* `AMReX/` — couples `EBGeometry` signed distance functions to
  [AMReX](https://amrex-codes.github.io/amrex/)'s embedded-boundary grid generation.
* `Chombo/` — couples `EBGeometry` signed distance functions to
  [Chombo](https://commons.lbl.gov/display/chombo/)'s embedded-boundary grid generation.

> [!IMPORTANT]
> These examples are **illustrative, not maintained or CI-tested**. Unlike everything under
> `Examples/`, which is compiled and run on every pull request, nothing here is built as part of
> `EBGeometry`'s continuous integration -- doing so would require installing and maintaining a
> matching version of AMReX and Chombo, which are third-party projects with their own release
> cadence, independent of `EBGeometry`. An example here may not compile against the current
> release of its target platform; treat it as a starting point for your own integration, not a
> guarantee that it works out of the box.

Each example requires the corresponding third-party library to be installed separately, with an
environment variable (`AMREX_HOME` or `CHOMBO_HOME`) pointing to it; see the README in each
example's own folder for exact build and run instructions.
