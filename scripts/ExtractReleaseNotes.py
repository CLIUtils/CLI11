#!/usr/bin/env python3

"""Print the CHANGELOG.md section for a version, for use as release notes.

Usage: ExtractReleaseNotes.py VERSION [CHANGELOG]

VERSION accepts a tag name ("v2.7.0") or a plain version ("2.7.0"). Older
sections are titled with two components and a name, such as
"## Version 2.5: Help Formatter", so "2.5.0" also matches "## Version 2.5".
Each section keeps its own link definitions, so the output needs no fixup.
"""

import re
import sys


def extract(text, version):
    heads = list(re.finditer(r"^## Version (\S+?):?(?:[ \t].*)?$", text, re.M))
    for candidate in (version, version.rsplit(".", 1)[0]):
        for i, head in enumerate(heads):
            if head.group(1) != candidate:
                continue
            end = heads[i + 1].start() if i + 1 < len(heads) else len(text)
            return text[head.end() : end].strip()
    return None


def main():
    if not 2 <= len(sys.argv) <= 3:
        sys.exit(__doc__)
    version = sys.argv[1].lstrip("v")
    changelog = sys.argv[2] if len(sys.argv) == 3 else "CHANGELOG.md"

    with open(changelog, encoding="utf-8") as fp:
        notes = extract(fp.read(), version)

    if notes is None:
        sys.exit(f"No '## Version {version}' section in {changelog}")
    print(notes)


if __name__ == "__main__":
    main()
