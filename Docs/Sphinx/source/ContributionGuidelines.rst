.. _Chap:ContributionGuidelines:

Contribution guidelines
=========================

.. contents:: On this page
   :local:
   :depth: 2

What a change needs
-------------------

Three folders hold programs, each with one job: ``Tests/`` *verifies* (it fails when the library is
wrong), ``Examples/`` *teaches* (it shows a user the calls to write, and checks nothing), and
``Performance/`` *measures* (it times and compares, and checks only that answers did not change). A new
program goes in the folder whose job it does.

* **A new public class or function** needs unit tests (see `Adding tests`_), an entry in
  ``Tests/InstantiateAll.cpp``, and documentation (below). If users build it or query it in bulk, it
  needs performance cases (:ref:`Chap:PerformanceTesting`); if users need to be shown how to use it (a new
  capability, not a new overload), an example.
* **A new device path for existing code** needs a ``[gpu]`` test and a host-and-device performance pair.
* **A bug fix** needs a test that fails without the fix.
* **A new example** is a folder with ``main.cpp``, ``CMakeLists.txt``, ``GNUmakefile`` and ``README.md``
  (copied from the nearest existing example, so that its three build routes and switches keep working), a
  page ``Example<Name>.rst`` listed on the page of its topic's group (``Examples<Group>.rst``, see
  :ref:`Chap:Examples`), and a line in the same group of ``Examples/README.md``'s reading order. An example that builds a shape ends by opening the viewer
  on it, and its page shows the first frame (``Scripts/render-example-figures.sh <Name>``).

Documentation
-------------

The two documentation systems have different readers:

* **This user guide** (``Docs/Sphinx/source``) explains concepts, how to use the API and how to choose
  between options. It names no internal classes or namespaces and gives no measured numbers: a choice is
  explained in words, with a pointer to measuring it (the probes, or an example). Short usage snippets
  are fine; anything longer is an example. ``literalinclude`` is not used: a page links to a class's
  Doxygen page instead.
* **The Doxygen comments** in the headers state the contract a user relies on (``@brief``, ``@param``,
  ``@return``), and in ``@details`` what a maintainer needs: the algorithm, invariants, device and
  performance constraints, why the design is as it is, and the measurements behind a rule.

Both are written in American English, and state what is true now: no "formerly", "since #...", or pull
request and issue numbers (the :ref:`Chap:History` page excepted). The user guide shows a feature with an
example program, never with a unit test. A change that alters a documented behavior updates the pages
that describe it in the same pull request.

Pull requests
-------------

Open a pull request as a draft, from the template (``.github/pull_request_template.md``): its *Summary*
sections, *Background*, *Solution*, *Side-effects*, *Alternative solutions*, *Performance* and *Where to
look*, describe the change; the reviewer checklist is for the reviewer. *Where to look* names the files
that carry the risk, what was verified and how, and what was not (a configuration not built, no run on a
GPU). Before it is marked ready for review, record the performance receipt on the finished code (see
:ref:`Chap:PerformanceTesting`) and summarize its verdicts in *Performance*.

Code style
------------

* Format all C++ files with ``clang-format`` version 21 before committing.
  The repository's ``.clang-format`` file defines the style; running
  ``clang-format -i <file>`` will apply it in-place.
* Follow the naming conventions already present in the codebase (``UpperCamel``
  for types, ``lowerCamel`` with ``a_`` prefix for function parameters,
  ``m_`` prefix for member variables).

Static and dynamic assertions
--------------------------------

EBGeometry guards its preconditions with three mechanisms. When adding new functionality, follow
the same split:

* Guard template-parameter invariants that are known at compile time (a floating-point type,
  an in-range branching factor, ...) with ``static_assert`` -- a violation should fail the build
  rather than exercise a runtime check that can never actually be reached.
