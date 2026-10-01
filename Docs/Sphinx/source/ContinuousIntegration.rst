.. _Chap:ContinuousIntegration:

Continuous integration
========================

Every pull request targeting ``main`` or ``dev``, and every push to ``main`` or ``dev`` (so the
merged result is checked too), triggers the CI pipeline defined in ``.github/workflows/CI.yml``, on
GitHub-hosted ``ubuntu-latest`` runners. Together, the jobs check: code formatting
(``clang-format``) and static analysis (``clang-tidy``, advisory); code correctness and assurance
(the Catch2 unit-test suite, under multiple compilers, SIMD levels, and both ``float`` and
``double`` precision; every bundled example, built and run via CMake, GNU Make, and direct compiler
invocation, under GCC, Clang, and Intel's ``icpx``; AddressSanitizer and UndefinedBehaviorSanitizer
runs of the same test suite; device compiles of the GPU-callable code with HIP and, advisory, CUDA);
spelling (``codespell``); license and copyright compliance (REUSE); and the project's documentation
(a warnings-as-errors Doxygen build, the ``check-docs`` rule, and HTML/PDF Sphinx builds, the
HTML one with warnings as errors). A single aggregator job (``CI-passed``) then gates on all of the
above so branch-protection rules only need to target one required check.

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

Runs the ``codespell`` pre-commit hook over the files it is configured for: ``Source/``,
``Docs/``, ``Examples/``, ``Tests/`` (except the fixture files in ``Tests/data/``),
``Integrations/``, ``Scripts/``, ``.github/``, and the top-level text files.

Reuse
~~~~~

Runs the ``reuse`` pre-commit hook (REUSE license/copyright header compliance) over every tracked
file.

Doxygen-check
~~~~~~~~~~~~~

Runs the ``doxygen-check`` pre-commit hook, which builds the Doxygen API reference from
``Docs/doxygen.conf`` with warnings treated as errors, and the ``check-docs`` hook (see
`Running CI checks locally with pre-commit`_ below).

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
builds the Sphinx HTML documentation with warnings treated as errors (``-W --keep-going``, so a
broken cross-reference or a missing figure fails the job) and the PDF documentation; uploads the
result as a workflow artifact.

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

GPU-HIP
~~~~~~~

Installs a HIP toolchain (``hipcc``, ``clang-17``) and compiles the unit-test files that carry a
device block in clang's HIP mode, with ``-DEBGEOMETRY_TEST_BOTH_PRECISIONS=ON``, so the
GPU-callable code must compile for the device. It builds a second time with host SIMD flags
(``-mavx -mfma -msse4.1``), which catches a SIMD block that is not excluded from device
compilation. The runner has no GPU, so the device-tagged tests it then runs skip before launching a
kernel: the step checks that the binaries start and that the device query is safe, not that device
code gives correct results. This job is required by ``CI-passed``.

GPU-CUDA
~~~~~~~~

The same device compile with ``nvcc`` (CUDA 12.5), without the second SIMD-flags build. Marked
``continue-on-error: true``, because its toolkit install is the least reliable step in the
workflow, so it is advisory and does not gate ``CI-passed``.

CI-passed
~~~~~~~~~

Aggregates every job above -- except the advisory ``Static-analysis`` and ``GPU-CUDA`` -- as a single
required status check. It runs even when one of those jobs fails or is cancelled, and fails unless
every one of them succeeded; a skipped dependency counts as a failure too, since a skipped required
check would not block a merge. Branch-protection rules can target this job instead of each
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
    +-- GPU-CUDA                 (advisory; not required by CI-passed)
    +-- GPU-HIP
         (all of the above except Static-analysis and GPU-CUDA) --> CI-passed

``Formatting``, ``Codespell``, ``Reuse``, and ``Doxygen-check`` themselves have no
dependencies and run first, in parallel; every other job depends on all four of them.

Package installs and time limits
----------------------------------

Every job that installs Ubuntu packages does so through ``.github/actions/apt-install``, a small
composite action that caches the package index and the downloaded ``.deb`` files between runs
(``actions/cache``). On a cache hit it installs with no network access at all, from the cached
index and packages, so maintainer scripts still run but neither the index refresh nor the download
touches a mirror. Ubuntu's mirrors are sometimes slow, or stop answering, for long enough to eat a
job's whole time limit; with the cache, that can only happen on the first run of a month. If the
offline install fails (the runner image has moved on, say), the action refreshes the index and
downloads, with timeouts and retries so a dead mirror cannot hang the job. The cache key holds the
runner image, the package list and the month, so a new image or a changed list starts a fresh
cache. The documentation deploy workflow (``docs.yml``) uses the same action. ``Linux-Intel``
installs Intel's compiler from Intel's own repository and is not cached.

``Build-documentation`` and ``Sanitizers`` have 30-minute limits, which leaves room for a first,
uncached run on a slow mirror.

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
* **codespell** — typo detection across ``Source/``, ``Docs/``, ``Examples/``, ``Tests/``
  (except ``Tests/data/``), ``Integrations/``, ``Scripts/``, ``.github/``, and the top-level text
  files (default stage).
* **doxygen-check** — builds Doxygen from ``Docs/doxygen.conf`` (warnings as
  errors; default stage). It runs whenever a header, ``EBGeometry.hpp``, ``Docs/mainpage.md`` or
  ``Docs/doxygen.conf`` changes.
* **clang-tidy** — static analysis over the library headers, via
  ``Scripts/clang-tidy-check.sh`` (``stages: [manual]``; needs a compile
  database, so it (re)configures the ``debug`` preset first).
* **build-tests** — compiles the unit test suite with the ``debug`` preset
  (``stages: [manual]``), catching template-instantiation errors locally
  before they show up first in CI.
* **check-docs** — enforces the ban on ``.. literalinclude::`` in the Sphinx docs
  (default stage, run whenever a ``Docs/Sphinx/source/*.rst`` file changes; CI's
  ``Doxygen-check`` job also runs it; see ``Scripts/CheckDocs.py``): it fails if any
  ``.. literalinclude::`` directive exists anywhere under ``Docs/Sphinx/source/``.
  A clean run only guarantees the banned directive is absent, not that the
  surrounding prose still accurately describes the code -- that still needs a
  manual read-through.
* **build-doc-figures** — renders the documentation figures from their
  LaTeX/TikZ sources under ``Docs/Sphinx/source/_static/`` (``stages: [manual]``;
  requires ``pdflatex`` and ``pdftoppm`` on ``PATH``, see :ref:`Chap:Contributing`).
* **sphinx-build-html** — builds the Sphinx HTML docs in a managed Python virtual
  environment, with warnings treated as errors (``stages: [manual]``; run with
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
