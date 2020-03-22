// Copyright (c) 2017-2020, University of Cincinnati, developed by Henry Schreiner
// under NSF AWARD 1414736 and by the respective contributors.
// All rights reserved.
//
// SPDX-License-Identifier: BSD-3-Clause

#include "app_helper.hpp"
#include <complex>
#include <cstdint>
#include <cstdlib>
#include <deque>
#include <forward_list>
#include <list>
#include <map>
#include <queue>
#include <set>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "gmock/gmock.h"

TEST_F(TApp, OneStringAgain) {
    std::string str;
    app.add_option("-s,--string", str);
    args = {"--string", "mystring"};
    run();
    EXPECT_EQ(1u, app.count("-s"));
    EXPECT_EQ(1u, app.count("--string"));
    EXPECT_EQ(str, "mystring");
}

TEST_F(TApp, OneStringFunction) {
    std::string str;
    app.add_option_function<std::string>("-s,--string", [&str](const std::string &val) { str = val; });
    args = {"--string", "mystring"};
    run();
    EXPECT_EQ(1u, app.count("-s"));
    EXPECT_EQ(1u, app.count("--string"));
    EXPECT_EQ(str, "mystring");
}

TEST_F(TApp, doubleFunction) {
    double res{0.0};
    app.add_option_function<double>("--val", [&res](double val) { res = std::abs(val + 54); });
    args = {"--val", "-354.356"};
    run();
    EXPECT_EQ(res, 300.356);
    // get the original value as entered as an integer
    EXPECT_EQ(app["--val"]->as<float>(), -354.356f);
}

TEST_F(TApp, doubleFunctionFail) {
    double res;
    app.add_option_function<double>("--val", [&res](double val) { res = std::abs(val + 54); });
    args = {"--val", "not_double"};
    EXPECT_THROW(run(), CLI::ConversionError);
}

TEST_F(TApp, doubleVectorFunction) {
    std::vector<double> res;
    app.add_option_function<std::vector<double>>("--val", [&res](const std::vector<double> &val) {
        res = val;
        std::transform(res.begin(), res.end(), res.begin(), [](double v) { return v + 5.0; });
    });
    args = {"--val", "5", "--val", "6", "--val", "7"};
    run();
    EXPECT_EQ(res.size(), 3u);
    EXPECT_EQ(res[0], 10.0);
    EXPECT_EQ(res[2], 12.0);
}

TEST_F(TApp, doubleVectorFunctionFail) {
    std::vector<double> res;
    std::string vstring = "--val";
    app.add_option_function<std::vector<double>>(vstring, [&res](const std::vector<double> &val) {
        res = val;
        std::transform(res.begin(), res.end(), res.begin(), [](double v) { return v + 5.0; });
    });
    args = {"--val", "five", "--val", "nine", "--val", "7"};
    EXPECT_THROW(run(), CLI::ConversionError);
    // check that getting the results through the results function generates the same error
    EXPECT_THROW(app[vstring]->results(res), CLI::ConversionError);
    auto strvec = app[vstring]->as<std::vector<std::string>>();
    EXPECT_EQ(strvec.size(), 3u);
}

TEST_F(TApp, doubleVectorFunctionRunCallbackOnDefault) {
    std::vector<double> res;
    auto opt = app.add_option_function<std::vector<double>>("--val", [&res](const std::vector<double> &val) {
        res = val;
        std::transform(res.begin(), res.end(), res.begin(), [](double v) { return v + 5.0; });
    });
    args = {"--val", "5", "--val", "6", "--val", "7"};
    run();
    EXPECT_EQ(res.size(), 3u);
    EXPECT_EQ(res[0], 10.0);
    EXPECT_EQ(res[2], 12.0);
    EXPECT_FALSE(opt->get_run_callback_for_default());
    opt->run_callback_for_default();
    opt->default_val(std::vector<int>{2, 1, -2});
    EXPECT_EQ(res[0], 7.0);
    EXPECT_EQ(res[2], 3.0);

    EXPECT_THROW(opt->default_val("this is a string"), CLI::ConversionError);
    auto vec = opt->as<std::vector<double>>();
    ASSERT_EQ(vec.size(), 3U);
    EXPECT_EQ(vec[0], 5.0);
    EXPECT_EQ(vec[2], 7.0);
    opt->check(CLI::Number);
    opt->run_callback_for_default(false);
    EXPECT_THROW(opt->default_val("this is a string"), CLI::ValidationError);
}

