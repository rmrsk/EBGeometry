.. _Chap:Octree:

Octree
=======

Octrees are tree-structures where each interior node has exactly eight children, each covering
one octant of the parent node's spatial region.
Such trees are usually used for spatial partitioning.

Unlike a bounding volume hierarchy (see :ref:`Chap:BVH`), an octree partitions *space* itself
rather than a set of primitives: a node's eight children always tile the parent's region exactly,
with no gaps and no overlap, regardless of what (if anything) is inside them. A BVH, by contrast,
partitions whatever primitives it is given, and its children's bounding volumes may legitimately
overlap.

A node is subdivided into its eight children only if some user-supplied *refinement criterion*
says the node needs to be resolved further; otherwise it remains a leaf. This makes an octree's
resolution adaptive: it can be made arbitrarily fine in regions that matter and left coarse
everywhere else, unlike a uniform grid over the same domain. :numref:`Fig:Octree` shows this
adaptivity in two dimensions (a quadtree, splitting each node into four children rather than
eight): the root is refined into four children, and one of those children is refined a second
time into its own four grandchildren, while the rest of the tree stops after one level.

.. _Fig:Octree:
.. figure:: /_static/Octree.png
   :width: 720px
   :align: center

   A 2D illustration of octree-style spatial partitioning (a quadtree): the domain :math:`D` is
   split into four sub-domains, one of which (:math:`D_4`) is refined once more into four
   further sub-domains. The tree on the right is the exact counterpart of the partitioning on
   the left: each node in the tree is one region in the partitioning, and a node's children are
   exactly the sub-regions it was split into.

Octree traversal is, generally speaking, quite similar to the traversal algorithms used for BVH
trees, see :ref:`Chap:BVH`: descending from a node visits some or all of its (up to eight)
children, in whatever order and subject to whatever pruning rule the traversal is customized
with, until the leaves relevant to the query at hand have been reached.
