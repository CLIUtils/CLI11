// Copyright (c) 2017-2024, University of Cincinnati, developed by Henry Schreiner
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
            CHECK(true);
        }
    } catch(const CLI::ConstructionError & /*e*/) {
        CHECK(true);
    }
}

TEST_CASE("file_fail") {
    CLI::FuzzApp fuzzdata;
    auto app = fuzzdata.generateApp();

    int index = GENERATE(range(1, 9));
    auto parseData = loadFailureFile("fuzz_file_fail", index);
    std::stringstream out(parseData);
    try {
        app->parse_from_stream(out);
    } catch(const CLI::ParseError & /*e*/) {
        CHECK(true);
    }
}

TEST_CASE("app_file_gen_fail") {
    CLI::FuzzApp fuzzdata;
    auto app = fuzzdata.generateApp();

    int index = GENERATE(range(1, 41));
    std::string optionString, flagString;
    auto parseData = loadFailureFile("fuzz_app_file_fail", index);
    if(parseData.size() > 25) {
        optionString = parseData.substr(0, 25);
        parseData.erase(0, 25);
    }
    if(parseData.size() > 25) {
        flagString = parseData.substr(0, 25);
        parseData.erase(0, 25);
    }
    try {

        if(!optionString.empty()) {
            app->add_option(optionString, fuzzdata.buffer);
        }
        if(!flagString.empty()) {
            app->add_flag(flagString, fuzzdata.intbuffer);
        }
        try {
            app->parse(parseData);
        } catch(const CLI::ParseError & /*e*/) {
            return;
        }
    } catch(const CLI::ConstructionError & /*e*/) {
        return;
    }
    std::string configOut = app->config_to_str();
    app->clear();
    std::stringstream out(configOut);
    app->parse_from_stream(out);
}
