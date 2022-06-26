// Copyright (c) 2017-2022, University of Cincinnati, developed by Henry Schreiner
// under NSF AWARD 1414736 and by the respective contributors.
// All rights reserved.
//
// SPDX-License-Identifier: BSD-3-Clause

// Code modified from https://github.com/CLIUtils/CLI11/issues/559

#include <CLI/CLI.hpp>
#include <iostream>
#include <string>

int main(int argc, const char *argv[]) {

    int logLevel{0};
    CLI::App app{"Test App"};

    app.add_option("-v", logLevel, "level");

    auto *subcom = app.add_subcommand("sub", "")->fallthrough();
    subcom->preparse_callback([&app](size_t) { app.get_subcommand("sub")->add_option_group("group"); });

    CLI11_PARSE(app, argc, argv);

    std::cout << "level: " << logLevel << std::endl;
}
