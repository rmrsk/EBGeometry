.. _Chap:Queries:

Queries
=======

The columns of the table in :ref:`Chap:Overview`: what can be asked of a piece of geometry, and what comes
back. Every query in this section runs on the host and, for geometry held in a memory pool or needing none,
on a device (see :ref:`Chap:RunningOnGPU`).

Distance and inside
-------------------

The first question is the value at a point: ``signedDistance(p)``, negative inside, positive outside and zero
on the surface (see :ref:`Chap:GeometryRepresentations`). Its sign says whether the point is inside; its size
says how far the surface is, as exactly as the shape's distance quality allows. Every shape answers it, and
it is the one query a shape of your own must answer.

A composition of several regions adds two: which region a point is in, and the distance to the surface of one
region (see :ref:`Chap:Regions`). A point cloud has no inside, so its distance is unsigned: how far the nearest
point is.

The value also gives the direction to the surface. Its gradient, the outward normal where the point is on the
surface, is taken from the value by finite differences for any function (:ref:`Chap:ImplemOctree`), and a
mesh's normals at its edges and corners are its pseudonormals (:ref:`Chap:Meshes`).

Closest
-------

The second question is which part of the geometry is closest: the face of a mesh, the sphere of a sphere
cloud, the k nearest points of a point cloud, or the leaf of a composition that decides its value at the
point. For a composition, the deciding leaf is not always the one whose surface is nearest, since the nearest
surface may be a part the composition removed. :ref:`Chap:LeavesAndSurfaces` explains which part decides,
and how a query drills into collections (a union of meshes, a sphere cloud inside a tape) down to the member,
its face and its label.

Rays
----

The third question is about a line instead of a point: where a ray, or a particle's step from one position to
the next, first crosses the surface, and whether it does at all. Every shape answers it, exactly for the
shapes that can be intersected with a line and by marching for the rest (:ref:`Chap:Rays`). A composition of
several regions also finds the first point where the region changes. A picture is many rays, one per pixel
(:ref:`Chap:Rendering`).

Neighbors
---------

The last question is about a discrete set: which members are near a given one. A point cloud finds each
point's k nearest other points, and a sphere cloud finds the spheres touching a sphere, or closer to its
surface than a gap (:ref:`Chap:PointCloud`). These are the neighbor lists and contact pairs of a particle
code.

One result, three names
-----------------------

Each family of queries returns its own plain result type, and several of them name the same idea:

.. list-table::
   :header-rows: 1
   :widths: 24 38 38

   * - Idea
     - Point queries
     - Ray queries
   * - Where, and how far
     - ``PointCloud::Hit``: a squared distance; the point itself is ``position(index)``
     - ``HitT``: the distance ``m_t`` along the ray and the point ``m_point``
   * - Which part
     - ``PointCloud::Hit::index``, the point's cloud index; ``ClosestFace::faceId``, a mesh face; ``LeafRef``,
       a composition's leaf
     - ``HitT::m_primitive``: the face, the sphere, the leaf or the member crossed
   * - Which surface, with its label
     - ``SurfaceRef``, from ``closestSurface()``
     - ``SurfaceHitT``: the ``HitT`` and the ``SurfaceRef`` of the surface crossed, from ``firstSurface()``

Every one is an index into something the caller holds: the positions a cloud was built from, the faces of the
mesh file, the leaves in the order the expression lists them. The library stores no user data with the
geometry; per-point, per-face or per-leaf data lives in the caller's own arrays, indexed by these numbers.
