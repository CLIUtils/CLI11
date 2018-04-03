#!/usr/bin/env python

from __future__ import print_function, unicode_literals

import os
import re
import argparse
import operator
from copy import copy
from subprocess import check_output, CalledProcessError
from functools import reduce

includes_local = re.compile(r"""^#include "(.*)"$""", re.MULTILINE)
includes_system = re.compile(r"""^#include \<(.*)\>$""", re.MULTILINE)
verbatim_tag_str = r"""
^               # Begin of line
[^\n^\[]+       # Some characters, not including [ or the end of a line
\[              # A literal [
[^\]^\n]*       # Anything except a closing ]
CLI11:verbatim  # The tag
[^\]^\n]*       # Anything except a closing ]
\]              # A literal ]
[^\n]*          # Up to end of line
$               # End of a line
"""
verbatim_all = re.compile(verbatim_tag_str + "(.*)" + verbatim_tag_str,
                          re.MULTILINE | re.DOTALL | re.VERBOSE)

DIR = os.path.dirname(os.path.abspath(__file__))

class HeaderFile(object):
    TAG = "Unknown git revision"

    def __init__(self, base, inc):
        with open(os.path.join(base, inc)) as f:
            inner = f.read()

        # add self.verbatim
        if 'CLI11:verbatim' in inner:
            self.verbatim = ["\n\n// Verbatim copy from {}".format(inc)]
            self.verbatim += verbatim_all.findall(inner)
            inner = verbatim_all.sub("", inner)
        else:
            self.verbatim = []

        self.headers = set(includes_system.findall(inner))

        self.body = '\n// From {}\n\n'.format(inc) + inner[inner.find('namespace'):]

    def __add__(self, other):
        out = copy(self)
        out.headers |= other.headers
        out.body += other.body
        out.verbatim += other.verbatim
        return out

    @property
    def header_str(self):
        return '\n'.join('#include <'+h+'>' for h in sorted(self.headers))

    @property
    def verbatim_str(self):
        return '\n'.join(self.verbatim)

    def __str__(self):
        return '''\
#pragma once

// Distributed under the 3-Clause BSD License.  See accompanying
// file LICENSE or https://github.com/CLIUtils/CLI11 for details.

// This file was generated using MakeSingleHeader.py in CLI11/scripts
// from: {self.TAG}
// This has the complete CLI library in one file.

{self.header_str}
{self.verbatim_str}
{self.body}
'''.format(self=self)


def MakeHeader(output, main_header, include_dir = '../include'):
    # Set tag if possible to class variable
    try:
        HeaderFile.TAG = check_output(['git', 'describe', '--tags', '--always'], cwd=str(DIR)).decode("utf-8")
    except CalledProcessError:
        pass

    base_dir = os.path.abspath(os.path.join(DIR, include_dir))
    main_header = os.path.join(base_dir, main_header)

    with open(main_header) as f:
        header = f.read()

    include_files = includes_local.findall(header)

    headers = [HeaderFile(base_dir, inc) for inc in include_files]
    single_header = reduce(operator.add, headers)

    with open(output, 'w') as f:
        f.write(str(single_header))

    print("Created", output)


if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument("output", help="Single header file output")
    parser.add_argument("--main", default='CLI/CLI.hpp', help="The main include file that defines the other files")
    parser.add_argument("--include", default='../include')
    args = parser.parse_args()

    MakeHeader(args.output, args.main, args.include)

