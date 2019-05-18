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

TEST_F(TApp, NumberWithUnitCorrecltySplitNumber) {
    std::map<std::string, int> mapping{{"a", 10}, {"b", 100}, {"cc", 1000}};

    int value = 0;
    app.add_option("-n", value)->transform(CLI::AsNumberWithUnit(mapping));

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

TEST_F(TApp, NumberWithUnitFloatTest) {
    std::map<std::string, double> mapping{{"a", 10}, {"b", 100}, {"cc", 1000}};
    double value = 0;
    app.add_option("-n", value)->transform(CLI::AsNumberWithUnit(mapping));

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

TEST_F(TApp, NumberWithUnitCaseSensitive) {
    std::map<std::string, int> mapping{{"a", 10}, {"A", 100}};

    int value = 0;
    app.add_option("-n", value)->transform(CLI::AsNumberWithUnit(mapping, CLI::AsNumberWithUnit::CASE_SENSITIVE));

    args = {"-n", "42a"};
    run();
    EXPECT_EQ(value, 420);

    args = {"-n", "42A"};
    run();
    EXPECT_EQ(value, 4200);
}

TEST_F(TApp, NumberWithUnitCaseInsensitive) {
    std::map<std::string, int> mapping{{"a", 10}, {"B", 100}};

    int value = 0;
    app.add_option("-n", value)->transform(CLI::AsNumberWithUnit(mapping, CLI::AsNumberWithUnit::CASE_INSENSITIVE));

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

TEST_F(TApp, NumberWithUnitMandatoryUnit) {
    std::map<std::string, int> mapping{{"a", 10}, {"A", 100}};

    int value;
    app.add_option("-n", value)
        ->transform(CLI::AsNumberWithUnit(mapping,
                                          CLI::AsNumberWithUnit::Options(CLI::AsNumberWithUnit::UNIT_REQUIRED |
                                                                         CLI::AsNumberWithUnit::CASE_SENSITIVE)));

    args = {"-n", "42a"};
    run();
    EXPECT_EQ(value, 420);

    args = {"-n", "42A"};
    run();
    EXPECT_EQ(value, 4200);

    args = {"-n", "42"};
    EXPECT_THROW(run(), CLI::ValidationError);
}

TEST_F(TApp, NumberWithUnitMandatoryUnit2) {
    std::map<std::string, int> mapping{{"a", 10}, {"B", 100}};

    int value;
    app.add_option("-n", value)
        ->transform(CLI::AsNumberWithUnit(mapping,
                                          CLI::AsNumberWithUnit::Options(CLI::AsNumberWithUnit::UNIT_REQUIRED |
                                                                         CLI::AsNumberWithUnit::CASE_INSENSITIVE)));

    args = {"-n", "42A"};
    run();
    EXPECT_EQ(value, 420);

    args = {"-n", "42b"};
    run();
    EXPECT_EQ(value, 4200);

    args = {"-n", "42"};
    EXPECT_THROW(run(), CLI::ValidationError);
}

TEST_F(TApp, NumberWithUnitBadMapping) {
    EXPECT_THROW(CLI::AsNumberWithUnit(std::map<std::string, int>{{"a", 10}, {"A", 100}},
                                       CLI::AsNumberWithUnit::CASE_INSENSITIVE),
                 CLI::ValidationError);
    EXPECT_THROW(CLI::AsNumberWithUnit(std::map<std::string, int>{{"a", 10}, {"9", 100}}), CLI::ValidationError);
    EXPECT_THROW(CLI::AsNumberWithUnit(std::map<std::string, int>{{"a", 10}, {"AA A", 100}}), CLI::ValidationError);
    EXPECT_THROW(CLI::AsNumberWithUnit(std::map<std::string, int>{{"a", 10}, {"", 100}}), CLI::ValidationError);
}

TEST_F(TApp, NumberWithUnitBadInput) {
    std::map<std::string, int> mapping{{"a", 10}, {"b", 100}};

    int value;
    app.add_option("-n", value)->transform(CLI::AsNumberWithUnit(mapping));

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

TEST_F(TApp, NumberWithUnitIntOverflow) {
    std::map<std::string, int> mapping{{"a", 1000000}, {"b", 100}, {"c", 101}};

    int32_t value;
    app.add_option("-n", value)->transform(CLI::AsNumberWithUnit(mapping));

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

TEST_F(TApp, NumberWithUnitFloatOverflow) {
    std::map<std::string, float> mapping{{"a", 2.f}, {"b", 1.f}, {"c", 0.f}};

    float value;
    app.add_option("-n", value)->transform(CLI::AsNumberWithUnit(mapping));

    args = {"-n", "3e+38 a"};
    EXPECT_THROW(run(), CLI::ValidationError);

    args = {"-n", "3e+38 b"};
    run();
    EXPECT_FLOAT_EQ(value, 3e+38f);

    args = {"-n", "3e+38 c"};
    run();
    EXPECT_FLOAT_EQ(value, 0.f);
}

TEST_F(TApp, AsSizeValue1000_1024) {
    uint64_t value;
    app.add_option("-s", value)->transform(CLI::AsSizeValue(true));

    args = {"-s", "10240"};
    run();
    EXPECT_EQ(value, 10240u);

    args = {"-s", "1b"};
    run();
    EXPECT_EQ(value, 1u);

    uint64_t k_value = 1000u;
    uint64_t ki_value = 1024u;
    args = {"-s", "1k"};
    run();
    EXPECT_EQ(value, k_value);
    args = {"-s", "1kb"};
    run();
    EXPECT_EQ(value, k_value);
    args = {"-s", "1 Kb"};
    run();
    EXPECT_EQ(value, k_value);
    args = {"-s", "1ki"};
    run();
    EXPECT_EQ(value, ki_value);
    args = {"-s", "1kib"};
    run();
    EXPECT_EQ(value, ki_value);

    k_value = 1000ull * 1000u;
    ki_value = 1024ull * 1024u;
    args = {"-s", "1m"};
    run();
    EXPECT_EQ(value, k_value);
    args = {"-s", "1mb"};
    run();
    EXPECT_EQ(value, k_value);
    args = {"-s", "1mi"};
    run();
    EXPECT_EQ(value, ki_value);
    args = {"-s", "1mib"};
    run();
    EXPECT_EQ(value, ki_value);

    k_value = 1000ull * 1000u * 1000u;
    ki_value = 1024ull * 1024u * 1024u;
    args = {"-s", "1g"};
    run();
    EXPECT_EQ(value, k_value);
    args = {"-s", "1gb"};
    run();
    EXPECT_EQ(value, k_value);
    args = {"-s", "1gi"};
    run();
    EXPECT_EQ(value, ki_value);
    args = {"-s", "1gib"};
    run();
    EXPECT_EQ(value, ki_value);

    k_value = 1000ull * 1000u * 1000u * 1000u;
    ki_value = 1024ull * 1024u * 1024u * 1024u;
    args = {"-s", "1t"};
    run();
    EXPECT_EQ(value, k_value);
    args = {"-s", "1tb"};
    run();
    EXPECT_EQ(value, k_value);
    args = {"-s", "1ti"};
    run();
    EXPECT_EQ(value, ki_value);
    args = {"-s", "1tib"};
    run();
    EXPECT_EQ(value, ki_value);

    k_value = 1000ull * 1000u * 1000u * 1000u * 1000u;
    ki_value = 1024ull * 1024u * 1024u * 1024u * 1024u;
    args = {"-s", "1p"};
    run();
    EXPECT_EQ(value, k_value);
    args = {"-s", "1pb"};
    run();
    EXPECT_EQ(value, k_value);
    args = {"-s", "1pi"};
    run();
    EXPECT_EQ(value, ki_value);
    args = {"-s", "1pib"};
    run();
    EXPECT_EQ(value, ki_value);

    k_value = 1000ull * 1000u * 1000u * 1000u * 1000u * 1000u;
    ki_value = 1024ull * 1024u * 1024u * 1024u * 1024u * 1024u;
    args = {"-s", "1e"};
    run();
    EXPECT_EQ(value, k_value);
    args = {"-s", "1eb"};
    run();
    EXPECT_EQ(value, k_value);
    args = {"-s", "1ei"};
    run();
    EXPECT_EQ(value, ki_value);
    args = {"-s", "1eib"};
    run();
    EXPECT_EQ(value, ki_value);
}

TEST_F(TApp, AsSizeValue1024) {
    uint64_t value;
    app.add_option("-s", value)->transform(CLI::AsSizeValue(false));

    args = {"-s", "10240"};
    run();
    EXPECT_EQ(value, 10240u);

    args = {"-s", "1b"};
    run();
    EXPECT_EQ(value, 1u);

    uint64_t ki_value = 1024u;
    args = {"-s", "1k"};
    run();
    EXPECT_EQ(value, ki_value);
    args = {"-s", "1kb"};
    run();
    EXPECT_EQ(value, ki_value);
    args = {"-s", "1 Kb"};
    run();
    EXPECT_EQ(value, ki_value);
    args = {"-s", "1ki"};
    run();
    EXPECT_EQ(value, ki_value);
    args = {"-s", "1kib"};
    run();
    EXPECT_EQ(value, ki_value);

    ki_value = 1024ull * 1024u;
    args = {"-s", "1m"};
    run();
    EXPECT_EQ(value, ki_value);
    args = {"-s", "1mb"};
    run();
    EXPECT_EQ(value, ki_value);
    args = {"-s", "1mi"};
    run();
    EXPECT_EQ(value, ki_value);
    args = {"-s", "1mib"};
    run();
    EXPECT_EQ(value, ki_value);

    ki_value = 1024ull * 1024u * 1024u;
    args = {"-s", "1g"};
    run();
    EXPECT_EQ(value, ki_value);
    args = {"-s", "1gb"};
    run();
    EXPECT_EQ(value, ki_value);
    args = {"-s", "1gi"};
    run();
    EXPECT_EQ(value, ki_value);
    args = {"-s", "1gib"};
    run();
    EXPECT_EQ(value, ki_value);

    ki_value = 1024ull * 1024u * 1024u * 1024u;
    args = {"-s", "1t"};
    run();
    EXPECT_EQ(value, ki_value);
    args = {"-s", "1tb"};
    run();
    EXPECT_EQ(value, ki_value);
    args = {"-s", "1ti"};
    run();
    EXPECT_EQ(value, ki_value);
    args = {"-s", "1tib"};
    run();
    EXPECT_EQ(value, ki_value);

    ki_value = 1024ull * 1024u * 1024u * 1024u * 1024u;
    args = {"-s", "1p"};
    run();
    EXPECT_EQ(value, ki_value);
    args = {"-s", "1pb"};
    run();
    EXPECT_EQ(value, ki_value);
    args = {"-s", "1pi"};
    run();
    EXPECT_EQ(value, ki_value);
    args = {"-s", "1pib"};
    run();
    EXPECT_EQ(value, ki_value);

    ki_value = 1024ull * 1024u * 1024u * 1024u * 1024u * 1024u;
    args = {"-s", "1e"};
    run();
    EXPECT_EQ(value, ki_value);
    args = {"-s", "1eb"};
    run();
    EXPECT_EQ(value, ki_value);
    args = {"-s", "1ei"};
    run();
    EXPECT_EQ(value, ki_value);
    args = {"-s", "1eib"};
    run();
    EXPECT_EQ(value, ki_value);
}
