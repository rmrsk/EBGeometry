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
:ref:`Chap:PointCloud` for the conceptual picture and the trade-off between the two. A third class,
``SphereCloudBVH`` (`below <#spherecloudbvh>`__), holds spheres of differing radii and answers for their surfaces
and for the contacts between them.

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
a much cheaper **build straight from the points**, and **turnkey query methods** that hide ``traverse()``
entirely. The tree is a *linear BVH*: the points are ordered along a Morton (Z-order) curve, and each node's
points are split where that order crosses from one half of space to the other. There is no intermediate
primitive list and no search for the best split, so the build is much faster than a full Surface-Area-Heuristic
tree, and for near-uniform clouds about as tight to query. The optional constructor argument sets the target
number of points per leaf. ``getBVH()`` exposes the held ``PackedBVH`` for anything that needs the general BVH
interface.

The held BVH and every cloud array are reserved from the one ``Pool`` passed to the constructor,
which must outlive the ``PointCloudBVH`` and every copy or view of it. A ``PointCloudBVH`` is
therefore trivially copyable and crosses to a device the same way a ``PackedBVH`` does (see :ref:`Chap:MemoryModel`):
mirror the pool, call ``rebasedView()``, and pass the returned value into a kernel.
``rebasedView()`` and ``deepCopy()`` both return a ``PointCloudBVH``. On the device,
``closestPoint()``, ``closestPoints()``, ``nearestNeighbor()`` and ``nearestNeighbors()`` are
callable; ``allNearestNeighbors()`` returns a ``std::vector`` and stays host-only.

Building where the BVH lives
____________________________

``PointCloudBVH::build()`` builds the BVH where its pool's memory is, exactly as
``PointCloudHashGrid::build()`` does (below): on the device for a pool in device memory, on the host
otherwise, from a *source* of positions, and the constructor is shorthand for it. Every side builds the same
tree, so a BVH built on a device and mirrored to the host answers there as one built on the host. A query
reports a point by its index in the source.

``rebuild()`` builds the tree again from new positions, in place, reserving nothing in the pool, so it works on
a frozen pool. It needs room, which ``build()`` reserves only when its last argument gives the most points a
rebuild may have: then the arrays are sized for any cloud of that many points, however clustered, which is larger
than a single build needs. Without it the BVH holds just what its cloud uses, and ``rebuild()`` is refused:

.. code-block:: c++

   EBGeometry::Pool pool(EBGeometry::GPU::memoryResource());

   auto bvh = PointCloudBVH<T>::build(pool, MySource{...}, n, 16 * W, maxPoints);   // once
   pool.freeze();

   bvh.rebuild(MySource{...}, n);                                                  // after the points move
   const auto view = bvh.rebasedView(pool);                                        // for the next kernel

A tree deeper than the traversal stack allows (``BVH::DeviceTraversalDepth`` levels on a device,
``BVH::HostTraversalDepth`` on the host) aborts the program: in ``build()`` or ``rebuild()`` for the side it runs
on, and in ``rebasedView()`` for a device. Only a cloud of many points packed far closer together than its extent
comes near that limit, but a code that rebuilds every step can meet it once its points cluster that far. A larger
target leaf size makes the tree shallower, and a build on the host allows a deeper one.

See the `PointCloudBVH doxygen page
<doxygen/html/classEBGeometry_1_1PointCloudBVH.html>`__ for the full interface, and
:ref:`Chap:ExamplePointClouds` for worked usage.

PointCloudHashGrid
------------------

``PointCloudHashGrid`` circumvents the tree entirely and stores the cloud in a **uniform grid**.
Points are sorted into a dense array of fixed-size cells: a build with no recursive partitioning and no tree
nodes, linear in the number of points on the host. A
query is an **expanding-shell** search outward from the query point's cell (Chebyshev radius
0, 1, 2, ...); it stops, exactly and without ever missing a neighbor, as soon as the k-th best
distance found is closer than any unvisited cell can hold. With the default cell size (about one
point per cell, set by the constructor's optional target points per cell) the search often stops
after the query's own cell and its first shell.

Like ``PointCloudBVH``, the grid keeps its arrays in the ``Pool`` passed to the constructor, which
must outlive it and every copy or view of it, is trivially copyable, and offers ``rebasedView()`` and ``deepCopy()``;
the same queries are callable on a device, and ``allNearestNeighbors()`` stays host-only.

Building where the grid lives
_____________________________

``PointCloudHashGrid::build()`` builds the grid where its pool's memory is. In device memory (a pool on
``GPU::memoryResource()`` in a CUDA or HIP build), the build runs on the device; in host memory, on the host.
Both give the same grid. The constructor is shorthand for ``build()`` from a ``std::vector`` of positions.

The positions come from a *source*: any trivially copyable struct whose call operator takes a point's index and
returns its ``Vec3T``, callable where the build runs. A source reads the points wherever and however the
program keeps them (one array per coordinate, a struct per particle, another precision), and
``PointCloud::FromArray`` reads a plain array of ``Vec3T``. The grid keeps its own copy of the positions, in cell
order, so a query reads a cell's points from consecutive memory. A query reports a point by its index in the
source, as always.

``rebuild()`` builds the grid again from new positions, in the arrays ``build()`` reserved, so it reserves and
copies nothing and works on a frozen pool; that suits points that move, such as particles. ``build()``'s last
argument gives the most points a rebuild may have. A view taken before a rebuild describes the old grid, so take
a new one after it:

