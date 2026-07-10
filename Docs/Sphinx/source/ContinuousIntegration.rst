.. _Chap:ContinuousIntegration:

Continuous integration
========================

Every pull request targeting ``main`` triggers the CI pipeline defined in
``.github/workflows/CI.yml``, on GitHub-hosted ``ubuntu-latest`` runners. Together, the jobs
check: code formatting (``clang-format``) and static analysis (``clang-tidy``, advisory); code
correctness and assurance (the Catch2 unit-test suite, under multiple compilers, SIMD levels,
and both ``float`` and ``double`` precision; every bundled example, built and run via CMake, GNU
Make, and direct compiler invocation, under GCC, Clang, and Intel's ``icpx``; AddressSanitizer
and UndefinedBehaviorSanitizer runs of the same test suite); spelling (``codespell``); license
and copyright compliance (REUSE); and the project's documentation (a warnings-as-errors Doxygen
build, and HTML/PDF Sphinx builds). A single aggregator job (``CI-passed``) then gates on all of
the above so branch-protection rules only need to target one required check.

.. contents:: On this page
   :local:
   :depth: 2

Jobs overview
---------------

Four *setup* jobs run first with no dependencies -- ``Formatting``, ``Codespell``, ``Reuse``, and
``Doxygen-check``. Every other job depends on all four of them and runs only once they pass; the
`Dependency graph`_ below shows the full picture. Each job is described in turn.

Formatting
~~~~~~~~~~

Runs ``clang-format`` (version 21) over ``Source/`` and ``Examples/`` (matrix over the two
directories) via `jidicula/clang-format-action
<https://github.com/jidicula/clang-format-action>`_. The PR is blocked if any file differs from the
formatted output.

Codespell
~~~~~~~~~

Runs the ``codespell`` pre-commit hook over every tracked file.

Reuse
~~~~~

Runs the ``reuse`` pre-commit hook (REUSE license/copyright header compliance) over every tracked
file.

Doxygen-check
~~~~~~~~~~~~~

Runs the ``doxygen-check`` pre-commit hook: builds the Doxygen API reference from
``Docs/doxygen.conf`` with warnings treated as errors.

Static-analysis
~~~~~~~~~~~~~~~

