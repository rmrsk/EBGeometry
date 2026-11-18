---
name: new-example
description: Create a new EBGeometry example program under Examples/ with its three build routes, its README, its Sphinx page wired into a group page, its place in the reading order and its rendered figure. Use when a new capability needs to be shown to users, or when asked to add an example. Not for a test (Tests/ verifies) or a performance case (Performance/ measures).
---

# Adding an example

The rules for what an example is (it teaches and checks nothing, one topic, about a hundred lines, no third-party
dependencies, cited by the docs instead of tests) are in `CLAUDE.md`, "Adding unit tests, examples and performance
cases". This is how to add one so that every build route and the docs keep working.

**When not to use it:** a variant of an existing example's topic (extend that one, within its hundred lines), or
code that needs a third-party library (that is an `Integrations/` program).

1. **Copy the nearest example** to `Examples/<Name>/`: `main.cpp`, `CMakeLists.txt`, `GNUmakefile`, `README.md`.
   Rename every occurrence of the old name in all four. The copied build files keep the switches working:
   `PRECISION`/`EBGEOMETRY_PRECISION`, `ASSERTIONS`/`EBGEOMETRY_ENABLE_ASSERTIONS`, `EBGEOMETRY_HOME`, and the GPU
   switch (`GPU=CUDA|HIP`, `-DEBGEOMETRY_EXAMPLE_GPU=...`). CMake and CI pick the folder up by themselves.
2. **Write `main.cpp` to be read:** the calls in the order a user writes them, comments saying why, printed results a
   reader can follow; exit 0. An example about device queries has four parts in order: the query functor, the
   construction, host use, the same query through `GPU::transform()`, skipping the device part when no GPU is
   visible. An example that builds a shape ends with `Render::view(shape, pool, argc, argv)` (`view(shape, argc,
   argv)` for a shape in no pool).
3. **Run all three routes** in a Release build (direct compiler call, `make`, standalone CMake), each placing
   `<Name>.ex` in the example's folder, and once with `PRECISION=float`. If it has a device part, build it with
   `GPU=CUDA` too. It must run in seconds in Release and within the 300 s ctest limit in Debug.
4. **The Sphinx page:** `Docs/Sphinx/source/Example<Name>.rst`, listed with a line and a toctree entry on its group
   page (`Examples<Group>.rst`). No `literalinclude`: describe the program and link the classes' Doxygen pages.
5. **The reading order:** a line in `Examples/README.md`, in the same group.
6. **The figure:** `Scripts/render-example-figures.sh <Name>` writes `Docs/Sphinx/source/_static/example_<Name>.jpg`
   (tracked); show it on the page.
7. **Cite it:** the `Implem*.rst` or concept page that describes the capability points to the new page.
8. Run the `docs-audit` skill's Sphinx build (warnings as errors).
