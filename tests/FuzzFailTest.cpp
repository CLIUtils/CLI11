// Copyright (c) 2017-2025, University of Cincinnati, developed by Henry Schreiner
// under NSF AWARD 1414736 and by the respective contributors.
// All rights reserved.
//
// SPDX-License-Identifier: BSD-3-Clause

#include "../fuzz/fuzzApp.hpp"
#include "app_helper.hpp"
#include <iostream>
#include <string>
#include <vector>

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

// this test uses the same tests as above just with a full roundtrip test
TEST_CASE("app_file_roundtrip") {
    CLI::FuzzApp fuzzdata;
    CLI::FuzzApp fuzzdata2;
    auto app = fuzzdata.generateApp();
    auto app2 = fuzzdata2.generateApp();
    int index = GENERATE(range(1, 41));
    // int index = GENERATE(range(8, 9));
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
            app2->add_option(optionString, fuzzdata2.buffer);
        }
        if(!flagString.empty()) {
            app->add_flag(flagString, fuzzdata.intbuffer);
            app2->add_flag(flagString, fuzzdata2.intbuffer);
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
    std::stringstream out(configOut);
    app2->parse_from_stream(out);
    bool result = fuzzdata2.compare(fuzzdata);
    /*
    if (!result)
    {
        configOut = app->config_to_str();
        result = fuzzdata2.compare(fuzzdata);
    }
    */
    INFO("Failure in test case " << index)
    CHECK(result);
}

// this test uses the same tests as above just with a full roundtrip test
TEST_CASE("app_roundtrip") {
    CLI::FuzzApp fuzzdata;
    CLI::FuzzApp fuzzdata2;
    auto app = fuzzdata.generateApp();
    auto app2 = fuzzdata2.generateApp();
    int index = GENERATE(range(1, 5));
    std::string optionString, flagString;
    auto parseData = loadFailureFile("round_trip_fail", index);
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
            app2->add_option(optionString, fuzzdata2.buffer);
        }
        if(!flagString.empty()) {
            app->add_flag(flagString, fuzzdata.intbuffer);
            app2->add_flag(flagString, fuzzdata2.intbuffer);
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
    std::stringstream out(configOut);
    app2->parse_from_stream(out);
    bool result = fuzzdata2.compare(fuzzdata);
    /*
    if (!result)
    {
    configOut = app->config_to_str();
    result = fuzzdata2.compare(fuzzdata);
    }
    */
    CHECK(result);
}

// same as above but just a single test for debugging
TEST_CASE("app_roundtrip_single") {
    CLI::FuzzApp fuzzdata;
    CLI::FuzzApp fuzzdata2;
    auto app = fuzzdata.generateApp();
    auto app2 = fuzzdata2.generateApp();
    int index = 5;
    std::string optionString, flagString;
    auto parseData = loadFailureFile("round_trip_fail", index);
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
            app2->add_option(optionString, fuzzdata2.buffer);
        }
        if(!flagString.empty()) {
            app->add_flag(flagString, fuzzdata.intbuffer);
            app2->add_flag(flagString, fuzzdata2.intbuffer);
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
    std::stringstream out(configOut);
    app2->parse_from_stream(out);
    bool result = fuzzdata2.compare(fuzzdata);
    /*
    if (!result)
    {
    configOut = app->config_to_str();
    result = fuzzdata2.compare(fuzzdata);
    }
    */
    CHECK(result);
}

/** test the fuzzer itself to support custom options*/
TEST_CASE("fuzz_config_test1") {
    CLI::FuzzApp fuzzdata;
    auto app = fuzzdata.generateApp();

    std::string config_string = "<option>--new_option</option><flag>--new_flag</flag><vector>--new_vector</vector>";
    auto loc = fuzzdata.add_custom_options(app.get(), config_string);
    config_string = config_string.substr(loc);
    CHECK(config_string.empty());
    CHECK(app->get_option_no_throw("--new_option") != nullptr);
    CHECK(app->get_option_no_throw("--new_flag") != nullptr);
    CHECK(app->get_option_no_throw("--new_vector") != nullptr);
}

/** test the fuzzer itself to support custom options*/
TEST_CASE("fuzz_config_test2") {
    CLI::FuzzApp fuzzdata;
    auto app = fuzzdata.generateApp();

    std::string config_string =
        "<option>--new_option</option><flag>--new_flag</flag><vector>--new_vector</vector> --new_flag --new_option 10";
    auto loc = fuzzdata.add_custom_options(app.get(), config_string);
    config_string = config_string.substr(loc);
    CHECK(!config_string.empty());
    CHECK(config_string == " --new_flag --new_option 10");
    CHECK(app->get_option_no_throw("--new_option") != nullptr);
    CHECK(app->get_option_no_throw("--new_flag") != nullptr);
    CHECK(app->get_option_no_throw("--new_vector") != nullptr);
}

