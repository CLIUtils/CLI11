#include "app_helper.hpp"
#include <Windows.h>

// This test verifies that CLI11 still works if
// Windows.h is included. #145

TEST_F(TApp, WindowsTestSimple) {
    app.add_flag("-c,--count");
    args = {"-c"};
    run();
    EXPECT_EQ((size_t)1, app.count("-c"));
    EXPECT_EQ((size_t)1, app.count("--count"));
}
