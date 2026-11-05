.. _Chap:ExampleParticleContacts:

Particle contacts
=================

Particles of differing sizes packed in a cylinder, and their contacts: for each particle, the particles that come
within a gap of it, through ``forEachContact()`` on a
`SphereCloudBVH <doxygen/html/classEBGeometry_1_1SphereCloudBVH.html>`__ (see :ref:`Chap:ImplemPointCloud`). The
source is at :file:`Examples/ParticleContacts/main.cpp`.

.. figure:: /_static/example_ParticleContacts.jpg
   :width: 60%
   :align: center

   3,000 particles packed in a cylinder, with radii between 0.01 and 0.04, none overlapping.

The code that runs on either side
---------------------------------

The functor ``Repulsion`` holds the cloud by value, and a range, and maps a particle's index to the push of the
particles closer to it than the range, each pushing along the line of centers in proportion to how much closer it
is: the kind of short-range force a particle code sums. Its ``EBGEOMETRY_HOST_DEVICE`` call operator asks the cloud
for the particle's contacts within the range, and passes a lambda that ``forEachContact()`` calls once for each:

.. code-block:: c++

   m_cloud.forEachContact(a_index, gap, [&](const SphereCloud::Hit<T>& a_contact) {
     // a_contact.index:    the other particle
     // a_contact.distance: the separation of the surfaces, negative when they overlap
   });

Two particles are in contact when their separation, :math:`|c_i - c_j| - (r_i + r_j)`, is less than the gap. A
contact is mutual: if j is a contact of i, then i is a contact of j, at the same separation.

Building the cloud
------------------

The particles are seeded at random in an upright cylinder. The cloud reads the program's own arrays of centers and
radii through ``SphereCloud::FromCentersAndRadii``, and a particle is known by its index in them. The last argument
of ``build()`` reserves room for rebuilds of up to that many particles.

Pushing the particles apart
---------------------------

Seeded at random, many particles overlap. A contact within a gap of 0 is an overlap, so the program counts them
with ``forEachContact()``, and while there are any, moves each particle by half the push of ``Repulsion`` over a
small range (half, since both particles of a pair move), keeps it inside the cylinder, and calls ``rebuild()`` on the
moved centers. This is the loop a particle code runs every step: query, move, rebuild. ``rebuild()`` works in the
arrays ``build()`` reserved, and reserves nothing.

One particle's contacts
-----------------------

The program prints particle 0's contacts within the gap and their separations, none of them negative now.
``forEachContact()`` visits them in no set order.

A contact list
--------------

A list holds each particle's contacts in one row, in arrays of the program's own. It is built in three steps:
count each particle's contacts, add up the counts into where each row starts, and fill the rows. Each contact
appears in the rows of both its particles. The same three steps run on a GPU, with ``GPU::transform()`` or
``GPU::parallelFor()`` for the first and the last and ``GPU::exclusiveScan()`` for the second.

A force loop does not need the list: it can call ``forEachContact()`` once per particle, as the next section does.

The near-contact force on a GPU
-------------------------------

The pool is frozen and mirrored into ``GPU::memoryResource()``, and ``rebasedView()`` gives a cloud that reads the
copy. ``GPU::transform()`` runs ``Repulsion`` over the gap once per particle, one thread each, with no list and no atomics:
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
Built for a GPU (see :ref:`Sec:ExamplesGPU`), the near-contact forces and the window run on the device.

.. code-block:: bash

   cd Examples/ParticleContacts
   ./ParticleContacts.ex