.. code-block:: c++

   EBGeometry::Pool pool(EBGeometry::GPU::memoryResource());

   auto grid = PointCloudHashGrid<T>::build(pool, MySource{...}, n, T(1), maxPoints);   // once
   pool.freeze();

   grid.rebuild(MySource{...}, n);                                                     // after the points move
   const auto view = grid.rebasedView(pool);                                           // for the next kernel

:ref:`Chap:ExampleParticleNeighbors` does this every time step, with the particles on the device.

For a near-uniform cloud the grid builds faster than the BVH and queries about as fast; for a
strongly clustered or multi-scale cloud a single global cell size is a poor fit -- a query in an
empty region visits many empty cells -- and ``PointCloudBVH`` is the better choice. The grid is also bounded-domain (dense cells sized to the bounding box, ``O(N)``
memory for a compact cloud) and serves only point queries. Neither it nor ``PointCloudBVH`` can be
composed as a primitive inside an outer BVH union or a tape: neither provides a
``signedDistance()``.

See the `PointCloudHashGrid doxygen page
<doxygen/html/classEBGeometry_1_1PointCloudHashGrid.html>`__ for the full interface, and
:ref:`Chap:ExamplePointClouds`, which runs the same queries through both classes.

SphereCloudBVH
--------------

``SphereCloudBVH`` (:file:`Source/EBGeometry_SphereCloudBVH.hpp`) holds spheres of differing radii -- polydisperse
particles -- and answers questions about their surfaces rather than their centers: ``signedDistance(p)``, the
smallest :math:`|p - c| - r` over the spheres, and ``closestSphere(p)``, the sphere that gives it, as a
``SphereCloud::Hit<T>`` (`doxygen <doxygen/html/structEBGeometry_1_1SphereCloud_1_1Hit.html>`__): the sphere's
cloud index and the *signed* distance to its surface, negative inside it. Of two spheres at the same distance,
the one with the lower index is reported. The distance is exact outside the spheres; inside, where spheres overlap,
it is the depth below the deepest sphere's surface, a lower bound on the distance to the union's surface (as for a
CSG ``Union`` of the spheres). With ``computeBoundingVolume()`` it makes the cloud a shape, which the viewer
renders and a tape takes as a primitive (below). For spheres of one radius, ``PointCloudBVH`` over the centers
gives the same distance, less the radius.

The tree is ``PointCloudBVH``'s, over the spheres' centers, with every box grown to hold its spheres whole, and
everything said above of ``PointCloudBVH`` holds for it: the pool must outlive it, it crosses to a device through
``rebasedView()``, where its queries are callable, and ``build()`` and ``rebuild()`` run where the pool's memory
is. The spheres come from a *source*: ``SphereCloud::FromArray`` reads one array of ``SphereCloud::Sphere``
(a center and a radius), and ``SphereCloud::FromCentersAndRadii`` joins a source of centers to a source of radii,
for a program that keeps them apart (``PointCloud::FromArray`` and ``SphereCloud::FromRadii`` read plain
arrays). The host constructors take either form:

.. code-block:: c++

   const SphereCloudBVH<T> cloud(pool, centers, radii);   // or (pool, spheres)

   const T   d   = cloud.signedDistance(p);                // < 0 inside a sphere
   const Hit hit = cloud.closestSphere(p);                 // hit.index, hit.distance

A radius of 0 is a point; a negative or non-finite radius, or a non-finite center, is refused.

The *contacts* of sphere i are the spheres j whose separation, :math:`|c_i - c_j| - (r_i + r_j)`, is less than a
gap. With a gap of 0, the spheres that overlap it are its contacts; with a small positive gap, so are those that
touch or nearly touch it.
``forEachContact(i, gap, visit)`` calls ``visit`` once for each, with a ``SphereCloud::Hit<T>`` holding the other
sphere's index and the separation (negative when they overlap), in no set order. Contacts are mutual: j is a
contact of i exactly when i is one of j, at the same separation. A second form takes any sphere, in the cloud or
not, and an index to skip, to ask what a new particle would touch. Both are callable on a device, so a contact
force is one thread per particle adding up its own contacts, with no list and no atomics:

.. code-block:: c++

   cloud.forEachContact(i, gap, [&](const SphereCloud::Hit<T>& c) {
     // c.index: the other particle; c.distance: the separation
   });

A list of every particle's contacts is built from it in three steps: count each particle's contacts, add up the
counts into where each row starts (``GPU::exclusiveScan()`` on a device), and fill the rows;
:ref:`Chap:ExampleParticleContacts` builds one.

Rays cross the cloud exactly (see :ref:`Chap:ImplemRays`): ``firstHit()`` walks its BVH along the ray and reports
the sphere crossed by its cloud index, in ``HitT::m_primitive``. Where spheres overlap, a ray crosses only the
surface of their union, and a ray starting inside a sphere leaves where no sphere holds it any more. The viewer
traces the cloud the same way, on a device included.

The cloud takes part in CSG as a mesh SDF does (see :ref:`Chap:ImplemCSG`), traced exactly there too: as an
operand of the fused operations, or as a tape primitive, its type listed among the tape's primitives and the tape
built in the cloud's pool:

.. code-block:: c++

   const auto porous = Fused::Difference(cylinder, cloud);

   const Tape<T, SphereCloudBVH<T>> porousTape(pool, Difference(Expr<T>(cylinder), Expr<T>(cloud)));

See the `SphereCloudBVH doxygen page <doxygen/html/classEBGeometry_1_1SphereCloudBVH.html>`__ for the full
interface, and :ref:`Chap:ExampleSphereClouds` and :ref:`Chap:ExampleParticleContacts` for worked usage.
