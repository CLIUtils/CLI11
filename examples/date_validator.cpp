// Copyright (c) 2017-2025, University of Cincinnati, developed by Henry Schreiner
// under NSF AWARD 1414736 and by the respective contributors.
// All rights reserved.
//
// SPDX-License-Identifier: BSD-3-Clause

#include <CLI/CLI.hpp>
#include <ctime>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>

// Custom validator is an alias of Validator, the constructor takes a function that takes as input and returns a string
const CLI::CustomValidator ISO8601(
    [](std::string &input) {
        std::tm tm = {};
        std::istringstream ss(input);

        ss >> std::get_time(&tm, "%Y-%m-%dT%H:%M:%S");
        if(ss.fail()) {
            return std::string("Failed to parse time string");
        }
        return std::string{};
    },
    "datetime[%Y-%m-%dT%H:%M:%S]");

int main(int argc, char **argv) {

    CLI::App app("custom validator testing");

    std::string value;
    app.add_option("--time", value, "enter a date in iso8601 format")->check(ISO8601)->required();

    CLI11_PARSE(app, argc, argv);

    std::cout << "date given = " << value << '\n';

    return 0;
}
