#include <cstdlib>
#include <iostream>

#include "app_helper.hpp"

#ifdef CLI11_OPTIONAL

TEST_F(TApp, OptionalTest) {
    CLI::optional<int> opt;
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

#else

TEST_F(TApp, DISABLED_OptionalTest) {}

#endif
