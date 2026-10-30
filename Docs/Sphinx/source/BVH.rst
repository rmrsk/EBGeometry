.. _Chap:BVH:

Bounding volume hierarchies
=============================

Bounding volume hierarchies (BVHs) are tree structures where the regular nodes are bounding volumes that enclose all geometric primitives (e.g. polygon faces or analytic shapes) further down in the hierarchy.
This means that every node in a BVH is associated with a *bounding volume*.
The bounding volume can, in principle, be any type of volume; EBGeometry uses axis-aligned
bounding boxes.
There are two types of nodes in a BVH:

* **Regular/interior nodes.** These do not contain any of the primitives/objects, but store references to subtrees (aka child nodes).
* **Leaf nodes.** These lie at the bottom of the BVH tree and each of them contains a subset of the geometric primitives.

:numref:`Fig:TrianglesBVH` shows a concept of BVH partitioning of a set of triangles.
Here, :math:`P` is a regular node whose bounding volume encloses all geometric primitives in its subtree.
Its bounding volume, an axis-aligned bounding box or AABB for short, is illustrated by a dashed rectangle.
The interior node :math:`P` stores references to the leaf nodes :math:`L` and :math:`R`.
As shown in :numref:`Fig:TrianglesBVH`, :math:`L` contains 5 triangles enclosed by another AABB.
The other child node :math:`R` contains 6 triangles that are also enclosed by an AABB.
Note that the bounding volume for :math:`P` encloses the bounding volumes of :math:`L` and :math:`R` and that the bounding volumes for :math:`L` and :math:`R` contain a small overlap.

.. _Fig:TrianglesBVH:
.. figure:: /_static/TrianglesBVH.png
   :width: 70%
   :align: center

   Example of BVH partitioning for enclosing triangles. The regular node :math:`P` contains two leaf nodes :math:`L` and :math:`R` which contain the primitives (triangles).

There is no fundamental limitation to what type of primitives/objects can be enclosed in BVHs, which makes BVHs useful beyond triangulated data sets.
For example, analytic signed distance functions can also be embedded in BVHs, provided that we can construct bounding volumes that enclose them.

.. important::

   EBGeometry is not limited to binary trees, but supports :math:`k`-ary trees, where each
   regular node has :math:`k` child nodes.

Construction
-------------

BVH construction is fairly flexible.
For example, the child nodes :math:`L` and :math:`R` in :numref:`Fig:TrianglesBVH` could be partitioned in any number of ways, with the only requirement being that each child node gets at least one triangle/primitive.

Although the rules for BVH construction are highly flexible, performant BVHs are completely reliant on having balanced trees with the following heuristic properties:

* **Tight bounding volumes** that enclose the primitives as tightly as possible.
* **Minimal overlap** between the bounding volumes.
* **Balanced**, in the sense that the tree depth does not vary greatly through the tree, and there is approximately the same number of primitives in each leaf node.

Construction of a BVH is often done recursively, from top to bottom (so-called top-down construction).
In this case, the BVH construction of a :math:`k`-ary tree can be represented through a single function:

.. math::
   :label: Partition

   \textrm{Partition}\left(\vec{O}\right): \vec{O} \rightarrow \left(\vec{O}_1, \vec{O}_2, \ldots, \vec{O}_k\right),

where :math:`\vec{O}` is the input list of objects/primitives, which is *partitioned* into :math:`k` new lists of primitives.
Note that the lists :math:`\vec{O}_i` do not contain duplicates; there is a unique set of primitives associated with each new leaf node.
Top-down construction can thus be illustrated as a recursive procedure:

.. code-block:: text

   topDownConstruction(Objects):
      partitionedObjects = Partition(Objects)

      forall p in partitionedObjects:
         child = insertChildNode(newObjects)

         if(enoughPrimitives(child)):
            child.topDownConstruction(child.objects)

In practice, the above procedure is supplemented by more sophisticated criteria for terminating the recursion, as well as routines for creating the bounding volumes around the newly inserted nodes.
EBGeometry implements top-down construction using a user-supplied partitioning rule (the
:math:`\mathrm{Partition}` above) and a termination criterion. Several ready-made partitioning
strategies are provided, splitting at the median bounding-volume centroid, at the spatial midpoint,
or on primitive centroids; a more expensive strategy based on the Surface Area Heuristic typically
yields the best query performance, at a higher construction cost, and is the recommended default.

Bottom-up construction is also possible, in which case one constructs the leaf nodes first, and then merges the nodes upward until one reaches a root node.
In EBGeometry, bottom-up construction is done by means of space-filling curves -- Morton codes,
Hilbert curves, or nested indices. The Hilbert curve has better spatial locality than Morton (its
consecutive codes are always spatially adjacent), so it tends to produce tighter leaf groupings.

How many primitives a leaf holds is a tuning choice that trades the cost of visiting more nodes
against the cost of testing more primitives per leaf. Its natural form depends on the construction:
a top-down build stops splitting at a maximum leaf size, while a bottom-up build picks the number of
leaves from a target leaf size.

.. important::

   BVHs do not need to be stored with pointer referencing between nodes; it is quite possible to pack nodes tightly in memory on a linear array with direct indexing. This is useful when
   applying SIMD instructions for querying multiple bounding volumes simultaneously.

