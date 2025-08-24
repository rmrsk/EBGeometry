.. _Chap:Concepts:

Geometry representations
========================

Signed distance fields
----------------------

The signed distance function is defined as a function :math:`S: \mathbb{R}^3 \rightarrow \mathbb{R}`, and returns the *signed distance* to the object.
It has the additional property

.. math::
   :label: Eikonal

   \left|\nabla S(\mathbf{x})\right| = 1 \quad\textrm{everywhere}.

The normal vector is always

.. math::

   \mathbf{n} = \nabla S\left(\mathbf{x}\right).
   
``EBGeometry`` uses the following convention for the sign:

.. math::

   S(\mathbf{x}) =
   \begin{cases}
   > 0, & \textrm{for points outside the object}, \\
   < 0, & \textrm{for points inside the object},
   \end{cases}

which means that the normal vector :math:`\mathbf{n}` points away from the object. 

Implicit functions
------------------

Like distance functions, implicit functions also determine whether or not a point :math:`\mathbf{x}` is inside or outside an object.
Signed distance functions are also *implicit functions*, but not vice versa. 
For example, the signed distance function for a sphere with center :math:`\mathbf{x}_0` and radius :math:`R` can be written

.. math::

   S_{\textrm{sph}}\left(\mathbf{x}\right) = \left|\mathbf{x} - \mathbf{x}_0\right| - R.

An example of an implicit function for the same sphere is

.. math::
   
   I_{\textrm{sph}}\left(\mathbf{x}\right) = \left|\mathbf{x} - \mathbf{x}_0\right|^2 - R^2.

An important difference between these is the Eikonal property in :eq:`Eikonal`, ensuring that the signed distance function always returns the exact distance to the object.
Signed distance functions are more useful objects, but many operations (e.g. CSG unions) do not preserve the signed distance property (but still provide *bounds* for the signed distance).

.. _Chap:DCEL:

DCEL
====

Principle
---------

``EBGeometry`` uses a doubly-connected edge list (DCEL) structure for storing surface meshes.
The DCEL structures consist of the following objects:

* Planar polygons (facets).
* Half-edges.
* Vertices.

As shown in :numref:`Fig:DCEL`, half-edges circulate the inside of the facet, with pointer access to the next half-edge.
A half-edge also stores a reference to its starting vertex, as well as a reference to its pair-edge.
From the DCEL structure we can obtain all edges or vertices belonging to a single facet by iterating through the half-edges, and also jump to a neighboring facet by fetching the pair edge. 

.. _Fig:DCEL:
.. figure:: /_static/DCEL.png
   :width: 480px
   :align: center

   DCEL mesh structure. Each half-edge stores references to next half-edge, the pair edge, and the starting vertex.
   Vertices store a coordinate as well as a reference to one of the outgoing half-edges.

In ``EBGeometry`` the half-edge data structure is implemented in its own namespace.
This is a comparatively standard implementation of the DCEL structure, supplemented by functions that permit signed distance computations to various features on such a mesh.

.. important::

   A signed distance field requires a *watertight and orientable* surface mesh.
   If the surface mesh consists of holes or flipped facets, neither the signed distance or implicit function exist.

Signed distance
---------------

The signed distance to a surface mesh is equivalent to the signed distance to the closest polygon face in the mesh. 
When computing the signed distance from a point :math:`\mathbf{x}` to a polygon face (e.g., a triangle), the closest feature on the polygon can be one of the vertices, edges, or the interior of the polygon face, see :numref:`Fig:PolygonProjection`.

.. _Fig:PolygonProjection:
.. figure:: /_static/PolygonProjection.png
   :width: 240px
   :align: center

   Possible closest-feature cases after projecting a point :math:`\mathbf{x}` to the plane of a polygon face.

Three cases can be distinguished:

#. **Facet/Polygon face**.
   
   When computing the distance from a point :math:`\mathbf{x}` to the polygon face we first determine if the projection of :math:`\mathbf{x}` to the face plane lies inside or outside the face.
   This is more involved than one might think, and it is done by first computing the two-dimensional projection of the polygon face, ignoring one of the coordinates.
   Next, we determine, using 2D algorithms, if the projected point lies inside the embedded 2D representation of the polygon face. 
   Various algorithms for this are available, such as computing the winding number, the crossing number, or the subtended angle between the projected point and the 2D polygon.

   .. tip::
   
      ``EBGeometry`` uses the crossing number algorithm by default. 
      
   If the point projects to the inside of the face, the signed distance is just :math:`\mathbf{n}_f\cdot\left(\mathbf{x} - \mathbf{x}_f\right)` where :math:`\mathbf{n}_f` is the face normal and :math:`\mathbf{x}_f` is a point on the face plane (e.g., a vertex).
   If the point projects to *outside* the polygon face, the closest feature is either an edge or a vertex.
   
#. **Edge**.
   
   When computing the signed distance to an edge, the edge is parametrized as :math:`\mathbf{e}(t) = \mathbf{x}_0 + \left(\mathbf{x}_1 - \mathbf{x}_0\right)t`, where :math:`\mathbf{x}_0` and :math:`\mathbf{x}_1` are the starting and ending vertex coordinates.
   The point :math:`\mathbf{x}` is projected to this line, and if the projection yields :math:`t^\prime \in [0,1]` then the edge is the closest point.
   In that case the signed distance is the projected distance and the sign is given by the sign of :math:`\mathbf{n}_e\cdot\left(\mathbf{x} - \mathbf{x}_0\right)` where :math:`\mathbf{n}_e` is the pseudonormal vector of the edge. 
   Otherwise, the closest point is one of the vertices.

#. **Vertex**.

   If the closest point is a vertex then the signed distance is simply :math:`\mathbf{n}_v\cdot\left(\mathbf{x}-\mathbf{x}_v\right)` where :math:`\mathbf{n}_v` is the vertex pseudonormal and :math:`\mathbf{x}_v` is the vertex position.

.. _Chap:NormalDCEL:

Normal vectors
--------------

The normal vectors for edges :math:`\mathbf{n}_e` and vertices :math:`\mathbf{n}_v` are, unlike the facet normal, not uniquely defined.
For both edges and vertices we use the pseudonormals from :cite:`1407857`:

.. math::

   \mathbf{n}_{e} = \frac{1}{2}\left(\mathbf{n}_{f} + \mathbf{n}_{f^\prime}\right).

where :math:`f` and :math:`f^\prime` are the two faces connecting the edge.
The vertex pseudonormal is given by

.. math::

  \mathbf{n}_{v} = \frac{\sum_i\alpha_i\mathbf{n}_{f_i}}{\left|\sum_i\alpha_i\right|},

where the sum runs over all faces which share :math:`v` as a vertex, and where :math:`\alpha_i` is the subtended angle of the face :math:`f_i`, see :numref:`Fig:Pseudonormal`. 

.. _Fig:Pseudonormal:
.. figure:: /_static/Pseudonormal.png
   :width: 240px
   :align: center

   Edge and vertex pseudonormals.

Triangles
=========

``EBGeometry`` also supports using direct triangles rather than DCEL polygons.
In this case the user can parse his/her files and ask for triangle description rather than a DCEL description.
Internally, triangles are represented as *pseudo-triangles* that contain normal vectors on both the edges and vertices, completely similar to a DCEL polygon.

.. _Chap:BVH:

Bounding volume hierarchies
===========================

Bounding volume hierarchies (BVHs) are tree structures where the regular nodes are bounding volumes that enclose all geometric primitives (e.g. polygon faces or implicit functions) further down in the hierarchy.
This means that every node in a BVH is associated with a *bounding volume*.
The bounding volume can, in principle, be any type of volume. 
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
   :width: 480px
   :align: center

   Example of BVH partitioning for enclosing triangles. The regular node :math:`P` contains two leaf nodes :math:`L` and :math:`R` which contain the primitives (triangles).

There is no fundamental limitation to what type of primitives/objects can be enclosed in BVHs, which makes BVHs useful beyond triangulated data sets.
For example, analytic signed distance functions can also be embedded in BVHs, provided that we can construct bounding volumes that enclose them.

.. important::
   
   ``EBGeometry`` is not limited to binary trees, but supports :math:`k` -ary trees where each regular node has :math:`k` child nodes. 

Construction
------------

BVH construction is fairly flexible.
For example, the child nodes :math:`L` and :math:`R` in :numref:`Fig:TrianglesBVH` could be partitioned in any number of ways, with the only requirement being that each child node gets at least one triangle/primitive. 

