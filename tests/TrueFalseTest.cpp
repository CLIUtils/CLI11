// Copyright (c) 2017-2020, University of Cincinnati, developed by Henry Schreiner
// under NSF AWARD 1414736 and by the respective contributors.
// All rights reserved.
//
// SPDX-License-Identifier: BSD-3-Clause

#include "app_helper.hpp"

/// This allows a set of strings to be run over by a test
struct TApp_TBO : public TApp_base, testing::TestWithParam<const char *> {};

TEST_P(TApp_TBO, TrueBoolOption) {
    bool value{false};  // Not used, but set just in case
    app.add_option("-b,--bool", value);
    args = {"--bool", GetParam()};
    run();
    EXPECT_EQ(1u, app.count("--bool"));
    EXPECT_TRUE(value);
}

// Change to INSTANTIATE_TEST_SUITE_P in GTest master
INSTANTIATE_TEST_SUITE_P(TrueBoolOptions_test, TApp_TBO, testing::Values("true", "on", "True", "ON"));

/// This allows a set of strings to be run over by a test
struct TApp_FBO : public TApp_base, public ::testing::TestWithParam<const char *> {};

TEST_P(TApp_FBO, FalseBoolOptions) {
    bool value{true};  // Not used, but set just in case
    app.add_option("-b,--bool", value);
    args = {"--bool", GetParam()};
    run();
    EXPECT_EQ(1u, app.count("--bool"));
    EXPECT_FALSE(value);
}

INSTANTIATE_TEST_SUITE_P(FalseBoolOptions_test, TApp_FBO, ::testing::Values("false", "off", "False", "OFF"));
