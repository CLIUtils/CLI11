#!/usr/bin/env python

# Requires pathlib on python 2

from __future__ import print_function, unicode_literals

import os
import re
import argparse
from subprocess import check_output

includes_local = re.compile(r"""^#include "(.*)"$""", re.MULTILINE)
includes_system = re.compile(r"""^#include \<(.*)\>$""", re.MULTILINE)

DIR = os.path.dirname(os.path.abspath(__file__)) # Path(__file__).resolve().parent
BDIR = os.path.join(os.path.dirname(DIR), 'include') # DIR.parent / 'include'

print("Git directory:", DIR)

TAG = check_output(['git', 'describe', '--tags', '--always'], cwd=str(DIR)).decode("utf-8")

def MakeHeader(out):
    main_header = os.path.join(BDIR, 'CLI', 'CLI.hpp')
    with open(main_header) as f:
        header = f.read()

    include_files = includes_local.findall(header)

    headers = set()
    output = ''
    for inc in include_files:
        with open(os.path.join(BDIR, inc)) as f:
            inner = f.read()
        headers |= set(includes_system.findall(inner))
        output += '\n// From {inc}\n\n'.format(inc=inc)
        output += inner[inner.find('namespace'):]

    header_list = '\n'.join('#include <'+h+'>' for h in headers)

    output = '''\
#pragma once

// Distributed under the 3-Clause BSD License.  See accompanying
// file LICENSE or https://github.com/CLIUtils/CLI11 for details.

// This file was generated using MakeSingleHeader.py in CLI11/scripts
// from: {tag}
// This has the complete CLI library in one file.

{header_list}
{output}'''.format(header_list=header_list, output=output, tag=TAG)

    with open(out, 'w') as f:
        f.write(output)

    print("Created {out}".format(out=out))


if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument("output", nargs='?', default=os.path.join(BDIR, 'CLI11.hpp'))
    args = parser.parse_args()
    MakeHeader(args.output)