/** test the fuzzer itself to support custom option modifiers*/
TEST_CASE("fuzz_config_modifier_test1") {
    CLI::FuzzApp fuzzdata;
    auto app = fuzzdata.generateApp();

    std::string config_string = "<option  modifiers=R2CG>--new_option</option><flag "
                                "modifiers=cFg>--new_flag</flag><vector modifiers=35s+>--new_vector</vector>";
    auto loc = fuzzdata.add_custom_options(app.get(), config_string);
    config_string = config_string.substr(loc);
    CHECK(config_string.empty());
    auto *opt1 = app->get_option_no_throw("--new_option");
    REQUIRE(opt1 != nullptr);
    CHECK(opt1->get_required());
    CHECK(opt1->get_expected_min() == 2);
    CHECK(opt1->get_configurable());
    CHECK(opt1->get_ignore_case());
    auto *opt2 = app->get_option_no_throw("--new_flag");
    REQUIRE(opt2 != nullptr);
    CHECK(opt2->get_disable_flag_override());
    CHECK(!opt2->get_configurable());
    CHECK(!opt2->get_ignore_case());
    auto *opt3 = app->get_option_no_throw("--new_vector");
    REQUIRE(opt3 != nullptr);
    CHECK(opt3->get_expected_min() == 0);
    CHECK(opt3->get_expected_max() == 3);
    CHECK(opt3->get_multi_option_policy() == CLI::MultiOptionPolicy::Sum);
}

/** test the fuzzer itself to support custom subcommand*/
TEST_CASE("fuzz_config_subcommand") {
    CLI::FuzzApp fuzzdata;
    auto app = fuzzdata.generateApp();

    std::string config_string =
        "<subcommand name=sub_custom><option  modifiers=R2CG>--new_option</option><flag "
        "modifiers=cFg>--new_flag</flag><vector modifiers=35s+>--new_vector</vector></subcommand>";
    auto loc = fuzzdata.add_custom_options(app.get(), config_string);
    config_string = config_string.substr(loc);
    CHECK(config_string.empty());
    auto *sub_c = app->get_subcommand("sub_custom");
    auto *opt1 = sub_c->get_option_no_throw("--new_option");
    REQUIRE(opt1 != nullptr);
    CHECK(opt1->get_required());
    CHECK(opt1->get_expected_min() == 2);
    CHECK(opt1->get_configurable());
    CHECK(opt1->get_ignore_case());
    auto *opt2 = sub_c->get_option_no_throw("--new_flag");
    REQUIRE(opt2 != nullptr);
    CHECK(opt2->get_disable_flag_override());
    CHECK(!opt2->get_configurable());
    CHECK(!opt2->get_ignore_case());
    auto *opt3 = sub_c->get_option_no_throw("--new_vector");
    REQUIRE(opt3 != nullptr);
    CHECK(opt3->get_expected_min() == 0);
    CHECK(opt3->get_expected_max() == 3);
    CHECK(opt3->get_multi_option_policy() == CLI::MultiOptionPolicy::Sum);
}

// this test enables the custom option creation operation
TEST_CASE("app_roundtrip_custom") {
    CLI::FuzzApp fuzzdata;
    CLI::FuzzApp fuzzdata2;
    auto app = fuzzdata.generateApp();
    auto app2 = fuzzdata2.generateApp();
    int index = GENERATE(range(1, 24));

    auto parseData = loadFailureFile("round_trip_custom", index);

    std::size_t pstring_start{0};
    pstring_start = fuzzdata.add_custom_options(app.get(), parseData);
    INFO("Failure in test case " << index << " file length=" << parseData.size() << " pstring start at "
                                 << pstring_start)
    if(pstring_start > 0) {
        app->parse(parseData.substr(pstring_start));
        CHECK_NOTHROW(app->help("", CLI::AppFormatMode::All));
    } else {
        app->parse(parseData);
    }
    if(fuzzdata.supports_config_file()) {
        // should be able to write the config to a file and read from it again
        std::string configOut = app->config_to_str();
        std::stringstream out(configOut);
        if(pstring_start > 0) {
            fuzzdata2.add_custom_options(app2.get(), parseData);
        }
        CHECK_NOTHROW(app2->parse_from_stream(out));
        auto result = fuzzdata2.compare(fuzzdata);
        if(!result) {
            result = fuzzdata.compare(fuzzdata2, true);
            std::cout << "\n:parsed:\n" << parseData;
            std::cout << "\n:config:\n" << configOut << '\n';
        }
        CHECK(result);
    }
}

// this test
TEST_CASE("app_roundtrip_parse_normal_fail") {
    // this is mostly checking that no unexpected errors occur
    // like HorribleErrors
    CLI::FuzzApp fuzzdata;
    auto app = fuzzdata.generateApp();
    int index = GENERATE(range(1, 11));
    auto parseData = loadFailureFile("parse_fail_check", index);
    std::size_t pstring_start{0};
    try {
        pstring_start = fuzzdata.add_custom_options(app.get(), parseData);
    } catch(const CLI::ConstructionError & /*ce*/) {
        CHECK(true);
        return;
    }
    INFO("Failure in test case " << index)
    try {
        if(pstring_start > 0) {
            app->parse(parseData.substr(pstring_start));
        } else {
            app->parse(parseData);
        }
    } catch(const CLI::HorribleError & /*he*/) {
        CHECK(false);
        return;
    } catch(const CLI::ParseError & /*e*/) {
        CHECK(true);
        return;
    }
    try {
        // should be able to write the config to a file and read from it again
        std::string configOut = app->config_to_str();
        app->clear();
        std::stringstream out(configOut);
        app->parse_from_stream(out);
    } catch(const CLI::HorribleError & /*he*/) {
        CHECK(false);
        return;
    } catch(const CLI::ParseError & /*e*/) {
        CHECK(false);
        return;
    }
}
