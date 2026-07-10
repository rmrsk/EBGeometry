Dev/GridNN — uniform-grid vs PointCloudBVH nearest-neighbor prototype
====================================================================

**Prototype only. Part of `Dev/`, delete before merge.** Answers the question: for
all-nearest-neighbor over a particle cloud, would a uniform spatial (hash) grid beat the
`PointCloudBVH` tree — circumventing the tree entirely?

Build and run:

    make          # or: g++ -std=c++17 -O3 -march=native -I../.. main.cpp -o GridNN.ex
    ./GridNN.ex

## What it does

Builds a 500k random cloud in the unit cube and, for every point, finds its nearest *other* point
three ways, all checked against a brute-force sample:

- **PointCloudBVH** — `bvh.nearestNeighbor(i)` in natural point order.
- **Uniform grid** — points counting-sorted into a dense CSR bucket array (`cellStart[]` +
  `pointIdx[]`) keyed by integer cell coordinates, then an **expanding-shell** query with an exact
  stopping rule: after searching every cell within Chebyshev radius `R`, any unsearched point is
  `>= R*h` away, so once the best distance found is `<= R*h` the search stops (no missed neighbors).
  Swept over several target occupancies (cell sizes).

## Representative result (500k, double, one machine)

```
Method                       Build(ms)  Query(us/pt)    vs brute  avg pts/cell
Brute force                         --       378            1.0x            --
PointCloudBVH                     76.8         0.55       687x            --
Grid (target 0.5/cell)            13.1         0.43       871x          0.50
Grid (target 1/cell)              11.1         0.40       939x          0.98
Grid (target 2/cell)               9.6         0.47       801x          2.00
Grid (target 4/cell)               8.9         0.63       602x          4.00
```

**The grid wins on this workload: ~7x faster to build and ~1.4x faster to query**, with the sweet
spot at ~1 point/cell. This is expected — for near-uniform density a uniform grid is close to
optimal: O(N) counting-sort build (no recursive partitioning, no BVH nodes) and O(1) query (a
handful of adjacent cells).

## The caveats that decide whether to actually switch

- **Density uniformity is the whole game.** The grid's cell size is global. On a *clustered* /
  multi-scale cloud (which EB/particle problems often are near refined boundaries) the fixed cell
  is simultaneously too coarse in dense regions (huge buckets → slow scans) and too fine in sparse
  ones (many empty shells before a hit). The BVH adapts its leaf size to local density and does not
  have this failure mode. A fair follow-up would re-run this on a deliberately clustered cloud.
- **Memory / unbounded domains.** The dense grid allocates `nCells ≈ N` offsets, fine here. But
  `nCells = bboxVolume / h^3`: a cloud that is sparse but spread over a large box blows that up.
  That is where true spatial *hashing* (an `unordered_map<cellId, bucket>` — same query logic, hash
  probe instead of array index) comes in; it bounds memory to O(N) at the cost of a slower constant
  factor, so it would land somewhere between the dense grid and the tree here.
- **Only k=1 is prototyped.** k>1 needs a k-best set carried through the shell expansion (stop when
  the k-th best is inside `R*h`); straightforward but not implemented here.
- **This is not the same query the tree also serves.** `PointCloudBVH` additionally does *external*
  closest-point queries (arbitrary points, not cloud members) and is usable as a primitive inside an
  outer BVH/CSG. The grid replaces only the self-kNN use case.

**Bottom line:** for a near-uniform cloud where all you need is the self-kNN graph, a uniform grid is
the faster, simpler tool and worth adopting for that path. Keep the tree for non-uniform clouds,
external queries, and composition into the larger BVH machinery.
