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
``Hit::index``, as :ref:`Chap:ExamplePointClouds` does.

``closestPoint`` / ``closestPoints`` answer an arbitrary external query point, while
``nearestNeighbor`` / ``nearestNeighbors`` (and the batch ``allNearestNeighbors``) answer a point
already in the cloud, excluding it from its own result. ``PointCloudBVH`` additionally seeds such a
search from the leaf the point lives in -- a strictly cheaper search an external point cannot use
(see :ref:`Chap:PointCloud`).

Both classes keep their candidates in a ``PointCloud::KBest`` set (`doxygen
<doxygen/html/classEBGeometry_1_1PointCloud_1_1KBest.html>`__): the k nearest found so far, sorted
in the caller's result buffer, with the k-th distance as the pruning bound.

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
a much cheaper **index-based build**, and **turnkey query methods** that hide ``traverse()``
entirely. The build splits the points directly, with no intermediate primitive list, and is much
faster than a full Surface-Area-Heuristic tree while, for near-uniform clouds, just as tight to
query. The optional constructor argument sets the target number of points per leaf. ``getBVH()``
exposes the held ``PackedBVH`` for anything that needs the general BVH interface.

The held BVH and every cloud array are reserved from the one ``Pool`` passed to the constructor,
which must outlive the ``PointCloudBVH`` and every copy or view of it. A ``PointCloudBVH`` is
therefore trivially copyable and crosses to a device the same way a ``PackedBVH`` does (see :ref:`Chap:MemoryModel`):
mirror the pool, call ``rebasedView()``, and pass the returned value into a kernel.
``rebasedView()`` and ``deepCopy()`` both return a ``PointCloudBVH``. On the device,
``closestPoint()``, ``closestPoints()``, ``nearestNeighbor()`` and ``nearestNeighbors()`` are
callable; ``allNearestNeighbors()`` returns a ``std::vector`` and stays host-only.

See the `PointCloudBVH doxygen page
<doxygen/html/classEBGeometry_1_1PointCloudBVH.html>`__ for the full interface, and
:ref:`Chap:ExamplePointClouds` for worked usage.

PointCloudHashGrid
------------------

``PointCloudHashGrid`` circumvents the tree entirely and stores the cloud in a **uniform grid**.
Points are counting-sorted into a dense array of fixed-size cells -- an ``O(N)`` build with no recursive partitioning and no tree nodes. A
query is an **expanding-shell** search outward from the query point's cell (Chebyshev radius
0, 1, 2, ...); it stops, exactly and without ever missing a neighbor, as soon as the k-th best
distance found is closer than any unvisited cell can hold. With the default cell size (about one
point per cell, set by the constructor's optional target points per cell) the search often stops
after the query's own cell and its first shell.

Like ``PointCloudBVH``, the grid keeps its arrays in the ``Pool`` passed to the constructor, which
must outlive it and every copy or view of it, is trivially copyable, and offers ``rebasedView()`` and ``deepCopy()``;
the same queries are callable on a device, and ``allNearestNeighbors()`` stays host-only.

For a near-uniform cloud the grid builds faster than the BVH and queries about as fast; for a
strongly clustered or multi-scale cloud a single global cell size is a poor fit -- a query in an
empty region visits many empty cells -- and ``PointCloudBVH`` is the better choice. The grid is also bounded-domain (dense cells sized to the bounding box, ``O(N)``
memory for a compact cloud) and serves only point queries. Neither it nor ``PointCloudBVH`` can be
composed as a primitive inside an outer BVH union or a tape: neither provides a
``signedDistance()``.

See the `PointCloudHashGrid doxygen page
<doxygen/html/classEBGeometry_1_1PointCloudHashGrid.html>`__ for the full interface, and
:ref:`Chap:ExamplePointClouds`, which runs the same queries through both classes.
