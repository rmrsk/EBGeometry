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

``noexcept`` policy
----------------------

* Every function that cannot propagate an exception — pure arithmetic, trivial
  accessors, constructors that only copy POD values — **must** be declared
  ``noexcept``.
* Functions that perform heap allocation (``std::vector``, ``std::make_shared``,
  ``std::string`` construction, file I/O) **must not** be declared ``noexcept``.
  Marking an allocating function ``noexcept`` silently converts a
  ``std::bad_alloc`` into ``std::terminate``, which is almost never the right
  behaviour.
* When in doubt, omit ``noexcept`` rather than adding it incorrectly.

``[[nodiscard]]``
--------------------

* Add ``[[nodiscard]]`` to every function whose return value conveys the
  result of a computation (signed-distance queries, accessors, parser
  functions, BVH builders, mathematical operators).  Silently discarding
  such values is almost always a programming error.

``EBGEOMETRY_EXPECT``
------------------------

* Guard all public-facing preconditions (non-zero radii, valid axis indices,
  non-null pointers in public API, non-empty containers) with
  ``EBGEOMETRY_EXPECT(cond)``.
* Do **not** guard internal invariants that the surrounding code already
  enforces — this adds noise without safety benefit.
* See :ref:`Sec:Assertions` for how the macro behaves with and without
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
