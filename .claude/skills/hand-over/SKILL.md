---
name: hand-over
description: Get an EBGeometry pull request ready for human review - the @claude review rounds, the receipt, promotion out of draft with labels, the full CI run, the PR body's Where to look section and the hand-over summary - and handle a reviewer's comments afterwards. Use when the maintainer asks to "get it ready for human review", to promote or hand over a PR, to run or follow up on @claude review rounds, or to answer review comments.
---

# Handing a pull request over for review

The rules (when a PR counts as handed over, promote once, mark ready only when asked) are in `CLAUDE.md`, "Human
review". This is the procedure, in its order. Do not skip or reorder a step: each later one assumes the earlier
ones are done.

## 1. The review sparring (while the PR is a draft)

- Request a round by commenting `@claude review` on the PR, after a push that changes code or docs.
- The reply comes as an issue comment whose body starts with `**Claude finished @... task in ...`. On this
  repository its author has been **`github-actions[bot]`**, though it may show as `claude[bot]`. When waiting for it, take a timestamp before posting and filter on either
  login, on "finished" in the body, and on `since=`.
- The review may say it skipped files; say what it did not read when reporting it.
- A round may reread only the latest commit and repeat earlier suggestions as still open. Check any such item
  against the pushed branch (`git show origin/<branch>:<file>`) before acting on it, and say in the summary when a
  round's list was stale.
- Fix each blocking finding in a new commit, push, and request another round. Each optional suggestion is taken,
  or declined with a one-line reason kept for the hand-over summary.
- Stop when a round has no blocking finding. At most five rounds: if the fifth still has one, stop and bring it to
  the maintainer.

## 2. The receipt

Recorded on the code the rounds converged on, never before or alongside them. It needs the maintainer's
quiet-machine go. Follow the `receipt` skill; commit and push the receipt, and summarize its verdicts in the PR
body's *Performance* section.

## 3. The docs and the PR body

- Run the `docs-audit` skill on the PR's diff if not done since the last code change.
- The PR body follows `.github/pull_request_template.md` and describes the final state. Its *Where to look*
  section names the files and places that carry the risk, what was verified and how, and what was not (no HIP run,
  no device run, a configuration not built). Every checklist item stays unchecked.

## 4. Promotion, once

- Only when the maintainer has asked for it ("get it ready for human review"). Otherwise stop here and report that
  the other conditions hold.
- `gh pr ready <number>`, and check the labels set when the PR was opened against what it changes now: add and remove
  (`gh pr edit --add-label/--remove-label`), from the existing set only (`gh label list`; never a new label), on the
  PR and on the issues it closes.
- Promotion starts CI's full matrix. Every later push runs it again, so a fix still needed after promotion (a CI
  failure, a reviewer's comment) goes in one push, never several.

## 5. A fully green CI

Wait for the full matrix on the final head, `CI-passed` included (it also checks the receipt). Promotion cancels the
draft-tier run of the same head, so `gh pr checks` briefly shows its jobs as failed (cancelled, with no duration and
unexpanded matrix names). Watch the run promotion started instead: the newest `pull_request` run in
`gh run list --branch <branch>`, followed with `gh run watch <id>`; cancelled jobs of a superseded run are not
failures. A PR that is still a
draft, or whose full run has not passed, has not been handed over.

## 6. The summary

In the session, briefly: that the PR is ready, what changed since the maintainer last looked, the optional review
suggestions that were declined and why, and what needs the maintainer's decision.

## Afterwards: the reviewer's comments

- Fix each in a new commit; never force-push over commits that were reviewed.
- Reply on its thread with what changed, and leave resolving the thread to the reviewer.
- Verify again what the change affects: the tests, a new receipt if the library or the probes changed, another
  `@claude review`. Only then say the PR is ready again.
