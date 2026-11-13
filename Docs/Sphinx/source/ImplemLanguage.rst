.. _Chap:ImplemLanguage:

Reading models in C++
=====================

A program reads a ``.ebg`` model (see :ref:`Chap:Language`) with ``Source/EBGeometry_Language.hpp``, which
``EBGeometry.hpp`` does not include: a program that reads no models compiles none of it. Everything here runs
on the host, except the tables, which a device reads. :ref:`Chap:ExampleLanguage` shows the whole path, a
device query included.

.. contents:: On this page
   :local:
   :depth: 1

Reading
-------

`Language::read() <doxygen/html/namespaceEBGeometry_1_1Language.html>`__ reads a ``.ebg`` file and the files
it includes, and ``Language::parse()`` reads text, into a
`Language::Model <doxygen/html/classEBGeometry_1_1Language_1_1Model.html>`__. Both take the pool that the
meshes the model reads are built into, which must outlive the object, and overrides for its params, written as
in a file:

.. code-block:: c++

   #include <Source/EBGeometry_Language.hpp>

   Pool pool(hostMemoryResource());

   const auto model = Language::read<double>("cable/setup.ebg", pool, {{"r_core", "0.6"}});   // throws Language::Error

   const Tape<double> tape(pool, model.object());     // the program chooses the tape's features and user primitives

A model with errors throws a `Language::Error <doxygen/html/classEBGeometry_1_1Language_1_1Error.html>`__
holding every `Diagnostic <doxygen/html/structEBGeometry_1_1Language_1_1Diagnostic.html>`__, each with its
file, line and column; ``model.warnings()`` lists the warnings of a model that was read. The precision is the
program's: the model is built in ``T``.

What a model holds
------------------

* ``object()``: the object, an ``Expr<T>`` to compile into a tape, and ``index()``, its
  `ExprIndex <doxygen/html/classEBGeometry_1_1ExprIndex.html>`__: the regions, materials and labels by name
  (see :ref:`Chap:ImplemRegions`).
* ``domain(name)`` and ``domain(i)``: its domains (``ModelDomain``, below); ``domain(name)`` throws a
  ``Language::Error`` listing the domains when there is none of that name.
* ``materialProperties(material)``: a material's properties; ``couplings()``: the couplings
  (`Coupling <doxygen/html/structEBGeometry_1_1Language_1_1Coupling.html>`__: their kind, the two domains
  and parts they join, their properties).
* ``params()``: the params (`Param <doxygen/html/structEBGeometry_1_1Language_1_1Param.html>`__: name, value
  after the overrides, type, where it is written), so that a program can show what it may change.
* ``files()``: every file the model read, and ``sources()``: their parsed text, for a program that watches
  them or edits them.
* ``view()``: the view block's properties, for a program that opens a window.
* ``shapes()`` and ``siteOfLeaf(leaf)``: the shapes the text builds and where
  (`ShapeSite <doxygen/html/structEBGeometry_1_1Language_1_1ShapeSite.html>`__: the call, its file and line,
  its parts and their labels), so that a program can name a surface by the line that made it.
