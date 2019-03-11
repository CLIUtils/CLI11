#include "app_helper.hpp"

#include <unordered_map>

TEST_F(TApp, SimpleTransform) {
    int value;
    auto opt = app.add_option("-s", value)->transform(CLI::Transformer({{"one", std::string("1")}}));
    args = {"-s", "one"};
    run();
    EXPECT_EQ(1u, app.count("-s"));
    EXPECT_EQ(1u, opt->count());
    EXPECT_EQ(value, 1);
}

TEST_F(TApp, SimpleTransformInitList) {
    int value;
    auto opt = app.add_option("-s", value)->transform(CLI::Transformer({{"one", "1"}}));
    args = {"-s", "one"};
    run();
    EXPECT_EQ(1u, app.count("-s"));
    EXPECT_EQ(1u, opt->count());
    EXPECT_EQ(value, 1);
}

TEST_F(TApp, SimpleNumericalTransform) {
    int value;
    auto opt = app.add_option("-s", value)->transform(CLI::Transformer(CLI::TransformPairs<int>{{"one", 1}}));
    args = {"-s", "one"};
    run();
    EXPECT_EQ(1u, app.count("-s"));
    EXPECT_EQ(1u, opt->count());
    EXPECT_EQ(value, 1);
}

TEST_F(TApp, EnumTransform) {
    enum class test : int16_t { val1 = 3, val2 = 4, val3 = 17 };
    test value;
    auto opt = app.add_option("-s", value)
                   ->transform(CLI::Transformer(
                       CLI::TransformPairs<test>{{"val1", test::val1}, {"val2", test::val2}, {"val3", test::val3}}));
    args = {"-s", "val1"};
    run();
    EXPECT_EQ(1u, app.count("-s"));
    EXPECT_EQ(1u, opt->count());
    EXPECT_EQ(value, test::val1);

    args = {"-s", "val2"};
    run();
    EXPECT_EQ(value, test::val2);

    args = {"-s", "val3"};
    run();
    EXPECT_EQ(value, test::val3);

    args = {"-s", "val4"};
    EXPECT_THROW(run(), CLI::ConversionError);

    // transformer doesn't do any checking so this still works
    args = {"-s", "5"};
    run();
    EXPECT_EQ(static_cast<int16_t>(value), int16_t(5));
}

TEST_F(TApp, EnumCheckedTransform) {
    enum class test : int16_t { val1 = 3, val2 = 4, val3 = 17 };
    test value;
    auto opt = app.add_option("-s", value)
                   ->transform(CLI::CheckedTransformer(
                       CLI::TransformPairs<test>{{"val1", test::val1}, {"val2", test::val2}, {"val3", test::val3}}));
    args = {"-s", "val1"};
    run();
    EXPECT_EQ(1u, app.count("-s"));
    EXPECT_EQ(1u, opt->count());
    EXPECT_EQ(value, test::val1);

    args = {"-s", "val2"};
    run();
    EXPECT_EQ(value, test::val2);

    args = {"-s", "val3"};
    run();
    EXPECT_EQ(value, test::val3);

    args = {"-s", "17"};
    run();
    EXPECT_EQ(value, test::val3);

    args = {"-s", "val4"};
    EXPECT_THROW(run(), CLI::ValidationError);

    args = {"-s", "5"};
    EXPECT_THROW(run(), CLI::ValidationError);
}

TEST_F(TApp, SimpleTransformFn) {
    int value;
    auto opt = app.add_option("-s", value)->transform(CLI::Transformer({{"one", "1"}}, CLI::ignore_case));
    args = {"-s", "ONE"};
    run();
    EXPECT_EQ(1u, app.count("-s"));
    EXPECT_EQ(1u, opt->count());
    EXPECT_EQ(value, 1);
}

TEST_F(TApp, SimpleNumericalTransformFn) {
    int value;
    auto opt =
        app.add_option("-s", value)
            ->transform(CLI::Transformer(std::vector<std::pair<std::string, int>>{{"one", 1}}, CLI::ignore_case));
    args = {"-s", "ONe"};
    run();
    EXPECT_EQ(1u, app.count("-s"));
    EXPECT_EQ(1u, opt->count());
    EXPECT_EQ(value, 1);
}

