#include <cstdlib>
#include <iostream>

#if CLI11_OPTIONAL

#include "app_helper.hpp"

TEST_F(TApp, OptionalTest) {
    optional<int> opt;
    app.add_option("-c,--count", opt);
    run();
    EXPECT_FALSE(opt);

    app.reset();
    args = {"-c", "1"};
    run();
    EXPECT_TRUE(opt);
    EXPECT_EQ(*opt, 1);

    app.reset();
    args = {"--count", "3"};
    run();
    EXPECT_TRUE(opt);
    EXPECT_EQ(*opt, 3);
}

#endif
