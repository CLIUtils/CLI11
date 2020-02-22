// Copyright (c) 2017-2020, University of Cincinnati, developed by Henry Schreiner
// under NSF AWARD 1414736 and by the respective contributors.
// All rights reserved.
//
// SPDX-License-Identifier: BSD-3-Clause

#ifdef CLI11_SINGLE_FILE
#include "CLI11.hpp"
#else
#include "CLI/CLI.hpp"
#endif

#include "gtest/gtest.h"

using input_t = std::vector<std::string>;

TEST(Basic, Empty) {

    {
        CLI::App app;
        input_t simpleput;
        app.parse(simpleput);
    }
    {
        CLI::App app;
        input_t spare = {"spare"};
        EXPECT_THROW(app.parse(spare), CLI::ExtrasError);
    }
    {
        CLI::App app;
        input_t simpleput;
        app.parse(simpleput);
    }
}
