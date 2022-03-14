.. _Chap:BVH:

Bounding volume hierarchies
===========================

Basic concept
-------------

Bounding Volume Hierarchies (BVHs) are comparatively simple data structures that can accelerate closest-point searches by orders of magnitude.
BVHs are tree structures where the regular nodes are bounding volumes that enclose all geometric primitives (e.g. polygon faces) further down in the hierarchy.
This means that every node in a BVH is associated with a *bounding volume*.
The bounding volume can, in principle, be any type of volume. 
Moreover, there are two types of nodes in a BVH:

* **Regular nodes.** These do not contain any of the primitives/objects.
  They are also called interior nodes, and store references to their child nodes. 
* **Leaf nodes.** These lie at the bottom of the BVH tree and each of them contain a subset of the geometric primitives.

:numref:`Fig:TrianglesBVH` shows a concept of BVH partitioning of a set of triangles.
Here, :math:`P` is a regular node whose bounding volume encloses all geometric primitives in it's subtree.
It's bounding volume, an axis-aligned bounding box or AABB for short, is illustrated by a dashed rectangle.
The interior node :math:`P` stores references to the leaf nodes :math:`L` and :math:`R`.
As shown in :numref:`Fig:TrianglesBVH`, :math:`L` contains 5 triangles enclosed by another AABB.
The other child node :math:`R` contains 6 triangles that are also enclosed by an AABB.
Note that the bounding volume for :math:`P` encloses the bounding volumes of :math:`L` and :math:`R` and that the bounding volumes for :math:`L` and :math:`R` contain a small overlap. 

.. _Fig:TrianglesBVH:
.. figure:: /_static/TrianglesBVH.png
   :width: 480px
   :align: center

   Example of BVH partitioning for enclosing triangles. The regular node :math:`P` contains two leaf nodes :math:`L` and :math:`R` which contain the primitives (triangles).

There is no fundamental limitation to what type of primitives/objects can be enclosed in BVHs, which makes BVHs useful beyond triangulated data sets.
For example, analytic signed distance functions can also be embedded in BVHs, provided that we can construct a bounding volume that encloses the object.

.. note::
   
   EBGeometry limited to binary trees, but supports :math:`k` -ary trees where each regular node has :math:`k` children nodes. 

Construction
------------

BVHs have extremely flexible rules regarding their construction.
For example, the child nodes :math:`L` and :math:`R` in :numref:`Fig:TrianglesBVH` could be partitioned in any number of ways, with the only requirement being that each child node gets at least one triangle. 

Although the rules for BVH construction are highly flexible, performant BVHs are completely reliant on having balanced trees with the following heuristic properties:

* **Tight bounding volumes** that enclose the primitives as tightly as possible.
* **Minimal overlap** between the bounding volumes.
* **Balanced**, in the sense that the tree depth does not vary greatly through the tree, and there is approximately the same number of primitives in each leaf node. 

Construction of a BVH is usually done recursively, from top to bottom (so-called top-down construction).
Alternative construction methods also exist, but are not used in EBGeometry. 
In this case one can represent the BVH construction of a :math:`k` -ary tree is done through a single function:

.. math::
   :label: Partition
   
   \textrm{Partition}\left(\vec{O}\right): \vec{O} \rightarrow \left(\vec{O}_1, \vec{O}_2, \ldots, \vec{O}_k\right), 
   
where :math:`\vec{O}` is an input a list of objects/primitives, which is *partitioned* into :math:`k` new list of primitives.
Note that the lists :math:`\vec{O}_i` do not contain duplicates, there is a unique set of primitives associated in each new leaf node. 
Top-down construction can thus be illustrated as a recursive procedure:

.. code-block:: text

   topDownConstruction(Objects):
      partitionedObjects = Partition(Objects)

      forall p in partitionedObjects:
         child = insertChildNode(newObjects)

	 if(enoughPrimitives(child)):
	    child.topDownConstruction(child.objects)

In practice, the above procedure is supplemented by more sophisticated criteria for terminating the recursion, as well as routines for creating the bounding volumes around the newly inserted nodes. 
EBGeometry provides these by letting the top-down construction calls take polymorphic lambdas as arguments for partitioning, termination, and bounding volume construction. 

Signed distance function
------------------------

When computing the signed distance function to objects embedded in a BVH, one takes advantage of the hierarcical embedding of the primitives.
Consider the case in :numref:`Fig:TreePruning`, where the goal of the BVH traversal is to minimize the number of branches and nodes that are visited.
For the traversal algorithm we consider the following steps:

* When descending from node :math:`P` we determine that we first investigate the left subtree (node :math:`A`) since its bounding volume is closer than the bounding volumes for the other subtree.
  The other subtree will is investigated after we have recursed to the bottom of the :math:`A` subtree. 
* Since :math:`A` is a leaf node, we find the signed distance from :math:`\mathbf{x}` to the primitives in :math:`A`.
  This requires us to iterate over all the triangles in :math:`A`. 
* When moving back to :math:`P`, we find that the distance to the primitives in :math:`A` is shorter than the distance from :math:`\mathbf{x}` to the bounding volume that encloses nodes :math:`B` and :math:`C`.
  This immediately permits us to prune the entire subtree containing :math:`B` and :math:`C`.

.. _Fig:TreePruning:
.. figure:: /_static/TreePruning.png
   :width: 480px
   :align: center

   Example of BVH tree pruning.


.. warning::
   
   Note that all BVH traversal algorithms become inefficient when the primitives are all at approximately the same distance from the query point.
   For example, it is necessary to traverse almost the entire tree when one tries to compute the signed distance at the origin of a tesselated sphere.