TEST_F(TApp, BoolAndIntFlags) {

    bool bflag{false};
    int iflag{0};
    unsigned int uflag{0};

    app.add_flag("-b", bflag);
    app.add_flag("-i", iflag);
    app.add_flag("-u", uflag);

    args = {"-b", "-i", "-u"};
    run();
    EXPECT_TRUE(bflag);
    EXPECT_EQ(1, iflag);
    EXPECT_EQ((unsigned int)1, uflag);

    args = {"-b", "-b"};
    ASSERT_NO_THROW(run());
    EXPECT_TRUE(bflag);

    bflag = false;

    args = {"-iiiuu"};
    run();
    EXPECT_FALSE(bflag);
    EXPECT_EQ(3, iflag);
    EXPECT_EQ((unsigned int)2, uflag);
}

TEST_F(TApp, BoolOption) {
    bool bflag{false};
    app.add_option("-b", bflag);

    args = {"-b", "false"};
    run();
    EXPECT_FALSE(bflag);

    args = {"-b", "1"};
    run();
    EXPECT_TRUE(bflag);

    args = {"-b", "-7"};
    run();
    EXPECT_FALSE(bflag);

    // cause an out of bounds error internally
    args = {"-b", "751615654161688126132138844896646748852"};
    run();
    EXPECT_TRUE(bflag);

    args = {"-b", "-751615654161688126132138844896646748852"};
    run();
    EXPECT_FALSE(bflag);
}

TEST_F(TApp, vectorDefaults) {
    std::vector<int> vals{4, 5};
    auto opt = app.add_option("--long", vals, "", true);

    args = {"--long", "[1,2,3]"};

    run();

    EXPECT_EQ(vals, std::vector<int>({1, 2, 3}));

    args.clear();
    run();
    auto res = app["--long"]->as<std::vector<int>>();
    EXPECT_EQ(res, std::vector<int>({4, 5}));

    app.clear();
    opt->expected(1)->take_last();
    res = app["--long"]->as<std::vector<int>>();
    EXPECT_EQ(res, std::vector<int>({5}));
    opt->take_first();
    res = app["--long"]->as<std::vector<int>>();
    EXPECT_EQ(res, std::vector<int>({4}));

    opt->expected(0, 1)->take_last();
    run();

    EXPECT_EQ(res, std::vector<int>({4}));
    res = app["--long"]->as<std::vector<int>>();
    EXPECT_EQ(res, std::vector<int>({5}));
}

TEST_F(TApp, CallbackBoolFlags) {

    bool value{false};

    auto func = [&value]() { value = true; };

    auto cback = app.add_flag_callback("--val", func);
    args = {"--val"};
    run();
    EXPECT_TRUE(value);
    value = false;
    args = {"--val=false"};
    run();
    EXPECT_FALSE(value);

    EXPECT_THROW(app.add_flag_callback("hi", func), CLI::IncorrectConstruction);
    cback->multi_option_policy(CLI::MultiOptionPolicy::Throw);
    args = {"--val", "--val=false"};
    EXPECT_THROW(run(), CLI::ArgumentMismatch);
}

