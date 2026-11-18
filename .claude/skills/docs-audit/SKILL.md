---
name: docs-audit
description: Audit an EBGeometry change against its documentation - the Sphinx pages, the Doxygen comments, Doxygen links, InstantiateAll.cpp and the coverage tables - and rebuild the docs with warnings as errors. Use before promoting a PR out of draft, after a PR out of draft changes again, after renaming or adding a public class or function, or when asked for a docs pass.
---

# Documentation audit of a change

The rules (no `literalinclude`, the docs are updated in the same PR, examples not tests, what goes in Sphinx and
what in the headers, no history) are in `CLAUDE.md`. This is the check that the PR keeps them.

Start from what actually changed: `git diff origin/<base> --name-only` (or `git diff --staged --name-only`). For
each changed `Source/*.hpp` file:

1. **No `literalinclude` crept back in**: `grep -rn "literalinclude" Docs/Sphinx/source/` must return nothing. If it
   does, replace it with a Doxygen link.
2. **Renamed or removed identifiers in prose**: for a renamed class, method or parameter,
   `grep -rn "<oldName>" Docs/Sphinx/source/ Docs/mainpage.md README.md CLAUDE.md`. A Doxygen link does not break
   when the linked class is renamed, so this needs an explicit check.
3. **A new public class or function**:
   - add it to `Tests/InstantiateAll.cpp` (clang-tidy and compile coverage under both precisions);
   - check whether it deserves a mention on its `Docs/Sphinx/source/Implem*.rst` page (concrete API,
     Doxygen-linked) and on its concept page in the same section of the guide (Geometry, Queries, Acceleration or
     Where it runs; the conceptual picture, no implementation classes named);
   - a cell in the geometry-by-queries table of `Overview.rst` if it adds one, and a row in a coverage table such
     as `TestingLocally.rst`'s, matching how sibling classes are documented.
4. **A changed signature** (a parameter added, removed or renamed, a changed return type): update its
   `@param`/`@tparam`/`@return` in the same header. Doxygen (`WARN_AS_ERROR = FAIL_ON_WARNINGS`) catches a missing
   `@param`, but not a stale description, nor an `.rst` page that still describes the old signature.
5. **Rebuild the Sphinx HTML** and check for warnings: `pre-commit run sphinx-build-html --hook-stage manual`, or
   `make html` in `Docs/Sphinx` with `-W`. A broken `:ref:`/`:numref:`/`:eq:` does not stop a plain build but
   surfaces as a warning, and CI's build fails on any warning. Build with the pinned `sphinx==9.1.1` (a virtualenv
   with `pip install sphinx==9.1.1 sphinx_rtd_theme sphinxcontrib-bibtex`); another version can warn where CI does
   not, or the other way round, and `rm -rf Docs/Sphinx/build` clears caches from another version. If the build
   throws an internal error in `sphinx.environment.collectors.toctree`, or the `rm` fails with "Directory not
   empty", check `pgrep -fa sphinx` for a stray `sphinx-autobuild` first.

**A new Doxygen link**: build Doxygen (`cd Docs && doxygen doxygen.conf`) and check that the target `.html` file and
anchor exist under `Docs/doxygen/html/`. A wrong mangled class name or a stale member anchor gives a silently
broken link.

**Then read the pages the change touches.** The `check-docs` hook (`Scripts/CheckDocs.py`) only catches the banned
directive; whether a page still describes what the code does needs a read-through. Check each changed or
affected page against the rules in `CLAUDE.md`: a user guide with no implementation details or measured numbers,
examples cited rather than tests, American English, no history.
