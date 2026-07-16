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
* [kd3](https://github.com/KaruroChori/kd3) — header-only SoA/SIMD KD-tree (point kNN), **requires C++23**
* [fcpw](https://github.com/rohan-sawhney/fcpw) — closest-point / SDF on triangle meshes
* [TriangleMeshDistance](https://github.com/InteractiveComputerGraphics/TriangleMeshDistance) — header-only signed distance to triangle meshes

Fetch them (and the top-level mesh submodule) with:

```bash
git submodule update --init Benchmark/nanoflann Benchmark/picoflann Benchmark/kd3 Benchmark/fcpw \
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

For every point in a 500,000-point cloud (double precision), find its nearest *other* point. Compares
EBGeometry's `PointCloudBVH` and `PointCloudHashGrid` against picoflann, nanoflann, and kd3. The same
comparison is run over two point distributions, since spatial structures behave very differently
depending on how the points fill space:

* **Uniform in the unit cube** — points fill a 3D volume evenly (the balanced, easy case).
* **On the unit-sphere surface** — points lie on a 2D manifold: locally dense, globally hollow. This
  is the harder case for a uniform grid, whose bounding box is then mostly empty interior cells.

Representative result (one machine, illustrative — see the note on machine dependence above):

```
                        uniform cube            sphere surface
Method               Build(ms)  Query(us/pt)  Build(ms)  Query(us/pt)
PointCloudBVH           ~118        0.62         ~105        0.48
PointCloudHashGrid       ~12        1.50          ~11        1.60
picoflann               ~104        0.75         ~104        0.48
nanoflann               ~225        0.41         ~230        0.36
kd3                      ~83        0.57          ~86        0.50
```

- The KD-tree queries are iterated in **Hilbert order** so their node cache is as warm as
  EBGeometry's leaf-order batch (querying in natural order is ~2x slower, which would not be a
  like-for-like comparison). That order costs the KD-trees a one-time spatial sort (~0.35 us/pt here);
  the EBGeometry structures reuse the ordering their build already produced.
- **`PointCloudHashGrid` trades query speed for build speed**: an O(N) uniform grid builds ~8x faster
  than the BVH but scans neighbor cells per query, so it queries ~2x slower. It is the weakest on the
  sphere surface (query ~1.6 us/pt) — the hollow distribution leaves its grid mostly empty while the
  occupied surface cells are denser than the ~1-point-per-cell target.
- The tree/BVH methods, by contrast, get *faster* on the sphere surface than in the cube (the local
  neighborhood is effectively lower-dimensional, so pruning is tighter). nanoflann has the fastest
  raw per-query traversal throughout; `PointCloudBVH` is competitive end-to-end because it gets its
  query order for free and builds faster. Which structure to pick depends on the build/query balance
  and the point distribution — that is the point of running both cases.
- **kd3's headline "~2.2x query throughput vs nanoflann" is a single-threaded `float` result** — in
  kd3's own benchmark both query loops are plain serial loops, so the speedup is its SoA/SIMD
  per-query kernel, *not* multithreading. Here kd3 is run in **double**, which halves that SIMD width;
  that is the main reason it doesn't reach 2x in this table, and it's why these numbers are a lower
  bound on what kd3 can do. It is compiled **without `-fopenmp`**, so its build is single-threaded
  like the others — a *fairer* build comparison than kd3's own docs, whose fast build number is
  OpenMP-parallel (its author notes it is still faster serially). In this fair single-threaded mode
  kd3 posts the fastest build and a competitive query. (kd3 needs C++23, so the benchmark is built
  with `-std=c++23`.)

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
