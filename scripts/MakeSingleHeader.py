#!/usr/bin/env python

# Requires Python 3.6

from plumbum import local, cli, FG
import re

includes_local = re.compile(r"""^#include "(.*)"$""", re.MULTILINE)
includes_system = re.compile(r"""^#include \<(.*)\>$""", re.MULTILINE)

DIR = local.path(__file__).dirname
BDIR = DIR / '../include'

class MakeHeader(cli.Application):

    def main(self, out : cli.NonexistentPath = BDIR / 'CLI11.hpp'):
        main_header = BDIR / 'CLI/CLI.hpp'
        header = main_header.read()

        include_files = includes_local.findall(header)

        headers = set()
        output = ''
        with open('output.hpp', 'w') as f:
            for inc in include_files:
                inner = (BDIR / inc).read()
                headers |= set(includes_system.findall(inner))
                output += f'\n// From {inc}\n\n'
                output += inner[inner.find('namespace'):]

        header_list = '\n'.join(f'#include <{h}>' for h in headers)

        output = f'''\
#pragma once

// Distributed under the LGPL version 3.0 license.  See accompanying
// file LICENSE or https://github.com/henryiii/CLI11 for details.

// This file was generated using MakeSingleHeader.py in CLI11/scripts
// This has the complete CLI library in one file.

{header_list}
{output}'''

        with out.open('w') as f:
            f.write(output)

        print(f"Created {out}")

if __name__ == '__main__':
    MakeHeader()


