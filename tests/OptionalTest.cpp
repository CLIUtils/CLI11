#include <cstdint>
#include <cstdlib>
#include <iostream>

#include "app_helper.hpp"

// You can explicitly enable or disable support
// by defining to 1 or 0. Extra check here to ensure it's in the stdlib too.
// We nest the check for __has_include and it's usage
#ifndef CLI11_STD_OPTIONAL
#ifdef __has_include
#if defined(CLI11_CPP17) && __has_include(<optional>)
#define CLI11_STD_OPTIONAL 1
#else
#define CLI11_STD_OPTIONAL 0
#endif
#else
#define CLI11_STD_OPTIONAL 0
#endif
#endif

#ifndef CLI11_EXPERIMENTAL_OPTIONAL
#define CLI11_EXPERIMENTAL_OPTIONAL 0
#endif

#ifndef CLI11_BOOST_OPTIONAL
#define CLI11_BOOST_OPTIONAL 0
#endif

#if CLI11_BOOST_OPTIONAL
#include <boost/version.hpp>
#if BOOST_VERSION < 106100
#error "This boost::optional version is not supported, use 1.61 or better"
#endif
#endif

#if CLI11_STD_OPTIONAL
#include <optional>
#endif
#if CLI11_EXPERIMENTAL_OPTIONAL
#include <experimental/optional>
#endif
#if CLI11_BOOST_OPTIONAL
#include <boost/optional.hpp>
#include <boost/optional/optional_io.hpp>
#endif
// [CLI11:verbatim]

#if CLI11_STD_OPTIONAL

#ifdef _MSC_VER
// this warning suppresses double to int conversions that are inherent in the test
// on windows.  This may be able to removed in the future as the add_option capability
// improves
#pragma warning(disable : 4244)
#endif

TEST_F(TApp, StdOptionalTest) {
    std::optional<int> opt;
    app.add_option("-c,--count", opt);
    run();
    EXPECT_FALSE(opt);

    args = {"-c", "1"};
    run();
    EXPECT_TRUE(opt);
    EXPECT_EQ(*opt, 1);

    args = {"--count", "3"};
    run();
    EXPECT_TRUE(opt);
    EXPECT_EQ(*opt, 3);
}

#ifdef _MSC_VER
#pragma warning(default : 4244)
#endif

#endif
#if CLI11_EXPERIMENTAL_OPTIONAL

TEST_F(TApp, ExperimentalOptionalTest) {
    std::experimental::optional<int> opt;
    app.add_option("-c,--count", opt);
    run();
    EXPECT_FALSE(opt);

    args = {"-c", "1"};
    run();
    EXPECT_TRUE(opt);
    EXPECT_EQ(*opt, 1);

    args = {"--count", "3"};
    run();
    EXPECT_TRUE(opt);
    EXPECT_EQ(*opt, 3);
}

#endif
#if CLI11_BOOST_OPTIONAL

TEST_F(TApp, BoostOptionalTest) {
    boost::optional<int> opt;
    app.add_option("-c,--count", opt);
    run();
    EXPECT_FALSE(opt);

    args = {"-c", "1"};
    run();
    EXPECT_TRUE(opt);
    EXPECT_EQ(*opt, 1);
    opt = {};
    args = {"--count", "3"};
    run();
    EXPECT_TRUE(opt);
    EXPECT_EQ(*opt, 3);
}

TEST_F(TApp, BoostOptionalTestZarg) {
    boost::optional<int> opt;
    app.add_option("-c,--count", opt)->expected(0, 1);
    run();
    EXPECT_FALSE(opt);

    args = {"-c", "1"};
    run();
    EXPECT_TRUE(opt);
    EXPECT_EQ(*opt, 1);
    opt = {};
    args = {"--count"};
    run();
    EXPECT_FALSE(opt);
}

TEST_F(TApp, BoostOptionalint64Test) {
    boost::optional<std::int64_t> opt;
    app.add_option("-c,--count", opt);
    run();
    EXPECT_FALSE(opt);

    args = {"-c", "1"};
    run();
    EXPECT_TRUE(opt);
    EXPECT_EQ(*opt, 1);
    opt = {};
    args = {"--count", "3"};
    run();
    EXPECT_TRUE(opt);
    EXPECT_EQ(*opt, 3);
}

TEST_F(TApp, BoostOptionalStringTest) {
    boost::optional<std::string> opt;
    app.add_option("-s,--string", opt);
    run();
    EXPECT_FALSE(opt);

    args = {"-s", "strval"};
    run();
    EXPECT_TRUE(opt);
    EXPECT_EQ(*opt, "strval");
    opt = {};
    args = {"--string", "strv"};
    run();
    EXPECT_TRUE(opt);
    EXPECT_EQ(*opt, "strv");
}

TEST_F(TApp, BoostOptionalEnumTest) {
    enum class eval : char { val0 = 0, val1 = 1, val2 = 2, val3 = 3, val4 = 4 };
    boost::optional<eval> opt;
    auto optptr = app.add_option<decltype(opt), eval>("-v,--val", opt);
    optptr->capture_default_str();

    auto dstring = optptr->get_default_str();
    EXPECT_TRUE(dstring.empty());
    run();
    EXPECT_FALSE(opt);

    args = {"-v", "3"};
    run();
    EXPECT_TRUE(opt);
    EXPECT_TRUE(*opt == eval::val3);
    opt = {};
    args = {"--val", "1"};
    run();
    EXPECT_TRUE(opt);
    EXPECT_TRUE(*opt == eval::val1);
}

TEST_F(TApp, BoostOptionalVector) {
    boost::optional<std::vector<int>> opt;
    app.add_option_function<std::vector<int>>(
           "-v,--vec", [&opt](const std::vector<int> &v) { opt = v; }, "some vector")
        ->expected(3);
    run();
    EXPECT_FALSE(opt);

    args = {"-v", "1", "4", "5"};
    run();
    EXPECT_TRUE(opt);
    std::vector<int> expV{1, 4, 5};
    EXPECT_EQ(*opt, expV);
}

TEST_F(TApp, BoostOptionalVectorEmpty) {
    boost::optional<std::vector<int>> opt;
    app.add_option<decltype(opt), std::vector<int>>("-v,--vec", opt)->expected(0, 3)->allow_extra_args();
    run();
    EXPECT_FALSE(opt);
    args = {"-v"};
    opt = std::vector<int>{4, 3};
    run();
    EXPECT_FALSE(opt);
    args = {"-v", "1", "4", "5"};
    run();
    EXPECT_TRUE(opt);
    std::vector<int> expV{1, 4, 5};
    EXPECT_EQ(*opt, expV);
}

#endif

#if !CLI11_OPTIONAL
TEST_F(TApp, DISABLED_OptionalTest) {}
#endif
