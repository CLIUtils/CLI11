// Copyright (c) 2017-2025, University of Cincinnati, developed by Henry Schreiner
// under NSF AWARD 1414736 and by the respective contributors.
// All rights reserved.
//
// SPDX-License-Identifier: BSD-3-Clause

#include "app_helper.hpp"

#include <cmath>

#include <array>
#include <chrono>
#include <cstdint>
#include <map>
#include <memory>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#if defined(CLI11_CPP17)
#if defined(__has_include)
#if __has_include(<string_view>)
#include <string_view>
#define CLI11_HAS_STRING_VIEW
#endif
#endif
#endif

#if (defined(CLI11_ENABLE_EXTRA_VALIDATORS) && CLI11_ENABLE_EXTRA_VALIDATORS == 1) ||                                  \
    (!defined(CLI11_DISABLE_EXTRA_VALIDATORS) || CLI11_DISABLE_EXTRA_VALIDATORS == 0)

TEST_CASE_METHOD(TApp, "SimpleTransform", "[transform]") {
    int value{0};
    auto *opt = app.add_option("-s", value)->transform(CLI::Transformer({{"one", std::string("1")}}));
    args = {"-s", "one"};
    run();
    CHECK(app.count("-s") == 1u);
    CHECK(opt->count() == 1u);
    CHECK(1 == value);
}

TEST_CASE_METHOD(TApp, "SimpleTransformInitList", "[transform]") {
    int value{0};
    auto *opt = app.add_option("-s", value)->transform(CLI::Transformer({{"one", "1"}}));
    args = {"-s", "one"};
    run();
    CHECK(app.count("-s") == 1u);
    CHECK(opt->count() == 1u);
    CHECK(1 == value);
}

TEST_CASE_METHOD(TApp, "SimpleNumericalTransform", "[transform]") {
    int value{0};
    auto *opt = app.add_option("-s", value)->transform(CLI::Transformer(CLI::TransformPairs<int>{{"one", 1}}));
    args = {"-s", "one"};
    run();
    CHECK(app.count("-s") == 1u);
    CHECK(opt->count() == 1u);
    CHECK(1 == value);
}

TEST_CASE_METHOD(TApp, "EnumTransform", "[transform]") {
    enum class test_cli : std::int16_t { val1 = 3, val2 = 4, val3 = 17 };
    test_cli value{test_cli::val2};
    auto *opt = app.add_option("-s", value)
                    ->transform(CLI::Transformer(CLI::TransformPairs<test_cli>{
                        {"val1", test_cli::val1}, {"val2", test_cli::val2}, {"val3", test_cli::val3}}));
    args = {"-s", "val1"};
    run();
    CHECK(app.count("-s") == 1u);
    CHECK(opt->count() == 1u);
    CHECK(test_cli::val1 == value);

    args = {"-s", "val2"};
    run();
    CHECK(test_cli::val2 == value);

    args = {"-s", "val3"};
    run();
    CHECK(test_cli::val3 == value);

    args = {"-s", "val4"};
    CHECK_THROWS_AS(run(), CLI::ConversionError);

    // transformer doesn't do any checking so this still works
    args = {"-s", "5"};
    run();
    CHECK(static_cast<std::int16_t>(5) == static_cast<std::int16_t>(value));
}

TEST_CASE_METHOD(TApp, "EnumCheckedTransform", "[transform]") {
    enum class test_cli : std::int16_t { val1 = 3, val2 = 4, val3 = 17 };
    test_cli value{test_cli::val1};
    auto *opt = app.add_option("-s", value)
                    ->transform(CLI::CheckedTransformer(CLI::TransformPairs<test_cli>{
                        {"val1", test_cli::val1}, {"val2", test_cli::val2}, {"val3", test_cli::val3}}));
    args = {"-s", "val1"};
    run();
    CHECK(app.count("-s") == 1u);
    CHECK(opt->count() == 1u);
    CHECK(test_cli::val1 == value);

    args = {"-s", "val2"};
    run();
    CHECK(test_cli::val2 == value);

    args = {"-s", "val3"};
    run();
    CHECK(test_cli::val3 == value);

    args = {"-s", "17"};
    run();
    CHECK(test_cli::val3 == value);

    args = {"-s", "val4"};
    CHECK_THROWS_AS(run(), CLI::ValidationError);

    args = {"-s", "5"};
    CHECK_THROWS_AS(run(), CLI::ValidationError);
}

