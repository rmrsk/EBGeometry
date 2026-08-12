.. _Chap:Contributing:

Overview
========

This section describes how to build and run the EBGeometry test suite locally,
what the continuous integration (CI) pipeline checks, and the conventions
expected of contributions.  It is split into three pages, one per topic:

* :ref:`Chap:TestingLocally` — building and running the Catch2 unit test suite, CMake presets,
  sanitizers, and a table of what each test binary covers.
* :ref:`Chap:ContinuousIntegration` — what the GitHub Actions CI pipeline checks on every pull
  request, and how to reproduce those checks locally with ``pre-commit``.
* :ref:`Chap:ContributionGuidelines` — code style, static and dynamic assertions, and test
  coverage expected of new code.

Contributions to the ongoing GPU port have an additional starting point: ``PORTING.md`` in the
repository root, which records the memory model the port is built on (see :ref:`Chap:MemoryModel`),
which components are ported so far, and the conventions for making a class device-callable.
