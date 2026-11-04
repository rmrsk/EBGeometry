.. _Chap:ExampleParticleContacts:

Particle contacts
=================

Particles of differing sizes and their contacts: for each particle, the particles it overlaps or comes within a
gap of, through ``forEachContact()`` on a
`SphereCloudBVH <doxygen/html/classEBGeometry_1_1SphereCloudBVH.html>`__ (see :ref:`Chap:ImplemPointCloud`). The
source is at :file:`Examples/ParticleContacts/main.cpp`.

.. figure:: /_static/example_ParticleContacts.jpg
   :width: 60%
   :align: center

   3,000 particles in the unit cube, with radii between 0.01 and 0.04.

The code that runs on either side
---------------------------------

The functor ``ContactForce`` holds the cloud by value and maps a particle's index to the force of the particles it
overlaps, each pushing along the line of centers in proportion to the overlap. Its ``EBGEOMETRY_HOST_DEVICE`` call
operator asks the cloud for the particle's contacts, and passes a lambda that ``forEachContact()`` calls once for
each:

.. code-block:: c++

   m_cloud.forEachContact(a_index, gap, [&](const SphereCloud::Hit<T>& a_contact) {
     // a_contact.index:    the other particle
     // a_contact.distance: the separation of the surfaces, negative when they overlap
   });

Two particles are in contact when their separation, :math:`|c_i - c_j| - (r_i + r_j)`, is less than the gap. A
contact is mutual: if j is a contact of i, then i is a contact of j, at the same separation.

Building the cloud
------------------

As in :ref:`Chap:ExampleSphereClouds`, the cloud is built from the program's arrays of centers and radii, and a
particle is known by its index in them.

One particle's contacts
-----------------------

The program prints particle 0's contacts and their separations. ``forEachContact()`` visits them in no set order.

A contact list
--------------

A list holds each particle's contacts in one row, in arrays of the program's own. It is built in three steps:
count each particle's contacts, add up the counts into where each row starts, and fill the rows. Each contact
appears in the rows of both its particles. The same three steps run on a GPU, with ``GPU::transform()`` or
``GPU::parallelFor()`` for the first and the last and ``GPU::exclusiveScan()`` for the second.

A force loop does not need the list: it can call ``forEachContact()`` once per particle, as the next section does.

The contact force on a GPU
--------------------------

The pool is frozen and mirrored into ``GPU::memoryResource()``, and ``rebasedView()`` gives a cloud that reads the
copy. ``GPU::transform()`` runs ``ContactForce`` once per particle, one thread each, with no list and no atomics:
each thread adds up the forces on its own particle. In a plain C++ build the same lines run on the host.

Looking at it
-------------

The example ends with ``Render::view(cloud, pool, argc, argv)``; X cuts the cloud open.

The window needs SDL2 and is built with ``make VIEWER=SDL2`` or ``cmake -DEBGEOMETRY_VIEWER=SDL2`` (see
:ref:`Sec:ConfigViewer`); otherwise, and with ``--no-view`` or without a display, ``view()`` prints why
and returns. Drag to orbit and scroll to zoom; the window lists its other keys (see
:ref:`Sec:ViewerKeys`).

Building and running
--------------------

See :ref:`Chap:Building` for how to compile it with CMake, GNU Make, or a direct compiler invocation.
Built for a GPU (see :ref:`Sec:ExamplesGPU`), the contact forces and the window run on the device.

.. code-block:: bash

   cd Examples/ParticleContacts
   ./ParticleContacts.ex
