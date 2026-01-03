// Copyright (c) 2025 University of Cincinnati, developed by Henry Schreiner
// under NSF AWARD 1414736 and by the respective contributors.
// All rights reserved.
//
// SPDX-License-Identifier: MIT

// modified from https://github.com/iTrooz/CppModules/blob/cli11 for use in CLI11 tests

#include <cassert>
#include <cstdlib>
#include <print>
#include <string>

import cli11;

using std::string;

using CLI::App;
using CLI::ParseError;

int main(int argc, char *argv[]) {
    App app{"Module test"};

    string value;
    app.add_option("value", value, "A test value")->required();

    try {
        app.parse(argc, argv);
    } catch(const ParseError &e) {
        return app.exit(e);
    }

    assert(!value.empty());

    std::println("OK: export module");

    return EXIT_SUCCESS;
}
