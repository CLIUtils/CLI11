// Copyright (c) 2017-2023, University of Cincinnati, developed by Henry Schreiner
// under NSF AWARD 1414736 and by the respective contributors.
// All rights reserved.
//
// SPDX-License-Identifier: BSD-3-Clause

#include "../fuzz/fuzzApp.hpp"
#include "app_helper.hpp"

std::string loadFailureFile(const std::string &type, int index) {
    std::string fileName(TEST_FILE_FOLDER "/fuzzFail/");
    fileName.append(type);
    fileName += std::to_string(index);
    std::ifstream crashFile(fileName, std::ios::in | std::ios::binary);
    if(crashFile) {
        std::vector<char> buffer(std::istreambuf_iterator<char>(crashFile), {});

        std::string cdata(buffer.begin(), buffer.end());
        return cdata;
    }
    return std::string{};
}

TEST_CASE("app_fail") {
    CLI::FuzzApp fuzzdata;
    auto app = fuzzdata.generateApp();

    int index = GENERATE(range(1, 4));
    std::string optionString;
    auto parseData = loadFailureFile("fuzz_app_fail", index);
    if(index >= 3 && parseData.size() > 25) {
        optionString = parseData.substr(0, 25);
        parseData.erase(0, 25);
    }

    try {

        if(!optionString.empty()) {
            app->add_option(optionString, fuzzdata.buffer);
        }
        try {
            app->parse(parseData);
        } catch(const CLI::ParseError & /*e*/) {
        }
    } catch(const CLI::ConstructionError & /*e*/) {
    }
}

TEST_CASE("file_fail") {
    CLI::FuzzApp fuzzdata;
    auto app = fuzzdata.generateApp();

    int index = GENERATE(range(1, 2));
    auto parseData = loadFailureFile("fuzz_file_fail", index);
    std::stringstream out(parseData);
    try {
        app->parse_from_stream(out);
    } catch(const CLI::ParseError & /*e*/) {
    }
}
