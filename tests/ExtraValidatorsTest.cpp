// Copyright (c) 2017-2025, University of Cincinnati, developed by Henry Schreiner
// under NSF AWARD 1414736 and by the respective contributors.
// All rights reserved.
//
// SPDX-License-Identifier: BSD-3-Clause

#include "app_helper.hpp"

#include <cmath>

#include <chrono>
#include <cstdint>
#include <map>
#include <string>
#include <utility>
#include <vector>

TEST_CASE("Validators:basic", "[helpers]") { CHECK(true); }

#if (defined(CLI11_ENABLE_EXTRA_VALIDATORS) && CLI11_ENABLE_EXTRA_VALIDATORS == 1) ||                                  \
    (!defined(CLI11_DISABLE_EXTRA_VALIDATORS) || CLI11_DISABLE_EXTRA_VALIDATORS == 0)

TEST_CASE("Validators: IPValidate1", "[helpers]") {
    std::string ip = "1.1.1.1";
    CHECK(CLI::ValidIPV4(ip).empty());
    ip = "224.255.0.1";
    CHECK(CLI::ValidIPV4(ip).empty());
    // check that it doesn't work with a trailing .
    ip = "224.255.0.1.";
    CHECK_FALSE(CLI::ValidIPV4(ip).empty());
    ip = "-1.255.0.1";
    CHECK_FALSE(CLI::ValidIPV4(ip).empty());
    ip = "1.256.0.1";
    CHECK_FALSE(CLI::ValidIPV4(ip).empty());
    ip = "1.223.0.";
    CHECK_FALSE(CLI::ValidIPV4(ip).empty());
    ip = "1.256.0.1";
    CHECK_FALSE(CLI::ValidIPV4(ip).empty());
    ip = "aaa";
    CHECK_FALSE(CLI::ValidIPV4(ip).empty());
    ip = "1.2.3.abc";
    CHECK_FALSE(CLI::ValidIPV4(ip).empty());
    ip = "11.22";
    CHECK_FALSE(CLI::ValidIPV4(ip).empty());
}

TEST_CASE("Validators: NumberValidator", "[helpers]") {
    std::string num = "1.1.1.1";
    CHECK_FALSE(CLI::Number(num).empty());
    num = "1.7";
    CHECK(CLI::Number(num).empty());
    num = "10000";
    CHECK(CLI::Number(num).empty());
    num = "-0.000";
    CHECK(CLI::Number(num).empty());
    num = "+1.55";
    CHECK(CLI::Number(num).empty());
    num = "a";
    CHECK_FALSE(CLI::Number(num).empty());
}

// Tests positionals validation at the end
TEST_CASE_METHOD(TApp, "PositionalValidation", "[app]") {
    std::string options;
    std::string foo;

    app.add_option("bar", options)->check(CLI::Number.name("valbar"));
    // disable the check on foo
    app.add_option("foo", foo)->check(CLI::Number.active(false));
    app.validate_positionals();
    args = {"1", "param1"};
    run();

    CHECK("1" == options);
    CHECK("param1" == foo);

    args = {"param1", "1"};
    CHECK_NOTHROW(run());

    CHECK("1" == options);
    CHECK("param1" == foo);

    CHECK(nullptr != app.get_option("bar")->get_validator("valbar"));
}

TEST_CASE_METHOD(TApp, "BoundTests", "[transform]") {
    double value = NAN;
    app.add_option("-s", value)->transform(CLI::Bound(3.4, 5.9));
    args = {"-s", "15"};
    run();
    CHECK(5.9 == value);

    args = {"-s", "3.689"};
    run();
    CHECK(std::stod("3.689") == value);

    // value can't be converted to int so it is just ignored
    args = {"-s", "abcd"};
    CHECK_THROWS_AS(run(), CLI::ValidationError);

    args = {"-s", "2.5"};
    run();
    CHECK(3.4 == value);

    auto help = app.help();
    CHECK(help.find("bounded to") != std::string::npos);
    CHECK(help.find("[3.4 - 5.9]") != std::string::npos);
}

TEST_CASE_METHOD(TApp, "typeCheck", "[app]") {

    /// Note that this must be a double in Range, too
    app.add_option("--one")->check(CLI::TypeValidator<unsigned int>());

    args = {"--one=1"};
    CHECK_NOTHROW(run());

    args = {"--one=-7"};
    CHECK_THROWS_AS(run(), CLI::ValidationError);

    args = {"--one=error"};
    CHECK_THROWS_AS(run(), CLI::ValidationError);

    args = {"--one=4.568"};
    CHECK_THROWS_AS(run(), CLI::ValidationError);
}

