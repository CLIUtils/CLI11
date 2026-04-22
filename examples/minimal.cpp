// Copyright (c) 2017-2026, University of Cincinnati, developed by Henry Schreiner
// under NSF AWARD 1414736 and by the respective contributors.
// All rights reserved.
//
// SPDX-License-Identifier: BSD-3-Clause

// Code modified from https://github.com/CLIUtils/CLI11/issues/1334

#include <CLI/CLI.hpp>

/** This example is a minimal example of using CLI11. It does not do anything, but it compiles and runs, and can be used
 * as a starting point for new projects.
 */
int main(int argc, char** argv) {

   
    CLI::App app{"App description"};
    CLI11_PARSE(app, argc, argv);
    return 0;
}
