#include "app_helper.hpp"

/// This allows a set of strings to be run over by a test
struct TApp_TBO : public TApp, public ::testing::WithParamInterface<const char *> {};

TEST_P(TApp_TBO, TrueBoolOption) {
    bool value = false; // Not used, but set just in case
    app.add_option("-b,--bool", value);
    args = {"--bool", GetParam()};
    run();
    EXPECT_EQ(1u, app.count("--bool"));
    EXPECT_TRUE(value);
}

// Change to INSTANTIATE_TEST_SUITE_P in GTest master
INSTANTIATE_TEST_CASE_P(TrueBoolOptions, TApp_TBO, ::testing::Values("true", "on", "True", "ON"), );

/// This allows a set of strings to be run over by a test
struct TApp_FBO : public TApp, public ::testing::WithParamInterface<const char *> {};

TEST_P(TApp_FBO, FalseBoolOptions) {
    bool value = true; // Not used, but set just in case
    app.add_option("-b,--bool", value);
    args = {"--bool", GetParam()};
    run();
    EXPECT_EQ(1u, app.count("--bool"));
    EXPECT_FALSE(value);
}

INSTANTIATE_TEST_CASE_P(FalseBoolOptions, TApp_FBO, ::testing::Values("false", "off", "False", "OFF"), );