TEST_CASE_METHOD(TApp, "NumberWithUnitCorrectlySplitNumber", "[transform]") {
    std::map<std::string, int> mapping{{"a", 10}, {"b", 100}, {"cc", 1000}};

    int value = 0;
    app.add_option("-n", value)->transform(CLI::AsNumberWithUnit(mapping));

    args = {"-n", "42"};
    run();
    CHECK(42 == value);

    args = {"-n", "42a"};
    run();
    CHECK(420 == value);

    args = {"-n", "  42  cc  "};
    run();
    CHECK(42000 == value);
    args = {"-n", "  -42  cc  "};
    run();
    CHECK(-42000 == value);
}

TEST_CASE_METHOD(TApp, "NumberWithUnitFloatTest", "[transform]") {
    std::map<std::string, double> mapping{{"a", 10}, {"b", 100}, {"cc", 1000}};
    double value{0.0};
    app.add_option("-n", value)->transform(CLI::AsNumberWithUnit(mapping));

    args = {"-n", "42"};
    run();
    CHECK(42 == Approx(value));

    args = {"-n", ".5"};
    run();
    CHECK(.5 == Approx(value));

    args = {"-n", "42.5 a"};
    run();
    CHECK(425 == Approx(value));

    args = {"-n", "42.cc"};
    run();
    CHECK(42000 == Approx(value));
}

TEST_CASE_METHOD(TApp, "NumberWithUnitCaseSensitive", "[transform]") {
    std::map<std::string, int> mapping{{"a", 10}, {"A", 100}};

    int value{0};
    app.add_option("-n", value)->transform(CLI::AsNumberWithUnit(mapping, CLI::AsNumberWithUnit::CASE_SENSITIVE));

    args = {"-n", "42a"};
    run();
    CHECK(420 == value);

    args = {"-n", "42A"};
    run();
    CHECK(4200 == value);
}

TEST_CASE_METHOD(TApp, "NumberWithUnitCaseInsensitive", "[transform]") {
    std::map<std::string, int> mapping{{"a", 10}, {"B", 100}};

    int value{0};
    app.add_option("-n", value)->transform(CLI::AsNumberWithUnit(mapping, CLI::AsNumberWithUnit::CASE_INSENSITIVE));

    args = {"-n", "42a"};
    run();
    CHECK(420 == value);

    args = {"-n", "42A"};
    run();
    CHECK(420 == value);

    args = {"-n", "42b"};
    run();
    CHECK(4200 == value);

    args = {"-n", "42B"};
    run();
    CHECK(4200 == value);
}

TEST_CASE_METHOD(TApp, "NumberWithUnitMandatoryUnit", "[transform]") {
    std::map<std::string, int> mapping{{"a", 10}, {"A", 100}};

    int value{0};
    app.add_option("-n", value)
        ->transform(CLI::AsNumberWithUnit(
            mapping, CLI::AsNumberWithUnit::UNIT_REQUIRED | CLI::AsNumberWithUnit::CASE_SENSITIVE));

    args = {"-n", "42a"};
    run();
    CHECK(420 == value);

    args = {"-n", "42A"};
    run();
    CHECK(4200 == value);

    args = {"-n", "42"};
    CHECK_THROWS_AS(run(), CLI::ValidationError);
}

TEST_CASE_METHOD(TApp, "NumberWithUnitMandatoryUnit2", "[transform]") {
    std::map<std::string, int> mapping{{"a", 10}, {"B", 100}};

    int value{0};
    app.add_option("-n", value)
        ->transform(CLI::AsNumberWithUnit(
            mapping, CLI::AsNumberWithUnit::UNIT_REQUIRED | CLI::AsNumberWithUnit::CASE_INSENSITIVE));

    args = {"-n", "42A"};
    run();
    CHECK(420 == value);

    args = {"-n", "42b"};
    run();
    CHECK(4200 == value);

    args = {"-n", "42"};
    CHECK_THROWS_AS(run(), CLI::ValidationError);
}

TEST_CASE_METHOD(TApp, "NumberWithUnitBadMapping", "[transform]") {
    CHECK_THROWS_AS(CLI::AsNumberWithUnit(std::map<std::string, int>{{"a", 10}, {"A", 100}},
                                          CLI::AsNumberWithUnit::CASE_INSENSITIVE),
                    CLI::ValidationError);
    CHECK_THROWS_AS(CLI::AsNumberWithUnit(std::map<std::string, int>{{"a", 10}, {"9", 100}}), CLI::ValidationError);
    CHECK_THROWS_AS(CLI::AsNumberWithUnit(std::map<std::string, int>{{"a", 10}, {"AA A", 100}}), CLI::ValidationError);
    CHECK_THROWS_AS(CLI::AsNumberWithUnit(std::map<std::string, int>{{"a", 10}, {"", 100}}), CLI::ValidationError);
}

