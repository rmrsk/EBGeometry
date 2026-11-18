---
name: new-csg-operation
description: Add a new CSG operation to EBGeometry (a combination, a transform, a value or point operation) in every place it must exist - the expression builder, CSGOps, the tape's clauses and evaluator, the fused operation, optionally its ray form - with tests, docs and the tape checks. Use when implementing a new operation such as a twist, a taper, a new blend or a repetition. Not for a new analytic shape (a primitive), nor for a change inside an existing operation.
---

# Adding a CSG operation

`CLAUDE.md` holds the rules this follows: composition goes through the tape or the fused operations, which share
their arithmetic and must give the same values; tape code only some expressions need is a `TapeFeature`; the tape
has no recursion. The order below keeps the two sides in step.

**When not to use it:** a new primitive shape (it needs a shape type, its place in the tape's type list, and the
compile-cost check of `tape-change-check`), or a fix or tweak inside an existing operation (change its arithmetic in
`CSGOps`, then run `tape-change-check`).

1. **The API first.** A new public function is approved before it is implemented (`CLAUDE.md`, "Implementing a
   feature", step 2): its declaration in `EBGeometry_Expr.hpp` with its Doxygen contract, and the `Fused::` function.
2. **The arithmetic, once,** in `EBGeometry_CSGOps.hpp`. Write the plain-`T` code as it would be written alone; a
   tagged variant (`CSGOps::Tagged<T>`, for `closestLeaf()`) goes in an `if constexpr` branch or a separate overload,
   never by rewriting the plain code generically.
3. **The builder** (`EBGeometry_Expr.hpp`, `EBGeometry_ExprImplem.hpp`): an `ExprDetail::Kind`, its node, its
   bounding box rule and its distance-quality rule (how far its value can be trusted as a distance given its
   operands').
4. **The tape** (`EBGeometry_Tape.hpp`, `EBGeometry_TapeImplem.hpp`): clauses in `TapeDetail::Op` (fields documented
   at the enumerator), their emission in `TapeDetail::Compiler`, and a case in `Tape::evaluateBody` (every `case`
   braced, a blank line before `break`). An operation that evaluates its operand several times or at other points
   compiles the operand into a sub-program and calls it through `evaluate<D - 1>`, within `Tape::MaxCallDepth`; no
   recursion, function pointers or virtual calls. Keep the value and point stacks within `ValueStackSize` and
   `PointStackSize`.
5. **The fused operation** (`EBGeometry_Fused.hpp`): a `Fused::` node type and function using the same `CSGOps`
   arithmetic, with its distance quality and box.
6. **Rays (optional).** Without more, a ray marches the operation. An exact ray form needs span arithmetic
   (`EBGeometry_RaySpans.hpp`), a `RayOp` in the tape's ray program, and a fused ray form
   (`EBGeometry_CSGRayForms.hpp`); in a tape it is a `TapeFeature` unless every tape needs it.
7. **Tests** (`Tests/TestCSG*.cpp`, `TestTape.cpp`, `TestFused.cpp`): the value against an independent reference
   (a closed form, a brute force), both precisions; the box and distance quality; `TestFused`'s comparison with the
   tape; a `[gpu]` case through `GPU::transform`; a death test for each new `EBGEOMETRY_REQUIRE`.
8. **Coverage and docs:** `Tests/InstantiateAll.cpp`; the operation's place in the CSG concept page and in
   `ImplemCSG.rst` (Doxygen-linked); an example if it is a new capability users need shown; performance cases in
   the `Tape` and `Fused` probes if it is something users call in bulk.
9. **Checks:** the `tape-change-check` skill (registers, `nm -C -S`, and the compile cost when the evaluator grew),
   then the quick performance look.