TEST_F(TApp, EnumTransformFn) {
    enum class test : int16_t { val1 = 3, val2 = 4, val3 = 17 };
    test value;
    auto opt = app.add_option("-s", value)
                   ->transform(CLI::Transformer(
                       CLI::TransformPairs<test>{{"val1", test::val1}, {"val2", test::val2}, {"val3", test::val3}},
                       CLI::ignore_case,
                       CLI::ignore_underscore));
    args = {"-s", "val_1"};
    run();
    EXPECT_EQ(1u, app.count("-s"));
    EXPECT_EQ(1u, opt->count());
    EXPECT_EQ(value, test::val1);

    args = {"-s", "VAL_2"};
    run();
    EXPECT_EQ(value, test::val2);

    args = {"-s", "VAL3"};
    run();
    EXPECT_EQ(value, test::val3);

    args = {"-s", "val_4"};
    EXPECT_THROW(run(), CLI::ConversionError);
}

TEST_F(TApp, EnumTransformFnMap) {
    enum class test : int16_t { val1 = 3, val2 = 4, val3 = 17 };
    std::map<std::string, test> map{{"val1", test::val1}, {"val2", test::val2}, {"val3", test::val3}};
    test value;
    auto opt = app.add_option("-s", value)->transform(CLI::Transformer(map, CLI::ignore_case, CLI::ignore_underscore));
    args = {"-s", "val_1"};
    run();
    EXPECT_EQ(1u, app.count("-s"));
    EXPECT_EQ(1u, opt->count());
    EXPECT_EQ(value, test::val1);

    args = {"-s", "VAL_2"};
    run();
    EXPECT_EQ(value, test::val2);

    args = {"-s", "VAL3"};
    run();
    EXPECT_EQ(value, test::val3);

    args = {"-s", "val_4"};
    EXPECT_THROW(run(), CLI::ConversionError);
}

TEST_F(TApp, EnumTransformFnPtrMap) {
    enum class test : int16_t { val1 = 3, val2 = 4, val3 = 17, val4 = 37 };
    std::map<std::string, test> map{{"val1", test::val1}, {"val2", test::val2}, {"val3", test::val3}};
    test value;
    auto opt = app.add_option("-s", value)->transform(CLI::Transformer(&map, CLI::ignore_case, CLI::ignore_underscore));
    args = {"-s", "val_1"};
    run();
    EXPECT_EQ(1u, app.count("-s"));
    EXPECT_EQ(1u, opt->count());
    EXPECT_EQ(value, test::val1);

    args = {"-s", "VAL_2"};
    run();
    EXPECT_EQ(value, test::val2);

    args = {"-s", "VAL3"};
    run();
    EXPECT_EQ(value, test::val3);

    args = {"-s", "val_4"};
    EXPECT_THROW(run(), CLI::ConversionError);

    map["val4"] = test::val4;
    run();
    EXPECT_EQ(value, test::val4);
}

TEST_F(TApp, EnumTransformFnSharedPtrMap) {
    enum class test : int16_t { val1 = 3, val2 = 4, val3 = 17, val4 = 37 };
    auto map = std::make_shared<std::unordered_map<std::string, test>>();
    auto &mp = *map;
    mp["val1"] = test::val1;
    mp["val2"] = test::val2;
    mp["val3"] = test::val3;

    test value;
    auto opt = app.add_option("-s", value)->transform(CLI::Transformer(map, CLI::ignore_case, CLI::ignore_underscore));
    args = {"-s", "val_1"};
    run();
    EXPECT_EQ(1u, app.count("-s"));
    EXPECT_EQ(1u, opt->count());
    EXPECT_EQ(value, test::val1);

    args = {"-s", "VAL_2"};
    run();
    EXPECT_EQ(value, test::val2);

    args = {"-s", "VAL3"};
    run();
    EXPECT_EQ(value, test::val3);

    args = {"-s", "val_4"};
    EXPECT_THROW(run(), CLI::ConversionError);

    mp["val4"] = test::val4;
    run();
    EXPECT_EQ(value, test::val4);
}

