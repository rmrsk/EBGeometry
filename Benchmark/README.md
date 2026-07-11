Benchmark
=========

Head-to-head performance comparisons of EBGeometry against other open-source geometry-query
libraries. Like `Integrations/`, this is **illustrative and not part of CI** — it depends on
external libraries with their own build systems and release cadences (see issue #109).

The comparison libraries are pinned as git submodules under [`Submodules/`](../Submodules):

* [nanoflann](https://github.com/jlblancoc/nanoflann) — header-only KD-tree (point kNN)
* [picoflann](https://github.com/rmsalinas/picoflann) — tiny header-only KD-tree (point kNN)
* [fcpw](https://github.com/rohan-sawhney/fcpw) — closest-point / SDF on triangle meshes

Fetch them (and the mesh submodule) with:

```bash
git submodule update --init Submodules/nanoflann Submodules/picoflann Submodules/fcpw Submodules/common-3d-test-models
git -C Submodules/fcpw submodule update --init deps/eigen   # fcpw's Eigen (skip the GPU slang-rhi dep)
```

Each benchmark has a `GNUmakefile` (`make && ./<name>.ex`). Every result is cross-checked against a
brute-force / independent baseline so a wrong answer shows up as a mismatch.

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
  `PointCloudBVH`'s leaf-order batch (natural order is ~2x slower — unfair). That order costs the
  KD-trees a one-time ~0.18 us/pt spatial sort; `PointCloudBVH` reuses the ordering its build already
  produced, for free. Folding the sort in: `PointCloudBVH` ~0.29 vs nanoflann ~0.43 vs picoflann ~0.65.
- nanoflann has the fastest raw traversal; `PointCloudBVH` wins the end-to-end all-NN job (free query
  order + faster build) and builds ~1.5x faster than nanoflann.

`MeshSDF/` — closest-point on a triangle mesh
---------------------------------------------

Closest-point queries against a triangle mesh (armadillo, ~100k triangles, float) — the task fcpw and
EBGeometry's `TriMeshSDF` are both built for. The mesh is parsed once; each library then builds its
own BVH over the same triangles and answers the same queries.

Representative result (one machine, 100k queries):

```
Method        Build(ms)   Query(us/query)
TriMeshSDF        ~65          2.6
fcpw              ~55          7.5
```

- **Caveat:** fcpw here runs its **scalar Eigen fallback** (`FCPW_USE_ENOKI` off), while `TriMeshSDF`
  is SIMD-vectorized (SoA triangle leaves). So the query gap flatters EBGeometry — fcpw's intended
  fast path uses Enoki CPU vectorization, which would narrow it. Enabling Enoki is a heavier build
  (an extra dependency) and is left as a follow-up. Build times and correctness are directly
  comparable; the cross-check confirms the distances agree.
- `TriMeshSDF` computes the *signed* distance (its purpose); fcpw's `findClosestPoint` returns the
  unsigned closest point, so the comparison is on unsigned closest-surface distance.
