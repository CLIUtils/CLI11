#!/usr/bin/env python

# Requires pathlib on python 2

import re
import argparse
from pathlib import Path

includes_local = re.compile(r"""^#include "(.*)"$""", re.MULTILINE)
includes_system = re.compile(r"""^#include \<(.*)\>$""", re.MULTILINE)

DIR = Path(__file__).resolve().parent
BDIR = DIR.parent / 'include'

def MakeHeader(out):
    main_header = BDIR / 'CLI/CLI.hpp'
    with main_header.open() as f:
        header = f.read()

    include_files = includes_local.findall(header)

    headers = set()
    output = ''
    with open('output.hpp', 'w') as f:
        for inc in include_files:
            with (BDIR / inc).open() as f:
                inner = f.read()
            headers |= set(includes_system.findall(inner))
            output += '\n// From {inc}\n\n'.format(inc=inc)
            output += inner[inner.find('namespace'):]

    header_list = '\n'.join(f'#include <'+h+'>' for h in headers)

    output = '''\
#pragma once

// Distributed under the LGPL version 3.0 license.  See accompanying
// file LICENSE or https://github.com/henryiii/CLI11 for details.

// This file was generated using MakeSingleHeader.py in CLI11/scripts
// This has the complete CLI library in one file.

{header_list}
{output}'''.format(header_list=header_list, output=output)

    with Path(out).open('w') as f:
        f.write(output)

    print("Created {out}".format(out=out))


if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument("output", nargs='?', default=BDIR / 'CLI11.hpp')
    args = parser.parse_args()
    MakeHeader(args.output)
