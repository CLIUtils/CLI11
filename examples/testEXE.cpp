// Copyright (c) 2017-2024, University of Cincinnati, developed by Henry Schreiner
// under NSF AWARD 1414736 and by the respective contributors.
// All rights reserved.
//
// SPDX-License-Identifier: BSD-3-Clause

// Code modified from https://github.com/CLIUtils/CLI11/issues/559

#include <CLI/CLI.hpp>
#include <iostream>
#include <optional>

int main(int argc, const char *argv[]) {

    CLI::App app;

    bool flag = false;
    std::optional<bool> optional_flag = std::nullopt;

    app.add_option("--tester");
    auto *m1=app.add_option_group("+tester");

    m1->add_option("--flag", flag, "description");
    m1->add_option("--optional_flag", optional_flag, "description");

    CLI11_PARSE(app,argc, argv);

    std::cout << "flag: " << flag << std::endl;

    if(optional_flag){
        std::cout << "optional flag: " << optional_flag.value() << std::endl;
    }
}
