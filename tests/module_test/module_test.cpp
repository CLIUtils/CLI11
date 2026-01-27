// Copyright (c) 2017-2026, University of Cincinnati, developed by Henry Schreiner
// under NSF AWARD 1414736 and by the respective contributors.
// All rights reserved.
//
// SPDX-License-Identifier: BSD-3-Clause

import cli11;

#include <print>
#include <string>

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

    std::println("OK: import cli11 module\nvalue = {}", value);

    return 0;
}
