// Copyright (c) 2017-2026, University of Cincinnati, developed by Henry Schreiner
// under NSF AWARD 1414736 and by the respective contributors.
// All rights reserved.
//
// SPDX-License-Identifier: BSD-3-Clause

#include "CLI/CLI.hpp"
#include <iostream>

int main(int argc, char **argv) {
    CLI::App app("Color formatter example");
    app.get_formatter()->enable_color();

    app.add_flag("--flag", "This is a flag");

    auto *sub1 = app.add_subcommand("one", "Description One");
    sub1->add_flag("--oneflag", "Some flag");
    auto *sub2 = app.add_subcommand("two", "Description Two");
    sub2->add_flag("--twoflag", "Some other flag");

    CLI11_PARSE(app, argc, argv);

    std::cout << "This app was meant to show off the colorful formatter, run with -h" << '\n';

    return 0;
}
