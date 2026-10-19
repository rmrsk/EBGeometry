# Summary

### Background

### Solution

### Side-effects

### Alternative solutions 

### Performance

<!-- The comparison against the base branch: paste build/perf-compare/report.md from
     cmake -DBASE=origin/<base> -DHEAD=<this branch> -P Performance/Compare.cmake
     (add -DGPU=CUDA if the change touches device code). Explain any minor or MAJOR verdict; a PR with
     an unexplained MAJOR regression is not merged. See Docs/Sphinx/source/PerformanceTesting.rst. -->

### Reviewer checklist (to be completed by a human)

- [ ] The test suite compiles and runs to completion without warnings or errors.
- [ ] All relevant new features are documented in the user documentation (Sphinx).
- [ ] This contribution does not break existing sections in the user documentation. 
- [ ] All relevant APIs are documented in the doxygen documentation.
- [ ] Appropriate labels have been assigned to this PR.
- [ ] New or revised proper licensing and copyright information is in place.
- [ ] A PR review has been run using `@claude review`.
- [ ] The continuous integration and testing hooks at GitHub run to completion.
- [ ] The Performance section carries a comparison against the base branch, and every regression in it is explained.

