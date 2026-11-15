.. _Chap:ImplemBVHTraversal:

BVH traversal
=============

Every search through a BVH (a mesh's distance, a union's, a point cloud's nearest neighbors, a ray's
first hit) is one walk with different pieces slotted in. This page describes the walk, the pieces, and
how to write a search of your own; :ref:`Chap:ImplemBVH` describes building the BVH.

``TreeBVH`` and ``PackedBVH`` each have a ``traverse()`` member. ``TreeBVH::traverse()`` is a host-only
walk over the pointer tree, driven by four callbacks (a prune predicate, a child orderer, a leaf
evaluator and a node-key factory; see the `BVH namespace's doxygen page
<doxygen/html/namespaceEBGeometry_1_1BVH.html>`__), for querying a tree before it is packed; an
exception from one of its callbacks reaches the caller. ``PackedBVH::traverse()`` is the walk every query runs, on the host
and on a device, and the rest of this page is about it.

.. _Chap:PackedTraverse:

``PackedBVH::traverse()``
-------------------------

``PackedBVH`` has no ``signedDistance()`` of its own and does not privilege any one query. It has one
walk, ``traverse()``, and every search in the library -- the mesh SDFs' distances and closest faces, the
BVH unions, the point clouds' neighbor searches, the tape's large unions, the ray queries -- is a call of
it with different pieces slotted in. A new search is written the same way: you supply the pieces, the
library supplies the walk, on the host and on a device. See :ref:`Chap:BVH` for the idea behind it.

.. code-block:: c++

   bvh.traverse(query, state, leaf, bound);                 // the four pieces every search needs
   bvh.traverse(query, state, leaf, bound, visit, order);   // and the two optional ones

What the walk does, and where your pieces run
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The walk is depth first, with a small fixed stack instead of recursion. In pseudocode, with the places
where it calls your pieces marked:

