---
name: prepare-release
description:
  Prepare a CLI11 release — changelog entry, version bump, README feature
  markers, and related version references.
---

# Prepare a CLI11 release

When preparing a release (`chore: prepare X.Y.Z release`):

## Collect the changes

List the merged PRs since the last release with
`git log --oneline --first-parent v<last>..main`. Every user-visible change gets
a changelog line.

## Changelog entry

Add a `## Version X.Y.Z: <Short title>` section at the top of `CHANGELOG.md`.
The heading becomes the name of the GitHub release, so give it a short title,
such as `## Version 2.7.0: Audit and documentation`.

- Start with a short prose paragraph that summarizes the release.
- Group the entries under `### Added`, `### Changed`, `### Fixed`,
  `### Documentation`, and `### Internal` (and `### Removed`/`### Deprecated` if
  needed). `Documentation` covers docs-only changes; `Internal` covers CI,
  tests, and tooling changes with no user-visible effect.
- Reference PRs as `[#1305][]` and put the matching link definitions
  (`[#1305]: https://github.com/CLIUtils/CLI11/pull/1305`) at the end of the
  version section — each section keeps its own definitions so an extracted
  section needs no fixup.

## Version bump

Bump `include/CLI/Version.hpp`: the `CLI11_VERSION_MAJOR`/`MINOR`/`PATCH` macros
and the `CLI11_VERSION` string.

## Other files

- Only if not a patch release: Update the feature markers in `README.md`:
  remove the old 🆕 markers (the previous release's features) and change 🚧
  markers (main-only features) to 🆕.  Removing an emoji from a heading also
  changes its TOC anchor — drop the trailing `-` from the matching TOC links.
- Check the copyright year in `LICENSE` and the version in
  `book/chapters/installation.md`.

## Verify

CI extracts the release notes from the changelog on every build, so a version
bump without a matching changelog section fails. Check locally:

```bash
python3 scripts/ExtractReleaseNotes.py --title "$(python3 scripts/ExtractVersion.py)"
python3 scripts/ExtractReleaseNotes.py "$(python3 scripts/ExtractVersion.py)"
```

## After the PR merges

Tell the user to tag the merge commit as `vX.Y.Z` and push the tag. CI then
creates the GitHub release: the name comes from the changelog heading, the body
from the changelog section, and the single header plus source packages are
attached automatically.