* Guard what a caller controls and what is checked once, when an object is built (positive radii,
  valid axis indices, matching array sizes, non-empty inputs, a suitable memory resource), with the
  always-on ``EBGEOMETRY_REQUIRE(cond, "Class: message", ...)``. A Release build must not carry on
  into a wrong answer or undefined behavior because of bad input.
* Guard internal invariants, and anything on a path that runs many times (a query, a traversal
  step), with ``EBGEOMETRY_EXPECT(cond)``, which compiles to nothing unless
  ``EBGEOMETRY_ENABLE_ASSERTIONS`` is defined. Do **not** guard invariants that the surrounding code
  already enforces -- this adds noise without safety benefit.

The file readers do not abort on a bad file; they throw, as described in :ref:`Chap:Parsers`.

See :ref:`Chap:ConfigurationOptions`'s "Compile-time assertions (``static_assert``)",
:ref:`Sec:AlwaysOnChecks` and :ref:`Sec:Assertions` subsections for the full detail on how each
mechanism behaves, including with and without ``EBGEOMETRY_ENABLE_ASSERTIONS``.

.. _Sec:WritingDeviceCode:

Writing device code
-------------------

Functions marked ``EBGEOMETRY_HOST_DEVICE`` are compiled for the GPU as well as the host. nvcc
rejects a call from device code to a ``constexpr`` host function -- ``std::min``, ``std::max``,
``std::clamp``, ``std::numeric_limits<T>::max()``, ``std::array::operator[]``, ``std::move`` and
many more -- unless every translation unit is compiled with ``--expt-relaxed-constexpr``.
EBGeometry does not require that flag of the projects that use it, and HIP accepts such calls, so
the HIP CI build cannot catch them. The library therefore keeps to two rules, which
``Scripts/CheckDeviceMath.py`` checks as a pre-commit hook and in CI:

* Nowhere under ``Source/``: ``std::min``, ``std::max``, ``std::clamp``, ``std::numeric_limits``,
  ``std::array``. Use ``Math::min``, ``Math::max``, ``Math::clamp`` and ``Math::Limits<T>`` from
  ``EBGeometry_Math.hpp``, and ``Array<T, N>`` from ``EBGeometry_Array.hpp``. They behave like their
  ``std`` counterparts and are callable on both host and device. The rule covers host-only code too,
  so that a function can later become ``EBGEOMETRY_HOST_DEVICE`` without a hidden failure.
* Inside an ``EBGEOMETRY_HOST_DEVICE`` function (and in the device code of the tests): no call to a
  ``std::`` function other than the math functions CUDA and HIP provide for device code, such as
  ``std::sqrt``, ``std::abs`` and ``std::isfinite``.

The CUDA test build deliberately omits ``--expt-relaxed-constexpr``, so that it also checks these
rules.

HIP inlines every device function by default, so a device function is copied into every place it is
called from. That is harmless until a function calls the next one in a chain from several places: the
tape's evaluator calls the level below it from each Blur tap, each Mollify tap and each kind of BVH
union, and fully inlined, the copies multiply at every level, until an optimized device compile no
longer finishes in reasonable time and memory. Mark such a function ``EBGEOMETRY_DEVICE_NOINLINE`` (``EBGeometry_GPU.hpp``),
which keeps it out of line in the device pass and changes nothing on the host. The ray queries are the
other place this matters: a march, a span list or a pixel of the renderer evaluates or traces the shape
from many places (the bisection, each way out of the march, the shadow and occlusion rays), and inlined,
each of them would be a copy of a mesh's distance or of a tape's trace, and the device compile would
take many times longer and much more memory. They reach the shape through a few out-of-line helpers
(``RayDetail::valueAt()``, ``primitiveOf()``, ``gradientNormal()``, ``march()`` and ``shapeHit()``, and
the renderer's ``Shader::trace()``), so a kernel holds one copy of each. New ray code should call the
shape through them. ``valueAt()`` inlines an analytic shape's value (only a shape in a pool or a fused
node is called out of line), since that small value is much faster inlined on a device.
CI's main HIP build is
optimized (``Release``), since an unoptimized build compiles such code quickly and so cannot show the
problem.