TEST_F(TApp, pair_check) {
    std::string myfile{"pair_check_file.txt"};
    bool ok = static_cast<bool>(std::ofstream(myfile.c_str()).put('a'));  // create file
    EXPECT_TRUE(ok);

    EXPECT_TRUE(CLI::ExistingFile(myfile).empty());
    std::pair<std::string, int> findex;

    auto v0 = CLI::ExistingFile;
    v0.application_index(0);
    auto v1 = CLI::PositiveNumber;
    v1.application_index(1);
    app.add_option("--file", findex)->check(v0)->check(v1);

    args = {"--file", myfile, "2"};

    EXPECT_NO_THROW(run());

    EXPECT_EQ(findex.first, myfile);
    EXPECT_EQ(findex.second, 2);

    args = {"--file", myfile, "-3"};

    EXPECT_THROW(run(), CLI::ValidationError);

    args = {"--file", myfile, "2"};
    std::remove(myfile.c_str());
    EXPECT_THROW(run(), CLI::ValidationError);
}

// this will require that modifying the multi-option policy for tuples be allowed which it isn't at present

TEST_F(TApp, pair_check_take_first) {
    std::string myfile{"pair_check_file2.txt"};
    bool ok = static_cast<bool>(std::ofstream(myfile.c_str()).put('a'));  // create file
    EXPECT_TRUE(ok);

    EXPECT_TRUE(CLI::ExistingFile(myfile).empty());
    std::pair<std::string, int> findex;

    auto opt = app.add_option("--file", findex)->check(CLI::ExistingFile)->check(CLI::PositiveNumber);
    EXPECT_THROW(opt->get_validator(3), CLI::OptionNotFound);
    opt->get_validator(0)->application_index(0);
    opt->get_validator(1)->application_index(1);
    opt->multi_option_policy(CLI::MultiOptionPolicy::TakeLast);
    args = {"--file", "not_a_file.txt", "-16", "--file", myfile, "2"};
    // should only check the last one
    EXPECT_NO_THROW(run());

    EXPECT_EQ(findex.first, myfile);
    EXPECT_EQ(findex.second, 2);

    opt->multi_option_policy(CLI::MultiOptionPolicy::TakeFirst);

    EXPECT_THROW(run(), CLI::ValidationError);
}

TEST_F(TApp, VectorFixedString) {
    std::vector<std::string> strvec;
    std::vector<std::string> answer{"mystring", "mystring2", "mystring3"};

    CLI::Option *opt = app.add_option("-s,--string", strvec)->expected(3);
    EXPECT_EQ(3, opt->get_expected());

    args = {"--string", "mystring", "mystring2", "mystring3"};
    run();
    EXPECT_EQ(3u, app.count("--string"));
    EXPECT_EQ(answer, strvec);
}

TEST_F(TApp, VectorDefaultedFixedString) {
    std::vector<std::string> strvec{"one"};
    std::vector<std::string> answer{"mystring", "mystring2", "mystring3"};

    CLI::Option *opt = app.add_option("-s,--string", strvec, "")->expected(3)->capture_default_str();
    EXPECT_EQ(3, opt->get_expected());

    args = {"--string", "mystring", "mystring2", "mystring3"};
    run();
    EXPECT_EQ(3u, app.count("--string"));
    EXPECT_EQ(answer, strvec);
}

TEST_F(TApp, VectorIndexedValidator) {
    std::vector<int> vvec;

    CLI::Option *opt = app.add_option("-v", vvec);

    args = {"-v", "1", "-1", "-v", "3", "-v", "-976"};
    run();
    EXPECT_EQ(4u, app.count("-v"));
    EXPECT_EQ(4u, vvec.size());
    opt->check(CLI::PositiveNumber.application_index(0));
    opt->check((!CLI::PositiveNumber).application_index(1));
    EXPECT_NO_THROW(run());
    EXPECT_EQ(4u, vvec.size());
    // v[3] would be negative
    opt->check(CLI::PositiveNumber.application_index(3));
    EXPECT_THROW(run(), CLI::ValidationError);
}

