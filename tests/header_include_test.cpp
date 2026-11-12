// Copyright (c) 2017-2026, University of Cincinnati, developed by Henry Schreiner
// under NSF AWARD 1414736 and by the respective contributors.
// All rights reserved.
//
// SPDX-License-Identifier: BSD-3-Clause

#include "../include/CLI/CLI.hpp"

int main() {
    CLI::App app;
    bool flag{false};
    app.add_flag("--flag", flag);
    app.parse("--flag");
    return flag ? 0 : 1;
}
