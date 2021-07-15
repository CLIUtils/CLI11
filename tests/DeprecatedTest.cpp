// Copyright (c) 2017-2021, University of Cincinnati, developed by Henry Schreiner
// under NSF AWARD 1414736 and by the respective contributors.
// All rights reserved.
//
// SPDX-License-Identifier: BSD-3-Clause

#include "app_helper.hpp"

using Catch::Matchers::Contains;

TEST_CASE("Deprecated: Empty", "[deprecated]") {
    // No deprecated features at this time.
    CHECK(true);
}