TEST_F(TApp, VectorUnlimString) {
    std::vector<std::string> strvec;
    std::vector<std::string> answer{"mystring", "mystring2", "mystring3"};

    CLI::Option *opt = app.add_option("-s,--string", strvec);
    EXPECT_EQ(1, opt->get_expected());
    EXPECT_EQ(CLI::detail::expected_max_vector_size, opt->get_expected_max());

    args = {"--string", "mystring", "mystring2", "mystring3"};
    run();
    EXPECT_EQ(3u, app.count("--string"));
    EXPECT_EQ(answer, strvec);

    args = {"-s", "mystring", "mystring2", "mystring3"};
    run();
    EXPECT_EQ(3u, app.count("--string"));
    EXPECT_EQ(answer, strvec);
}

// From https://github.com/CLIUtils/CLI11/issues/420
TEST_F(TApp, stringLikeTests) {
    struct nType {
        explicit nType(const std::string &a_value) : m_value{a_value} {}

        explicit operator std::string() const { return std::string{"op str"}; }

        std::string m_value;
    };

    nType m_type{"abc"};
    app.add_option("--type", m_type, "type")->capture_default_str();
    run();

    EXPECT_EQ(app["--type"]->as<std::string>(), "op str");
    args = {"--type", "bca"};
    run();
    EXPECT_EQ(std::string(m_type), "op str");
    EXPECT_EQ(m_type.m_value, "bca");
}

TEST_F(TApp, VectorExpectedRange) {
    std::vector<std::string> strvec;

    CLI::Option *opt = app.add_option("--string", strvec);
    opt->expected(2, 4)->multi_option_policy(CLI::MultiOptionPolicy::Throw);

    args = {"--string", "mystring", "mystring2", "mystring3"};
    run();
    EXPECT_EQ(3u, app.count("--string"));

    args = {"--string", "mystring"};
    EXPECT_THROW(run(), CLI::ArgumentMismatch);

    args = {"--string", "mystring", "mystring2", "string2", "--string", "string4", "string5"};
    EXPECT_THROW(run(), CLI::ArgumentMismatch);

    EXPECT_EQ(opt->get_expected_max(), 4);
    EXPECT_EQ(opt->get_expected_min(), 2);
    opt->expected(4, 2);  // just test the handling of reversed arguments
    EXPECT_EQ(opt->get_expected_max(), 4);
    EXPECT_EQ(opt->get_expected_min(), 2);
    opt->expected(-5);
    EXPECT_EQ(opt->get_expected_max(), 5);
    EXPECT_EQ(opt->get_expected_min(), 5);
    opt->expected(-5, 7);
    EXPECT_EQ(opt->get_expected_max(), 7);
    EXPECT_EQ(opt->get_expected_min(), 5);
}

TEST_F(TApp, VectorFancyOpts) {
    std::vector<std::string> strvec;
    std::vector<std::string> answer{"mystring", "mystring2", "mystring3"};

    CLI::Option *opt = app.add_option("-s,--string", strvec)->required()->expected(3);
    EXPECT_EQ(3, opt->get_expected());

    args = {"--string", "mystring", "mystring2", "mystring3"};
    run();
    EXPECT_EQ(3u, app.count("--string"));
    EXPECT_EQ(answer, strvec);

    args = {"one", "two"};
    EXPECT_THROW(run(), CLI::RequiredError);

    EXPECT_THROW(run(), CLI::ParseError);
}

// #87
TEST_F(TApp, CustomDoubleOption) {

    std::pair<int, double> custom_opt;

    auto opt = app.add_option("posit", [&custom_opt](CLI::results_t vals) {
        custom_opt = {stol(vals.at(0)), stod(vals.at(1))};
        return true;
    });
    opt->type_name("INT FLOAT")->type_size(2);

    args = {"12", "1.5"};

    run();
    EXPECT_EQ(custom_opt.first, 12);
    EXPECT_DOUBLE_EQ(custom_opt.second, 1.5);
}

// now with tuple support this is possible
TEST_F(TApp, CustomDoubleOptionAlt) {

    std::pair<int, double> custom_opt;

    app.add_option("posit", custom_opt);

    args = {"12", "1.5"};

    run();
    EXPECT_EQ(custom_opt.first, 12);
    EXPECT_DOUBLE_EQ(custom_opt.second, 1.5);
}

