.. _Chap:ImplemPointCloud:

Point clouds
============

EBGeometry provides two turnkey classes for nearest-neighbor and closest-point work over a point
cloud: ``PointCloudBVH`` (:file:`Source/EBGeometry_PointCloudBVH.hpp`) and ``PointCloudHashGrid``
(:file:`Source/EBGeometry_PointCloudHashGrid.hpp`). They answer the same queries and expose the
**same public interface** -- the same ``Hit`` result type, the same ``closestPoint`` /
``closestPoints`` / ``nearestNeighbor`` / ``nearestNeighbors`` / ``allNearestNeighbors`` query
methods, the same ``O(N)`` brute-force reference queries, and the same ``position()`` /
``metadata()`` accessors -- so the two are drop-in interchangeable. They differ only in the spatial
acceleration structure they build: a hierarchical tree, or a uniform grid. See :ref:`Chap:PointCloud`
for the conceptual picture and the trade-off between the two.

Both are built directly from a raw cloud -- point positions plus a parallel array of user metadata --
and both return the matched point's **cloud index** (its position in the input arrays) together with
the squared distance; the user metadata is reachable through ``metadata()``. ``closestPoint`` /
``closestPoints`` answer an arbitrary external query point, while ``nearestNeighbor`` /
``nearestNeighbors`` (and the batch ``allNearestNeighbors``) answer a point already in the cloud,
excluding it from its own result and seeding the search from the group it lives in -- a strictly
cheaper search an external point cannot use (see :ref:`Chap:PointCloud`).

Each accelerated query also has an ``O(N)`` brute-force counterpart -- ``closestPointBruteForce`` /
``closestPointsBruteForce`` / ``nearestNeighborBruteForce`` / ``nearestNeighborsBruteForce`` -- that
answers the same question by a full linear scan. These are reference implementations for testing and
debugging (verify an accelerated result against ground truth, or A/B-test a suspected bug against an
unaccelerated path) and are not meant for production queries.

PointCloudBVH
-------------

``PointCloudBVH`` specializes the :ref:`Chap:ImplemBVH` machinery: it is a subclass of
``BVH::PackedBVH`` with two things the general path does not offer -- a much cheaper **index-based
build**, and **turnkey query methods** that hide ``pruneTraverse()`` entirely. It is built by
partitioning an index permutation in place with a longest-axis midpoint split and packing the
``PointAoSoA`` leaves inline (no intermediate primitive list, no ``shared_ptr``, no separate packing
pass), which is several times faster to build than a full Surface-Area-Heuristic tree and, for
near-uniform clouds, just as tight to query. Because it is a BVH, it can also be composed as a
primitive inside an outer BVH or CSG tree.

See the `PointCloudBVH doxygen page
<doxygen/html/classEBGeometry_1_1PointCloudBVH.html>`__ for the full interface, and
``Examples/ClosestPointBVH`` / ``Examples/NearestNeighborBVH`` for worked usage.

PointCloudHashGrid
------------------

``PointCloudHashGrid`` circumvents the tree entirely and stores the cloud in a **uniform grid**.
Points are counting-sorted into a dense array of fixed-size cells (a CSR bucket array keyed by
integer cell coordinates) -- an ``O(N)`` build with no recursive partitioning and no tree nodes. A
query is an **expanding-shell** search outward from the query point's cell (Chebyshev radius
0, 1, 2, ...); it stops, exactly and without ever missing a neighbor, as soon as the k-th best
distance found is closer than any unvisited cell can hold. With the default cell size (~1 point/cell)
that is almost always one or two shells.

For a near-uniform cloud the grid both builds and queries faster than the BVH; for a strongly
clustered or multi-scale cloud a single global cell size is a poor fit and ``PointCloudBVH`` is the
better choice. The grid is also bounded-domain (dense cells sized to the bounding box, ``O(N)``
memory for a compact cloud) and serves only point queries; unlike ``PointCloudBVH`` it cannot be
composed as a primitive inside an outer BVH/CSG.

See the `PointCloudHashGrid doxygen page
<doxygen/html/classEBGeometry_1_1PointCloudHashGrid.html>`__ for the full interface, and
``Examples/NearestNeighborHashGrid`` for a worked comparison against the BVH example.
