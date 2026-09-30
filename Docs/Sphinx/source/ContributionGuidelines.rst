.. _Chap:ContributionGuidelines:

Contribution guidelines
=========================

.. contents:: On this page
   :local:
   :depth: 2

Code style
------------

* Format all C++ files with ``clang-format`` version 18 before committing.
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
  into a wrong answer or undefined behaviour because of bad input.
* Guard internal invariants, and anything on a path that runs many times (a query, a traversal
  step), with ``EBGEOMETRY_EXPECT(cond)``, which compiles to nothing unless
  ``EBGEOMETRY_ENABLE_ASSERTIONS`` is defined. Do **not** guard invariants that the surrounding code
  already enforces -- this adds noise without safety benefit.

See :ref:`Chap:ConfigurationOptions`'s "Compile-time assertions (``static_assert``)",
:ref:`Sec:AlwaysOnChecks` and :ref:`Sec:Assertions` subsections for the full detail on how each
mechanism behaves, including with and without ``EBGEOMETRY_ENABLE_ASSERTIONS``.

Adding tests
--------------

New classes and functions should be accompanied by Catch2 tests under
``Tests/``.  Register the test binary in ``Tests/CMakeLists.txt`` using the
``ebgeometry_add_test(TestName)`` helper, which handles linking against
``Catch2::Catch2WithMain``, setting the C++17 standard, and exposing
``EBGEOMETRY_TEST_DATA_DIR`` for any test data files placed under
``Tests/data/``.  See :ref:`Chap:TestingLocally` for the existing test binaries
and what each one covers — new tests should follow the same one-binary-per-class
convention.

Adding new public classes should also be reflected in ``Tests/InstantiateAll.cpp``,
which explicitly instantiates every public class template so that ``clang-tidy``
and the project's warning set analyse them regardless of what the tests exercise.