// Test a cascade of transform functions
TEST_F(TApp, TransformCascade) {

    std::string output;
    auto opt = app.add_option("-s", output);
    opt->transform(CLI::Transformer({{"abc", "abcd"}, {"bbc", "bbcd"}, {"cbc", "cbcd"}}, CLI::ignore_case));
    opt->transform(
        CLI::Transformer({{"ab", "abc"}, {"bc", "bbc"}, {"cb", "cbc"}}, CLI::ignore_case, CLI::ignore_underscore));
    opt->transform(CLI::Transformer({{"a", "ab"}, {"b", "bb"}, {"c", "cb"}}, CLI::ignore_case));
    opt->check(CLI::IsMember({"abcd", "bbcd", "cbcd"}));
    args = {"-s", "abcd"};
    run();
    EXPECT_EQ(output, "abcd");

    args = {"-s", "Bbc"};
    run();
    EXPECT_EQ(output, "bbcd");

    args = {"-s", "C_B"};
    run();
    EXPECT_EQ(output, "cbcd");

    args = {"-s", "A"};
    run();
    EXPECT_EQ(output, "abcd");
}

// Test a cascade of transform functions
TEST_F(TApp, TransformCascadeDeactivate) {

    std::string output;
    auto opt = app.add_option("-s", output);
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
    EXPECT_EQ(output, "abcd");

    args = {"-s", "Bbc"};
    run();
    EXPECT_EQ(output, "bbcd");

    args = {"-s", "C_B"};
    EXPECT_THROW(run(), CLI::ValidationError);

    auto validator = opt->get_validator("tform2");
    EXPECT_FALSE(validator->get_active());
    EXPECT_EQ(validator->get_name(), "tform2");
    validator->active();
    EXPECT_TRUE(validator->get_active());
    args = {"-s", "C_B"};
    run();
    EXPECT_EQ(output, "cbcd");

    opt->get_validator("check")->active(false);
    args = {"-s", "gsdgsgs"};
    run();
    EXPECT_EQ(output, "gsdgsgs");

    EXPECT_THROW(opt->get_validator("sdfsdf"), CLI::OptionNotFound);
}

TEST_F(TApp, IntTransformFn) {
    std::string value;
    app.add_option("-s", value)
        ->transform(
            CLI::CheckedTransformer(std::map<int, int>{{15, 5}, {18, 6}, {21, 7}}, [](int in) { return in - 10; }));
    args = {"-s", "25"};
    run();
    EXPECT_EQ(value, "5");

    args = {"-s", "6"};
    run();
    EXPECT_EQ(value, "6");

    args = {"-s", "45"};
    EXPECT_THROW(run(), CLI::ValidationError);

    args = {"-s", "val_4"};
    EXPECT_THROW(run(), CLI::ValidationError);
}

TEST_F(TApp, IntTransformNonConvertible) {
    std::string value;
    app.add_option("-s", value)->transform(CLI::Transformer(std::map<int, int>{{15, 5}, {18, 6}, {21, 7}}));
    args = {"-s", "15"};
    run();
    EXPECT_EQ(value, "5");

    args = {"-s", "18"};
    run();
    EXPECT_EQ(value, "6");

    // value can't be converted to int so it is just ignored
    args = {"-s", "abcd"};
    run();
    EXPECT_EQ(value, "abcd");
}

TEST_F(TApp, IntTransformNonMerge) {
    std::string value;
    app.add_option("-s", value)
        ->transform(CLI::Transformer(std::map<int, int>{{15, 5}, {18, 6}, {21, 7}}) &
                        CLI::Transformer(std::map<int, int>{{25, 5}, {28, 6}, {31, 7}}),
                    "merge");
    args = {"-s", "15"};
    run();
    EXPECT_EQ(value, "5");

    args = {"-s", "18"};
    run();
    EXPECT_EQ(value, "6");

    // value can't be converted to int so it is just ignored
    args = {"-s", "abcd"};
    run();
    EXPECT_EQ(value, "abcd");

    args = {"-s", "25"};
    run();
    EXPECT_EQ(value, "5");

    args = {"-s", "31"};
    run();
    EXPECT_EQ(value, "7");

    auto help = app.help();
    EXPECT_TRUE(help.find("15->5") != std::string::npos);
    EXPECT_TRUE(help.find("25->5") != std::string::npos);

    auto validator = app.get_option("-s")->get_validator();
    help = validator->get_description();
    EXPECT_TRUE(help.find("15->5") != std::string::npos);
    EXPECT_TRUE(help.find("25->5") != std::string::npos);

    auto validator2 = app.get_option("-s")->get_validator("merge");
    EXPECT_EQ(validator2, validator);
}