.. code-block:: text

   key = query.key(root box)                                  ← query
   if key > bound(state): stop                                ← bound
   node = root
   loop:
       if visit(state, node, key) is false: skip node        ← visit (optional)
       else if node is a leaf:
           leaf(state, first primitive, count)                ← leaf: the only place primitives are read
       else:
           b    = bound(state)                                ← bound, read fresh at every node
           keys = query.keys(node's K child boxes)            ← query, all K children in one pass
           kept = the children with key <= b
           sort kept by order                                 ← order (optional; nearest first by default)
           descend into the first of kept now; push the rest of kept for later
           continue
       pop the next waiting child c of the deepest level:
           key = query.key(c's box)                           ← query, recomputed bit for bit
           if key > bound(state):                             ← bound, read fresh again
               skip c (and, nearest first, the rest of its level, which are no nearer)
           else: node = c, continue
       stop when nothing is waiting

.. _Fig:TraversalWalk:
.. figure:: /_static/TraversalWalk.png
   :width: 85%
   :align: center

   A nearest-primitive search walking a small tree (``K = 2``). The numbered events are where the walk
   calls the pieces the search supplies, colored by piece: the query keys the root with ``key(box)``
   and both children of a node in one pass with ``keys(row)`` (1--3), the order puts the nearer child
   first, the leaf callback finds a primitive and so tightens the bound (4), and on the way back the
   query's ``key(row, k)`` recomputes each waiting child's key for the bound as it is then, which
   prunes ``L2`` and the whole subtree under ``N2`` (5--6). The panel below shows ``visit`` refusing a
   node that passed the bound, after adding the caller's summary of its subtree to the state (7).

Three things follow from this shape:

* **The bound is read again at every node.** A leaf you visit early tightens the pruning of every node
  after it, in any part of the tree -- which is why visiting the most promising child first pays off.
* **The state is yours.** The walk only passes it to your callbacks; only ``leaf`` (and ``visit``, if you
  give one) change it, and ``bound`` reads it.
* **Keys are compared, never stored.** A child that waits on the stack gets its key again when the walk
  comes back to it, so the comparison sees the bound as it is *then*.

The pieces you supply
~~~~~~~~~~~~~~~~~~~~~

.. list-table::
   :header-rows: 1
   :widths: 13 30 22 35

   * - Piece
     - Signature
     - Called
     - What it is for
   * - ``query``
     - a query type (``BVH::PointQuery``, ``BVH::RayQuery``, or your own)
     - at every interior node (all ``K`` children), at the root, and when a waiting child is resumed
     - what is tracked through the tree: it gives each box a *key*, how far the box is from the query
   * - ``state``
     - any type, passed by reference
     - --
     - what the search remembers between leaves: the best value so far, the best primitive, a list of
       hits
   * - ``leaf``
     - ``(State&, size_t offset, size_t count)``
     - once per leaf reached
     - test the leaf's primitives, ``getPrimitives()[offset .. offset + count)``, and update the state
   * - ``bound``
     - ``(const State&) -> Key``
     - at every node expanded and every child resumed
     - how far a box may be and still be entered: a box whose key is above it is skipped with its
       whole subtree
   * - ``visit`` (optional)
     - ``(State&, uint32_t nodeIndex, const Node&, const Key&) -> bool``
     - for every node about to be entered (root, interior, leaf), after the bound check
     - decide on the node itself: return ``false`` to skip it and its subtree, after updating the
       state if you like (prune on your own per-node data, summarize a far subtree, count nodes).
       Default ``BVH::DescendAll``, which compiles away
   * - ``order`` (optional)
     - ``BVH::NearestFirst``, ``BVH::FarthestFirst``, ``BVH::AsStored``, or a comparator
       ``(keyA, keyB) -> bool`` or ``(keyA, nodeA, keyB, nodeB) -> bool``
     - when a node's kept children are sorted
     - which child to visit first. ``NearestFirst`` (default) tightens a nearest-type bound fastest;
       ``AsStored`` (first slot first, no sort) suits searches that visit everything they reach or stop
       at the first hit

The rules your pieces follow:

* **They are template parameters** -- lambdas or functors, not ``std::function`` -- so the same call runs
  in a kernel. On a device, write them as functors with ``EBGEOMETRY_HOST_DEVICE`` call operators, or as
  lambdas inside such a function. ``traverse()`` is ``noexcept``: an exception from a callback ends the
  program.
* **Under** ``NearestFirst`` **the bound may only tighten** during a walk (never grow), since the walk
  drops the rest of a level once one child is out of it. Every search that narrows in on a best answer
  does this naturally. A search whose bound can grow should pick another order.
* **To stop the walk**, make ``bound`` return :math:`-\infty` (below every key): an any-hit search does
  this after its first hit.
* **A key of** :math:`+\infty` **is never entered**, whatever the bound: a ray query gives it to a box the
  ray misses. A bound of :math:`+\infty` therefore means "no bound yet", and a ray search can start
  there.

Query types
~~~~~~~~~~~

* ``BVH::PointQuery{p}`` -- the key of a box is the squared distance from ``p`` to it (0 inside it). The
  bound is a squared distance too. In a function that also runs on a device, write the precision out,
  ``BVH::PointQuery<T>{p}``: HIP's compiler treats the deduction guide behind ``PointQuery{p}`` as a host
  function.
* ``BVH::RayQuery(origin, direction, tMin)`` -- the key of a box is the distance along the ray (unit
  direction) at which it enters the box, from ``tMin`` on, or :math:`+\infty` if it misses it. A segment
  from ``a`` to ``b`` is the ray from ``a`` toward ``b`` with the bound starting at ``|b - a|``.
* **Your own query type** -- anything with ``using Scalar = T``; ``keys(row)`` returning the ``K``
  children's keys from the node's row of child boxes (``BVH::ChildBoxes``: ``m_lo[axis][child]``,
  ``m_hi[axis][child]``, laid out for SIMD); ``key(row, k)`` returning one child's key, bit for bit what
  ``keys()`` gave; and ``key(box)`` for the root. It may declare ``using Key = ...`` for a key that is not
  a number, such as a distance with a tie-break (it needs ``<=``, ``>`` and, for the nearest/farthest
  orders, ``<``).

Recipes
~~~~~~~

The nearest primitive to a point (what the mesh SDFs and point clouds do):

.. code-block:: c++

   struct Best { T d2 = inf; uint32_t id = none; };
   Best best;

   bvh.traverse(BVH::PointQuery<T>{p}, best,
                [&](Best& b, size_t offset, size_t count) {          // distance to each primitive
                  for (size_t i = offset; i < offset + count; i++) {
                    const T d2 = distance2(prims[i], p);
                    if (d2 < b.d2) { b = Best{d2, uint32_t(i)}; }
                  }
                },
                [](const Best& b) { return b.d2; });                 // farther boxes cannot do better

The first primitive a ray hits: the same, with a ``RayQuery``, a ray--primitive test in ``leaf`` and the
nearest hit so far as the bound. Whether *anything* is hit (a shadow ray, a blocked segment): keep a flag,
return :math:`-\infty` from ``bound`` once it is set, and use ``AsStored``, since any hit will do:

.. code-block:: c++

   bool hit = false;

   bvh.traverse(BVH::RayQuery<T>(a, direction), hit,
                [&](bool& h, size_t offset, size_t count) { /* h = h || any primitive hit before |b - a| */ },
                [&](const bool& h) { return h ? -inf : length; },
                BVH::DescendAll{}, BVH::AsStored{});

Everything within a radius ``r`` of a point: collect in ``leaf``, return ``r * r`` from ``bound`` (it
never changes), and use ``AsStored``. A far-field approximation: in ``visit``, when a node is far enough
for its summary (kept in your own array, indexed by ``nodeIndex``), add the summary to the state and
return ``false``; otherwise return ``true`` and let the walk descend.

The :ref:`Chap:ExampleBVHTraversal` example runs the first three in full, on the host and on a device.
For the exact signatures, see the Doxygen reference for
`PackedBVH <doxygen/html/classEBGeometry_1_1BVH_1_1PackedBVH.html>`__ and the `BVH namespace
<doxygen/html/namespaceEBGeometry_1_1BVH.html>`__.

**SIMD and the same answers everywhere.** ``PointQuery`` computes the ``K`` squared distances in a
single SIMD batch where the ``(K, T)`` pair has a compiled path (see :ref:`Chap:SIMDClasses`), and
with a scalar loop otherwise, which is also what device code runs; ``RayQuery`` does the same for its
slab test. The paths agree bit for bit, so a query answered on a build with no SIMD returns what the
same query returns on an AVX-512 build or on a device. The one exception is a compiler that contracts
the scalar path's multiply and add into a fused multiply-add (Clang by default, and GCC in its GNU
language modes, whenever FMA is enabled, e.g. by ``-march=native``): ``PointQuery``'s paths can then
differ in the last bit. ``RayQuery`` is robust at the edges: a ray through a face or edge shared by
two boxes enters at least one of them, and a ray lying in a box's face plane is not counted as
missing it.

.. _Sec:TraversalStack:

The traversal stack
-------------------

The stack is a fixed-size array, with one entry per level of the tree, so the traversal allocates
nothing and runs unchanged on a device. That bounds the depth of a tree it can walk: 256 levels on
the host (``BVH::HostTraversalDepth``) and 32 on a device (``BVH::DeviceTraversalDepth``, smaller since
device stack memory is per thread), at every ``K``.

Every ``PackedBVH`` is checked against the host depth when it is built, and every device view
against the device depth when ``rebasedView()`` makes it, in every build: a tree too deep aborts with
a message giving its depth, rather than overflowing the stack, which in Release would be a silent
out-of-bounds write.

Splitting the bound from the leaf callback like this is what lets very different searches share one
walk: a nearest-neighbor search over a point cloud tracks a plain running squared distance as its state
(no ``abs()``, no square root in the hot path) with a bound that returns the state unchanged, whereas
``MeshSDF::signedDistance()`` (see :ref:`Chap:ImplemMeshSDF`) tracks a signed distance and squares its
magnitude for the bound (``TriMeshSDF`` tracks a squared unsigned distance on the host and signs the
winner at the end), and a closest-hit ray search tracks the nearest crossing -- all
ordinary calls of the same ``traverse()`` with a ``PointQuery`` or a ``RayQuery``, not special cases
in ``PackedBVH``. The first needs nothing more than a bare point struct with no ``signedDistance()``
member at all.
