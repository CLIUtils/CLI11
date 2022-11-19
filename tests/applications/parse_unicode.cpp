// Copyright (c) 2017-2022, University of Cincinnati, developed by Henry Schreiner
// under NSF AWARD 1414736 and by the respective contributors.
// All rights reserved.
//
// SPDX-License-Identifier: BSD-3-Clause

#include <CLI/CLI.hpp>
#include <fstream>
#include <string>

int main(int argc, char **argv) {
    CLI::App app;
    std::string str;

    app.add_option("str", str, "utf-8 string");
    CLI11_PARSE(app);

    std::ofstream{"parse_unicode.out.txt"} << str;
}