TEST_F(TApp, IntTransformMergeWithCustomValidator) {
    std::string value;
    auto opt = app.add_option("-s", value)
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
    EXPECT_EQ(value, "5");

    args = {"-s", "18"};
    run();
    EXPECT_EQ(value, "6");

    // value can't be converted to int so it is just ignored
    args = {"-s", "frog"};
    run();
    EXPECT_EQ(value, "hops");

    args = {"-s", "25"};
    run();
    EXPECT_EQ(value, "25");

    auto help = app.help();
    EXPECT_TRUE(help.find("15->5") != std::string::npos);
    EXPECT_TRUE(help.find("OR") == std::string::npos);

    auto validator = opt->get_validator("check");
    EXPECT_EQ(validator->get_name(), "check");
    validator->active(false);
    help = app.help();
    EXPECT_TRUE(help.find("15->5") == std::string::npos);
}

TEST_F(TApp, BoundTests) {
    double value;
    app.add_option("-s", value)->transform(CLI::Bound(3.4, 5.9));
    args = {"-s", "15"};
    run();
    EXPECT_EQ(value, 5.9);

    args = {"-s", "3.689"};
    run();
    EXPECT_EQ(value, std::stod("3.689"));

    // value can't be converted to int so it is just ignored
    args = {"-s", "abcd"};
    EXPECT_THROW(run(), CLI::ValidationError);

    args = {"-s", "2.5"};
    run();
    EXPECT_EQ(value, 3.4);

    auto help = app.help();
    EXPECT_TRUE(help.find("bounded to") != std::string::npos);
    EXPECT_TRUE(help.find("[3.4 - 5.9]") != std::string::npos);
}

TEST_F(TApp, SuffixedNumberCorrecltySplitSuffix) {
    std::map<std::string, int> mapping{{"a", 10}, {"b", 100}, {"cc", 1000}};

    int value;
    app.add_option("-n", value)->transform(CLI::SuffixedNumber(mapping));

    args = {"-n", "42"};
    run();
    EXPECT_EQ(value, 42);

    args = {"-n", "42a"};
    run();
    EXPECT_EQ(value, 420);

    args = {"-n", "  42  cc  "};
    run();
    EXPECT_EQ(value, 42000);
    args = {"-n", "  -42  cc  "};
    run();
    EXPECT_EQ(value, -42000);
}

TEST_F(TApp, SuffixedNumberFloatTest) {
    std::map<std::string, double> mapping{{"a", 10}, {"b", 100}, {"cc", 1000}};
    double value;
    app.add_option("-n", value)->transform(CLI::SuffixedNumber(mapping));

    args = {"-n", "42"};
    run();
    EXPECT_DOUBLE_EQ(value, 42);

    args = {"-n", ".5"};
    run();
    EXPECT_DOUBLE_EQ(value, .5);

    args = {"-n", "42.5 a"};
    run();
    EXPECT_DOUBLE_EQ(value, 425);

    args = {"-n", "42.cc"};
    run();
    EXPECT_DOUBLE_EQ(value, 42000);
}

TEST_F(TApp, SuffixedNumberCaseSensetive) {
    std::map<std::string, int> mapping{{"a", 10}, {"A", 100}};

    int value;
    app.add_option("-n", value)->transform(CLI::SuffixedNumber(mapping, CLI::SuffixedNumber::CASE_SENSITIVE));

    args = {"-n", "42a"};
    run();
    EXPECT_EQ(value, 420);

    args = {"-n", "42A"};
    run();
    EXPECT_EQ(value, 4200);
}

TEST_F(TApp, SuffixedNumberCaseInsensitive) {
    std::map<std::string, int> mapping{{"a", 10}, {"B", 100}};

    int value;
    app.add_option("-n", value)->transform(CLI::SuffixedNumber(mapping, CLI::SuffixedNumber::CASE_INSENSITIVE));

    args = {"-n", "42a"};
    run();
    EXPECT_EQ(value, 420);

    args = {"-n", "42A"};
    run();
    EXPECT_EQ(value, 420);

    args = {"-n", "42b"};
    run();
    EXPECT_EQ(value, 4200);

    args = {"-n", "42B"};
    run();
    EXPECT_EQ(value, 4200);
}

