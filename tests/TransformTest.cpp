#include "app_helper.hpp"

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
    auto opt = app.add_option("-s", value)->transform(CLI::Transformer({{"one", std::string("1")}}, CLI::ignore_case));
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