TEST_CASE_METHOD(TApp, "NumberWithUnitBadInput", "[transform]") {
    std::map<std::string, int> mapping{{"a", 10}, {"b", 100}};

    int value{0};
    app.add_option("-n", value)->transform(CLI::AsNumberWithUnit(mapping));

    args = {"-n", "13 a b"};
    CHECK_THROWS_AS(run(), CLI::ValidationError);
    args = {"-n", "13 c"};
    CHECK_THROWS_AS(run(), CLI::ValidationError);
    args = {"-n", "a"};
    // Assume 1.0 unit
    CHECK_NOTHROW(run());
    args = {"-n", "12.0a"};
    CHECK_THROWS_AS(run(), CLI::ValidationError);
    args = {"-n", "a5"};
    CHECK_THROWS_AS(run(), CLI::ValidationError);
    args = {"-n", ""};
    CHECK_THROWS_AS(run(), CLI::ValidationError);
    args = {"-n", "13 a-"};
    CHECK_THROWS_AS(run(), CLI::ValidationError);
}

TEST_CASE_METHOD(TApp, "NumberWithUnitIntOverflow", "[transform]") {
    std::map<std::string, int> mapping{{"a", 1000000}, {"b", 100}, {"c", 101}};

    std::int32_t value = 0;
    app.add_option("-n", value)->transform(CLI::AsNumberWithUnit(mapping));

    args = {"-n", "1000 a"};
    run();
    CHECK(1000000000 == value);

    args = {"-n", "1000000 a"};
    CHECK_THROWS_AS(run(), CLI::ValidationError);

    args = {"-n", "-1000000 a"};
    CHECK_THROWS_AS(run(), CLI::ValidationError);

    args = {"-n", "21474836 b"};
    run();
    CHECK(2147483600 == value);

    args = {"-n", "21474836 c"};
    CHECK_THROWS_AS(run(), CLI::ValidationError);
}

TEST_CASE_METHOD(TApp, "NumberWithUnitFloatOverflow", "[transform]") {
    std::map<std::string, float> mapping{{"a", 2.f}, {"b", 1.f}, {"c", 0.f}};

    float value{0.0F};
    app.add_option("-n", value)->transform(CLI::AsNumberWithUnit(mapping));

    args = {"-n", "3e+38 a"};
    CHECK_THROWS_AS(run(), CLI::ValidationError);

    args = {"-n", "3e+38 b"};
    run();
    CHECK(3e+38f == Approx(value));

    args = {"-n", "3e+38 c"};
    run();
    CHECK(0.f == Approx(value));
}

TEST_CASE_METHOD(TApp, "AsSizeValue1000_1024", "[transform]") {
    std::uint64_t value{0};
    app.add_option("-s", value)->transform(CLI::AsSizeValue(true));

    args = {"-s", "10240"};
    run();
    CHECK(10240u == value);

    args = {"-s", "1b"};
    run();
    CHECK(1u == value);

    std::uint64_t k_value{1000u};
    std::uint64_t ki_value{1024u};
    args = {"-s", "1k"};
    run();
    CHECK(k_value == value);
    args = {"-s", "1kb"};
    run();
    CHECK(k_value == value);
    args = {"-s", "1 Kb"};
    run();
    CHECK(k_value == value);
    args = {"-s", "1ki"};
    run();
    CHECK(ki_value == value);
    args = {"-s", "1kib"};
    run();
    CHECK(ki_value == value);

    k_value = 1000ull * 1000u;
    ki_value = 1024ull * 1024u;
    args = {"-s", "1m"};
    run();
    CHECK(k_value == value);
    args = {"-s", "1mb"};
    run();
    CHECK(k_value == value);
    args = {"-s", "1mi"};
    run();
    CHECK(ki_value == value);
    args = {"-s", "1mib"};
    run();
    CHECK(ki_value == value);

    k_value = 1000ull * 1000u * 1000u;
    ki_value = 1024ull * 1024u * 1024u;
    args = {"-s", "1g"};
    run();
    CHECK(k_value == value);
    args = {"-s", "1gb"};
    run();
    CHECK(k_value == value);
    args = {"-s", "1gi"};
    run();
    CHECK(ki_value == value);
    args = {"-s", "1gib"};
    run();
    CHECK(ki_value == value);

    k_value = 1000ull * 1000u * 1000u * 1000u;
    ki_value = 1024ull * 1024u * 1024u * 1024u;
    args = {"-s", "1t"};
    run();
    CHECK(k_value == value);
    args = {"-s", "1tb"};
    run();
    CHECK(k_value == value);
    args = {"-s", "1ti"};
    run();
    CHECK(ki_value == value);
    args = {"-s", "1tib"};
    run();
    CHECK(ki_value == value);

    k_value = 1000ull * 1000u * 1000u * 1000u * 1000u;
    ki_value = 1024ull * 1024u * 1024u * 1024u * 1024u;
    args = {"-s", "1p"};
    run();
    CHECK(k_value == value);
    args = {"-s", "1pb"};
    run();
    CHECK(k_value == value);
    args = {"-s", "1pi"};
    run();
    CHECK(ki_value == value);
    args = {"-s", "1pib"};
    run();
    CHECK(ki_value == value);

    k_value = 1000ull * 1000u * 1000u * 1000u * 1000u * 1000u;
    ki_value = 1024ull * 1024u * 1024u * 1024u * 1024u * 1024u;
    args = {"-s", "1e"};
    run();
    CHECK(k_value == value);
    args = {"-s", "1eb"};
    run();
    CHECK(k_value == value);
    args = {"-s", "1ei"};
    run();
    CHECK(ki_value == value);
    args = {"-s", "1eib"};
    run();
    CHECK(ki_value == value);
}