// now with independent type sizes and expected this is possible
TEST_F(TApp, vectorPair) {

    std::vector<std::pair<int, std::string>> custom_opt;

    auto opt = app.add_option("--dict", custom_opt);

    args = {"--dict", "1", "str1", "--dict", "3", "str3"};

    run();
    ASSERT_EQ(custom_opt.size(), 2u);
    EXPECT_EQ(custom_opt[0].first, 1);
    EXPECT_EQ(custom_opt[1].second, "str3");

    args = {"--dict", "1", "str1", "--dict", "3", "str3", "--dict", "-1", "str4"};
    run();
    ASSERT_EQ(custom_opt.size(), 3u);
    EXPECT_EQ(custom_opt[2].first, -1);
    EXPECT_EQ(custom_opt[2].second, "str4");
    opt->check(CLI::PositiveNumber.application_index(0));

    EXPECT_THROW(run(), CLI::ValidationError);
}

TEST_F(TApp, vectorPairFail) {

    std::vector<std::pair<int, std::string>> custom_opt;

    app.add_option("--dict", custom_opt);

    args = {"--dict", "1", "str1", "--dict", "str3", "1"};

    EXPECT_THROW(run(), CLI::ConversionError);
}

TEST_F(TApp, vectorPairTypeRange) {

    std::vector<std::pair<int, std::string>> custom_opt;

    auto opt = app.add_option("--dict", custom_opt);

    opt->type_size(2, 1);  // just test switched arguments
    EXPECT_EQ(opt->get_type_size_min(), 1);
    EXPECT_EQ(opt->get_type_size_max(), 2);

    args = {"--dict", "1", "str1", "--dict", "3", "str3"};

    run();
    ASSERT_EQ(custom_opt.size(), 2u);
    EXPECT_EQ(custom_opt[0].first, 1);
    EXPECT_EQ(custom_opt[1].second, "str3");

    args = {"--dict", "1", "str1", "--dict", "3", "--dict", "-1", "str4"};
    run();
    ASSERT_EQ(custom_opt.size(), 3u);
    EXPECT_TRUE(custom_opt[1].second.empty());
    EXPECT_EQ(custom_opt[2].first, -1);
    EXPECT_EQ(custom_opt[2].second, "str4");

    opt->type_size(-2, -1);  // test negative arguments
    EXPECT_EQ(opt->get_type_size_min(), 1);
    EXPECT_EQ(opt->get_type_size_max(), 2);
    // this type size spec should run exactly as before
    run();
    ASSERT_EQ(custom_opt.size(), 3u);
    EXPECT_TRUE(custom_opt[1].second.empty());
    EXPECT_EQ(custom_opt[2].first, -1);
    EXPECT_EQ(custom_opt[2].second, "str4");
}

// now with independent type sizes and expected this is possible
TEST_F(TApp, vectorTuple) {

    std::vector<std::tuple<int, std::string, double>> custom_opt;

    auto opt = app.add_option("--dict", custom_opt);

    args = {"--dict", "1", "str1", "4.3", "--dict", "3", "str3", "2.7"};

    run();
    ASSERT_EQ(custom_opt.size(), 2u);
    EXPECT_EQ(std::get<0>(custom_opt[0]), 1);
    EXPECT_EQ(std::get<1>(custom_opt[1]), "str3");
    EXPECT_EQ(std::get<2>(custom_opt[1]), 2.7);

    args = {"--dict", "1", "str1", "4.3", "--dict", "3", "str3", "2.7", "--dict", "-1", "str4", "-1.87"};
    run();
    ASSERT_EQ(custom_opt.size(), 3u);
    EXPECT_EQ(std::get<0>(custom_opt[2]), -1);
    EXPECT_EQ(std::get<1>(custom_opt[2]), "str4");
    EXPECT_EQ(std::get<2>(custom_opt[2]), -1.87);
    opt->check(CLI::PositiveNumber.application_index(0));

    EXPECT_THROW(run(), CLI::ValidationError);

    args.back() = "haha";
    args[9] = "45";
    EXPECT_THROW(run(), CLI::ConversionError);
}