// from to-mas-kral Issue #1086
TEST_CASE_METHOD(TApp, "EnumCheckedTransformUint8", "[transform]") {
    enum class FooType : std::uint8_t { A, B };
    auto type = FooType::B;

    const std::map<std::string, FooType> foo_map{
        {"a", FooType::A},
        {"b", FooType::B},
    };

    app.add_option("-f,--foo", type, "FooType")
        ->transform(CLI::CheckedTransformer(foo_map, CLI::ignore_case))
        ->default_val(FooType::A)
        ->force_callback();

    run();
    CHECK(type == FooType::A);
}

// from jzakrzewski Issue #330
TEST_CASE_METHOD(TApp, "EnumCheckedDefaultTransform", "[transform]") {
    enum class existing : std::int16_t { abort, overwrite, remove };
    app.add_option("--existing", "What to do if file already exists in the destination")
        ->transform(
            CLI::CheckedTransformer(std::unordered_map<std::string, existing>{{"abort", existing::abort},
                                                                              {"overwrite", existing ::overwrite},
                                                                              {"delete", existing::remove},
                                                                              {"remove", existing::remove}}))
        ->default_val("abort");
    args = {"--existing", "overwrite"};
    run();
    CHECK(existing::overwrite == app.get_option("--existing")->as<existing>());
    args.clear();
    run();
    CHECK(existing::abort == app.get_option("--existing")->as<existing>());
}

// test from https://github.com/CLIUtils/CLI11/issues/369  [Jakub Zakrzewski](https://github.com/jzakrzewski)
TEST_CASE_METHOD(TApp, "EnumCheckedDefaultTransformCallback", "[transform]") {
    enum class existing : std::int16_t { abort, overwrite, remove };
    auto cmd = std::make_shared<CLI::App>("deploys the repository somewhere", "deploy");
    cmd->add_option("--existing", "What to do if file already exists in the destination")
        ->transform(
            CLI::CheckedTransformer(std::unordered_map<std::string, existing>{{"abort", existing::abort},
                                                                              {"overwrite", existing::overwrite},
                                                                              {"delete", existing::remove},
                                                                              {"remove", existing::remove}}))
        ->default_val("abort");

    cmd->callback([cmd]() { CHECK(cmd->get_option("--existing")->as<existing>() == existing::abort); });
    app.add_subcommand(cmd);

    args = {"deploy"};
    run();
}

TEST_CASE_METHOD(TApp, "SimpleTransformFn", "[transform]") {
    int value{0};
    auto *opt = app.add_option("-s", value)->transform(CLI::Transformer({{"one", "1"}}, CLI::ignore_case));
    args = {"-s", "ONE"};
    run();
    CHECK(app.count("-s") == 1u);
    CHECK(opt->count() == 1u);
    CHECK(1 == value);
}

#if defined(CLI11_HAS_STRING_VIEW)
TEST_CASE_METHOD(TApp, "StringViewTransformFn", "[transform]") {
    std::string value;
    std::map<std::string_view, std::string_view> map = {// key length > std::string().capacity() [SSO length]
                                                        {"a-rather-long-argument", "mapped"}};
    app.add_option("-s", value)->transform(CLI::CheckedTransformer(map));
    args = {"-s", "a-rather-long-argument"};
    run();
    CHECK("mapped" == value);
}

#endif

TEST_CASE_METHOD(TApp, "SimpleNumericalTransformFn", "[transform]") {
    int value{0};
    auto *opt =
        app.add_option("-s", value)
            ->transform(CLI::Transformer(std::vector<std::pair<std::string, int>>{{"one", 1}}, CLI::ignore_case));
    args = {"-s", "ONe"};
    run();
    CHECK(app.count("-s") == 1u);
    CHECK(opt->count() == 1u);
    CHECK(1 == value);
}

