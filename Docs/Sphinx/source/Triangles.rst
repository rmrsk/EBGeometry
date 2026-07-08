.. _Chap:Triangles:

Triangle meshes
================

EBGeometry also supports representing a mesh directly as a collection of self-contained
triangles, without storing the half-edge topology described in :ref:`Chap:DCEL`.
Triangles in EBGeometry are actually *pseudo-triangles* that store all the vertex positions, as
well as the face, vertex, and edge *pseudo-normals*, which are used when computing the signed
distance to a triangle mesh.

Triangle meshes in EBGeometry have no internal pointer chasing; they are stored in raw form,
so that groups of them can be packed together tightly in memory as a single block, covering
several triangles at once. Doing so enables evaluating the signed distance to several triangles
simultaneously, using vectorized (SIMD) instructions rather than one triangle at a time -- see :ref:`Chap:SIMDAcceleration`.
