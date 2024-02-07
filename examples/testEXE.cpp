// Copyright (c) 2017-2024, University of Cincinnati, developed by Henry Schreiner
// under NSF AWARD 1414736 and by the respective contributors.
// All rights reserved.
//
// SPDX-License-Identifier: BSD-3-Clause

// Code modified from https://github.com/CLIUtils/CLI11/issues/559

#include <CLI/CLI.hpp>
#include <filesystem>
#include <iostream>
#include <string>

int main(int argc, char *argv[]) {
    CLI::App app{"MyApp"};
    std::filesystem::path binpath1 = "../example/myfile.bin";
    std::filesystem::path binpath2 = "../example/myfile.bin";
    std::filesystem::path binpath3 = "../example/myfile.bin";

    app.add_option("-f,--file1",
                   binpath1,
                   "Path to a .bin file");  // this works but the path "../example/myfile.bin" won't show up in --help.
                                            // I need it to show somewhere that this is a required argument but it's
                                            // populated with a default value - running the app.exe without the arg will
                                            // work and use the default value, but the user can overwrite it.

    app.add_option("-g,--file2", binpath2, "Path to a .bin file")
        ->capture_default_str();  // this doesn't seem to change anything, and I don't understand the documentation,
                                  // it's not clear to me what it says.

    app.add_option("-i,--file3", binpath3, "Path to a .bin file")
        ->default_str("defFile.bin")
        ->check(CLI::ExistingFile);  // this doesn't seem to change anything either, and I also don't understand the
                                     // documentation, it's not clear to me what it should do or change.

    try {
        app.parse(argc, argv);
    } catch(const CLI::ParseError &e) {
        return app.exit(e);
    }
}
