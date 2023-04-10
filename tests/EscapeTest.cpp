// Copyright (c) 2023, Scott Bailey
// All rights reserved.
//
// SPDX-License-Identifier: BSD-3-Clause

#include "app_helper.hpp"

#include <cstdio>
#include <sstream>


TEST_CASE_METHOD(TApp, "NoEscape1", "[escape]") {
    // Ensure an positional argument isn't split.
    std::string str;
    app.add_option("-s,--string,string", str);
    // example command line: `exe string\ one` or `exe "string one"`
    std::vector<std::string> cli_args = {"string one"};
    std::reverse(cli_args.begin(),cli_args.end());
    app.parse(cli_args);
    CHECK("string one" == str);
}

TEST_CASE_METHOD(TApp, "NoEscape2", "[escape]") {
    // Ensure argument isn't split.
    std::string str;
    app.add_option("-s,--string,string", str);
    // Example command line: `exe -s string\ one` or `exe -s "string one"`
    std::vector<std::string> cli_args = {"-s","string one"};
    std::reverse(cli_args.begin(),cli_args.end());
    app.parse(cli_args);
    CHECK("string one" == str);
}

TEST_CASE_METHOD(TApp, "NoEscape3", "[escape]") {
    // Ensure our argument isn't misinterpreted.
    std::string str;
    app.add_option("-s,--string,string", str);
    // Example command line: `exe -s -s`
    std::vector<std::string> cli_args = {"-s","-s"};
    std::reverse(cli_args.begin(),cli_args.end());
    app.parse(cli_args);
    CHECK("-s" == str);
}

TEST_CASE_METHOD(TApp, "NoEscape4", "[escape]") {
    // Ensure our positional isn't misinterpreted.
    std::string str;
    app.add_option("-s,--string,string", str);
    // Example command line: `exe string`
    std::vector<std::string> cli_args = {"string"};
    std::reverse(cli_args.begin(),cli_args.end());
    app.parse(cli_args);
    CHECK("string" == str);
}

TEST_CASE_METHOD(TApp, "Escape1", "[escape]") {
    // Ensure escaped argument is resolved correctly.
    std::string str;
    app.add_option("-s,--string,string", str);
    // Example command line: `exe -s \\-s` or `exe -s "\-s"`
    std::vector<std::string> cli_args = {"-s",R"(\-s)"};
    std::reverse(cli_args.begin(),cli_args.end());
    app.parse(cli_args);
    CHECK("-s" == str);
}

TEST_CASE_METHOD(TApp, "Escape2", "[escape]") {
    // Ensure escaped positional is resolved correctly.
    std::string str;
    app.add_option("-s,--string,string", str);
    // Example command line: `exe \\-s` or `exe "\-s"`
    std::vector<std::string> cli_args = {R"(\-s)"};
    std::reverse(cli_args.begin(),cli_args.end());
    app.parse(cli_args);
    CHECK("-s" == str);
}

TEST_CASE_METHOD(TApp, "Escape3", "[escape]") {
    // Ensure escaped argument is resolved correctly.
    std::string str;
    app.add_option("-s,--string,string", str);
    // Example command line: `exe -s \\\-s` or `exe -s "\\-s"`
    std::vector<std::string> cli_args = {"-s",R"(\\-s)"};
    std::reverse(cli_args.begin(),cli_args.end());
    app.parse(cli_args);
    CHECK(R"(\-s)" == str);
}

TEST_CASE_METHOD(TApp, "Escape4", "[escape]") {
    // Ensure escaped positional is resolved correctly.
    std::string str;
    app.add_option("-s,--string,string", str);
    // Example command line: `exe \\\-s` or `exe "\\-s"`
    std::vector<std::string> cli_args = {R"(\\-s)"};
    std::reverse(cli_args.begin(),cli_args.end());
    app.parse(cli_args);
    CHECK(R"(\-s)" == str);
}

TEST_CASE_METHOD(TApp, "Numbers", "[escape]") {
    // Ensure negative numbers are resolved correctly.
    std::string str;
    app.add_option("--number", str);
    // Example command line: `exe \\\-s` or `exe "\\-s"`
    std::vector<std::string> cli_args = {"--number",R"(-10)"};
    std::reverse(cli_args.begin(),cli_args.end());
    app.parse(cli_args);
    CHECK("-10" == str);
}
