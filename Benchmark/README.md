Benchmark
=========

Reproducible micro-benchmarks that put EBGeometry's geometry queries next to other open-source
libraries on tasks they have in common, so a reader can see how the tradeoffs play out and re-run
the measurements on their own hardware. The point is the methodology and the cross-checks, not a
scoreboard — numbers vary widely across machines, compilers, and ISAs, so the tables below are
illustrative single-machine snapshots rather than definitive results.

The comparison libraries have their own build systems and release cadences, so they are not linked
into EBGeometry's normal test build. CI *compiles* each benchmark (a smoke test against bit-rot) but
does not *run* or time them (see issue #109). They are pinned as git submodules directly under
`Benchmark/`:

* [nanoflann](https://github.com/jlblancoc/nanoflann) — header-only KD-tree (point kNN)
* [picoflann](https://github.com/rmsalinas/picoflann) — tiny header-only KD-tree (point kNN)
* [fcpw](https://github.com/rohan-sawhney/fcpw) — closest-point / SDF on triangle meshes
* [TriangleMeshDistance](https://github.com/InteractiveComputerGraphics/TriangleMeshDistance) — header-only signed distance to triangle meshes

Fetch them (and the top-level mesh submodule) with:

```bash
git submodule update --init Benchmark/nanoflann Benchmark/picoflann Benchmark/fcpw \
                            Benchmark/TriangleMeshDistance common-3d-test-models
git -C Benchmark/fcpw submodule update --init deps/eigen   # fcpw's Eigen (skip the GPU slang-rhi dep)
```

fcpw's Enoki CPU-vectorization headers are vendored inside the fcpw submodule, so no extra fetch is
needed for it.

Each benchmark has a `GNUmakefile` (`make && ./<name>.ex`). Every result is cross-checked against a
brute-force / independent baseline so a wrong answer shows up as a mismatch — correctness is the part
that transfers across machines even when the timings do not.

`NearestNeighbor/` — all-nearest-neighbor on a point cloud
----------------------------------------------------------

For every point in a 500,000-point uniform cloud in the unit cube (double precision), find its
nearest *other* point. Compares `PointCloudBVH` vs picoflann vs nanoflann.

Representative result (one machine):

```
Method          Build(ms)   Query(us/pt)
PointCloudBVH        ~78         0.29
nanoflann           ~120         0.25
picoflann            ~65         0.47
```

- The KD-tree queries are iterated in **Hilbert order** so their node cache is as warm as
  `PointCloudBVH`'s leaf-order batch (querying in natural order is ~2x slower, which would not be a
  like-for-like comparison). That order costs the KD-trees a one-time ~0.18 us/pt spatial sort;
  `PointCloudBVH` reuses the ordering its build already produced. Folding the sort in:
  `PointCloudBVH` ~0.29 vs nanoflann ~0.43 vs picoflann ~0.65.
- On this machine nanoflann has the fastest raw per-query traversal, while `PointCloudBVH` comes out
  ahead on the end-to-end all-nearest-neighbor job because it gets the query order for free and builds
  ~1.5x faster. Which end of that tradeoff matters depends on the workload.

`MeshSDF/` — closest-point on a triangle mesh
---------------------------------------------

Closest-point queries against a triangle mesh (armadillo, ~100k triangles) — the task `TriMeshSDF`,
fcpw, and TriangleMeshDistance are all built for. The mesh is parsed once; each library then builds
its own structure over the same triangles and answers the same queries.

Representative result (one machine, 100k queries):

```
Method                  Build(ms)   Query(us/query)
TriMeshSDF                  ~63          2.6
fcpw (Enoki)                ~46          3.5
TriangleMeshDistance       ~115         10.5
```

- **fcpw** is built with its **Enoki CPU vectorization** (`FCPW_USE_ENOKI`, vectorized MBVH), the same
  SIMD fast path `TriMeshSDF` uses — a like-for-like float/SIMD comparison. (`FCPW_SIMD_WIDTH` in the
  `GNUmakefile` should match your ISA: 4=SSE, 8=AVX2, 16=AVX-512.) fcpw's `findClosestPoint` returns
  the *unsigned* closest point.
- **TriangleMeshDistance** is a header-only, **double-precision, scalar** (non-SIMD) signed-distance
  library — so its query runs in double and is not a same-precision comparison; it is included as a
  widely-used point of reference.
- `TriMeshSDF` and TriangleMeshDistance compute the *signed* distance (their purpose); the comparison
  is on unsigned closest-surface distance, and the cross-check confirms all three agree.
