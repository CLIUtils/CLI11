// Copyright (c) 2017-2026, University of Cincinnati, developed by Henry Schreiner
// under NSF AWARD 1414736 and by the respective contributors.
// All rights reserved.
//
// SPDX-License-Identifier: BSD-3-Clause

// Simple example to demonstrate envname display in help

#include <CLI/CLI.hpp>
#include <iostream>

int main(int argc, char **argv) {
    CLI::App app{"Demonstration of environment variable name in help output"};

    std::string server_host = "localhost";
    int port = 8080;
    std::string config_file;
    bool verbose = false;

    // Option with envname, default value and description
    app.add_option("-H,--host", server_host, "Server host address")
        ->envname("APP_HOST")
        ->default_str("localhost");

    // Option with envname and default value
    app.add_option("-p,--port", port, "Server port")
        ->envname("APP_PORT")
        ->default_str("8080");

    // Option with envname only
    app.add_option("-c,--config", config_file, "Configuration file path")
        ->envname("APP_CONFIG");

    // Option without envname (for comparison)
    app.add_flag("-v,--verbose", verbose, "Enable verbose output");

    CLI11_PARSE(app, argc, argv);

    std::cout << "Host: " << server_host << "\n";
    std::cout << "Port: " << port << "\n";
    if(!config_file.empty()) {
        std::cout << "Config: " << config_file << "\n";
    }
    std::cout << "Verbose: " << (verbose ? "yes" : "no") << "\n";

    return 0;
}
