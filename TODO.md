# EBGeometry — open hardening tasks

Items discovered during the July 2026 `noexcept` audit and broader source review.
Checked boxes are already fixed.

---

## 1. `noexcept` correctness bugs (immediate: `std::terminate` risk)

Functions declared `noexcept` that call operations which can throw, turning any
exception into a silent `std::terminate` with no recovery path.

- [x] `Vec3T::operator[]` (×2) — `.at()` replaced with `EBGEOMETRY_EXPECT` + `m_X[i]`
- [x] `Vec3T::unit()` — `.at()` replaced with `EBGEOMETRY_EXPECT` + `m_X[a_dir]`
- [x] `Vec3T::lessLX()` — `std::tuple_cat` replaced with direct 3-way comparison
- [x] `MeshT::incrementWarning` — `map::at()` replaced with `operator[]`; `noexcept` removed from `incrementWarning`, `printWarnings`, `sanityCheck`

---

## 2. Improper `noexcept`: allocating functions (remove `noexcept`)

Policy from `Contributing.rst`: *"Functions that perform heap allocation must not
be declared `noexcept`."*

### Parser (`EBGeometry_ParserImplem.hpp`)

- [x] All 18 `read*` overloads — `noexcept` removed; `[[nodiscard]]` added

### Transform (`EBGeometry_TransformImplem.hpp`)

- [x] `Complement`, `Translate`, `Rotate`, `Scale`, `Offset`, `Annular`, `Blur`, `Mollify`, `Elongate`, `Reflect` — `noexcept` removed from factory functions

### CSG (`EBGeometry_CSGImplem.hpp`)

- [x] `Union` (both overloads), `SmoothUnion` (both), `FastUnion` — `noexcept` removed
- [x] `Intersection` (both), `SmoothIntersection` (both), `Difference`, `SmoothDifference`, `FiniteRepetition` — `noexcept` removed
- [x] `UnionIF`, `SmoothUnionIF`, `FastUnionIF` constructors — `noexcept` removed

### BVH (`EBGeometry_BVHImplem.hpp`)

- [x] `TreeBVH(const vector<PrimAndBV>&)` constructor
- [x] `topDownSortAndPartition`, `bottomUpSortAndPartition`
- [x] `pack()`, `packWith()`, `PackedBVH::buildSoA()`

### DCEL Face / Vertex

- [x] `FaceT::gatherVertices()`, `gatherEdges()`, `getAllVertexCoordinates()`, `computePolygon2D()` — `noexcept` removed
- [x] `FaceT::reconcile()`, `computeNormal()`, `computeCentroid()`, `computeArea()`, `getSmallestCoordinate()`, `getHighestCoordinate()` — `noexcept` removed (transitive callers)
- [x] `VertexT::addFace()`, `computeVertexNormalAngleWeighted()` (both overloads) — `noexcept` removed

### MeshDistanceFunctions (`EBGeometry_MeshDistanceFunctionsImplem.hpp`)

- [x] `buildFullBVH()`, `buildTriMeshFullBVH()` — `noexcept` removed
- [x] `FastMeshSDF` constructor, `FastCompactMeshSDF` constructor — `noexcept` removed
- [x] `FastMeshSDF::getClosestFaces()`, `FastCompactMeshSDF::getClosestFaces()` — `noexcept` removed
- [x] `MeshSDF::computeBoundingVolume()` — `noexcept` removed

### ImplicitFunction (`EBGeometry_ImplicitFunctionImplem.hpp`)

- [x] `approximateBoundingVolumeOctree()` — `noexcept` removed

---

## 3. Missing `noexcept` (add it)

- [x] `Vec2T<T>::Vec2T()` — added `noexcept`
- [x] `Vec2T<T>::Vec2T(const T&, const T&)` — added `noexcept`
- [x] `EdgeT<T,Meta>::EdgeT()`, `EdgeT(const VertexPtr&)`, `EdgeT(const Edge&)` — added `noexcept`
- [x] `EdgeIteratorT(Face&)`, `EdgeIteratorT(const Face&)` — added `noexcept`
- [x] `~ComplementIF()`, `~TranslateIF()`, `~RotateIF()`, `~OffsetIF()`, `~AnnularIF()` — added `noexcept` (was inconsistent with siblings)

---

## 4. Correctness: wrong Doxygen comment

- [x] Both `MeshT::signedDistance` overloads (`EBGeometry_DCEL_Mesh.hpp`) — updated
  `@return` from "negative inside" to "positive inside, negative outside (DCEL
  inward-normal convention)".

---

## 5. Robustness and error handling

- [x] **Parser `[[nodiscard]]`** — added to all 18 `read*` declarations and definitions.

- [ ] **`sanityCheck` return type** — currently returns `void` and prints to
  `std::cerr`. Callers cannot check programmatically whether the mesh is valid.
  Consider returning `bool` or a `size_t` warning count. (API-breaking change —
  defer to a separate PR.)

- [x] **`EBGEOMETRY_EXPECT` coverage** — added to `Vec3T::minDir`, `Vec3T::maxDir`,
  `Vec3T::operator[]` (×2), `Vec3T::unit()`.

---

## 6. Performance

- [x] **`getAllVertexCoordinates` — `reserve` before loop** — added
  `reserve(3)` in `FaceT::gatherVertices/gatherEdges/getAllVertexCoordinates`
  and `reserve(m_vertices.size())` in `MeshT::getAllVertexCoordinates`.

---

## 7. Warnings found in debug build (`-Wall -Wextra`)

- [x] `EBGeometry_TransformImplem.hpp:303–305` — `MollifyIF` constructor: loop
  variables `int i/j/k` compared against `const size_t a_numPoints`
  (`-Wsign-compare`). Fixed: changed loop variables to `size_t`.

---

## Open items

- `sanityCheck` return type (§5 above) — deferred, API-breaking.
