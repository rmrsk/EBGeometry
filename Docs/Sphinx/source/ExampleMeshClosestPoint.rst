.. _Chap:ExampleMeshClosestPoint:

MeshClosestPoint
================

Finds the point of a mesh closest to a few query points, and the gradient of the signed distance from it,
on the host and through ``GPU::transform``. The source is at :file:`Examples/MeshClosestPoint/main.cpp`.

.. figure:: /_static/example_MeshClosestPoint.jpg
   :width: 60%
   :align: center

   The cow as the viewer first shows it.

The code that runs on either side
---------------------------------

The query handed to ``GPU::transform`` holds a
`TriMeshSDF <doxygen/html/classEBGeometry_1_1TriMeshSDF.html>`__ by value and returns its ``gradient()`` at a
point, a ``Vec3T``.

Read the mesh
-------------

The mesh comes from the command line or, by default, ``cow.obj`` from the ``common-3d-test-models``
submodule (see :ref:`Sec:Cloning`), read into a ``TriMeshSDF``. ``FlatMeshSDF`` and ``MeshSDF`` have the same
two calls.

Use it on the host
------------------

For each point, ``closestPoint()`` returns a
`ClosestPoint <doxygen/html/structEBGeometry_1_1ClosestPoint.html>`__: the signed distance, the closest point,
the face it lies on and the pseudonormal there. ``gradient()`` returns the unit gradient of the distance,
computed from the closest point (see :ref:`Sec:MeshGradient`). The program prints both, and shows that
:math:`\mathbf{x} - S(\mathbf{x})\,\nabla S(\mathbf{x})` is the closest point: a point moves onto the surface in
one step. Beside it, the finite-difference ``normal()`` of the same function agrees to the accuracy its step
allows, from six distance queries instead of one. On the surface itself, the gradient is the pseudonormal of
the face, edge or vertex the point is on.

The same query on a GPU
-----------------------

The pool is frozen and mirrored to the device, and ``GPU::transform`` runs the gradient query there with a
view of the ``TriMeshSDF`` on the mirror. In a plain C++ build the same lines run as a host loop.

Look at it
----------

``Render::view(sdf, pool, argc, argv)`` opens the viewer on the mesh, if the example was built with it (see
:ref:`Chap:Viewer`).
