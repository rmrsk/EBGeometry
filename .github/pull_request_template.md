# Summary

### Background

### Solution

### Side-effects

### Alternative solutions 

### Performance

<!-- The comparison against the base branch, run locally on the committed PR branch:
     Scripts/run-performance.sh compare origin/<base> --record
     (add a --gpu cuda run if the change touches device code). Commit its receipt,
     Performance/Receipt/host.md, which CI checks, and paste build/performance/compare/report.md here.
     Explain any minor or MAJOR verdict; a MAJOR one is not merged unless accepted, with the reason
     after "accepted:" in the receipt. See Docs/Sphinx/source/PerformanceTesting.rst. -->

### Reviewer checklist (to be completed by a human)

- [ ] The test suite compiles and runs to completion without warnings or errors.
- [ ] All relevant new features are documented in the user documentation (Sphinx).
- [ ] This contribution does not break existing sections in the user documentation. 
- [ ] All relevant APIs are documented in the doxygen documentation.
- [ ] Appropriate labels have been assigned to this PR.
- [ ] New or revised proper licensing and copyright information is in place.
- [ ] A PR review has been run using `@claude review`.
- [ ] The continuous integration and testing hooks at GitHub run to completion.
- [ ] The performance receipt is current (CI's Performance-receipt passes), and every regression in it is explained.

