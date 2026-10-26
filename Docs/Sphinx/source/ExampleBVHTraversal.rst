.. _Chap:ExampleBVHTraversal:

BVHTraversal
============

Walking a BVH with a search of your own, through the general
`PackedBVH::traverse() <doxygen/html/classEBGeometry_1_1BVH_1_1PackedBVH.html>`__ (see
:ref:`Chap:PackedTraverse`). The scene is 20,000 small balls, a primitive type the program defines
itself, and the program asks three questions: which ball surface is nearest to a point, which ball a
ray hits first, and whether the segment between two points is blocked. The library supplies the walk
and the query types; the program supplies the primitive, the state, the leaf test and the bound.

The source is at :file:`Examples/BVHTraversal/main.cpp`. See :ref:`Chap:Building` for how to compile
it with CMake, GNU Make, or a direct compiler invocation.

.. code-block:: bash

   cd Examples/BVHTraversal
   ./BVHTraversal.ex

The code that runs on either side
---------------------------------

A search is four things: a **query type** (what is tracked through the tree), a **state** (what is
remembered between leaves), a **leaf callback** (what happens at a leaf) and a **bound** (how near a
node must be to be entered). The two searches that also run on a GPU are written as functors with an
``EBGEOMETRY_HOST_DEVICE`` call operator, holding the BVH by value:

* ``NearestBall`` walks with a ``BVH::PointQuery``, whose key for a box is the squared distance from the
  point to it. The state is the nearest surface distance so far and its ball; the leaf callback
  measures the distance to each ball's surface; the bound is the state's distance, squared to match
  the key, so a box farther than the nearest surface found is skipped.
* ``FirstHit`` walks with a ``BVH::RayQuery``, whose key for a box is the distance along the ray at
  which it enters it; a box the ray misses is never entered. The state is the nearest hit so far; the
  leaf callback is a ray--sphere test the program writes itself; the bound is the nearest hit, so a box
  the ray enters beyond it is skipped. The default order, nearest first, finds a near hit early. It
  also passes a **visit callback**, which the walk calls for every node it enters, and which could
  refuse a node; here it only counts them.

.. code-block:: c++

   m_bvh.traverse(EBGeometry::BVH::RayQuery<T>(a_origin, m_direction), hit, leaf, bound, visit);

Building the BVH
----------------

A ``PackedBVH`` is built from ``(primitive, bounding box)`` pairs into a pool, which must outlive it.
The primitive can be any trivially copyable type; ``Ball`` has no member functions at all:

.. code-block:: c++

   EBGeometry::Pool pool(EBGeometry::hostMemoryResource());

   const BVH bvh = BVH::buildBottomUp(pool, balls, std::size_t(4));

Using it on the host
--------------------

The functors are called like functions. The ray's search reports how many of the BVH's nodes it
entered, a small fraction of them. ``blocked()`` asks whether anything lies between two points: a ray
along the segment, whose bound starts at the segment's length (nothing beyond the far end counts) and
drops to :math:`-\infty` at the first ball hit, which ends the walk. Since any hit will do, it visits
children in the order they are stored (``AsStored``), without sorting:

.. code-block:: c++

   a_bvh.traverse(EBGeometry::BVH::RayQuery<T>(a_from, direction), reach, leaf, bound,
                  EBGeometry::BVH::DescendAll{}, EBGeometry::BVH::AsStored{});

The same queries on a GPU
-------------------------

The pool is frozen and mirrored to ``GPU::memoryResource()``, the BVH is rebased onto the copy, and
``GPU::transform()`` runs ``NearestBall`` and ``FirstHit`` once per input in a kernel, one query per
thread. In a plain C++ build the same lines run on the host; a GPU build without a visible GPU skips
them.