* ``names()``: the names a viewer shows (``RegionNames``: the regions, materials and labels, each surface by
  the shape call it comes from, the domains and their groups, and the view's colors).

Domains and their properties
----------------------------

A domain of the model, `ModelDomain <doxygen/html/classEBGeometry_1_1Language_1_1ModelDomain.html>`__, holds
its ``DomainSetup`` with the names and properties of its subdomains and surface groups. The program builds the
domain for its tape from the setup (see :ref:`Chap:ImplemRegions`), with the same numbers: subdomain *i* is
label *i*, surface group *g* is group *g*.

.. code-block:: c++

   const auto&  cfd = model.domain("cfd");
   const Domain fluid(pool, tape, cfd.setup());

`Properties <doxygen/html/classEBGeometry_1_1Language_1_1Properties.html>`__ are what a material, subdomain,
surface group or coupling carries: ``number()``, ``vector<T>()``, ``string()`` and ``flag()`` (true or false)
by key, each with an optional fallback, and ``code(key, {"no_slip", "slip"})``, a string as its index among
names, for a kernel to switch on. A missing key, or a value of another type, throws an ``Error`` at the line in
the file. A subdomain's properties are its own over its regions' material's.

Tables on a device
------------------

The program declares its own row type and fills it on the host, from the properties; the rows go into a pool as
a `Language::Table <doxygen/html/classEBGeometry_1_1Language_1_1Table.html>`__, a pool handle like the tape
(the pool must outlive it, and ``rebasedView()`` moves it to a mirror). A kernel indexes it by the numbers
EBGeometry computes there:

* ``model.regions<S>(pool, fill)``: a row per region, by ``tape.region(p)`` (a region with no material,
  ``outside`` included, gets empty properties: give fallbacks);
* ``cfd.volumes<S>(pool, fill)``: a row per subdomain, by ``domain.subdomainAt(p)`` (``Domain<T>::Excluded`` has no
  row);
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
   // in a kernel: const uint32_t l = d.subdomainAt(x); if (l != Domain<double>::Excluded) { g[l].rho ... }

Geometry and properties live in the one pool: one mirror, one lifetime. A program's other data is its own.

Checks and text
---------------

* ``Language::completeness(model, tape, "cfd")`` checks a domain (see :ref:`Chap:LanguageDomains`): a
  `Completeness <doxygen/html/structEBGeometry_1_1Language_1_1Completeness.html>`__, whether it is complete
  (``m_complete``) and what is missing, as diagnostics at the lines to change (``m_findings``).
* ``inventory(model)`` lists every name the model offers, as text: its regions and materials, its labels,
  each shape's parts and their labels, and its domains.
* ``partNames("box")`` gives a built-in shape's part names.
* ``print(model)`` writes the model in canonical form; ``print(model, true)`` resolves the includes into one
  text.

The namespace ``Language::Syntax`` holds the language's text as a tool sees it: ``tokenize()``, ``parse()``
into a syntax tree (``Syntax::File``) with every position, and ``isReserved()``. It is what a program that
edits ``.ebg`` files, rather than reading models, builds on.

Reading a model reports errors instead of ending the program. The checks the library makes of the values it is
given (a negative radius, say) abort a program built in C++, where a value is the program's own; the reader
turns them into errors at their call by building inside a
`ThrowOnRequire <doxygen/html/classEBGeometry_1_1ThrowOnRequire.html>`__ scope, where a failed check throws
``RequireError``. A program that builds objects from input of its own can do the same. The scope holds on the
thread that opens it only, and a check in a ``noexcept`` function still ends the program.

From C++ to .ebg
----------------

The language writes what the C++ API builds:

.. list-table::
   :header-rows: 1
   :widths: 45 55

   * - C++
     - .ebg
   * - ``SphereSDF<T>(center, radius)``, ``BoxSDF<T>(lo, hi)``, ...
     - ``sphere(center, radius)``, ``box(lo, hi)``, ...
   * - ``Parser::readIntoTriMeshSDF<T>(file, pool)``
     - ``mesh(file)``
   * - ``Union(a, b)``, ``Difference(a, b)``, ``Translate(a, v)``, ...
     - ``union(a, b)``, ``difference(a, b)``, ``translate(a, v)`` or ``a.translate(v)``, ...
   * - ``Union(objects, Accelerate::BVH)``
     - ``bvh_union(...)``
   * - ``SurfaceLabel(a, "name")``, labels per part
     - ``a.label("name")``, ``a.label(xmin = "inlet")``
   * - ``Overlay(RegionName(MaterialLabel(a, "m"), "r"), ...)``
     - ``overlay { region r = a material m ... }``
   * - ``DomainSetup::addSubdomain()``, ``addSurfaceGroup()`` and ``add(group, selector)``
     - ``domain d { subdomain ... group ... }``
   * - ``SurfaceSelector::label()``, ``interface()``, ``region()``, ``materials()``, ``rest()``
     - ``label()``, ``interface()``, ``region()``, ``materials()``, ``remaining()``
   * - per-material and per-subdomain data in the program's own arrays
     - ``material`` and subdomain properties, read into a ``Language::Table``

What the language does not express is the program's: a tape's features and user primitives (the reader builds
the built-in shapes, and the program chooses the tape type it compiles the object into), and shapes of your
own.
