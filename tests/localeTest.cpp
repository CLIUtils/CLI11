// Copyright (c) 2017-2025, University of Cincinnati, developed by Henry Schreiner
// under NSF AWARD 1414736 and by the respective contributors.
// All rights reserved.
//
// SPDX-License-Identifier: BSD-3-Clause

#include "app_helper.hpp"

#include <complex>
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <locale>
#include <numeric>
#include <string>
#include <utility>
#include <vector>

// Custom facet for thousands separator
class CustomThousandsSeparator : public std::numpunct<char> {
  protected:
    char do_thousands_sep() const override { return '|'; }     // Space separator
    std::string do_grouping() const override { return "\2"; }  // Group digits in sets of 2
};

#if CLI11_HAS_RTTI != 0
// derived from https://github.com/CLIUtils/CLI11/pull/1160
TEST_CASE_METHOD(TApp, "locale", "[separators]") {
    std::locale customLocale(std::locale::classic(), new CustomThousandsSeparator);
    std::locale::global(customLocale);  // Set as the default system-wide locale

    // Ensure standard streams use the custom locale automatically
    std::cout.imbue(std::locale());
    std::int64_t foo{0};
    std::uint64_t bar{0};
    float qux{0};

    app.add_option("FOO", foo, "Foo option")->default_val(1234567)->force_callback();
    app.add_option("BAR", bar, "Bar option")->default_val(2345678)->force_callback();
    app.add_option("QUX", qux, "QUX option")->default_val(3456.78)->force_callback();

    CHECK_NOTHROW(run());
    CHECK(foo == 1234567);
    CHECK(bar == 2345678);
    CHECK_THAT(qux, Catch::Matchers::WithinAbs(3456.78, 0.01));
}
#endif
