// Copyright (c) 2017-2025, University of Cincinnati, developed by Henry Schreiner
// under NSF AWARD 1414736 and by the respective contributors.
// All rights reserved.
//
// SPDX-License-Identifier: BSD-3-Clause

// Code modified from Loic Gouarin(https://github.com/gouarin) https://github.com/CLIUtils/CLI11/issues/1135

#include <CLI/CLI.hpp>
#include <array>
#include <iostream>

int main(int argc, char **argv) {
    std::array<int, 2> a{0, 1};
    CLI::App app{"My app"};
    app.add_option("--a", a, "an array")->capture_default_str();
    app.parse(argc, argv);

    std::cout << "pass\n";
    return 0;
}
