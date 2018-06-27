#include <cstdlib>
#include <iostream>

#include "app_helper.hpp"

#if CLI11_STD_OPTIONAL

TEST_F(TApp, StdOptionalTest) {
    std::optional<int> opt;
    app.add_option("-c,--count", opt);
    run();
    EXPECT_FALSE(opt);

    args = {"-c", "1"};
    run();
    EXPECT_TRUE(opt);
    EXPECT_EQ(*opt, 1);

    args = {"--count", "3"};
    run();
    EXPECT_TRUE(opt);
    EXPECT_EQ(*opt, 3);
}

#endif
#if CLI11_EXPERIMENTAL_OPTIONAL

TEST_F(TApp, ExperimentalOptionalTest) {
    std::experimental::optional<int> opt;
    app.add_option("-c,--count", opt);
    run();
    EXPECT_FALSE(opt);

    args = {"-c", "1"};
    run();
    EXPECT_TRUE(opt);
    EXPECT_EQ(*opt, 1);

    args = {"--count", "3"};
    run();
    EXPECT_TRUE(opt);
    EXPECT_EQ(*opt, 3);
}

#endif
#if CLI11_BOOST_OPTIONAL

TEST_F(TApp, BoostOptionalTest) {
    boost::optional<int> opt;
    app.add_option("-c,--count", opt);
    run();
    EXPECT_FALSE(opt);

    args = {"-c", "1"};
    run();
    EXPECT_TRUE(opt);
    EXPECT_EQ(*opt, 1);

    args = {"--count", "3"};
    run();
    EXPECT_TRUE(opt);
    EXPECT_EQ(*opt, 3);
}

#endif

#if !CLI11_OPTIONAL
TEST_F(TApp, DISABLED_OptionalTest) {}
#endif
