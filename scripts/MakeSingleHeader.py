#!/usr/bin/env python

from __future__ import print_function, unicode_literals

import os
import re
from argparse import ArgumentParser
from subprocess import Popen, PIPE
import warnings

tag_str = r"""
^                  # Begin of line
[/\s]+             # Whitespace or comment // chars
\[                 # A literal [
{tag}:             # The tag
(?P<name>[\w_]+)   # name: group name
:                  # Colon
(?P<action>[\w_]+) # action: type of include
\]                 # A literal ]
\s*                # Whitespace
$                  # End of a line

(?P<content>.*)    # All

^                  # Begin of line
[/\s]+             # Whitespace or comment // chars
\[                 # A literal [
{tag}:             # The tag
(?P=name)          # Repeated name
:                  # Colon
end                # Literal "end"
\]                 # A literal ]
\s*                # Whitespace
$                  # End of a line
"""

DIR = os.path.dirname(os.path.abspath(__file__))


class HeaderGroups(dict):
    def __init__(self, tag):
        """
        A dictionary that also can read headers given a tag expression.

        TODO: might have gone overboard on this one, could maybe be two functions.
        """
        self.re_matcher = re.compile(
            tag_str.format(tag=tag), re.MULTILINE | re.DOTALL | re.VERBOSE
        )
        super(HeaderGroups, self).__init__()

    def read_header(self, filename):
        """
        Read a header file in and add items to the dict, based on the item's action.
        """
        with open(filename) as f:
            inner = f.read()

        matches = self.re_matcher.findall(inner)

        if not matches:
            warnings.warn(
                "Failed to find any matches in {filename}".format(filename=filename)
            )

        for name, action, content in matches:
            if action == "verbatim":
                assert (
                    name not in self
                ), "{name} read in more than once! Quitting.".format(name=name)
                self[name] = content
            elif action == "set":
                self[name] = self.get(name, set()) | set(content.strip().splitlines())
            else:
                raise RuntimeError("Action not understood, must be verbatim or set")

    def post_process(self):
        """
        Turn sets into multiple line strings.
        """
        for key in self:
            if isinstance(self[key], set):
                self[key] = "\n".join(self[key])


def make_header(output, main_header, files, tag, namespace, macro=None, version=None):
    """
    Makes a single header given a main header template and a list of files.
    """
    groups = HeaderGroups(tag)

    # Set tag if possible to class variable
    try:
        proc = Popen(
            ["git", "describe", "--tags", "--always"], cwd=str(DIR), stdout=PIPE
        )
        out, _ = proc.communicate()
        groups["git"] = out.decode("utf-8").strip() if proc.returncode == 0 else ""
    except OSError:
        groups["git"] = ""

    for f in files:
        groups.read_header(f)

    groups["namespace"] = namespace
    groups["version"] = version or groups["git"]

    groups.post_process()

    with open(main_header) as f:
        single_header = f.read().format(**groups)

    if macro is not None:
        before, after = macro
        print("Converting macros", before, "->", after)
        single_header.replace(before, after)

    if output is not None:
        with open(output, "w") as f:
            f.write(single_header)

        print("Created", output)
    else:
        print(single_header)


if __name__ == "__main__":
    parser = ArgumentParser(
        usage="Convert source to single header include. Can optionally add namespace and search-replace replacements (for macros)."
    )
    parser.add_argument("--output", default=None, help="Single header file output")
    parser.add_argument(
        "--main",
        default="CLI11.hpp.in",
        help="The main include file that defines the other files",
    )
    parser.add_argument("files", nargs="*", help="The header files")
    parser.add_argument("--namespace", default="CLI", help="Set the namespace")
    parser.add_argument("--tag", default="CLI11", help="Tag to look up")
    parser.add_argument(
        "--macro", nargs=2, help="Replaces OLD_PREFIX_ with NEW_PREFIX_"
    )
    parser.add_argument("--version", help="Include this version in the generated file")
    args = parser.parse_args()

    make_header(
        args.output,
        args.main,
        args.files,
        args.tag,
        args.namespace,
        args.macro,
        args.version,
    )
