// Copyright (c) 2017-2025, University of Cincinnati, developed by Henry Schreiner
// under NSF AWARD 1414736 and by the respective contributors.
// All rights reserved.
//
// SPDX-License-Identifier: BSD-3-Clause

#include "app_helper.hpp"

#include "catch.hpp"

#include <algorithm>
#include <complex>
#include <cstdint>
#include <cstdlib>
#include <deque>
#include <forward_list>
#include <limits>
#include <list>
#include <map>
#include <queue>
#include <set>
#include <string>
#include <tuple>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

using Catch::Matchers::WithinRel;

TEST_CASE_METHOD(TApp, "doubleFunction", "[optiontype]") {
    double res{0.0};
    app.add_option_function<double>("--val", [&res](double val) { res = std::abs(val + 54); });
    args = {"--val", "-354.356"};
    run();
    CHECK_THAT(res, WithinRel(300.356));
    // get the original value as entered as an integer
    CHECK_THAT(app["--val"]->as<float>(), WithinRel(-354.356f));
}

TEST_CASE_METHOD(TApp, "doubleFunctionFail", "[optiontype]") {
    double res = NAN;
    app.add_option_function<double>("--val", [&res](double val) { res = std::abs(val + 54); });
    args = {"--val", "not_double"};
    CHECK_THROWS_AS(run(), CLI::ConversionError);
}

TEST_CASE_METHOD(TApp, "doubleVectorFunction", "[optiontype]") {
    std::vector<double> res;
    app.add_option_function<std::vector<double>>("--val", [&res](const std::vector<double> &val) {
        res = val;
        std::transform(res.begin(), res.end(), res.begin(), [](double v) { return v + 5.0; });
    });
    args = {"--val", "5", "--val", "6", "--val", "7"};
    run();
    CHECK(3u == res.size());
    CHECK_THAT(res[0], WithinRel(10.0));
    CHECK_THAT(res[2], WithinRel(12.0));
}

TEST_CASE_METHOD(TApp, "doubleVectorFunctionFail", "[optiontype]") {
    std::vector<double> res;
    std::string vstring = "--val";
    app.add_option_function<std::vector<double>>(vstring, [&res](const std::vector<double> &val) {
        res = val;
        std::transform(res.begin(), res.end(), res.begin(), [](double v) { return v + 5.0; });
    });
    args = {"--val", "five", "--val", "nine", "--val", "7"};
    CHECK_THROWS_AS(run(), CLI::ConversionError);
    // check that getting the results through the results function generates the same error
    CHECK_THROWS_AS(app[vstring]->results(res), CLI::ConversionError);
    auto strvec = app[vstring]->as<std::vector<std::string>>();
    CHECK(3u == strvec.size());
}

TEST_CASE_METHOD(TApp, "doubleVectorFunctionRunCallbackOnDefault", "[optiontype]") {
    std::vector<double> res;
    auto *opt = app.add_option_function<std::vector<double>>("--val", [&res](const std::vector<double> &val) {
        res = val;
        std::transform(res.begin(), res.end(), res.begin(), [](double v) { return v + 5.0; });
    });
    args = {"--val", "5", "--val", "6", "--val", "7"};
    run();
    CHECK(3u == res.size());
    CHECK(10.0 == res[0]);
    CHECK(12.0 == res[2]);
    CHECK(!opt->get_run_callback_for_default());
    opt->run_callback_for_default();
    opt->default_val(std::vector<int>{2, 1, -2});
    CHECK(7.0 == res[0]);
    CHECK(3.0 == res[2]);

    CHECK_THROWS_AS(opt->default_val("this is a string"), CLI::ConversionError);
    auto vec = opt->as<std::vector<double>>();
    REQUIRE(3U == vec.size());
    CHECK(5.0 == vec[0]);
    CHECK(7.0 == vec[2]);
    opt->check(CLI::Number);
    opt->run_callback_for_default(false);
    CHECK_THROWS_AS(opt->default_val("this is a string"), CLI::ValidationError);
}

static const std::map<std::string, double> testValuesDouble{
    {"3.14159", 3.14159},
    {"-3.14159", -3.14159},
    {"-3.14159\t", -3.14159},
    {"-3.14159  ", -3.14159},
    {"+1.0", 1.0},
    {"-0.01", -0.01},
    {"-.01", -0.01},
    {"-.3251", -0.3251},
    {"+.3251", 0.3251},
    {"5e22", 5e22},
    {" 5e22", 5e22},
    {" 5e22  ", 5e22},
    {"-2E-2", -2e-2},
    {"5e+22", 5e22},
    {"1e06", 1e6},
    {"6.626e-34", 6.626e-34},
    {"6.626e+34", 6.626e34},
    {"-6.626e-34", -6.626e-34},
    {"224_617.445_991", 224617.445991},
    {"224'617.445'991", 224617.445991},
    {"inf", std::numeric_limits<double>::infinity()},
    {"+inf", std::numeric_limits<double>::infinity()},
    {"-inf", -std::numeric_limits<double>::infinity()},
    {"nan", std::numeric_limits<double>::signaling_NaN()},
    {"+nan", std::numeric_limits<double>::signaling_NaN()},
    {"-nan", -std::numeric_limits<double>::signaling_NaN()},

};