TEST_CASE_METHOD(TApp, "duration_test", "[transform]") {
    std::chrono::seconds duration{1};

    app.option_defaults()->ignore_case();
    app.add_option_function<std::size_t>(
           "--duration",
           [&](size_t a_value) { duration = std::chrono::seconds{a_value}; },
           "valid units: sec, min, h, day.")
        ->capture_default_str()
        ->transform(CLI::AsNumberWithUnit(
            std::map<std::string, std::size_t>{{"sec", 1}, {"min", 60}, {"h", 3600}, {"day", 24 * 3600}}));
    CHECK_NOTHROW(app.parse(std::vector<std::string>{"1 day", "--duration"}));

    CHECK(std::chrono::seconds(86400) == duration);
}

TEST_CASE_METHOD(TApp, "AsSizeValue1024", "[transform]") {
    std::uint64_t value{0};
    app.add_option("-s", value)->transform(CLI::AsSizeValue(false));

    args = {"-s", "10240"};
    run();
    CHECK(10240u == value);

    args = {"-s", "1b"};
    run();
    CHECK(1u == value);

    std::uint64_t ki_value{1024u};
    args = {"-s", "1k"};
    run();
    CHECK(ki_value == value);
    args = {"-s", "1kb"};
    run();
    CHECK(ki_value == value);
    args = {"-s", "1 Kb"};
    run();
    CHECK(ki_value == value);
    args = {"-s", "1ki"};
    run();
    CHECK(ki_value == value);
    args = {"-s", "1kib"};
    run();
    CHECK(ki_value == value);

    ki_value = 1024ull * 1024u;
    args = {"-s", "1m"};
    run();
    CHECK(ki_value == value);
    args = {"-s", "1mb"};
    run();
    CHECK(ki_value == value);
    args = {"-s", "1mi"};
    run();
    CHECK(ki_value == value);
    args = {"-s", "1mib"};
    run();
    CHECK(ki_value == value);

    ki_value = 1024ull * 1024u * 1024u;
    args = {"-s", "1g"};
    run();
    CHECK(ki_value == value);
    args = {"-s", "1gb"};
    run();
    CHECK(ki_value == value);
    args = {"-s", "1gi"};
    run();
    CHECK(ki_value == value);
    args = {"-s", "1gib"};
    run();
    CHECK(ki_value == value);

    ki_value = 1024ull * 1024u * 1024u * 1024u;
    args = {"-s", "1t"};
    run();
    CHECK(ki_value == value);
    args = {"-s", "1tb"};
    run();
    CHECK(ki_value == value);
    args = {"-s", "1ti"};
    run();
    CHECK(ki_value == value);
    args = {"-s", "1tib"};
    run();
    CHECK(ki_value == value);

    ki_value = 1024ull * 1024u * 1024u * 1024u * 1024u;
    args = {"-s", "1p"};
    run();
    CHECK(ki_value == value);
    args = {"-s", "1pb"};
    run();
    CHECK(ki_value == value);
    args = {"-s", "1pi"};
    run();
    CHECK(ki_value == value);
    args = {"-s", "1pib"};
    run();
    CHECK(ki_value == value);

    ki_value = 1024ull * 1024u * 1024u * 1024u * 1024u * 1024u;
    args = {"-s", "1e"};
    run();
    CHECK(ki_value == value);
    args = {"-s", "1eb"};
    run();
    CHECK(ki_value == value);
    args = {"-s", "1ei"};
    run();
    CHECK(ki_value == value);
    args = {"-s", "1eib"};
    run();
    CHECK(ki_value == value);
}

