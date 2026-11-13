.. _Chap:ImplemLanguage:

Reading models in C++
=====================

A program reads a ``.ebg`` model (see :ref:`Chap:Language`) with ``Source/EBGeometry_Language.hpp``, which
``EBGeometry.hpp`` does not include: a program that reads no models compiles none of it. Everything here runs on the
host, except the tables, which a device reads. :ref:`Chap:ExampleLanguage` shows the whole path, a device query
included.

Reading
-------

`Language::read() <doxygen/html/namespaceEBGeometry_1_1Language.html>`__ reads a ``.ebg`` file or a model folder,
``Language::parse()`` text, into a
`Language::Model <doxygen/html/classEBGeometry_1_1Language_1_1Model.html>`__, with overrides for its params written as
in a file. A model with errors throws a
`Language::Error <doxygen/html/classEBGeometry_1_1Language_1_1Error.html>`__ holding every
`Diagnostic <doxygen/html/structEBGeometry_1_1Language_1_1Diagnostic.html>`__, each with its file, line and column;
``warnings()`` lists the rest. Meshes the model reads are built into the pool given to ``read()``, which must outlive
the object.

.. code-block:: c++

   #include <Source/EBGeometry_Language.hpp>

   Pool pool(hostMemoryResource());

   const auto model = Language::read<double>("cable/", pool, {{"r_core", "0.6"}});   // throws Language::Error

   const Tape<double> tape(pool, model.object());     // the program compiles: it picks features and user primitives

The model holds the object (an ``Expr`` to compile), its index (``ExprIndex``: the regions, materials and labels by
name), its materials, domains, couplings and params, the files it read, and the ``view`` block. ``names()`` gives the
names a viewer shows (``RegionNames``: regions, materials, labels, each surface by the shape call it comes from, the
domains and their groups, and the view's colors).

Domains and their properties
----------------------------

A domain of the model, `ModelDomain <doxygen/html/classEBGeometry_1_1Language_1_1ModelDomain.html>`__, holds its
``DomainSetup`` with the names and properties of its subdomains and surface groups. The program builds the domain for
its tape from the setup (see :ref:`Chap:ClosestPrimitive`), with the same numbers: subdomain *i* is label *i*, surface
group *g* is group *g*.

.. code-block:: c++

   const auto&  cfd = model.domain("cfd");             // throws Language::Error listing the domains
   const Domain fluid(pool, tape, cfd.setup());

`Properties <doxygen/html/classEBGeometry_1_1Language_1_1Properties.html>`__ are what a material, subdomain, surface
group or coupling carries: ``number()``, ``vector<T>()``, ``string()`` and ``flag()`` (true or false) by key, each with an optional fallback, and
``code(key, {"no_slip", "slip"})``, a string as its index among names, for a kernel to switch on. A missing key or a
value of another type throws an ``Error`` at the line in the file. A subdomain's properties are its own over its
regions' material's.

Tables on a device
------------------

The program declares its own row type and fills it on the host, from the properties; the rows go into a pool as a
`Language::Table <doxygen/html/classEBGeometry_1_1Language_1_1Table.html>`__, a pool handle like the tape (the pool must
outlive it, ``rebasedView()`` moves it to a mirror). A kernel indexes it by the numbers EBGeometry computes there:

* ``model.regions<S>(pool, fill)``: a row per region, by ``tape.region(p)`` (a region with no material, ``outside``
  included, gets empty properties: give fallbacks);
* ``cfd.volumes<S>(pool, fill)``: a row per subdomain, by ``domain.labelAt(p)`` (``Domain<T>::Excluded`` has no row);
* ``cfd.surfaces<S>(pool, fill)``: a row per surface group, by ``domain.surfaceGroupOf(surface)``.

.. code-block:: c++

   struct Gas { double rho, mu; };

   const auto gas = cfd.volumes<Gas>(pool, [](const Language::Properties& p) {
     return Gas{p.number("density"), p.number("viscosity")};
   });

   pool.freeze();
   const Pool device = Pool::mirror(pool, GPU::memoryResource());
   const auto d = fluid.rebasedView(device);
   const auto g = gas.rebasedView(device);
   // in a kernel: const uint32_t l = d.labelAt(x); if (l != Domain<double>::Excluded) { g[l].rho ... }

Geometry and properties live in the one pool: one mirror, one lifetime. A program's other data is its own.

Checks and text
---------------

``Language::completeness(model, tape, "cfd")`` says whether a domain is complete, with what is missing as diagnostics at
the lines to change; ``inventory(model)`` lists every name the model offers; ``partNames("box")`` a built-in shape's
part names; ``print(model)`` writes the model in canonical form (``print(model, true)`` resolves the includes into one
text).

Reading a model reports errors instead of ending the program. The checks the library makes of the values it is given
(a negative radius, say) abort a program built in C++, where a value is the program's own; the reader turns them into
errors at their call by building inside a `ThrowOnRequire <doxygen/html/classEBGeometry_1_1ThrowOnRequire.html>`__
scope, where a failed check throws ``RequireError``. A program that builds objects from input of its own can do the
same. The scope holds on the thread that opens it only, and a check in a ``noexcept`` function still ends the program.
