#ifdef CLI11_SINGLE_FILE
#include "CLI11.hpp"
#else
#include "CLI/CLI.hpp"
#endif

#include "gtest/gtest.h"

TEST(Deprecated, Emtpy) {
    // No deprecated features at this time.
    EXPECT_TRUE(true);
}