// now with independent type sizes and expected this is possible
TEST_F(TApp, vectorVector) {

    std::vector<std::vector<int>> custom_opt;

    auto opt = app.add_option("--dict", custom_opt);

    args = {"--dict", "1", "2", "4", "--dict", "3", "1"};

    run();
    ASSERT_EQ(custom_opt.size(), 2u);
    EXPECT_EQ(custom_opt[0].size(), 3u);
    EXPECT_EQ(custom_opt[1].size(), 2u);

    args = {"--dict", "1", "2", "4", "--dict", "3", "1", "--dict", "3", "--dict",
            "3",      "3", "3", "3", "3",      "3", "3", "3",      "3", "-3"};
    run();
    ASSERT_EQ(custom_opt.size(), 4u);
    EXPECT_EQ(custom_opt[0].size(), 3u);
    EXPECT_EQ(custom_opt[1].size(), 2u);
    EXPECT_EQ(custom_opt[2].size(), 1u);
    EXPECT_EQ(custom_opt[3].size(), 10u);
    opt->check(CLI::PositiveNumber.application_index(9));

    EXPECT_THROW(run(), CLI::ValidationError);
    args.pop_back();
    EXPECT_NO_THROW(run());

    args.back() = "haha";
    EXPECT_THROW(run(), CLI::ConversionError);

    args = {"--dict", "1", "2", "4", "%%", "3", "1", "%%", "3", "%%", "3", "3", "3", "3", "3", "3", "3", "3", "3", "3"};
    run();
    ASSERT_EQ(custom_opt.size(), 4u);
}

// now with independent type sizes and expected this is possible
TEST_F(TApp, vectorVectorFixedSize) {

    std::vector<std::vector<int>> custom_opt;

    auto opt = app.add_option("--dict", custom_opt)->type_size(4);

    args = {"--dict", "1", "2", "4", "3", "--dict", "3", "1", "2", "8"};

    run();
    ASSERT_EQ(custom_opt.size(), 2u);
    EXPECT_EQ(custom_opt[0].size(), 4u);
    EXPECT_EQ(custom_opt[1].size(), 4u);

    args = {"--dict", "1", "2", "4", "--dict", "3", "1", "7", "6"};
    EXPECT_THROW(run(), CLI::ConversionError);
    // this should reset it
    opt->type_size(CLI::detail::expected_max_vector_size);
    opt->type_size(1, CLI::detail::expected_max_vector_size);
    EXPECT_NO_THROW(run());
    ASSERT_EQ(custom_opt.size(), 2U);
}

// now with independent type sizes and expected this is possible
TEST_F(TApp, tuplePair) {
    std::tuple<std::pair<int, double>> custom_opt;

    app.add_option("--pr", custom_opt);

    args = {"--pr", "1", "2"};

    run();
    EXPECT_EQ(std::get<0>(custom_opt).first, 1);
    EXPECT_EQ(std::get<0>(custom_opt).second, 2.0);
}
// now with independent type sizes and expected this is possible
TEST_F(TApp, tupleintPair) {
    std::tuple<int, std::pair<int, double>> custom_opt;

    app.add_option("--pr", custom_opt);

    args = {"--pr", "3", "1", "2"};

    run();
    EXPECT_EQ(std::get<0>(custom_opt), 3);
    EXPECT_EQ(std::get<1>(custom_opt).first, 1);
    EXPECT_EQ(std::get<1>(custom_opt).second, 2.0);
}

static_assert(CLI::detail::is_mutable_container<std::set<std::string>>::value, "set should be a container");
static_assert(CLI::detail::is_mutable_container<std::map<std::string, std::string>>::value,
              "map should be a container");
