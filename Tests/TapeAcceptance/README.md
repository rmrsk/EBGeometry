Tape acceptance tests
=====================

Programs that the tape must make work again. None of them compiles against the current API, and
nothing builds them: CMake, the GNU makefiles and CI all leave them out.

Each one composes objects through the virtual `ImplicitFunction` layer, which the analytic shapes
and the mesh SDFs no longer derive from now that they are plain, device-callable value types. The
tape, EBGeometry's value-type expression builder, replaces that layer (decisions D4, D5 and D13 in
`AUDIT.md`). When it lands, each program is ported to it, must produce the same geometry as
described below, and returns to where it came from.

| Program | What it composes | Returns to |
|---------|------------------|------------|
| `CSGUnion.cpp` (here) | A BVH-accelerated union of a mesh SDF and a sphere: two primitives of different types, which needs the tape's runtime dispatch. The union must equal the smaller of the two distances everywhere | `Examples/CSGUnion`, with a page in the Sphinx Examples section |
| `Integrations/AMReX/Shapes` | Each analytic shape, plus `Offset`, `Elongate`, `Annular`, `SmoothDifference` and `Complement` | Stays in place, ported |
| `Integrations/Chombo/Shapes` | Each analytic shape, plus `Offset`, `Annular` and `Complement` | Stays in place, ported |

A union of many objects of one type does not need the tape: `BVHUnion` and `BVHSmoothUnion` do it
today, on the host and on a device (see `Examples/PackedSpheres` and `Examples/RandomCity`).
