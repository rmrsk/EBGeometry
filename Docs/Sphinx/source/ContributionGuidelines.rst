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

EBGeometry guards its preconditions with two complementary mechanisms: a compile-time
``static_assert`` for anything decidable from template parameters alone, and the runtime
``EBGEOMETRY_EXPECT(cond)`` macro for anything that can only be checked from actual argument
values. When adding new functionality, follow the same split:

* Guard all public-facing runtime preconditions (non-zero radii, valid axis indices,
  non-null pointers in public API, non-empty containers) with
  ``EBGEOMETRY_EXPECT(cond)``. Do **not** guard internal invariants that the surrounding code
  already enforces — this adds noise without safety benefit.
* Guard template-parameter invariants that are known at compile time (a floating-point type,
  an in-range branching factor, ...) with ``static_assert`` instead — a violation should fail
  the build rather than exercise a runtime check that can never actually be reached.

See :ref:`Chap:ConfigurationOptions`'s "Compile-time assertions (``static_assert``)" and
:ref:`Sec:Assertions` ("Runtime assertions (``EBGEOMETRY_EXPECT``)") subsections for the full
detail on how each mechanism behaves, including with and without
``EBGEOMETRY_ENABLE_ASSERTIONS``.

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
