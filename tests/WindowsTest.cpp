#include "app_helper.hpp"
#include <Windows.h>

// This test verifies that CLI11 still works if
// Windows.h is included. #145

TEST_F(TApp, WindowsTestSimple) {
    app.add_flag("-c,--count");
    args = {"-c"};
    run();
    EXPECT_EQ(1u, app.count("-c"));
    EXPECT_EQ(1u, app.count("--count"));
}