TEST_CASE_METHOD(TApp, "floatingConversions", "[optiontype]") {
    auto test_data = GENERATE(from_range(testValuesDouble));

    double val{0};
    app.add_option("--val", val);

    args = {"--val", test_data.first};

    run();
    if(std::isnan(test_data.second)) {
        CHECK(std::isnan(val));
    } else {

        CHECK_THAT(val, WithinRel(test_data.second, 1e-11));
    }
}

static const std::map<std::string, std::int64_t> testValuesInt{
    {"+99", 99},
    {"99", 99},
    {"-99", -99},
    {"-99 ", -99},
    {"0xDEADBEEF", 0xDEADBEEF},
    {"0xdeadbeef", 0xDEADBEEF},
    {"0XDEADBEEF", 0xDEADBEEF},
    {"0Xdeadbeef", 0xDEADBEEF},
    {"0xdead_beef", 0xDEADBEEF},
    {"0xdead'beef", 0xDEADBEEF},
    {"0o01234567", 001234567},
    {"0o755", 0755},
    {"0755", 0755},
    {"995862_262", 995862262},
    {"995862262", 995862262},
    {"-995862275", -995862275},
    {"\t-995862275\t", -995862275},
    {"-995'862'275", -995862275},
    {"0b11010110", 0xD6},
    {"0b1101'0110", 0xD6},
    {"0B11010110", 0xD6},
    {"0B1101'0110", 0xD6},
    {"1_2_3_4_5", 12345},
};

TEST_CASE_METHOD(TApp, "intConversions", "[optiontype]") {

    auto test_data = GENERATE(from_range(testValuesInt));

    std::int64_t val{0};
    app.add_option("--val", val);

    args = {"--val", test_data.first};

    run();

    CHECK(val == test_data.second);
}

TEST_CASE_METHOD(TApp, "intConversionsErange", "[optiontype]") {

    std::int64_t val{0};
    app.add_option("--val", val);

    args = {"--val", "0o11545241241415151512312415123125667"};

    CHECK_THROWS_AS(run(), CLI::ParseError);

    args = {"--val", "0b1011000001101011001100110011111000101010101011111111111111111111111001010111011100"};

    CHECK_THROWS_AS(run(), CLI::ParseError);

    args = {"--val", "0B1011000001101011001100110011111000101010101011111111111111111111111001010111011100"};

    CHECK_THROWS_AS(run(), CLI::ParseError);
}

static const std::map<std::string, std::uint64_t> testValuesUInt{
    {"+99", 99},
    {"99", 99},
    {" 99 ", 99},
    {"0xDEADBEEF", 0xDEADBEEF},
    {"0xdeadbeef", 0xDEADBEEF},
    {"0XDEADBEEF", 0xDEADBEEF},
    {"0Xdeadbeef", 0xDEADBEEF},
    {"0xdead_beef", 0xDEADBEEF},
    {"0xdead'beef", 0xDEADBEEF},
    {"0o01234567", 001234567},
    {"0o755", 0755},
    {"0o755\t", 0755},
    {"0755", 0755},
    {"995862_262", 995862262},
    {"995862262", 995862262},
    {"+995862275", +995862275},
    {"+995862275         \n\t", +995862275},
    {"995'862'275", 995862275},
    {"0b11010110", 0xD6},
    {"0b1101'0110", 0xD6},
    {"0b1101'0110                                                       ", 0xD6},
    {"0B11010110", 0xD6},
    {"0B1101'0110", 0xD6},
    {"1_2_3_4_5", 12345},
};

TEST_CASE_METHOD(TApp, "uintConversions", "[optiontype]") {

    auto test_data = GENERATE(from_range(testValuesUInt));

    std::uint64_t val{0};
    app.add_option("--val", val);

    args = {"--val", test_data.first};

    run();

    CHECK(val == test_data.second);
}

TEST_CASE_METHOD(TApp, "uintConversionsErange", "[optiontype]") {

    std::uint64_t val{0};
    app.add_option("--val", val);

    args = {"--val", "0o11545241241415151512312415123125667"};

    CHECK_THROWS_AS(run(), CLI::ParseError);

    args = {"--val", "0b1011000001101011001100110011111000101010101011111111111111111111111001010111011100"};

    CHECK_THROWS_AS(run(), CLI::ParseError);

    args = {"--val", "0B1011000001101011001100110011111000101010101011111111111111111111111001010111011100"};

    CHECK_THROWS_AS(run(), CLI::ParseError);
}

