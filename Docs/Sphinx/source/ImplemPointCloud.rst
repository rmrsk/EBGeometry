.. _Chap:ImplemPointCloud:

Point clouds
============

EBGeometry provides two turnkey classes for nearest-neighbor and closest-point work over a point
cloud: ``PointCloudBVH`` (:file:`Source/EBGeometry_PointCloudBVH.hpp`) and ``PointCloudHashGrid``
(:file:`Source/EBGeometry_PointCloudHashGrid.hpp`). They answer the same queries with the same
query methods -- ``closestPoint`` / ``closestPoints`` / ``nearestNeighbor`` / ``nearestNeighbors`` /
``allNearestNeighbors`` -- the same ``O(N)`` brute-force reference queries, and the same
``position()`` / ``metadata()`` accessors, and each returns its own ``Hit`` type of the same shape.
They are not drop-in interchangeable, though: ``PointCloudBVH`` is built in a ``Pool`` and can be
queried on a GPU, while ``PointCloudHashGrid`` owns ``std::vector`` storage and is host-only. They
differ in the spatial acceleration structure they build: a hierarchical tree, or a uniform grid. See
:ref:`Chap:PointCloud` for the conceptual picture and the trade-off between the two.

Both are built directly from a raw cloud -- point positions plus a parallel array of user metadata --
and both return the matched point's **cloud index** (its position in the input arrays) together with
the squared distance; the user metadata is reachable through ``metadata()``. ``closestPoint`` /
``closestPoints`` answer an arbitrary external query point, while ``nearestNeighbor`` /
``nearestNeighbors`` (and the batch ``allNearestNeighbors``) answer a point already in the cloud,
excluding it from its own result and seeding the search from the group it lives in -- a strictly
cheaper search an external point cannot use (see :ref:`Chap:PointCloud`). A query with no match (an
empty cloud, or a self-query on a single point) returns a ``Hit`` whose ``valid()`` is false: its
index is ``std::numeric_limits<std::size_t>::max()`` and its squared distance the largest ``T``.
Slots a multi-result query cannot fill hold the same value.

Each accelerated query also has an ``O(N)`` brute-force counterpart -- ``closestPointBruteForce`` /
``closestPointsBruteForce`` / ``nearestNeighborBruteForce`` / ``nearestNeighborsBruteForce`` -- that
answers the same question by a full linear scan. These are reference implementations for testing and
debugging (verify an accelerated result against ground truth, or A/B-test a suspected bug against an
unaccelerated path) and are not meant for production queries.

PointCloudBVH
-------------

``PointCloudBVH`` is built on the :ref:`Chap:ImplemBVH` machinery: it holds a ``BVH::PackedBVH``
over ``PointAoSoA`` leaf groups as a member, and adds two things the general path does not offer --
a much cheaper **index-based build**, and **turnkey query methods** that hide ``pruneTraverse()``
entirely. It is built by partitioning an index permutation in place with a longest-axis midpoint
split (falling back to a split by count where the midpoint cannot separate the points, as with
coincident points, so the tree stays shallow) and packing the ``PointAoSoA`` leaves inline (no intermediate primitive list, no
``shared_ptr``, no separate packing pass), which is several times faster to build than a full
Surface-Area-Heuristic tree and, for near-uniform clouds, just as tight to query. ``getBVH()`` exposes
the held ``PackedBVH`` for anything that needs the general BVH interface.

The held BVH and every cloud array (positions, user metadata, the per-point seeding tables, the leaf
order) are reserved from the one ``Pool`` passed to the constructor, so a ``PointCloudBVH`` is
trivially copyable and crosses to a device the same way a ``PackedBVH`` does (see
:ref:`Chap:MemoryModel`): mirror the pool, call ``rebasedView()``, and pass the returned value into a
kernel. ``rebasedView()`` and ``deepCopy()`` both return a ``PointCloudBVH``. On the device,
``closestPoint()``, ``closestPoints()``, ``nearestNeighbor()``, ``nearestNeighbors()`` and the
single-result brute-force references are callable; ``allNearestNeighbors()`` and the k-result
brute-force references return or allocate ``std::vector``\ s and stay host-only. Because the metadata
lives in pool memory, the ``Meta`` template argument must be trivially copyable.

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
memory for a compact cloud) and serves only point queries. Neither it nor ``PointCloudBVH`` can be
composed as a primitive inside an outer BVH union or CSG: neither provides a ``signedDistance()``,
and neither is an ``ImplicitFunction``.

See the `PointCloudHashGrid doxygen page
<doxygen/html/classEBGeometry_1_1PointCloudHashGrid.html>`__ for the full interface, and
``Examples/NearestNeighborHashGrid`` for a worked comparison against the BVH example.
