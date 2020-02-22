// Copyright (c) 2017-2020, University of Cincinnati, developed by Henry Schreiner
// under NSF AWARD 1414736 and by the respective contributors.
// All rights reserved.
//
// SPDX-License-Identifier: BSD-3-Clause

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
