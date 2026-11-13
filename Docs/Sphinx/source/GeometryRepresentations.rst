.. _Chap:GeometryRepresentations:

Geometry as a function
======================

Everything EBGeometry holds, whether a sphere, a triangle mesh, a cloud of particles, or a composition of
thousands of them, is a function of a point: :math:`S(\mathbf{x})` says how far the point :math:`\mathbf{x}`
is from the object, and on which side. The pages of this section describe the kinds of geometry, how they
are composed, and the questions they answer:

* :ref:`Chap:Meshes`: triangle and polygon meshes as signed distance functions;
* :ref:`Chap:BVH`: the bounding volume hierarchies that make a mesh, or many objects, fast to query;
* :ref:`Chap:PointCloud`: points and spheres, and their nearest neighbors;
* :ref:`Chap:ConstructiveSolidGeometry` and :ref:`Chap:Tape`: composing objects, and compiling the
  composition into a program that runs on a CPU or a GPU;
* :ref:`Chap:LeavesAndSurfaces` and :ref:`Chap:Regions`: which part of an object a point or a surface
  belongs to, and objects made of several materials;
* :ref:`Chap:Rays`, :ref:`Chap:RayTracing`, :ref:`Chap:SphereTracing` and :ref:`Chap:Rendering`: where a
  ray or a particle's step meets the object, and pictures made that way.

Signed distance functions
-------------------------

A signed distance function :math:`S: \mathbb{R}^3 \rightarrow \mathbb{R}` gives the distance from a
point to the object's surface, signed by the side the point is on. EBGeometry uses one convention
everywhere:

.. math::

   S(\mathbf{x}) =
   \begin{cases}
   > 0, & \textrm{for points outside the object}, \\
   = 0, & \textrm{on its surface}, \\
   < 0, & \textrm{for points inside the object}.
   \end{cases}

A code that needs the opposite sign (one that reads a negative value as fluid, say) negates the value in its
own adapter.

Almost everywhere, a signed distance function has the property

.. math::
   :label: Eikonal

   \left|\nabla S(\mathbf{x})\right| = 1,

and its gradient is then the outward unit normal of the level set through :math:`\mathbf{x}`:

.. math::

   \mathbf{n} = \nabla S\left(\mathbf{x}\right).

The exceptions are the points equally far from two parts of the surface (inside a box, along its
diagonals), where the gradient jumps, and the surface's own edges and corners, where it has no single
value. A normal there is a choice, which each kind of geometry makes for itself (a mesh's pseudonormals,
see :ref:`Chap:Meshes`).

Implicit functions and distance quality
---------------------------------------

An *implicit function* only has the sign right: it is negative inside, positive outside and zero on the
surface, and its magnitude may mean anything. The ball of radius :math:`R` around :math:`\mathbf{x}_0` is
described by the signed distance

.. math::

   S_{\textrm{sph}}\left(\mathbf{x}\right) = \left|\mathbf{x} - \mathbf{x}_0\right| - R,

and equally by the implicit function

.. math::

   I_{\textrm{sph}}\left(\mathbf{x}\right) = \left|\mathbf{x} - \mathbf{x}_0\right|^2 - R^2,

whose value is not a distance. Many operations on signed distance functions (a union, an intersection,
a smooth blend) give back a function that is no longer an exact distance, but still never overestimates
one. EBGeometry therefore records, for every object, how far its value can be trusted: its *distance
quality*.

.. list-table::
   :header-rows: 1
   :widths: 20 80

   * - Distance quality
     - The value's magnitude is
   * - ``Exact``
     - the distance to the surface;
   * - ``Bound``
     - at most the distance to the surface: it may underestimate, never overestimate;
   * - ``NotADistance``
     - nothing: only the sign means something (Perlin noise, an implicit function of your own).

The quality decides what may be done with the value. A bound is enough to skip whatever lies farther away
than the value (the pruning of a BVH, :ref:`Chap:BVH`) and to step along a ray by the value without
stepping over the surface (:ref:`Chap:SphereTracing`); a function that is not a distance allows neither,
and is searched with fixed steps instead. A composition works out its own quality from its operands' when
it is built (:ref:`Chap:ConstructiveSolidGeometry`).

Shapes
------

A *shape* is anything with a signed value at a point and a bounding box: the analytic shapes (a sphere, a
box, a cylinder, ...), the mesh distance functions, the point and sphere clouds, a composition, and any type
of your own with the same two members (see :ref:`Chap:ExampleCustomShape`). Shapes are plain values that
can be copied as a block of memory, so that the same shape is evaluated on the host and, copied to a GPU,
on the device; those holding large data (a mesh, a BVH) keep it in a memory pool, which is what is copied
(see :ref:`Chap:MemoryModel`). Every shape can be composed with any other, placed in a BVH union of many
objects, ray traced, rendered, and asked which of its parts a point belongs to.

.. _Chap:Octree:

Bounding boxes of implicit functions
------------------------------------

A composition's bounding box is worked out from its operands' boxes, but a function of your own, or a
composition whose operations move the surface by amounts known only point by point, may have no box that
can be written down. EBGeometry then estimates one by subdividing space as an octree does: a box is split
into its eight octants, and an octant is kept only if the surface may pass through it. The value at an
octant's center rules that out when its magnitude is larger than the octant's half-diagonal, since a
distance cannot change faster than the point moves. The box around the octants kept at the finest level is
the estimate.

.. _Fig:Octree:
.. figure:: /_static/Octree.png
   :width: 70%
   :align: center

   Subdividing space in two dimensions (a quadtree): the domain :math:`D` is split into four
   sub-domains, one of which (:math:`D_4`) is split once more. The tree on the right is the
   partitioning on the left: each node is one region, and its children are the sub-regions it was split
   into.

The estimate is as tight as the finest octants, and only as safe as the value is a distance: a function
that changes faster than the point moves can hide a piece of surface in an octant it rules out, which a
safety factor on the test compensates for. See :ref:`Chap:ImplemOctree` for the call.