.. _Fig:CompactBVH:
.. figure:: /_static/CompactBVH.png
   :width: 25%
   :align: center

   A pointer-based tree (top) and an equivalent, flattened representation of the same tree
   (bottom). The tree is traversed top-to-bottom along its branches and laid out in that same
   order in a linear array; each interior node then stores index *offsets* to its children
   within the array, rather than pointers.

Refitting
---------

Once a BVH has been built, its tree *topology* -- which primitive lives in which leaf, and how the
nodes nest -- is independent of where the primitives actually sit in space. If a geometry *moves*
slightly (a rigid displacement, or a small per-primitive jitter between frames) the topology is
still perfectly usable; only the bounding volumes have gone stale. *Refitting* exploits this: it
walks the tree from the leaves up, recomputing each leaf's bounding volume from its (now moved)
primitives and each interior node's volume as the union of its children's, without re-partitioning
anything. This is far cheaper than a full rebuild and is the standard way to keep a BVH valid for a
moving geometry.

The trade-off is quality rather than correctness: a refitted tree always *encloses* its primitives
(so traversal stays correct), but because the partitioning is frozen, a geometry that deforms enough
for primitives to drift into what used to be a neighbour's region produces increasingly loose,
overlapping bounding volumes and slower queries. A periodic rebuild restores tight volumes once the
deformation has grown large. See :ref:`Chap:ImplemBVH` for the concrete ``refit()`` member functions
on both BVH representations.

Tree traversal
---------------

When computing the signed distance function to objects embedded in a BVH, one takes advantage of the hierarchical embedding of the primitives.
Consider the case in :numref:`Fig:TreePruning`, where the goal of the BVH traversal is to minimize the number of branches and nodes that are visited.
For the traversal algorithm we consider the following steps:

* When descending from node :math:`P` we determine that we first investigate the left subtree (node :math:`A`) since its bounding volume is closer than the bounding volumes for the other subtree.
  The other subtree is investigated after we have recursed to the bottom of the :math:`A` subtree.
* Since :math:`A` is a leaf node, we compute the signed distance from :math:`\mathbf{x}` to the primitives in :math:`A`.
  This requires us to iterate over all the triangles in :math:`A` -- it is often beneficial to use an SoA layout for the triangles so this calculation can be vectorized.
* When investigating the other child node of :math:`P`, we find that the distance to the primitives in :math:`A` is shorter than the distance from :math:`\mathbf{x}` to
  the bounding volume that encloses nodes :math:`B` and :math:`C`. This immediately permits us to prune the entire subtree containing :math:`B` and :math:`C`, and there is
  never any need to compute the distance between :math:`\mathbf{x}` and the triangles in this subtree.

.. _Fig:TreePruning:
.. figure:: /_static/TreePruning.png
   :width: 70%
   :align: center

   Example of BVH tree pruning.

One traversal for many searches
_______________________________

The example above computes a signed distance, but nothing in the walk is specific to distances. Every
search in EBGeometry -- the distance to a mesh, the nearest point of a cloud, the first surface a ray
hits, whether a particle's path from one point to another crosses anything -- walks the tree the same
way, and differs only in a few pieces that the search supplies:

* **A query**: what is tracked through the tree. It assigns every bounding volume a *key*, a measure of
  how far the volume is from the query. For a point, the key is the distance to the box; for a ray, the
  distance along the ray at which it enters the box. A box the ray misses gets an infinite key and is
  never entered.
* **A state**: what the search has found so far, carried through the whole walk -- the nearest distance
  and the primitive it came from, the nearest hit along a ray, or a list of everything found.
* **A leaf test**: what happens when the walk reaches a leaf. Here, and only here, the primitives
  themselves are examined, and the state is updated.
* **A bound**: read from the state, it says how far a volume may be and still be worth entering. For a
  nearest-type search it is the best value found so far; for a ray, the nearest hit so far; for a
  segment, its length.

The walk itself is always the same. Starting at the root, it enters only the volumes whose key is
within the bound, nearest first unless the search asks otherwise, and runs the leaf test at every leaf
it reaches. Because the bound is read again every time the walk comes back to a waiting child, a child
that was worth visiting when its level was first reached may no longer be, once something nearer has
been found. The walk stops when nothing within the bound is left, or as soon as the bound drops below
every possible key (an any-hit search does this at its first hit).

The pruning in the example above is exactly this: the subtree holding :math:`B` and :math:`C` is never
entered because, by the time the walk comes back for it, the bound from :math:`A`'s triangles is smaller
than its key.

Two optional pieces widen what a search can do. A search can choose the **order** in which a node's
children are visited -- nearest first, farthest first, as stored, or by its own rule -- and it can
**decide on each node before entering it**, on information of its own: skip a subtree that is flagged,
too deep or irrelevant, or account for a far subtree as a whole from a summary it keeps per node (as
far-field approximations and levels of detail do) instead of descending into it.

The walk allocates nothing and runs unchanged on a GPU, one search per thread.

.. warning::

   BVH traversal has :math:`\log N` complexity on average.
   However, in the worst case the traversal algorithm may have linear complexity if the primitives are all at approximately the same distance from the query point.
   For example, it is necessary to traverse almost the entire tree when one tries to compute the signed distance at the origin of a tessellated sphere since all triangles and their bounding volumes are approximately at the same distance from the center.
