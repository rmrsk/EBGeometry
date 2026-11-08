.. _Chap:ExampleMaterials:

Materials
=========

Which part of an object a query found, by label, and a table of your own indexed by it: the armadillo
with a cylinder bored through its chest. The source is at :file:`Examples/Materials/main.cpp`.

.. figure:: /_static/example_Materials.jpg
   :width: 60%
   :align: center

   The armadillo colored by leaf, from a table of its own: the skin sand, the bore's wall (the cylinder's) steel gray.

.. important::

   The default mesh comes from the ``common-3d-test-models`` git submodule. Clone with
   ``--recurse-submodules``, or run ``git submodule update --init`` in the repository (see
   :ref:`Sec:Cloning`); otherwise pass a mesh of your own as the first argument.

Labels
------

``Label(object, "name")`` names a part; every leaf below it gets the label (see
:ref:`Chap:ClosestPrimitive`). Two queries name the leaf a part of the answer comes from, and the tape's
``getLabel()`` gives its label:

* ``closestLeaf(p)``, the leaf the value at ``p`` comes from;
* ``firstHit(shape, ray, tMin, tMax).m_primitive``, the leaf the ray crossed, with the point and the normal
  (see :ref:`Chap:ImplemRays`).

The label is a number the tape reports on the host and on a GPU alike. An ``ExprIndex`` built from the same
expression numbers the labels the same way and holds their names, so a table of your own (material names
here; in an application, materials or boundary conditions) is filled by name and indexed by the label:

.. code-block:: c++

   const Expr<T> object = Difference(Label(armadillo, "skin"), Label(bore, "bore"));

   const Tape<T>      tape(pool, object);
   const ExprIndex<T> index(object);

   std::vector<std::string> materials(tape.getNumLabels());

   for (const auto& [name, material] : {std::pair<const char*, const char*>{"skin", "skin (the armadillo)"},
                                        {"bore", "machined (the cylinder's bore)"}}) {
     const uint32_t label = index.findLabel(name);   // LeafRef<T>::None for a name the expression lacks

     if (label < materials.size()) {
       materials[label] = material;
     }
   }

   const auto hit = firstHit(tape, ray, T(0), T(6));

   if (hit.m_status == HitStatus::Hit) {
     std::cout << materials[tape.getLabel(hit.m_primitive)] << '\n';
   }

An unlabeled leaf has no label (``LeafRef::None``), so check the number before indexing. A mesh is one
leaf; its faces are told apart by ``getClosestFace()`` on the mesh itself (see
:ref:`Chap:ExampleMeshSDF`).

The program prints the value and material at three points, and what three rays hit: the chest from in
front, nothing straight through the bore, and the bore's wall sideways from inside it.

Looking at it
-------------

``Settings::m_colorBy = ColorBy::Primitive`` colors each surface by the leaf, ``HitT::m_primitive``. A tape
numbers its leaves in expression order, depth first: the armadillo is leaf 0 and the cylinder leaf 1.
``Settings::m_primitiveColors`` gives each leaf a color of its own (a leaf left out gets the renderer's
palette), and the camera looks from low in front, where the bore shows:

.. code-block:: c++

   Render::Settings<T> settings;
   settings.m_colorBy            = Render::ColorBy::Primitive;
   settings.m_primitiveColors[0] = Vec3(0.78, 0.62, 0.42); // the skin, sand
   settings.m_primitiveColors[1] = Vec3(0.55, 0.57, 0.60); // the bore, steel gray

   Render::view(tape, pool, camera, settings, argc, argv);

The window needs SDL2 and is built with ``make VIEWER=SDL2`` or ``cmake -DEBGEOMETRY_VIEWER=SDL2`` (see
:ref:`Sec:ConfigViewer`); otherwise, and with ``--no-view`` or without a display, ``view()`` prints why
and returns. Drag to orbit and scroll to zoom; the window lists its other keys (see
:ref:`Sec:ViewerKeys`).

Building and running
--------------------

See :ref:`Chap:Building` for how to compile it with CMake, GNU Make, or a direct compiler invocation.
Built for a GPU (see :ref:`Sec:ExamplesGPU`), the window renders on the device.

.. code-block:: bash

   cd Examples/Materials
   ./Materials.ex
