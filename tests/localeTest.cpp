// Copyright (c) 2017-2026, University of Cincinnati, developed by Henry Schreiner
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
#include <stdexcept>
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

namespace {
struct RestoreLocale {
    std::locale previous = std::locale();
    ~RestoreLocale() { std::locale::global(previous); }
};
}  // namespace

#if CLI11_HAS_RTTI != 0
TEST_CASE_METHOD(TApp, "SumOptFloatDecimalCommaFacet", "[locale]") {
    class DecimalComma : public std::numpunct<char> {
        CLI11_NODISCARD char do_decimal_point() const override { return ','; }
    };
    RestoreLocale restore;
    const std::locale custom_locale(std::locale::classic(), new DecimalComma);
    std::locale::global(custom_locale);

    double value = 0;
    auto *option = app.add_option("--val", value)->multi_option_policy(CLI::MultiOptionPolicy::Sum);

    args = {"--val=1.25", "--val=2.5"};
    run();
    CHECK(value == 3.75);
    CHECK(option->as<std::string>() == "3.75");

    args = {"--val=-1.25", "--val=-2.5"};
    run();
    CHECK(value == -3.75);
    CHECK(option->as<std::string>() == "-3.75");
}

TEST_CASE_METHOD(TApp, "FloatDefaultDecimalCommaFacet", "[locale]") {
    class DecimalComma : public std::numpunct<char> {
        CLI11_NODISCARD char do_decimal_point() const override { return ','; }
    };
    RestoreLocale restore;
    std::locale::global(std::locale(std::locale::classic(), new DecimalComma));

    double value = 0;
    float fvalue = 2.25F;
    app.add_option("--val", value)->default_val(3.5);
    auto *option = app.add_option("--fval", fvalue)->capture_default_str();

    run();
    CHECK(value == 3.5);
    CHECK(option->get_default_str() == "2.25");
}
#endif

TEST_CASE_METHOD(TApp, "SumOptFloatDecimalCommaNamedLocale", "[locale]") {
    RestoreLocale restore;
    bool found{false};
    for(const char *name : {"de_DE.UTF-8", "de_DE.utf8", "fr_FR.UTF-8", "fr_FR.utf8"}) {
        try {
            std::locale::global(std::locale(name));
            found = true;
            break;
        } catch(const std::runtime_error &) {
            continue;  // locale not installed
        }
    }
    if(!found) {
        WARN("No decimal comma locale installed, skipping");
        return;
    }

    double value = 0;
    app.add_option("--val", value)->multi_option_policy(CLI::MultiOptionPolicy::Sum);

    args = {"--val=1,25", "--val=2,5"};
    run();
    CHECK(value == 3.75);

    args = {"--val=1,25"};
    run();
    CHECK(value == 1.25);
}