Runs ``clang-tidy-18`` (via ``run-clang-tidy-18``) over every ``Tests/*.cpp`` and ``Examples/*.cpp``
translation unit, using a compile-command database exported from the ``debug`` preset built with
``clang++-14``. Marked ``continue-on-error: true`` (there is a known review backlog), so it is
advisory and does not gate ``CI-passed``.

Linux-GNU
~~~~~~~~~

Compiles and runs every example under ``Examples/`` directly with ``g++`` (matrix over
``{g++-11, g++-12}`` × the six example directories), using ``-std=c++17 -pedantic -Wall -Wextra``
plus a large set of additional diagnostic flags.

Linux-Intel
~~~~~~~~~~~

Compiles and runs a subset of examples (``MeshSDF``, ``PackedSpheres``, ``RandomCity``, ``Shapes``)
with Intel's ``icpx`` compiler, ``-std=c++17 -Wall -Werror`` plus additional diagnostic flags.

Examples-GNUMake
~~~~~~~~~~~~~~~~

Builds and runs every example (matrix over the six example directories) via its own ``GNUmakefile``
(``make run``).

Examples-CMake
~~~~~~~~~~~~~~

Configures and builds the top-level project with the ``debug`` preset (assertions on, ``double``
precision) and runs every example via ``ctest --preset examples``. This is the path that exercises
the CMake-driven build with assertions enabled, unlike ``Linux-GNU``/``Linux-Intel`` (raw compiler
invocation, no assertions) or ``Unit-Tests``/``Sanitizers`` (examples disabled).

Examples-FloatPrecision
~~~~~~~~~~~~~~~~~~~~~~~~~

The same as ``Examples-CMake``, but configured with ``-DEBGEOMETRY_PRECISION=float`` (a cache
variable shared by every example's own ``CMakeLists.txt``).

Build-documentation
~~~~~~~~~~~~~~~~~~~

Installs Doxygen, Graphviz, a LaTeX toolchain, Poppler (for the documentation figure pipeline), and
Sphinx (with ``sphinx_rtd_theme`` and ``sphinxcontrib-bibtex``); builds the Doxygen API reference;
renders the documentation figures from their LaTeX/TikZ sources (``Scripts/build-doc-figures.sh``);
builds the Sphinx HTML and PDF documentation; uploads the result as a workflow artifact.

Unit-Tests
~~~~~~~~~~

Configures with the ``debug`` preset (examples disabled) across a matrix of compilers
``{g++-12, clang++-14}`` × SIMD levels ``{none, avx, avx512}``, with
``-DEBGEOMETRY_TEST_BOTH_PRECISIONS=ON``, and runs ``ctest --preset debug``. The ``avx512``
configurations always compile (catching ISA-specific errors in the SIMD-accelerated code paths) but
only actually run on a runner whose CPU supports AVX-512F.

Release-Test
~~~~~~~~~~~~

Configures with the ``release-test`` preset (optimised, AVX, examples and tests both enabled) plus
``-DEBGEOMETRY_TEST_BOTH_PRECISIONS=ON``, and runs ``ctest --preset release-test`` (the full
unit-test and example suite).

Sanitizers
~~~~~~~~~~

Configures with the ``debug-san`` preset (examples disabled) across a matrix of compilers
``{g++-12, clang++-14}`` × SIMD levels ``{none, avx}``, with ``-DEBGEOMETRY_TEST_BOTH_PRECISIONS=ON``,
and runs ``ctest --preset debug-san`` under AddressSanitizer and UndefinedBehaviorSanitizer.

CI-passed
~~~~~~~~~

Dummy job whose only role is to aggregate every job above -- except the advisory ``Static-analysis``
-- as a single required status check. Branch-protection rules can target this job instead of each
individual job.

Dependency graph
------------------

.. code-block:: text

   Formatting, Codespell, Reuse, Doxygen-check
    +-- Static-analysis          (advisory; not required by CI-passed)
    +-- Linux-GNU
    +-- Linux-Intel
    +-- Examples-GNUMake
    +-- Examples-CMake
    +-- Examples-FloatPrecision
    +-- Build-documentation
    +-- Unit-Tests
    +-- Release-Test
    +-- Sanitizers
         (all of the above except Static-analysis) --> CI-passed

``Formatting``, ``Codespell``, ``Reuse``, and ``Doxygen-check`` themselves have no
dependencies and run first, in parallel; every other job depends on all four of them.

Running CI checks locally with ``pre-commit``
------------------------------------------------

A subset of the CI checks can be reproduced locally before pushing using
`pre-commit <https://pre-commit.com>`_:

.. code-block:: bash

   pip install pre-commit
   pre-commit install          # installs the git hook
   pre-commit run --all-files  # run all hooks on the whole tree

The hooks configured in ``.pre-commit-config.yaml`` include:

* **clang-format** — formats ``Source/`` and ``Examples/`` C/C++ files (default stage; runs on
  every ``git commit`` once ``pre-commit install`` has been run).
* **reuse** — REUSE license/copyright header compliance (default stage).
* **codespell** — typo detection across ``Source/``, ``Docs/``, and ``Exec/`` (default stage).
* **doxygen-check** — builds Doxygen from ``Docs/doxygen.conf`` (warnings as
  errors; default stage).
* **clang-tidy** — static analysis over the library headers, via
  ``Scripts/clang-tidy-check.sh`` (``stages: [manual]``; needs a compile
  database, so it (re)configures the ``debug`` preset first).
* **build-tests** — compiles the unit test suite with the ``debug`` preset
  (``stages: [manual]``), catching template-instantiation errors locally
  before they show up first in CI.
* **check-docs** — enforces the ban on ``.. literalinclude::`` in the Sphinx docs
  (``stages: [manual]``; see ``Scripts/CheckDocs.py``): it fails if any
  ``.. literalinclude::`` directive exists anywhere under ``Docs/Sphinx/source/``.
  A clean run only guarantees the banned directive is absent, not that the
  surrounding prose still accurately describes the code -- that still needs a
  manual read-through.
* **build-doc-figures** — renders the documentation figures from their
  LaTeX/TikZ sources under ``Docs/Sphinx/source/_static/`` (``stages: [manual]``;
  requires ``pdflatex`` and ``pdftoppm`` on ``PATH``, see :ref:`Chap:Contributing`).
* **sphinx-build-html** — builds the Sphinx HTML docs in a managed Python virtual
  environment (``stages: [manual]``; run with
  ``pre-commit run sphinx-build-html --hook-stage manual``).
* **sphinx-build-pdf** — builds the Sphinx PDF docs via ``make latexpdf``
  (``stages: [manual]``; requires a full LaTeX toolchain — ``texlive-latex-extra``
  and ``latexmk`` — on ``PATH``; run with
  ``pre-commit run sphinx-build-pdf --hook-stage manual``).

.. tip::

   The manual-stage hooks above are not run by a plain ``pre-commit run``
   invocation (they need system LaTeX/Doxygen/CMake tooling and take longer than
   the default hooks); use the explicit ``--hook-stage manual`` flag (with the
   specific hook id, or omit it to run all manual-stage hooks) when you want to
   verify them locally. Run ``build-doc-figures`` before either ``sphinx-build-*``
   hook if you changed a figure's ``.tex`` source — pre-commit runs local hooks in
   the order they appear in the config file, so ``pre-commit run --all-files
   --hook-stage manual`` already gets the ordering right.