#if (defined(CLI11_ENABLE_EXTRA_VALIDATORS) && CLI11_ENABLE_EXTRA_VALIDATORS == 1)

#if defined CLI11_HAS_FILESYSTEM && CLI11_HAS_FILESYSTEM > 0
#include <filesystem>

TEST_CASE_METHOD(TApp, "FileExistsForRead", "[validate]") {
    std::string myfile{"TestNonFileNotUsed.txt"};
    if(std::filesystem::exists(myfile)) {
        std::filesystem::remove(myfile);
    }
    CHECK(!CLI::ReadPermissions(myfile).empty());

    bool ok = static_cast<bool>(std::ofstream(myfile.c_str()).put('a'));  // create file
    CHECK(ok);

    std::string filename = "Failed";
    app.add_option("--file", filename)->check(CLI::ReadPermissions);
    args = {"--file", myfile};

    run();

    CHECK(myfile == filename);

    std::filesystem::permissions(std::filesystem::path(myfile), std::filesystem::perms::owner_exec);

#if !defined(_WIN32)
    // not sure how to make a file unreadable on windows in this context
    CHECK_THROWS_AS(run(), CLI::ValidationError);
#endif
    std::filesystem::permissions(std::filesystem::path(myfile), std::filesystem::perms::owner_write);
    std::filesystem::remove(myfile);
}

TEST_CASE_METHOD(TApp, "FileExistsForWrite", "[validate]") {
    std::string myfile{"TestNonFileNotUsed.txt"};
    if(std::filesystem::exists(myfile)) {
        std::filesystem::remove(myfile);
    }
    CHECK(!CLI::WritePermissions(myfile).empty());

    bool ok = static_cast<bool>(std::ofstream(myfile.c_str()).put('a'));  // create file
    CHECK(ok);

    std::string filename = "Failed";
    app.add_option("--file", filename)->check(CLI::WritePermissions);
    args = {"--file", myfile};

    run();

    CHECK(myfile == filename);

    std::filesystem::permissions(std::filesystem::path(myfile), std::filesystem::perms::owner_read);
    CHECK_THROWS_AS(run(), CLI::ValidationError);

    std::remove(myfile.c_str());
}

TEST_CASE_METHOD(TApp, "FileExistsForExec", "[validate]") {
    std::string myfile{"TestNonFileNotUsed.txt"};
    if(std::filesystem::exists(myfile)) {
        std::filesystem::remove(myfile);
    }
    CHECK(!CLI::ExecPermissions(myfile).empty());

    bool ok = static_cast<bool>(std::ofstream(myfile.c_str()).put('a'));  // create file
    CHECK(ok);

    std::string filename = "Failed";
    app.add_option("--file", filename)->check(CLI::ExecPermissions);
    args = {"--file", myfile};

    std::filesystem::permissions(std::filesystem::path(myfile),
                                 std::filesystem::perms::owner_exec | std::filesystem::perms::owner_read);
    run();

    CHECK(myfile == filename);
#if !defined(_WIN32)
    std::filesystem::permissions(std::filesystem::path(myfile), std::filesystem::perms::owner_read);
    CHECK_THROWS_AS(run(), CLI::ValidationError);
    // exec permission not really a thing on windows
#endif

    std::remove(myfile.c_str());
}

TEST_CASE_METHOD(TApp, "noPermissionCheck", "[validate]") {
    std::string myfile{"TestNonFileNotUsed.txt"};
    if(std::filesystem::exists(myfile)) {
        std::filesystem::remove(myfile);
    }
    CHECK(!CLI::detail::PermissionValidator(CLI::detail::Permission::none)(myfile).empty());

    bool ok = static_cast<bool>(std::ofstream(myfile.c_str()).put('a'));  // create file
    CHECK(ok);

    std::string filename = "Failed";
    app.add_option("--file", filename)->check(CLI::detail::PermissionValidator(CLI::detail::Permission::none));
    args = {"--file", myfile};

    run();

    CHECK(myfile == filename);
    std::remove(myfile.c_str());
}
#endif
#endif
#endif
