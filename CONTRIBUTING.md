# How to Contribute

We'd love to accept your patches and contributions to this project. There are
just a few small guidelines you need to follow.

## Contributor License Agreement

Contributions to this project must be accompanied by a Contributor License
Agreement. You (or your employer) retain the copyright to your contribution;
this simply gives us permission to use and redistribute your contributions as
part of the project. Head over to <https://cla.developers.google.com/> to see
your current agreements on file or to sign a new one.

You generally only need to submit a CLA once, so if you've already submitted one
(even if it was for a different project), you probably don't need to do it
again.

## Code reviews

All submissions, including submissions by project members, require review. We
use GitHub pull requests for this purpose. Consult
[GitHub Help](https://help.github.com/articles/about-pull-requests/) for more
information on using pull requests.

## Community Guidelines

This project follows [Google's Open Source Community
Guidelines](https://opensource.google/conduct/).

## Pull Request Guidelines

We have the following guidelines for pull requests:

* Each pull request must contain exactly **one** commit (excluding fixup commits) and must be submitted using **squash merges**.
* Each pull request (and per above each commit) must **logically make a single change**. Note that the boundary of this is fuzzy and it’s up to authors to decide whether they combine multiple commits into a single commit for review. Reviewers should point out obvious violations (such as, medium to large scale renaming as part of a functionality change), but smaller violations should be allowed (such as, renaming a function as part of a functionality change).
* Every pull request (and per above every commit) must include a *what* and a *why* in the change description. The “why” can be a link to a bug/issue (in which case the bug should contain enough context).

## Commit message Guidelines

Commit messages must follow the following rules:

### Commit header (subject line)
* One line, 72 characters max, but aim for 50
* Capitalized (starts with a capital letter)
* Doesn’t end with a period
* Imperative style (“Fix bug” rather than “Fixed bug” or “Fixes bug”)

### Commit body
* Separated from subject line by blank line
* Wrapped to 72 characters
* Explain *what* and *why* rather than *how*
* Add a Test: stanza describing how the change was tested. Examples may be `Test: run OrbitBaseUnitTests` or `Test: start orbit, capture, save, restart orbit, load capture, see that it did not crash`.
* If possible add a `Bug:` stanza at the end of the commit message to link to existing bugs/issues. Make sure to include http:// - it helps tools recognize the link and let git log reader open it without copy and pasting it first. Example: `Bug: http://b/42`



