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
    std::string script_path{};
    std::string toolchain_path{};
    std::string build_dir{};
    std::string install_dir{};
    std::string output{};
    App app{"Module test"};

    string value;
    app.add_option("value", value, "A test value")->required();
    // test with long and short options and subcommands
    auto config = app.add_subcommand("configure", "Configure the project");
    config->add_option("-S,--script-path", script_path, "Path to build script");
    config->add_option("-T,--toolchain-path", toolchain_path, "Compiler toolchain to use");
    config->add_option("-B,--build-dir", build_dir, "Build directory");
    // Build Subcommand
    auto build = app.add_subcommand("build", "Execute build targets");
    build->add_option("-B,--build-dir", build_dir, "Build directory");
    // Install Subcommand
    auto install = app.add_subcommand("install", "Install build artifacts");
    install->add_option("-B,--build-dir", build_dir, "Source build directory");
    install->add_option("-I,--install-dir", install_dir, "Installation prefix");
    // Scan Subcommand
    auto scan = app.add_subcommand("scan", "Scan for source changes/dependencies");
    scan->add_option("-B,--build-dir", build_dir, "Build directory");
    // test with option group
    auto group = app.add_option_group("output group", "type of output");
    group->add_option("-o,--output", value, "output type");

    try {
        app.parse(argc, argv);
    } catch(const ParseError &e) {
        return app.exit(e);
    }

    std::println("OK: import cli11 module\nvalue = {}", value);

    return 0;
}