.. _Sec:DeviceCallableClass:

Making a class device-callable
------------------------------

A class that a kernel can use follows the memory model in :ref:`Chap:MemoryModel`:

1. Every member is a plain value. A cross-reference is a ``uint32_t`` index into the owning container's
   arrays (sentinel ``UINT32_MAX``), resolved by passing that owner in explicitly, never a cached
   pointer, which is only valid in the address space that set it.
2. Array storage is a ``PODVector`` reserved from a caller-supplied ``Pool``. Build-phase mutators take
   the ``Pool&`` explicitly, since ``reserve()`` can move the base.
3. ``EBGEOMETRY_HOST_DEVICE`` marks anything that resolves purely through values, indices and other
   device-callable calls; ``EBGEOMETRY_HOST`` anything touching a ``Pool`` (including ``rebasedView()``,
   which reads one), ``std::vector``, ``std::string``, ``std::cerr`` or ``std::map``. The rules of
   :ref:`Sec:WritingDeviceCode` apply.
4. ``static_assert(std::is_trivially_copyable_v<...>)`` on the class, and on any user type parameter it
   stores in pool memory.
5. A method returning a container (``std::vector``) that is useful on a device gets a *streaming*
   sibling instead of an annotation, as ``FaceT::getSmallestCoordinate()``/``getHighestCoordinate()`` do.
6. The class goes into ``Tests/InstantiateAll.cpp``, gets a ``[gpu]``-tagged test (below), and its test
   binary goes into ``EBGEOMETRY_GPU_TESTS`` in ``Tests/CMakeLists.txt``.
7. The Sphinx page that documents the class is updated.

Adding tests
--------------

New classes and functions should be accompanied by Catch2 tests under
``Tests/``.  Register the test binary in ``Tests/CMakeLists.txt`` using the
``ebgeometry_add_test(TestName)`` helper, which handles linking against
``Catch2::Catch2WithMain``, setting the C++17 standard, and exposing
``EBGEOMETRY_TEST_DATA_DIR`` for any test data files placed under
``Tests/data/``.  See :ref:`Chap:TestingLocally` for the existing test binaries
and what each one covers — new tests should follow the same one-file-per-component
convention.

Adding new public classes should also be reflected in ``Tests/InstantiateAll.cpp``,
which explicitly instantiates every public class template so that ``clang-tidy``
and the project's warning set analyze them regardless of what the tests exercise.

A class that is callable on a GPU also gets a device test: a test case tagged ``[gpu]`` in its test
file, whose binary is listed in ``EBGEOMETRY_GPU_TESTS`` in ``Tests/CMakeLists.txt``.
``Tests/TestGPU.hpp`` holds the harness. Write the check as a functor -- a trivially copyable struct
holding the object under test (a ``rebasedView()`` onto a pool mirrored into
``deviceTestResource()``, for anything pool-resident) whose ``EBGEOMETRY_HOST_DEVICE`` call operator
maps one query to one result -- then run it over many queries with ``evaluateOnDevice`` (one thread
per query) and ``evaluateOnHost``, and compare the two element by element with
``requireSameResults``. ``evaluateOnDevice`` calls the library's ``GPU::transform``, the same call
the examples use: under a GPU backend the functor runs in a real kernel, every runtime call
(including the launch and its completion) is checked and aborts the test on an error, and the test
skips when no GPU is present. In a host build the same test runs in emulation:
``deviceTestResource()`` is the library's ``emulatedDeviceMemoryResource()``, host memory that reports
itself device-accessible, so the view takes the
device-view path, and ``GPU::transform`` copies the functor and runs it in a host loop. Every ordinary build, including the sanitizer build,
therefore runs the device tests; only the device compile and the kernel launch need a GPU lane.