TEST_CASE_METHOD(TApp, "SimpleNumericalTransformFnVector", "[transform]") {
    std::vector<std::pair<std::string, int>> conversions{{"one", 1}, {"two", 2}};
    int value{0};
    auto *opt = app.add_option("-s", value)->transform(CLI::Transformer(conversions, CLI::ignore_case));
    args = {"-s", "ONe"};
    run();
    CHECK(app.count("-s") == 1u);
    CHECK(opt->count() == 1u);
    CHECK(1 == value);
}

TEST_CASE_METHOD(TApp, "SimpleNumericalTransformFnArray", "[transform]") {
    std::array<std::pair<std::string, int>, 2> conversions;
    conversions[0] = std::make_pair(std::string("one"), 1);
    conversions[1] = std::make_pair(std::string("two"), 2);

    int value{0};
    auto *opt = app.add_option("-s", value)->transform(CLI::Transformer(conversions, CLI::ignore_case));
    args = {"-s", "ONe"};
    run();
    CHECK(app.count("-s") == 1u);
    CHECK(opt->count() == 1u);
    CHECK(1 == value);
}

#ifdef CLI11_CPP14
// zero copy constexpr array operation with transformer example and test
TEST_CASE_METHOD(TApp, "SimpleNumericalTransformFnconstexprArray", "[transform]") {
    constexpr std::pair<const char *, int> p1{"one", 1};
    constexpr std::pair<const char *, int> p2{"two", 2};
    constexpr std::array<std::pair<const char *, int>, 2> conversions_c{{p1, p2}};

    int value{0};
    auto *opt = app.add_option("-s", value)->transform(CLI::Transformer(&conversions_c, CLI::ignore_case));
    args = {"-s", "ONe"};
    run();
    CHECK(app.count("-s") == 1u);
    CHECK(opt->count() == 1u);
    CHECK(1 == value);

    args = {"-s", "twO"};
    run();
    CHECK(app.count("-s") == 1u);
    CHECK(opt->count() == 1u);
    CHECK(2 == value);
}
#endif

TEST_CASE_METHOD(TApp, "EnumTransformFn", "[transform]") {
    enum class test_cli : std::int16_t { val1 = 3, val2 = 4, val3 = 17 };
    test_cli value{test_cli::val2};
    auto *opt = app.add_option("-s", value)
                    ->transform(CLI::Transformer(CLI::TransformPairs<test_cli>{{"val1", test_cli::val1},
                                                                               {"val2", test_cli::val2},
                                                                               {"val3", test_cli::val3}},
                                                 CLI::ignore_case,
                                                 CLI::ignore_underscore));
    args = {"-s", "val_1"};
    run();
    CHECK(app.count("-s") == 1u);
    CHECK(opt->count() == 1u);
    CHECK(test_cli::val1 == value);

    args = {"-s", "VAL_2"};
    run();
    CHECK(test_cli::val2 == value);

    args = {"-s", "VAL3"};
    run();
    CHECK(test_cli::val3 == value);

    args = {"-s", "val_4"};
    CHECK_THROWS_AS(run(), CLI::ConversionError);
}

TEST_CASE_METHOD(TApp, "EnumTransformFnMap", "[transform]") {
    enum class test_cli : std::int16_t { val1 = 3, val2 = 4, val3 = 17 };
    std::map<std::string, test_cli> map{{"val1", test_cli::val1}, {"val2", test_cli::val2}, {"val3", test_cli::val3}};
    test_cli value{test_cli::val3};
    auto *opt = app.add_option("-s", value)->transform(CLI::Transformer(map, CLI::ignore_case, CLI::ignore_underscore));
    args = {"-s", "val_1"};
    run();
    CHECK(app.count("-s") == 1u);
    CHECK(opt->count() == 1u);
    CHECK(test_cli::val1 == value);

    args = {"-s", "VAL_2"};
    run();
    CHECK(test_cli::val2 == value);

    args = {"-s", "VAL3"};
    run();
    CHECK(test_cli::val3 == value);

    args = {"-s", "val_4"};
    CHECK_THROWS_AS(run(), CLI::ConversionError);
}

