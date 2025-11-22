// Copyright (c) 2024 scivision
// Copyright (c) 2025 University of Cincinnati, developed by Henry Schreiner
// under NSF AWARD 1414736 and by the respective contributors.
// All rights reserved.
//
// SPDX-License-Identifier: MIT

// modified from https://github.com/iTrooz/CppModules/blob/cli11 for use in CLI11 tests

#include <cassert>
#include <cstdio>
#include <cstdlib>

import cmodule;

int main() {
    int a = 1;
    int b = 2;

    int absum = add(a, b);
    int abdif = subtract(a, b);

    assert(a + b == absum);
    assert(a - b == abdif);

    // used this instead of <iostream> to work with older compilers that may choke on <iostream> implicit includes
    printf("OK: export module\n");

    return EXIT_SUCCESS;
}
