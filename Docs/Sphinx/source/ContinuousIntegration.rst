.. _Chap:ContinuousIntegration:

Continuous integration
========================

Every pull request targeting ``main`` triggers the CI pipeline defined in
``.github/workflows/CI.yml``.  The pipeline runs on GitHub-hosted
``ubuntu-latest`` runners and is structured as follows.

.. contents:: On this page
   :local:
   :depth: 2

Jobs overview
---------------

.. list-table::
   :header-rows: 1
   :widths: 25 75

   * - Job
     - What it does
   * - ``Formatting``
     - Runs ``clang-format`` (version 18) over ``Source/`` and ``Examples/``
       via `jidicula/clang-format-action
       <https://github.com/jidicula/clang-format-action>`_.  The PR is blocked
       if any file differs from the formatted output.
   * - ``Linux-GNU``
     - Compiles every example under ``Examples/`` with GCC 9, 10, 11, and 12
       using ``-std=c++17 -pedantic -Wall -Wextra`` (plus many additional
       diagnostic flags) and runs the resulting binary.  Depends on
       ``Formatting``.
   * - ``Linux-Intel``
     - Compiles a subset of examples with the Intel ``icpx`` compiler using
       ``-std=c++17 -Wall -Werror``.  Depends on ``Formatting``.
   * - ``Build-documentation``
     - Installs Doxygen, Graphviz, LaTeX, Poppler (for the documentation
       figure pipeline), and Sphinx (with ``sphinx_rtd_theme`` and
       ``sphinxcontrib-bibtex``); renders the documentation figures from
       their LaTeX/TikZ sources (see :ref:`Chap:Contributing`'s figure-build
       step below); builds HTML and PDF documentation; uploads the result as
       a workflow artifact.  Depends on ``Formatting``.
   * - ``Unit-Tests``
     - Configures with ``cmake --preset debug`` and builds the Catch2 suite
       across a 2 × 2 matrix: compilers ``{g++-12, clang++-14}`` × SIMD levels
       ``{none, avx}``.  Runs ``ctest --preset debug`` (unit label only).
       Depends on ``Formatting``.
   * - ``CI-passed``
     - Dummy job whose only role is to aggregate all of the above as a
       single required status check.  Branch-protection rules can target
       this job instead of each individual job.

Dependency graph
------------------

.. code-block:: text

   Formatting
    +-- Linux-GNU
    +-- Linux-Intel
    +-- Build-documentation
    +-- Unit-Tests
         (all four) --> CI-passed

All jobs must pass for the ``CI-passed`` gate to turn green.

Running CI checks locally with ``pre-commit``
------------------------------------------------

A subset of the CI checks can be reproduced locally before pushing using
`pre-commit <https://pre-commit.com>`_:

.. code-block:: bash

   pip install pre-commit
   pre-commit install          # installs the git hook
   pre-commit run --all-files  # run all hooks on the whole tree

The hooks configured in ``.pre-commit-config.yaml`` include:

* **codespell** — typo detection across all tracked files.
* **doxygen-check** — builds Doxygen from ``Docs/doxygen.conf`` (warnings as
  errors).
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

   The three documentation hooks above are declared ``stages: [manual]`` because
   they need system LaTeX/Doxygen tooling and take longer to run than the default
   hooks. They are not run by a plain ``pre-commit run`` invocation; use the
   explicit ``--hook-stage manual`` flag (with the specific hook id, or omit it
   to run all manual-stage hooks) when you want to verify documentation changes
   locally. Run ``build-doc-figures`` before either ``sphinx-build-*`` hook if
   you changed a figure's ``.tex`` source — pre-commit runs local hooks in the
   order they appear in the config file, so ``pre-commit run --all-files
   --hook-stage manual`` already gets the ordering right.