TEST_CASE_METHOD(TApp, "EnumTransformFnPtrMap", "[transform]") {
    enum class test_cli : std::int16_t { val1 = 3, val2 = 4, val3 = 17, val4 = 37 };
    std::map<std::string, test_cli> map{{"val1", test_cli::val1}, {"val2", test_cli::val2}, {"val3", test_cli::val3}};
    test_cli value{test_cli::val2};
    auto *opt =
        app.add_option("-s", value)->transform(CLI::Transformer(&map, CLI::ignore_case, CLI::ignore_underscore));
    args = {"-s", "val_1"};
    run();
    CHECK(app.count("-s") == 1u);
    CHECK(opt->count() == 1u);
    CHECK(test_cli::val1 == value);

    args = {"-s", "VAL_2"};
    run();
    CHECK(test_cli::val2 == value);

    args = {"-s", "VAL3"};
    run();
    CHECK(test_cli::val3 == value);

    args = {"-s", "val_4"};
    CHECK_THROWS_AS(run(), CLI::ConversionError);

    map["val4"] = test_cli::val4;
    run();
    CHECK(test_cli::val4 == value);
}

TEST_CASE_METHOD(TApp, "EnumTransformFnSharedPtrMap", "[transform]") {
    enum class test_cli : std::int16_t { val1 = 3, val2 = 4, val3 = 17, val4 = 37 };
    auto map = std::make_shared<std::unordered_map<std::string, test_cli>>();
    auto &mp = *map;
    mp["val1"] = test_cli::val1;
    mp["val2"] = test_cli::val2;
    mp["val3"] = test_cli::val3;

    test_cli value{test_cli::val2};
    auto *opt = app.add_option("-s", value)->transform(CLI::Transformer(map, CLI::ignore_case, CLI::ignore_underscore));
    args = {"-s", "val_1"};
    run();
    CHECK(app.count("-s") == 1u);
    CHECK(opt->count() == 1u);
    CHECK(test_cli::val1 == value);

    args = {"-s", "VAL_2"};
    run();
    CHECK(test_cli::val2 == value);

    args = {"-s", "VAL3"};
    run();
    CHECK(test_cli::val3 == value);

    args = {"-s", "val_4"};
    CHECK_THROWS_AS(run(), CLI::ConversionError);

    mp["val4"] = test_cli::val4;
    run();
    CHECK(test_cli::val4 == value);
}

// Test a cascade of transform functions
TEST_CASE_METHOD(TApp, "TransformCascade", "[transform]") {

    std::string output;
    auto *opt = app.add_option("-s", output);
    opt->transform(CLI::Transformer({{"abc", "abcd"}, {"bbc", "bbcd"}, {"cbc", "cbcd"}}, CLI::ignore_case));
    opt->transform(
        CLI::Transformer({{"ab", "abc"}, {"bc", "bbc"}, {"cb", "cbc"}}, CLI::ignore_case, CLI::ignore_underscore));
    opt->transform(CLI::Transformer({{"a", "ab"}, {"b", "bb"}, {"c", "cb"}}, CLI::ignore_case));
    opt->check(CLI::IsMember({"abcd", "bbcd", "cbcd"}));
    args = {"-s", "abcd"};
    run();
    CHECK("abcd" == output);

    args = {"-s", "Bbc"};
    run();
    CHECK("bbcd" == output);

    args = {"-s", "C_B"};
    run();
    CHECK("cbcd" == output);

    args = {"-s", "A"};
    run();
    CHECK("abcd" == output);
}

