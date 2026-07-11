.. _Chap:ImplemPointCloud:

Point clouds
============

For nearest-neighbor and closest-point work over particle clouds, EBGeometry provides
``PointCloudBVH`` (:file:`Source/EBGeometry_PointCloudBVH.hpp`). It specializes the :ref:`Chap:ImplemBVH`
machinery: it is a subclass of ``BVH::PackedBVH`` with two things the general path does not offer -- a
much cheaper **index-based build**, and **turnkey query methods** that hide ``pruneTraverse()``
entirely.

It is built directly from a raw cloud -- particle positions plus a parallel array of user metadata --
by partitioning an index permutation in place with a longest-axis midpoint split and packing the
``PointAoSoA`` leaves inline (no intermediate primitive list, no ``shared_ptr``, no separate packing
pass), which is several times faster to build than a full Surface-Area-Heuristic tree and, for
near-uniform clouds, just as tight to query.

Queries return the matched particle's cloud index (and squared distance); the user metadata is
reachable through ``metadata()``. ``closestPoint`` / ``closestPoints`` answer an arbitrary external
point, while ``nearestNeighbor`` / ``nearestNeighbors`` (and the batch ``allNearestNeighbors``) answer
a particle already in the cloud and additionally seed the search bound from that particle's own leaf
-- a strictly cheaper search an external point cannot use.

Each accelerated query also has an ``O(N)`` brute-force counterpart -- ``closestPointBruteForce`` /
``closestPointsBruteForce`` / ``nearestNeighborBruteForce`` / ``nearestNeighborsBruteForce`` -- that
answers the same question by a full linear scan. These are reference implementations for testing and
debugging (verify an accelerated result against ground truth, or A/B-test a suspected tree/traversal
bug against an unaccelerated path) and are not meant for production queries.

See the `PointCloudBVH doxygen page
<doxygen/html/classEBGeometry_1_1PointCloudBVH.html>`__ for the full interface, and
``Examples/ClosestPointBVH`` / ``Examples/NearestNeighborBVH`` for worked usage.
