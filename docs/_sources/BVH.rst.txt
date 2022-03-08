.. _Chap:BVH:

Bounding volume hierarchies
===========================

Basic concept
-------------

Bounding Volume Hierarchies (BVHs) are comparatively simple data structures that can accelerate closest-point searches by orders of magnitude.
BVHs are tree structures where the regular nodes are bounding volumes that enclose all geometric primitives (e.g. polygon faces) further down in the hierarchy.
There are two types of nodes in a BVH:

* *Regular nodes*, which do not contain any of the primitives/objects, but stores references to child nodes.
* *Leaf nodes*, which contain a subset of the primitives.

Both types of nodes contain a *bounding volume* which closes all geometric primitives in the subtree below the node.

:numref:`Fig:TrianglesBVH` shows a concept of BVH partitioning of a set of triangles.
Here, :math:`P` is a regular node whose bounding volume encloses all geometric primitives in it's subtree.

.. _Fig:TrianglesBVH:
.. figure:: /_static/TrianglesBVH.png
   :width: 480px
   :align: center

   Example of BVH partitioning for enclosing triangles. The regular node :math:`P` contains two leaf nodes :math:`L` and :math:`R` which contain the primitives (triangles).

There is no fundamental limitation to what type of primitives/objects can be enclosed in BVHs, which makes BVHs useful beyond triangulated data sets.
For example, analytic signed distance functions can also be embedded in BVHs, provided that we can construct a bounding volume that encloses the object.

.. note::
   
   BVHs are not limited to binary trees.
   EBGeometry supports :math:`k` -ary trees where each regular node has :math:`k` children nodes. 

Construction
------------

BVHs have extremely flexible rules regarding their construction.
Since they are hierarchical structures, the only requirement when dividing a leaf node is that each child node gets at least one primitive.

Although the rules for BVH construction are flexible, performant BVHs are completely reliant on having balanced trees with the following heuristic properties:

* Bounding volumes should enclose primitives as tightly as possible.
* There should be a minimal overlap between the bounding volumes.
* The BVH trees should be approximately *balanced*.

Construction of a BVH is usually done recursively, from top to bottom (so-called top-down construction).
Alternative construction methods also exist, but are not used in EBGeometry. 
In this case one can represent the BVH construction of a :math:`k` -ary tree is done through a single function:

.. math::

   \textrm{Partition}\left(\vec{O}\right): \vec{O} \rightarrow \left(\vec{O}_1, \vec{O}_2, \ldots, \vec{O}_k\right), 
   
where :math:`\vec{O}` is an input a list of objects/primitives, which is *partitioned* into :math:`k` new list of primitives.
Note that the lists :math:`\vec{O}_i` do not contain duplicates.
Top-down construction can thus be illustrated as a recursive procedure:

.. code-block:: text

   topDownConstruction(Objects):
      partitionedObjects = Partition(Objects)

      forall p in partitionedObjects:
         child = insertChildNode(newObjects)

	 if(enoughPrimitives(child)):
	    child.topDownConstruction(child.objects)

In practice, the above procedure is supplemented by more sophisticated criteria for terminating the recursion, as well as routines for creating the bounding volumes around the newly inserted nodes. 



Signed distance function
------------------------

When computing the signed distance function to objects embedded in a BVH, one takes advantage of the hierarcical embedding of the primitives.
Consider the case in :numref:`Fig:TreePruning`, where the goal of the BVH traversal is to minimize the number of branches and nodes that are visited.
We consider the following steps:

* When descending from node :math:`P` we determine that we first investigate the left subtree (node :math:`A`) since its bounding volume is closer than the bounding volumes for the other subtree.
  The other subtree will be investigated later. 
* Since :math:`A` is a leaf node, we find the signed distance from :math:`\mathbf{x}` to the primitives in :math:`A`.
* When moving back to :math:`P`, we find that the distance to the primitives in :math:`A` is shorter than the distance from :math:`\mathbf{x}` to the bounding volume that encloses nodes :math:`B` and :math:`C`.
  Consequently, the entire subtree containing :math:`B` and :math:`C` can be pruned.

.. _Fig:TreePruning:
.. figure:: /_static/TreePruning.png
   :width: 480px
   :align: center

   Example of BVH tree pruning. 