static_assert(CLI::detail::is_mutable_container<std::unordered_map<std::string, double>>::value,
              "unordered_map should be a container");

static_assert(CLI::detail::is_mutable_container<std::list<std::pair<int, std::string>>>::value,
              "list should be a container");

static_assert(CLI::detail::type_count<std::set<std::string>>::value == 1, "set should have a type size of 1");
static_assert(CLI::detail::type_count<std::set<std::tuple<std::string, int, int>>>::value == 3,
              "tuple set should have size of 3");
static_assert(CLI::detail::type_count<std::map<std::string, std::string>>::value == 2,
              "map should have a type size of 2");
static_assert(CLI::detail::type_count<std::unordered_map<std::string, double>>::value == 2,
              "unordered_map should have a type size of 2");

static_assert(CLI::detail::type_count<std::list<std::pair<int, std::string>>>::value == 2,
              "list<int,string> should have a type size of 2");
static_assert(CLI::detail::type_count<std::map<std::string, std::pair<int, std::string>>>::value == 3,
              "map<string,pair<int,string>> should have a type size of 3");

template <class T> class TApp_container_single : public TApp {
  public:
    using container_type = T;
    container_type cval{};
    TApp_container_single() : TApp() {}
};

using containerTypes_single =
    ::testing::Types<std::vector<int>, std::deque<int>, std::set<int>, std::list<int>, std::unordered_set<int>>;

TYPED_TEST_SUITE(TApp_container_single, containerTypes_single, );

TYPED_TEST(TApp_container_single, containerInt) {

    auto &cv = TApp_container_single<TypeParam>::cval;
    CLI::Option *opt = (TApp::app).add_option("-v", cv);

    TApp::args = {"-v", "1", "-1", "-v", "3", "-v", "-976"};
    TApp::run();
    EXPECT_EQ(4u, (TApp::app).count("-v"));
    EXPECT_EQ(4u, cv.size());
    opt->check(CLI::PositiveNumber.application_index(0));
    opt->check((!CLI::PositiveNumber).application_index(1));
    EXPECT_NO_THROW(TApp::run());
    EXPECT_EQ(4u, cv.size());
    // v[3] would be negative
    opt->check(CLI::PositiveNumber.application_index(3));
    EXPECT_THROW(TApp::run(), CLI::ValidationError);
}

template <class T> class TApp_container_pair : public TApp {
  public:
    using container_type = T;
    container_type cval{};
    TApp_container_pair() : TApp() {}
};

using isp = std::pair<int, std::string>;
using containerTypes_pair = ::testing::Types<std::vector<isp>,
                                             std::deque<isp>,
                                             std::set<isp>,
                                             std::list<isp>,
                                             std::map<int, std::string>,
                                             std::unordered_map<int, std::string>>;

TYPED_TEST_SUITE(TApp_container_pair, containerTypes_pair, );

TYPED_TEST(TApp_container_pair, containerPair) {

    auto &cv = TApp_container_pair<TypeParam>::cval;
    (TApp::app).add_option("--dict", cv);

    TApp::args = {"--dict", "1", "str1", "--dict", "3", "str3"};

    TApp::run();
    EXPECT_EQ(cv.size(), 2u);

    TApp::args = {"--dict", "1", "str1", "--dict", "3", "--dict", "-1", "str4"};
    TApp::run();
    EXPECT_EQ(cv.size(), 3u);
}

template <class T> class TApp_container_tuple : public TApp {
  public:
    using container_type = T;
    container_type cval{};
    TApp_container_tuple() : TApp() {}
};

using tup_obj = std::tuple<int, std::string, double>;
using containerTypes_tuple = ::testing::Types<std::vector<tup_obj>,
                                              std::deque<tup_obj>,
                                              std::set<tup_obj>,
                                              std::list<tup_obj>,
                                              std::map<int, std::pair<std::string, double>>,
                                              std::unordered_map<int, std::tuple<std::string, double>>>;

