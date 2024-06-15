// Copyright (c) 2017-2024, University of Cincinnati, developed by Henry Schreiner
// under NSF AWARD 1414736 and by the respective contributors.
// All rights reserved.
//
// SPDX-License-Identifier: BSD-3-Clause

#include "fuzzApp.hpp"
#include <CLI/CLI.hpp>
#include <cstring>
#include <exception>
#include <string>

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *Data, size_t Size) {
    if(Size == 0) {
        return 0;
    }
    std::string parseString(reinterpret_cast<const char *>(Data), Size);
    std::string optionString;
    std::string flagString;
    if(parseString.size() > 25) {
        optionString = parseString.substr(0, 25);
        parseString.erase(0, 25);
    }
    if(parseString.size() > 25) {
        flagString = parseString.substr(0, 25);
        parseString.erase(0, 25);
    }
    CLI::FuzzApp fuzzdata;
    auto app = fuzzdata.generateApp();
    try {
        if(!optionString.empty()) {
            app->add_option(optionString, fuzzdata.buffer);
        }
        if(!flagString.empty()) {
            app->add_flag(flagString, fuzzdata.intbuffer);
        }
    } catch(const CLI::ConstructionError &e) {
        return 0;  // Non-zero return values are reserved for future use.
    }

    try {
        app->parse(parseString);

    } catch(const CLI::ParseError &e) {
        //(app)->exit(e);
        // this just indicates we caught an error known by CLI
        return 0;  // Non-zero return values are reserved for future use.
    }
    // should be able to write the config to a file and read from it again
    std::string configOut = app->config_to_str();
    app->clear();
    std::stringstream out(configOut);
    app->parse_from_stream(out);
    return 0;
}
