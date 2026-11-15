.. _Chap:Contributing:

Contributing
============

Contributions are welcome: open an issue or a discussion to see whether a feature is planned, or fork the
repository and open a pull request. This section is for contributors:

* :ref:`Chap:ContributionGuidelines`: what a change needs (tests, examples, performance cases,
  documentation), the code conventions, and how a pull request is reviewed.
* :ref:`Chap:TestingLocally`: building and running the unit tests, the CMake presets, the sanitizers, and
  what each test binary covers.
* :ref:`Chap:PerformanceTesting`: the performance probes, and the comparison against its base that every
  pull request carries.
* :ref:`Chap:ContinuousIntegration`: what continuous integration checks on every pull request and every
  merge, and how to run the same checks locally.

A pull request goes through the same steps whoever wrote it:

#. **Draft.** Open it as a draft, filled in from the pull request template, so that a fast tier of
   continuous integration runs on every push.
#. **Finish it as a draft.** Code, tests, examples and documentation in the same pull request; then the
   performance comparison, whose receipt is committed with it (:ref:`Chap:PerformanceTesting`).
#. **Review.** It is marked ready for review once it is finished, which runs the full continuous
   integration; a maintainer reviews it and merges it.

``Scripts/run-all-checks.sh`` runs locally nearly everything continuous integration checks, in one
command (see :ref:`Chap:ContinuousIntegration`).
