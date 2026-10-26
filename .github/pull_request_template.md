# Summary

### Background

### Solution

### Side-effects

### Alternative solutions 

### Performance

<!-- The comparison against the base branch, run locally on the committed PR branch once the PR is
     ready for review (CI checks the receipt only then; about 8 minutes):
     Scripts/run-performance.sh compare origin/<base> --record
     (and with --gpu cuda if the change touches device code). Commit the receipt it writes,
     Performance/receipt.md, which CI checks, and paste build/performance/compare/report.md here.
     Only a green receipt passes CI on its own: a yellow (minor regression, changed checksum) or red
     (MAJOR) one needs its reason on the receipt's "**Accepted:**" line, a MAJOR one the maintainer's. See Docs/Sphinx/source/PerformanceTesting.rst. -->

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

