// Copyright (c) 2017-2026, University of Cincinnati, developed by Henry Schreiner
// under NSF AWARD 1414736 and by the respective contributors.
// All rights reserved.
//
// SPDX-License-Identifier: BSD-3-Clause

// Keep the includes before the import; GCC rejects textual includes that
// follow an import of a module whose global module fragment overlaps them.
#include <iostream>
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

    std::cout << "OK: import cli11 module\nvalue = " << value << '\n';

    return 0;
}
