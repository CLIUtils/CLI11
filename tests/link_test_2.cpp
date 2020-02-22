// Copyright (c) 2017-2020, University of Cincinnati, developed by Henry Schreiner
// under NSF AWARD 1414736 and by the respective contributors.
// All rights reserved.
//
// SPDX-License-Identifier: BSD-3-Clause

#include "CLI/CLI.hpp"
#include "CLI/Timer.hpp"
#include <gtest/gtest.h>

int do_nothing();

// Verifies there are no unguarded inlines
TEST(Link, DoNothing) {
    int a = do_nothing();
    EXPECT_EQ(7, a);
}
