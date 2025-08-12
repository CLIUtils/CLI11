// Copyright (c) 2017-2025, University of Cincinnati, developed by Henry Schreiner
// under NSF AWARD 1414736 and by the respective contributors.
// All rights reserved.
//
// SPDX-License-Identifier: BSD-3-Clause

//this definition won't be necessary until 3.0 but is included for now
#define CLI11_ENABLE_EXTRA_VALIDATORS 1

#include <CLI/CLI.hpp>
#include <iostream>
#include <string>

int main(int argc, char **argv) {

    CLI::App app("Validator checker");

    std::string file;
    app.add_option("-f,--file,file", file, "File name")->check(CLI::ExistingFile);

    int count{0};
    app.add_option("-v,--value", count, "Value in range")->check(CLI::Range(3, 6));
    CLI11_PARSE(app, argc, argv);

    std::cout << "Try printing help or failing the validator" << '\n';

    return 0;
}