TYPED_TEST_SUITE(TApp_container_tuple, containerTypes_tuple, );

TYPED_TEST(TApp_container_tuple, containerTuple) {

    auto &cv = TApp_container_tuple<TypeParam>::cval;
    (TApp::app).add_option("--dict", cv);

    TApp::args = {"--dict", "1", "str1", "4.3", "--dict", "3", "str3", "2.7"};

    TApp::run();
    EXPECT_EQ(cv.size(), 2u);

    TApp::args = {"--dict", "1", "str1", "4.3", "--dict", "3", "str3", "2.7", "--dict", "-1", "str4", "-1.87"};
    TApp::run();
    EXPECT_EQ(cv.size(), 3u);
}

using icontainer1 = std::vector<int>;
using icontainer2 = std::list<int>;
using icontainer3 = std::set<int>;
using icontainer4 = std::pair<int, std::vector<int>>;

using containerTypes_container = ::testing::Types<std::vector<icontainer1>,
                                                  std::list<icontainer1>,
                                                  std::set<icontainer1>,
                                                  std::deque<icontainer1>,
                                                  std::vector<icontainer2>,
                                                  std::list<icontainer2>,
                                                  std::set<icontainer2>,
                                                  std::deque<icontainer2>,
                                                  std::vector<icontainer3>,
                                                  std::list<icontainer3>,
                                                  std::set<icontainer3>,
                                                  std::deque<icontainer3>>;

template <class T> class TApp_container_container : public TApp {
  public:
    using container_type = T;
    container_type cval{};
    TApp_container_container() : TApp() {}
};

TYPED_TEST_SUITE(TApp_container_container, containerTypes_container, );

TYPED_TEST(TApp_container_container, containerContainer) {

    auto &cv = TApp_container_container<TypeParam>::cval;
    (TApp::app).add_option("--dict", cv);

    TApp::args = {"--dict", "1", "2", "4", "--dict", "3", "1"};

    TApp::run();
    EXPECT_EQ(cv.size(), 2u);

    TApp::args = {"--dict", "1", "2", "4", "--dict", "3", "1", "--dict", "3", "--dict",
                  "3",      "3", "3", "3", "3",      "3", "3", "3",      "3", "-3"};
    TApp::run();
    EXPECT_EQ(cv.size(), 4u);
}

TEST_F(TApp, containerContainer) {

    std::vector<icontainer4> cv;
    app.add_option("--dict", cv);

    args = {"--dict", "1", "2", "4", "--dict", "3", "1"};

    run();
    EXPECT_EQ(cv.size(), 2u);

    args = {"--dict", "1", "2", "4", "--dict", "3", "1", "--dict", "3", "",  "--dict",
            "3",      "3", "3", "3", "3",      "3", "3", "3",      "3", "-3"};
    run();
    EXPECT_EQ(cv.size(), 4u);
}

TEST_F(TApp, unknownContainerWrapper) {

    class vopt {
      public:
        vopt() = default;
        explicit vopt(const std::vector<double> &vdub) : val_{vdub} {};
        std::vector<double> val_{};
    };

    vopt cv;
    app.add_option<vopt, std::vector<double>>("--vv", cv);

    args = {"--vv", "1", "2", "4"};

    run();
    EXPECT_EQ(cv.val_.size(), 3u);
    args = {"--vv", ""};

    run();
    EXPECT_TRUE(cv.val_.empty());
}

TEST_F(TApp, tupleTwoVectors) {

    std::tuple<std::vector<int>, std::vector<int>> cv;
    app.add_option("--vv", cv);

    args = {"--vv", "1", "2", "4"};

    run();
    EXPECT_EQ(std::get<0>(cv).size(), 3U);
    EXPECT_TRUE(std::get<1>(cv).empty());

    args = {"--vv", "1", "2", "%%", "4", "4", "5"};

    run();
    EXPECT_EQ(std::get<0>(cv).size(), 2U);
    EXPECT_EQ(std::get<1>(cv).size(), 3U);
}
