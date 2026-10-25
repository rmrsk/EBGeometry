.. _Chap:ExampleRayTracing:

RayTracing
==========

Casting rays at a triangle mesh: where a ray first crosses the surface, with the face it crosses and the
normal there; whether a particle's step from one point to the next crosses it, and where; and the same
first crossing from a walk of the mesh's BVH written by hand, for when the ready-made queries do not
fit (see :ref:`Chap:Rays` and :ref:`Chap:ImplemRays`).

The source is at :file:`Examples/RayTracing/main.cpp`. See :ref:`Chap:Building` for how to compile it
with CMake, GNU Make, or a direct compiler invocation. It reads the armadillo from the
``common-3d-test-models`` submodule unless given another mesh file:

.. code-block:: bash

   cd Examples/RayTracing
   ./RayTracing.ex                 # or ./RayTracing.ex my_mesh.stl

The code that runs on either side
---------------------------------

``firstHit()`` and ``intersects()`` are called like ``signedDistance()``: from your own loop on the
host, or from your own functor in a kernel. ``CastTowards`` holds the mesh by value and casts one ray
from a point towards a target. Its direction is of unit length, so that ``t`` is a distance along the
ray, and it returns the whole ``HitT``, which is trivially copyable:

.. code-block:: c++

   return EBGeometry::firstHit(m_mesh, Ray{a_origin, d / d.length()}, T(0), EBGeometry::Math::Limits<T>::infinity());

``HandWritten`` finds the same crossing by walking the mesh's BVH itself, with
``PackedBVH::traverse()`` and a ``BVH::RayQuery`` (see :ref:`Chap:PackedTraverse`). The key of a box is
the distance along the ray at which the ray enters it; the state is the nearest crossing so far and its
face; the leaf callback asks each group of triangles where the ray crosses it
(``TriangleAoSoA::rayCrossing()``); and the bound is the nearest crossing, so boxes beyond it are
skipped. Writing the walk yourself is how to keep something the queries do not report, or to stop on a
condition of your own.

Building the mesh
-----------------

The mesh is read into a ``TriMeshSDF`` in a pool, which must outlive it:

.. code-block:: c++

   const Mesh mesh = EBGeometry::Parser::readIntoTriMeshSDF<T, K, W>(file, pool);

Using it on the host
--------------------

A ray from in front of the mesh towards it reports the distance to the crossing, the point, the face
(its index in the file), the face's outward normal, and whether the ray enters the object there. A
particle's step is a segment: ``intersects(mesh, from, to)`` says whether it crosses the surface,
stopping at the first crossing it finds, and ``firstHit(mesh, from, to)`` says where, with ``t`` the
distance from ``from``. A crossing at the start of a segment counts, so a particle that stopped on the
wall and moves on starts its next search a little past it, with ``tMin``; it then finds where it
leaves the object:

.. code-block:: c++

   EBGeometry::firstHit(mesh, wall.m_point, to, stepOff);

The hand-written walk then finds the same crossing and face as ``firstHit()``.

The same query on a GPU
-----------------------

The pool is frozen and mirrored to ``GPU::memoryResource()``, the mesh is rebased onto the copy, and
``GPU::transform()`` runs ``CastTowards`` once per origin in a kernel, one ray per thread. In a plain
C++ build the same lines run on the host; a GPU build without a visible GPU skips them.
