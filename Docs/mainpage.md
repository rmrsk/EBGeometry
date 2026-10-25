## EBGeometry {#mainpage}

EBGeometry is a header-only C++17 library for

1. Turning watertight and orientable surface grids into signed distance functions (SDFs).
2. Fast evaluation of such grids using bounding volume hierarchies (BVHs).
3. Providing fast constructive solid geometry (CSG) unions using BVHs.

This page is the entry point for the **Doxygen API reference**, generated directly from the
`Source/` headers. For a narrative, example-driven introduction (installation, build-system
integration, the underlying geometric concepts), see the
[user documentation](https://rmrsk.github.io/EBGeometry/) instead.

### Where to start reading

* EBGeometry::Expr and EBGeometry::Tape — the expression builder that composes shapes (the analytic
  shapes such as EBGeometry::SphereSDF, the mesh distance fields such as EBGeometry::TriMeshSDF, or
  your own) with transforms and CSG operations, and the compiled, trivially copyable form that
  evaluates the result without virtual dispatch.
* EBGeometry::DCEL::MeshT — the half-edge (doubly-connected edge list) surface mesh
  representation, together with EBGeometry::DCEL::VertexT, EBGeometry::DCEL::EdgeT, and
  EBGeometry::DCEL::FaceT.
* EBGeometry::BVH::TreeBVH and EBGeometry::BVH::PackedBVH — the pointer-based build-time BVH and
  its flattened, SIMD-traversable counterpart.
* EBGeometry::MeshSDF, EBGeometry::TriMeshSDF, and EBGeometry::FlatMeshSDF — the concrete
  signed-distance-function classes that wrap a mesh in a BVH (or not, for `FlatMeshSDF`).
* EBGeometry::Parser — free functions for reading STL/PLY/OBJ/VTK files directly into a DCEL
  mesh, a BVH-backed SDF, or a triangle-only SIMD BVH.

### Namespace map

| Namespace | Contents |
|---|---|
| `EBGeometry` | Vectors (`Vec2T`, `Vec3T`), implicit functions, analytic shapes, CSG operators, transformations, the SDF/BVH wrapper classes (`MeshSDF`, `TriMeshSDF`, `FlatMeshSDF`), the BVH unions (`BVHUnion`, `BVHSmoothUnion`), the queries on any function of a point (`approximateBoundingVolumeOctree`, `normal`), and the ray queries on any shape (`RayT`, `HitT`, `firstHit`, `intersects`, `RayForm`) |
| `EBGeometry::DCEL` | The half-edge surface mesh: `VertexT`, `EdgeT`, `FaceT`, `MeshT`, and mesh iterators |
| `EBGeometry::BVH` | `TreeBVH`, `PackedBVH`, partitioners, traversal callback types (`LeafEvaluator`, `PrunePredicate`, `ChildOrderer`, `NodeKeyFactory`) |
| `EBGeometry::BoundingVolumes` | `AABBT` (axis-aligned box) |
| `EBGeometry::Random` | Random sampling utilities for the examples; not included by `EBGeometry.hpp` (include `Source/EBGeometry_Random.hpp`) |
| `EBGeometry::SFC` | Space-filling curves (`Morton`, `Nested`, `Hilbert`), point-to-grid binning (`computeBins`), and curve ordering of points (`order`), used for bottom-up BVH construction and spatial sorting |
| `EBGeometry::Soup` | Polygon-soup compression and soup-to-DCEL conversion |
| `EBGeometry::TriangleRay` | The triangle ray-crossing kernel, for one triangle or a SIMD pack of them |
| `EBGeometry::TriangleSoA` | Structure-of-arrays triangle groups used as SIMD-friendly `PackedBVH` leaves |
| `EBGeometry::PointSoA` | True structure-of-arrays point-position groups (`PointSoAT`), for nearest-neighbor-style `PackedBVH` leaves over point clouds |
| `EBGeometry::Parser` | File readers (STL, PLY, OBJ, VTK) that build polygon soups (`PolygonSoup`, with the files' vertex and face data), DCEL meshes, mesh SDFs, or triangle BVHs |

### Elsewhere

* Source repository: <https://github.com/rmrsk/EBGeometry>
* User documentation (HTML): <https://rmrsk.github.io/EBGeometry/>
* User documentation (PDF): <https://rmrsk.github.io/EBGeometry/ebgeometry.pdf>
