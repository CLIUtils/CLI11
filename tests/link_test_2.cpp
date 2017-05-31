#include "CLI/CLI.hpp"
#include "CLI/Timer.hpp"
#include <gtest/gtest.h>

int do_nothing();

// Verifies there are no ungarded inlines
TEST(Link, DoNothing) {
    int a = do_nothing();
    EXPECT_EQ(7, a);
}
