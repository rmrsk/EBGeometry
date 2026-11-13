.. _Chap:ExampleLanguage:

Language
========

``Examples/Language`` reads a model written in the :ref:`Chap:Language` at run time: the cable of
:ref:`Chap:Language`'s first model, a copper core inside a PTFE jacket, in a box of air with a flow across it.
The source is at :file:`Examples/Language/main.cpp`, and the model in :file:`Examples/Language/cable/`.

.. figure:: _static/example_Language.jpg
   :width: 60%
   :align: center

   The cable in its box of air, the air hidden, colored by material.

The model
---------

``cable/model.ebg`` holds the geometry: the shapes, the overlay of regions with their materials, and the box's
faces labeled ``inlet``, ``outlet``, ``far`` and ``ends``. ``cable/setup.ebg`` includes it and holds the
materials' properties, two domains (electrostatics, in the jacket and the air; and cfd, the air with four
surface groups) and the couplings between them. Both are shown and explained in :ref:`Chap:Language`.

Reading it
----------

The program reads ``cable/setup.ebg`` with ``Language::read()``, which also reads what it includes; a model with
errors throws them all, and the program prints them and stops. It then compiles the object into a tape, builds
the cfd domain for that tape from the model's setup, and checks it with ``completeness()``.

What a solver keeps
-------------------

The language passes properties through unread, so the program says what they mean: a row of its own type,
``Fluid``, a density and a viscosity, filled from each subdomain's properties into a ``Language::Table`` in the
tape's pool. The subdomain's properties are its own over its regions' material's, so the air's density comes
from the ``air`` material.

On the host and on a GPU
------------------------

A query reads the fluid's density at a point, by the domain label there, along a line across the cable: on
the host, then the same through ``GPU::transform``, after the pool is mirrored to the device. Last, the window
shows the regions, colored by material, with the model's names; its panel switches between the two domains.

Editing the files changes the model with no compiling: run the program again.

Building and running
--------------------

Build it as any example (see :ref:`Chap:Examples`), and run it from its folder, where ``cable/`` is:

.. code-block:: bash

   cd Examples/Language
   make && ./Language.ex                    # or ./Language.ex path/to/setup.ebg

See :ref:`Chap:ImplemLanguage` for the calls.
