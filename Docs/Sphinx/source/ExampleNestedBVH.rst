.. _Chap:ExampleNestedBVH:

NestedBVH
=========

.. warning::

   **Temporarily disabled.** The BVH-accelerated CSG unions (``BVHUnionIF``, ``BVHSmoothUnionIF``
   and their ``BVHUnion``/``BVHSmoothUnion`` factories) are compiled out during the GPU port, behind
   ``EBGEOMETRY_ENABLE_BVH_CSG_UNION`` in :file:`Source/EBGeometry_CSG.hpp`. They stored their
   primitives as ``std::shared_ptr<const ImplicitFunction<T>>``, which the removed
   ``BVH::SharedPtrStorage`` policy provided and neither remaining policy can (see
   :ref:`Sec:PolymorphicPrimitives`). They return with the index-based redesign of the
   implicit-function and CSG layer. Until then this program builds, but prints a notice and exits
   without doing any work.

Builds a *nested* bounding volume hierarchy: an outer, BVH-accelerated CSG union whose primitives
are themselves BVH-backed mesh signed distance functions. One triangle mesh is loaded once into a
``TriMeshSDF`` (which owns an inner ``PackedBVH`` over its triangle groups) and then instanced at
several positions -- each placement a ``Translate`` wrapper sharing a pointer to that same
``TriMeshSDF`` -- and the placements are combined with ``BVHUnion``, which builds the outer
``PackedBVH`` over them. A single distance query therefore descends two levels of BVH — the outer
union hierarchy to locate the nearby placement, then the mesh's own inner hierarchy to find the
nearest triangle (see :ref:`Chap:BVH` and :ref:`Chap:ImplemCSG`).

The outer union shared each placement by pointer rather than copying it, so the one inner mesh BVH
was built and stored just once. That sharing is exactly what the removed ``BVH::SharedPtrStorage``
policy provided; see :ref:`Sec:PolymorphicPrimitives` for why neither remaining policy can stand in
for it, and what replaces the pattern.

The source for this example is at :file:`Examples/NestedBVH/main.cpp`. See :ref:`Chap:Building`
for how to compile it with CMake, GNU Make, or a direct compiler invocation. Unlike the mesh-reading
examples above, it defaults to the ``dodecahedron.stl`` fixture shipped in the repository, so it
needs no submodule.

.. code-block:: bash

   cd Examples/NestedBVH
   ./NestedBVH.ex                       # defaults to ../../Tests/data/dodecahedron.stl
   ./NestedBVH.ex path/to/mesh.stl      # place copies of your own triangle mesh