TEST_CASE_METHOD(TApp, "CharOption", "[optiontype]") {
    char c1{'t'};
    app.add_option("-c", c1);

    args = {"-c", "g"};
    run();
    CHECK('g' == c1);

    args = {"-c", "1"};
    run();
    CHECK('1' == c1);

    args = {"-c", "77"};
    run();
    CHECK(77 == c1);

    // convert hex for digit
    args = {"-c", "0x44"};
    run();
    CHECK(0x44 == c1);

    args = {"-c", "751615654161688126132138844896646748852"};
    CHECK_THROWS_AS(run(), CLI::ConversionError);
}

TEST_CASE_METHOD(TApp, "IntegerOverFlowShort", "[optiontype]") {
    std::int16_t A{0};
    std::uint16_t B{0};

    app.add_option("-a", A);
    app.add_option("-b", B);

    args = {"-a", "2626254242"};
    CHECK_THROWS_AS(run(), CLI::ConversionError);

    args = {"-b", "2626254242"};
    CHECK_THROWS_AS(run(), CLI::ConversionError);

    args = {"-b", "-26262"};
    CHECK_THROWS_AS(run(), CLI::ConversionError);

    args = {"-b", "-262624262525"};
    CHECK_THROWS_AS(run(), CLI::ConversionError);
}

TEST_CASE_METHOD(TApp, "IntegerOverFlowInt", "[optiontype]") {
    int A{0};
    unsigned int B{0};

    app.add_option("-a", A);
    app.add_option("-b", B);

    args = {"-a", "262625424225252"};
    CHECK_THROWS_AS(run(), CLI::ConversionError);

    args = {"-b", "262625424225252"};
    CHECK_THROWS_AS(run(), CLI::ConversionError);

    args = {"-b", "-2626225252"};
    CHECK_THROWS_AS(run(), CLI::ConversionError);

    args = {"-b", "-26262426252525252"};
    CHECK_THROWS_AS(run(), CLI::ConversionError);
}

TEST_CASE_METHOD(TApp, "IntegerOverFlowLong", "[optiontype]") {
    std::int32_t A{0};
    std::uint32_t B{0};

    app.add_option("-a", A);
    app.add_option("-b", B);

    args = {"-a", "1111111111111111111111111111"};
    CHECK_THROWS_AS(run(), CLI::ConversionError);

    args = {"-b", "1111111111111111111111111111"};
    CHECK_THROWS_AS(run(), CLI::ConversionError);

    args = {"-b", "-2626225252"};
    CHECK_THROWS_AS(run(), CLI::ConversionError);

    args = {"-b", "-111111111111111111111111"};
    CHECK_THROWS_AS(run(), CLI::ConversionError);
}

TEST_CASE_METHOD(TApp, "IntegerOverFlowLongLong", "[optiontype]") {
    std::int64_t A{0};
    std::uint64_t B{0};

    app.add_option("-a", A);
    app.add_option("-b", B);

    args = {"-a", "1111111111111111111111111111111111111111111111111111111111"};
    CHECK_THROWS_AS(run(), CLI::ConversionError);

    args = {"-b", "1111111111111111111111111111111111111111111111111111111111"};
    CHECK_THROWS_AS(run(), CLI::ConversionError);

    args = {"-b", "-2626225252"};
    CHECK_THROWS_AS(run(), CLI::ConversionError);

    args = {"-b", "-111111111111111111111111111111111111111111111111111111111"};
    CHECK_THROWS_AS(run(), CLI::ConversionError);
}

// now with tuple support this is possible
TEST_CASE_METHOD(TApp, "floatPair", "[optiontype]") {

    std::pair<float, float> custom_opt;

    auto *opt = app.add_option("--fp", custom_opt)->delimiter(',');
    opt->default_str("3.4,2.7");

    args = {"--fp", "12", "1.5"};

    run();
    CHECK(12.0f == Approx(custom_opt.first));
    CHECK(1.5f == Approx(custom_opt.second));
    args = {};
    opt->force_callback();
    run();
    CHECK(3.4f == Approx(custom_opt.first));
    CHECK(2.7f == Approx(custom_opt.second));
}

// now with tuple support this is possible
TEST_CASE_METHOD(TApp, "doubleVector", "[optiontype]") {

    std::vector<double> custom_opt;

    app.add_option("--fp", custom_opt);

    args = {"--fp", "12.7", "1.5"};
    run();
    CHECK(12.7 == Approx(custom_opt[0]));
    CHECK(1.5 == Approx(custom_opt[1]));
    args = {"--fp", "12.7", "-.5"};
    run();
    CHECK(12.7 == Approx(custom_opt[0]));
    CHECK(-0.5 == Approx(custom_opt[1]));

    args = {"--fp", "-.7", "+.5"};
    run();
    CHECK(-0.7 == Approx(custom_opt[0]));
    CHECK(0.5 == Approx(custom_opt[1]));
}