// Test a cascade of transform functions
TEST_CASE_METHOD(TApp, "TransformCascadeDeactivate", "[transform]") {

    std::string output;
    auto *opt = app.add_option("-s", output);
    opt->transform(
        CLI::Transformer({{"abc", "abcd"}, {"bbc", "bbcd"}, {"cbc", "cbcd"}}, CLI::ignore_case).name("tform1"));
    opt->transform(
        CLI::Transformer({{"ab", "abc"}, {"bc", "bbc"}, {"cb", "cbc"}}, CLI::ignore_case, CLI::ignore_underscore)
            .name("tform2")
            .active(false));
    opt->transform(CLI::Transformer({{"a", "ab"}, {"b", "bb"}, {"c", "cb"}}, CLI::ignore_case).name("tform3"));
    opt->check(CLI::IsMember({"abcd", "bbcd", "cbcd"}).name("check"));
    args = {"-s", "abcd"};
    run();
    CHECK("abcd" == output);

    args = {"-s", "Bbc"};
    run();
    CHECK("bbcd" == output);

    args = {"-s", "C_B"};
    CHECK_THROWS_AS(run(), CLI::ValidationError);

    auto *validator = opt->get_validator("tform2");
    CHECK(!validator->get_active());
    CHECK("tform2" == validator->get_name());
    validator->active();
    CHECK(validator->get_active());
    args = {"-s", "C_B"};
    run();
    CHECK("cbcd" == output);

    opt->get_validator("check")->active(false);
    args = {"-s", "gsdgsgs"};
    run();
    CHECK("gsdgsgs" == output);

    CHECK_THROWS_AS(opt->get_validator("sdfsdf"), CLI::OptionNotFound);
}

TEST_CASE_METHOD(TApp, "IntTransformFn", "[transform]") {
    std::string value;
    app.add_option("-s", value)
        ->transform(
            CLI::CheckedTransformer(std::map<int, int>{{15, 5}, {18, 6}, {21, 7}}, [](int in) { return in - 10; }));
    args = {"-s", "25"};
    run();
    CHECK("5" == value);

    args = {"-s", "6"};
    run();
    CHECK("6" == value);

    args = {"-s", "45"};
    CHECK_THROWS_AS(run(), CLI::ValidationError);

    args = {"-s", "val_4"};
    CHECK_THROWS_AS(run(), CLI::ValidationError);
}

TEST_CASE_METHOD(TApp, "IntTransformNonConvertible", "[transform]") {
    std::string value;
    app.add_option("-s", value)->transform(CLI::Transformer(std::map<int, int>{{15, 5}, {18, 6}, {21, 7}}));
    args = {"-s", "15"};
    run();
    CHECK("5" == value);

    args = {"-s", "18"};
    run();
    CHECK("6" == value);

    // value can't be converted to int so it is just ignored
    args = {"-s", "abcd"};
    run();
    CHECK("abcd" == value);
}

TEST_CASE_METHOD(TApp, "IntTransformNonMerge", "[transform]") {
    std::string value;
    app.add_option("-s", value)
        ->transform(CLI::Transformer(std::map<int, int>{{15, 5}, {18, 6}, {21, 7}}) &
                        CLI::Transformer(std::map<int, int>{{25, 5}, {28, 6}, {31, 7}}),
                    "merge");
    args = {"-s", "15"};
    run();
    CHECK("5" == value);

    args = {"-s", "18"};
    run();
    CHECK("6" == value);

    // value can't be converted to int so it is just ignored
    args = {"-s", "abcd"};
    run();
    CHECK("abcd" == value);

    args = {"-s", "25"};
    run();
    CHECK("5" == value);

    args = {"-s", "31"};
    run();
    CHECK("7" == value);

    auto help = app.help();
    CHECK(help.find("15->5") != std::string::npos);
    CHECK(help.find("25->5") != std::string::npos);

    auto *validator = app.get_option("-s")->get_validator();
    help = validator->get_description();
    CHECK(help.find("15->5") != std::string::npos);
    CHECK(help.find("25->5") != std::string::npos);

    auto *validator2 = app.get_option("-s")->get_validator("merge");
    CHECK(validator == validator2);
}