TEST_F(TApp, SuffixedNumberMandatorySuffix) {
    std::map<std::string, int> mapping{{"a", 10}, {"A", 100}};

    int value;
    app.add_option("-n", value)
        ->transform(CLI::SuffixedNumber(
            mapping,
            CLI::SuffixedNumber::Options(CLI::SuffixedNumber::MANDATORY_SUFFIX | CLI::SuffixedNumber::CASE_SENSITIVE)));

    args = {"-n", "42a"};
    run();
    EXPECT_EQ(value, 420);

    args = {"-n", "42A"};
    run();
    EXPECT_EQ(value, 4200);

    args = {"-n", "42"};
    EXPECT_THROW(run(), CLI::ValidationError);
}

TEST_F(TApp, SuffixedNumberMandatorySuffix2) {
    std::map<std::string, int> mapping{{"a", 10}, {"B", 100}};

    int value;
    app.add_option("-n", value)
        ->transform(CLI::SuffixedNumber(mapping,
                                        CLI::SuffixedNumber::Options(CLI::SuffixedNumber::MANDATORY_SUFFIX |
                                                                     CLI::SuffixedNumber::CASE_INSENSITIVE)));

    args = {"-n", "42A"};
    run();
    EXPECT_EQ(value, 420);

    args = {"-n", "42b"};
    run();
    EXPECT_EQ(value, 4200);

    args = {"-n", "42"};
    EXPECT_THROW(run(), CLI::ValidationError);
}

TEST_F(TApp, SuffixedNumberBadMapping) {
    EXPECT_THROW(
        CLI::SuffixedNumber(std::map<std::string, int>{{"a", 10}, {"A", 100}}, CLI::SuffixedNumber::CASE_INSENSITIVE),
        CLI::ValidationError);
    EXPECT_THROW(CLI::SuffixedNumber(std::map<std::string, int>{{"a", 10}, {"9", 100}}), CLI::ValidationError);
    EXPECT_THROW(CLI::SuffixedNumber(std::map<std::string, int>{{"a", 10}, {"AA A", 100}}), CLI::ValidationError);
    EXPECT_THROW(CLI::SuffixedNumber(std::map<std::string, int>{{"a", 10}, {"", 100}}), CLI::ValidationError);
}

TEST_F(TApp, SuffixedNumberBadInput) {
    std::map<std::string, int> mapping{{"a", 10}, {"b", 100}};

    int value;
    app.add_option("-n", value)->transform(CLI::SuffixedNumber(mapping));

    args = {"-n", "13 a b"};
    EXPECT_THROW(run(), CLI::ValidationError);
    args = {"-n", "13 c"};
    EXPECT_THROW(run(), CLI::ValidationError);
    args = {"-n", "a"};
    EXPECT_THROW(run(), CLI::ValidationError);
    args = {"-n", "12.0a"};
    EXPECT_THROW(run(), CLI::ValidationError);
    args = {"-n", "a5"};
    EXPECT_THROW(run(), CLI::ValidationError);
    args = {"-n", ""};
    EXPECT_THROW(run(), CLI::ValidationError);
    args = {"-n", "13 a-"};
    EXPECT_THROW(run(), CLI::ValidationError);
}

TEST_F(TApp, SuffixedNumberIntOverlow) {
    std::map<std::string, int> mapping{{"a", 1000000}, {"b", 100}, {"c", 101}};

    int32_t value;
    app.add_option("-n", value)->transform(CLI::SuffixedNumber(mapping));

    args = {"-n", "1000 a"};
    run();
    EXPECT_EQ(value, 1000000000);

    args = {"-n", "1000000 a"};
    EXPECT_THROW(run(), CLI::ValidationError);

    args = {"-n", "-1000000 a"};
    EXPECT_THROW(run(), CLI::ValidationError);

    args = {"-n", "21474836 b"};
    run();
    EXPECT_EQ(value, 2147483600);

    args = {"-n", "21474836 c"};
    EXPECT_THROW(run(), CLI::ValidationError);
}

TEST_F(TApp, SuffixedNumberFloatOverlow) {
    std::map<std::string, float> mapping{{"a", 2}, {"b", 1}, {"c", 0}};

    float value;
    app.add_option("-n", value)->transform(CLI::SuffixedNumber(mapping));

    args = {"-n", "3e+38 a"};
    EXPECT_THROW(run(), CLI::ValidationError);

    args = {"-n", "3e+38 b"};
    run();
    EXPECT_FLOAT_EQ(value, 3e+38);

    args = {"-n", "3e+38 c"};
    run();
    EXPECT_FLOAT_EQ(value, 0);
}