Although the rules for BVH construction are highly flexible, performant BVHs are completely reliant on having balanced trees with the following heuristic properties:

* **Tight bounding volumes** that enclose the primitives as tightly as possible.
* **Minimal overlap** between the bounding volumes.
* **Balanced**, in the sense that the tree depth does not vary greatly through the tree, and there is approximately the same number of primitives in each leaf node. 

Construction of a BVH is usually done recursively, from top to bottom (so-called top-down construction).
Alternative construction methods also exist, but are not used in ``EBGeometry``. 
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

Bottom-up construction is also possible, in which case one constructs the leaf nodes first, and then merge the nodes upward until one reaches a root node.
In ``EBGeometry``, bottom-up construction is done by means of space-filling curves (e.g., Morton codes).

Tree traversal
--------------

When computing the signed distance function to objects embedded in a BVH, one takes advantage of the hierarchical embedding of the primitives.
Consider the case in :numref:`Fig:TreePruning`, where the goal of the BVH traversal is to minimize the number of branches and nodes that are visited.
For the traversal algorithm we consider the following steps:

* When descending from node :math:`P` we determine that we first investigate the left subtree (node :math:`A`) since its bounding volume is closer than the bounding volumes for the other subtree.
  The other subtree will is investigated after we have recursed to the bottom of the :math:`A` subtree. 
* Since :math:`A` is a leaf node, we compute the signed distance from :math:`\mathbf{x}` to the primitives in :math:`A`.
  This requires us to iterate over all the triangles in :math:`A`. 
* When investigating the other child node of :math:`P`, we find that the distance to the primitives in :math:`A` is shorter than the distance from :math:`\mathbf{x}` to the bounding volume that encloses nodes :math:`B` and :math:`C`.
  This immediately permits us to prune the entire subtree containing :math:`B` and :math:`C`.

.. _Fig:TreePruning:
.. figure:: /_static/TreePruning.png
   :width: 480px
   :align: center

   Example of BVH tree pruning.

.. warning::
   
   BVH traversal has :math:`\log N` complexity on average.
   However in the worst case the traversal algorithm may have linear complexity if the primitives are all at approximately the same distance from the query point.
   For example, it is necessary to traverse almost the entire tree when one tries to compute the signed distance at the origin of a tessellated sphere since all triangles and their bounding volumes are approximately at the same distance from the center.

Other types of tree traversal (that do not compute the signed distance) are also possible.
``EBGeometry`` supports a fairly flexible approach to the tree traversal and update algorithms such that the user is permitted to use the hierarhical traversal algorithm also for other types of operations (e.g., for finding all facets within a specified distance from a point).

Octree
======

Octrees are tree-structures where each interior node has exactly eight children.
Such trees are usually used for spatial partitioning.
Unlike BVH trees, the eight child nodes have no spatial overlap.

Octree construction can be done in (at least) two ways:

#. In depth-first order where entire sub-trees are built first.
#. In breadth-first order where tree levels are added one at a time.

``EBGeometry`` supports both of these methods. 
Octree traversal is generally speaking quite similar to the traversal algorithms used for BVH trees.

Constructive solid geometry
===========================

Basic transformations
---------------------

Implicit functions, and by extension also signed distance fields, can be manipulated using basic transformations (like rotations).
``EBGeometry`` supports many of these:

* Rotations.
* Translations.
* Surface offsets.
* Shell extraction.
* Mollification (e.g., smoothing)
* ... and others.

.. warning::
   
   Some of these operations preserve the signed distance property, and others do not.

Combining objects
-----------------

``EBGeometry`` supports standard operations in which implicit functions can be combined:

* Union.
* Intersection.
* Difference.

Some of these CSG operations also have smooth equivalents, i.e. for smoothing the transition between combined objects.
Fast CSG operations are also supported by ``EBGeometry``, e.g. the BVH-accelerated CSG union where one uses the BVH when searching for the relevant geometric primitive(s).
This functionality is motivated by the fact that a CSG union is normally implemented as :math:`\min\left(I_1, I_2, I_3, \ldots,I_N\right)`, which has :math:`\mathcal{O}\left(N\right)` complexity when there are :math:`N` objects.
BVH trees can reduce this to :math:`\mathcal{O}\left(\log N\right)` complexity.