TEST_CASE_METHOD(TApp, "IntTransformMergeWithCustomValidator", "[transform]") {
    std::string value;
    auto *opt = app.add_option("-s", value)
                    ->transform(CLI::Transformer(std::map<int, int>{{15, 5}, {18, 6}, {21, 7}}) |
                                    CLI::Validator(
                                        [](std::string &element) {
                                            if(element == "frog") {
                                                element = "hops";
                                            }
                                            return std::string{};
                                        },
                                        std::string{}),
                                "check");
    args = {"-s", "15"};
    run();
    CHECK("5" == value);

    args = {"-s", "18"};
    run();
    CHECK("6" == value);

    // value can't be converted to int so it is just ignored
    args = {"-s", "frog"};
    run();
    CHECK("hops" == value);

    args = {"-s", "25"};
    run();
    CHECK("25" == value);

    auto help = app.help();
    CHECK(help.find("15->5") != std::string::npos);
    CHECK(help.find("OR") == std::string::npos);

    auto *validator = opt->get_validator("check");
    CHECK("check" == validator->get_name());
    validator->active(false);
    help = app.help();
    CHECK(help.find("15->5") == std::string::npos);
}

TEST_CASE_METHOD(TApp, "IntTransformMergeWithCustomSharedValidator", "[transform]") {
    std::string value;

    CLI::Validator_p ctrans = std::make_shared<CLI::Validator>(
        CLI::Transformer(std::map<int, int>{{15, 5}, {18, 6}, {21, 7}}) | CLI::Validator(
                                                                              [](std::string &element) {
                                                                                  if(element == "frog") {
                                                                                      element = "hops";
                                                                                  }
                                                                                  return std::string{};
                                                                              },
                                                                              std::string{}));
    ctrans->name("check");
    auto *opt = app.add_option("-s", value)->transform(ctrans);
    args = {"-s", "15"};
    run();
    CHECK("5" == value);

    args = {"-s", "18"};
    run();
    CHECK("6" == value);

    // value can't be converted to int so it is just ignored
    args = {"-s", "frog"};
    run();
    CHECK("hops" == value);

    args = {"-s", "25"};
    run();
    CHECK("25" == value);

    auto help = app.help();
    CHECK(help.find("15->5") != std::string::npos);
    CHECK(help.find("OR") == std::string::npos);

    auto *validator = opt->get_validator("check");
    CHECK("check" == validator->get_name());
    validator->active(false);
    help = app.help();
    CHECK(help.find("15->5") == std::string::npos);
}
#endif
static const std::map<std::string, std::string> validValues = {
    {"test\\u03C0\\u00e9", from_u8string(u8"test\u03C0\u00E9")},
    {"test\\u03C0\\u00e9", from_u8string(u8"test\u73C0\u0057")},
    {"test\\U0001F600\\u00E9", from_u8string(u8"test\U0001F600\u00E9")},
    {R"("this\nis\na\nfour\tline test")", "this\nis\na\nfour\tline test"},
    {"'B\"(\\x35\\xa7\\x46)\"'", std::string{0x35, static_cast<char>(0xa7), 0x46}},
    {"B\"(\\x35\\xa7\\x46)\"", std::string{0x35, static_cast<char>(0xa7), 0x46}},
    {"test\\ntest", "test\ntest"},
    {"\"test\\ntest", "\"test\ntest"},
    {R"('this\nis\na\nfour\tline test')", R"(this\nis\na\nfour\tline test)"},
    {R"("this\nis\na\nfour\tline test")", "this\nis\na\nfour\tline test"},
    {R"(`this\nis\na\nfour\tline test`)", R"(this\nis\na\nfour\tline test)"}};

TEST_CASE_METHOD(TApp, "StringEscapeValid", "[transform]") {

    auto test_data = GENERATE(from_range(validValues));

    std::string value{};

    app.add_option("-n", value)->transform(CLI::EscapedString);

    args = {"-n", test_data.first};

    run();
    CHECK(test_data.second == value);
}

static const std::vector<std::string> invalidValues = {"test\\U0001M600\\u00E9",
                                                       "test\\U0001E600\\u00M9",
                                                       "test\\U0001E600\\uD8E9",
                                                       "test\\U0001E600\\uD8",
                                                       "test\\U0001E60",
                                                       "test\\qbad"};

TEST_CASE_METHOD(TApp, "StringEscapeInvalid", "[transform]") {

    auto test_data = GENERATE(from_range(invalidValues));

    std::string value{};

    app.add_option("-n", value)->transform(CLI::EscapedString);

    args = {"-n", test_data};

    CHECK_THROWS_AS(run(), CLI::ValidationError);
}
