#!/usr/bin/env python3

"""Print the CHANGELOG.md section for a version, for use as release notes.

VERSION accepts a tag name ("v2.7.0") or a plain version ("2.7.0"). Older
sections are titled with two components and a name, such as
"## Version 2.5: Help Formatter", so "2.5.0" also matches "## Version 2.5".
Each section keeps its own link definitions, so the output needs no fixup.
"""

from __future__ import annotations

import argparse
import re
from pathlib import Path


def extract(text: str, version: str) -> str | None:
    heads = list(re.finditer(r"^## Version (\S+?):?(?:[ \t].*)?$", text, re.M))
    for candidate in (version, version.rsplit(".", 1)[0]):
        for i, head in enumerate(heads):
            if head.group(1) != candidate:
                continue
            end = heads[i + 1].start() if i + 1 < len(heads) else len(text)
            return text[head.end() : end].strip()
    return None


def main() -> None:
    parser = argparse.ArgumentParser(
        description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter
    )
    parser.add_argument("version", help='tag ("v2.7.0") or version ("2.7.0")')
    parser.add_argument("changelog", nargs="?", type=Path, default=Path("CHANGELOG.md"))
    args = parser.parse_args()

    version = args.version.lstrip("v")
    notes = extract(args.changelog.read_text(encoding="utf-8"), version)
    if notes is None:
        raise SystemExit(f"No '## Version {version}' section in {args.changelog}")
    print(notes)


if __name__ == "__main__":
    main()
