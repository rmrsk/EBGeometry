.. _Chap:ImplemPointCloud:

Point clouds
============

EBGeometry provides two turnkey classes for nearest-neighbor and closest-point work over a point
cloud: ``PointCloudBVH`` (:file:`Source/EBGeometry_PointCloudBVH.hpp`) and ``PointCloudHashGrid``
(:file:`Source/EBGeometry_PointCloudHashGrid.hpp`). They are interchangeable: both are built in a
``Pool`` from point positions alone, answer the same queries with the same methods --
``closestPoint`` / ``closestPoints`` / ``nearestNeighbor`` / ``nearestNeighbors`` /
``allNearestNeighbors`` -- return the same result type, and can be queried on a GPU. They differ in
the spatial acceleration structure they build: a hierarchical tree, or a uniform grid. See
:ref:`Chap:PointCloud` for the conceptual picture and the trade-off between the two.

Results and cloud indices
-------------------------

A point is identified by its **cloud index**, its position in the positions array the structure was
built from. A query returns a ``PointCloud::Hit<T>`` (:file:`Source/EBGeometry_PointCloud.hpp`, `doxygen
<doxygen/html/structEBGeometry_1_1PointCloud_1_1Hit.html>`__): the matched point's ``uint32_t``
cloud index and its squared distance. A query with no match (an empty cloud, or a self-query on a
single point) returns a ``Hit`` whose ``valid()`` is false, with index ``PointCloud::InvalidIndex``
and the largest ``T`` as its squared distance; the result slots a multi-result query cannot fill
hold the same value, and those queries also return the count found. A cloud may hold fewer than
``PointCloud::InvalidIndex`` points, all with finite coordinates; the constructors check both.

The classes store no per-point user data. Keep it in your own array and index it with
``Hit::index``, as ``Examples/ClosestPointBVH`` does (see :ref:`Chap:ExampleClosestPointBVH`).

``closestPoint`` / ``closestPoints`` answer an arbitrary external query point, while
``nearestNeighbor`` / ``nearestNeighbors`` (and the batch ``allNearestNeighbors``) answer a point
already in the cloud, excluding it from its own result. ``PointCloudBVH`` additionally seeds such a
search from the leaf the point lives in -- a strictly cheaper search an external point cannot use
(see :ref:`Chap:PointCloud`).

Both classes keep their candidates in a ``PointCloud::KBest`` set (`doxygen
<doxygen/html/classEBGeometry_1_1PointCloud_1_1KBest.html>`__): the k nearest found so far, sorted
in the caller's result buffer, with the k-th distance as the pruning bound. Each point is offered to
it once per query, so it needs no de-duplication.

Brute-force references
______________________

``PointCloud::closestPointsBruteForce()`` and ``PointCloud::closestPointBruteForce()`` answer the same
questions by a full linear scan over a positions array, with the same result contract; pass a cloud
index as the last argument to exclude that point, as a self-query does. They are reference
implementations for testing and debugging -- verify an accelerated result against ground truth, or
A/B-test a suspected bug against an unaccelerated path -- not for production queries. They are
callable on the host and on a device.

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

Each leaf lane of a ``PointAoSoA`` group carries its point's cloud index, and a leaf scan stops at
the group's real points (``numValid()``), so the padded lanes are never reported.

The held BVH and every cloud array (positions, the per-point seeding tables, the leaf order) are
reserved from the one ``Pool`` passed to the constructor, so a ``PointCloudBVH`` is trivially
copyable and crosses to a device the same way a ``PackedBVH`` does (see :ref:`Chap:MemoryModel`):
mirror the pool, call ``rebasedView()``, and pass the returned value into a kernel.
``rebasedView()`` and ``deepCopy()`` both return a ``PointCloudBVH``. On the device,
``closestPoint()``, ``closestPoints()``, ``nearestNeighbor()`` and ``nearestNeighbors()`` are
callable; ``allNearestNeighbors()`` returns a ``std::vector`` and stays host-only.

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
distance found is closer than any unvisited cell can hold. The bound is taken at the true cell
faces, less a few ulps for the rounding in locating a point's cell, so with the default cell size
(~1 point/cell) the search often stops after the query's own cell and its first shell.

Like ``PointCloudBVH``, the grid keeps its arrays (positions and the two CSR arrays) in the ``Pool``
passed to the constructor, is trivially copyable, and offers ``rebasedView()`` and ``deepCopy()``;
the same queries are callable on a device, and ``allNearestNeighbors()`` stays host-only.

For a near-uniform cloud the grid builds faster than the BVH and queries about as fast; for a
strongly clustered or multi-scale cloud a single global cell size is a poor fit -- a query in an
empty region visits many empty cells -- and ``PointCloudBVH`` is the better choice. The grid is also bounded-domain (dense cells sized to the bounding box, ``O(N)``
memory for a compact cloud) and serves only point queries. Neither it nor ``PointCloudBVH`` can be
composed as a primitive inside an outer BVH union or CSG: neither provides a ``signedDistance()``,
and neither is an ``ImplicitFunction``.

See the `PointCloudHashGrid doxygen page
<doxygen/html/classEBGeometry_1_1PointCloudHashGrid.html>`__ for the full interface, and
``Examples/NearestNeighborHashGrid`` for a worked comparison against the BVH example.
