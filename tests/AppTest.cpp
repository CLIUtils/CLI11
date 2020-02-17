#include "app_helper.hpp"
#include <complex>
#include <cstdint>
#include <cstdlib>

#include "gmock/gmock.h"

TEST_F(TApp, OneFlagShort) {
    app.add_flag("-c,--count");
    args = {"-c"};
    run();
    EXPECT_EQ(1u, app.count("-c"));
    EXPECT_EQ(1u, app.count("--count"));
}

TEST_F(TApp, OneFlagShortValues) {
    app.add_flag("-c{v1},--count{v2}");
    args = {"-c"};
    run();
    EXPECT_EQ(1u, app.count("-c"));
    EXPECT_EQ(1u, app.count("--count"));
    auto v = app["-c"]->results();
    EXPECT_EQ(v[0], "v1");

    EXPECT_THROW(app["--invalid"], CLI::OptionNotFound);
}

TEST_F(TApp, OneFlagShortValuesAs) {
    auto flg = app.add_flag("-c{1},--count{2}");
    args = {"-c"};
    run();
    auto opt = app["-c"];
    EXPECT_EQ(opt->as<int>(), 1);
    args = {"--count"};
    run();
    EXPECT_EQ(opt->as<int>(), 2);
    flg->take_first();
    args = {"-c", "--count"};
    run();
    EXPECT_EQ(opt->as<int>(), 1);
    flg->take_last();
    EXPECT_EQ(opt->as<int>(), 2);
    flg->multi_option_policy(CLI::MultiOptionPolicy::Throw);
    EXPECT_THROW(opt->as<int>(), CLI::ArgumentMismatch);
    flg->multi_option_policy(CLI::MultiOptionPolicy::TakeAll);
    auto vec = opt->as<std::vector<int>>();
    EXPECT_EQ(vec[0], 1);
    EXPECT_EQ(vec[1], 2);
    flg->multi_option_policy(CLI::MultiOptionPolicy::Join);
    EXPECT_EQ(opt->as<std::string>(), "1\n2");
    flg->delimiter(',');
    EXPECT_EQ(opt->as<std::string>(), "1,2");
}

TEST_F(TApp, OneFlagShortWindows) {
    app.add_flag("-c,--count");
    args = {"/c"};
    app.allow_windows_style_options();
    run();
    EXPECT_EQ(1u, app.count("-c"));
    EXPECT_EQ(1u, app.count("--count"));
}

TEST_F(TApp, WindowsLongShortMix1) {
    app.allow_windows_style_options();

    auto a = app.add_flag("-c");
    auto b = app.add_flag("--c");
    args = {"/c"};
    run();
    EXPECT_EQ(1u, a->count());
    EXPECT_EQ(0u, b->count());
}

TEST_F(TApp, WindowsLongShortMix2) {
    app.allow_windows_style_options();

    auto a = app.add_flag("--c");
    auto b = app.add_flag("-c");
    args = {"/c"};
    run();
    EXPECT_EQ(1u, a->count());
    EXPECT_EQ(0u, b->count());
}

TEST_F(TApp, CountNonExist) {
    app.add_flag("-c,--count");
    args = {"-c"};
    run();
    EXPECT_THROW(app.count("--nonexist"), CLI::OptionNotFound);
}

TEST_F(TApp, OneFlagLong) {
    app.add_flag("-c,--count");
    args = {"--count"};
    run();
    EXPECT_EQ(1u, app.count("-c"));
    EXPECT_EQ(1u, app.count("--count"));
}

TEST_F(TApp, DashedOptions) {
    app.add_flag("-c");
    app.add_flag("--q");
    app.add_flag("--this,--that");

    args = {"-c", "--q", "--this", "--that"};
    run();
    EXPECT_EQ(1u, app.count("-c"));
    EXPECT_EQ(1u, app.count("--q"));
    EXPECT_EQ(2u, app.count("--this"));
    EXPECT_EQ(2u, app.count("--that"));
}

TEST_F(TApp, DashedOptionsSingleString) {
    app.add_flag("-c");
    app.add_flag("--q");
    app.add_flag("--this,--that");

    app.parse("-c --q --this --that");
    EXPECT_EQ(1u, app.count("-c"));
    EXPECT_EQ(1u, app.count("--q"));
    EXPECT_EQ(2u, app.count("--this"));
    EXPECT_EQ(2u, app.count("--that"));
}

TEST_F(TApp, RequireOptionsError) {
    using ::testing::HasSubstr;
    using ::testing::Not;
    app.add_flag("-c");
    app.add_flag("--q");
    app.add_flag("--this,--that");
    app.require_option(1, 2);
    try {
        app.parse("-c --q --this --that");
    } catch(const CLI::RequiredError &re) {
        EXPECT_THAT(re.what(), Not(HasSubstr("-h,--help")));
    }

    EXPECT_NO_THROW(app.parse("-c --q"));
    EXPECT_NO_THROW(app.parse("-c --this --that"));
}

TEST_F(TApp, BoolFlagOverride) {
    bool val{false};
    auto flg = app.add_flag("--this,--that", val);

    app.parse("--this");
    EXPECT_TRUE(val);
    app.parse("--this=false");
    EXPECT_FALSE(val);
    flg->disable_flag_override(true);
    app.parse("--this");
    EXPECT_TRUE(val);
    // this is allowed since the matching string is the default
    app.parse("--this=true");
    EXPECT_TRUE(val);

    EXPECT_THROW(app.parse("--this=false"), CLI::ArgumentMismatch);
    // try a string that specifies 'use default val'
    EXPECT_NO_THROW(app.parse("--this={}"));
}

TEST_F(TApp, OneFlagRef) {
    int ref{0};
    app.add_flag("-c,--count", ref);
    args = {"--count"};
    run();
    EXPECT_EQ(1u, app.count("-c"));
    EXPECT_EQ(1u, app.count("--count"));
    EXPECT_EQ(1, ref);
}

TEST_F(TApp, OneFlagRefValue) {
    int ref{0};
    app.add_flag("-c,--count", ref);
    args = {"--count=7"};
    run();
    EXPECT_EQ(1u, app.count("-c"));
    EXPECT_EQ(1u, app.count("--count"));
    EXPECT_EQ(7, ref);
}

TEST_F(TApp, OneFlagRefValueFalse) {
    int ref{0};
    auto flg = app.add_flag("-c,--count", ref);
    args = {"--count=false"};
    run();
    EXPECT_EQ(1u, app.count("-c"));
    EXPECT_EQ(1u, app.count("--count"));
    EXPECT_EQ(-1, ref);

    EXPECT_FALSE(flg->check_fname("c"));
    args = {"--count=0"};
    run();
    EXPECT_EQ(1u, app.count("-c"));
    EXPECT_EQ(1u, app.count("--count"));
    EXPECT_EQ(-1, ref);

    args = {"--count=happy"};
    EXPECT_THROW(run(), CLI::ConversionError);
}

TEST_F(TApp, FlagNegation) {
    int ref{0};
    auto flg = app.add_flag("-c,--count,--ncount{false}", ref);
    args = {"--count", "-c", "--ncount"};
    EXPECT_FALSE(flg->check_fname("count"));
    EXPECT_TRUE(flg->check_fname("ncount"));
    run();
    EXPECT_EQ(3u, app.count("-c"));
    EXPECT_EQ(3u, app.count("--count"));
    EXPECT_EQ(3u, app.count("--ncount"));
    EXPECT_EQ(1, ref);
}

TEST_F(TApp, FlagNegationShortcutNotation) {
    int ref{0};
    app.add_flag("-c,--count{true},!--ncount", ref);
    args = {"--count=TRUE", "-c", "--ncount"};
    run();
    EXPECT_EQ(3u, app.count("-c"));
    EXPECT_EQ(3u, app.count("--count"));
    EXPECT_EQ(3u, app.count("--ncount"));
    EXPECT_EQ(1, ref);
}

TEST_F(TApp, FlagNegationShortcutNotationInvalid) {
    int ref{0};
    app.add_flag("-c,--count,!--ncount", ref);
    args = {"--ncount=happy"};
    EXPECT_THROW(run(), CLI::ConversionError);
}

TEST_F(TApp, OneString) {
    std::string str;
    app.add_option("-s,--string", str);
    args = {"--string", "mystring"};
    run();
    EXPECT_EQ(1u, app.count("-s"));
    EXPECT_EQ(1u, app.count("--string"));
    EXPECT_EQ(str, "mystring");
}

TEST_F(TApp, OneStringWindowsStyle) {
    std::string str;
    app.add_option("-s,--string", str);
    args = {"/string", "mystring"};
    app.allow_windows_style_options();
    run();
    EXPECT_EQ(1u, app.count("-s"));
    EXPECT_EQ(1u, app.count("--string"));
    EXPECT_EQ(str, "mystring");
}

TEST_F(TApp, OneStringSingleStringInput) {
    std::string str;
    app.add_option("-s,--string", str);

    app.parse("--string mystring");
    EXPECT_EQ(1u, app.count("-s"));
    EXPECT_EQ(1u, app.count("--string"));
    EXPECT_EQ(str, "mystring");
}

TEST_F(TApp, OneStringEqualVersion) {
    std::string str;
    app.add_option("-s,--string", str);
    args = {"--string=mystring"};
    run();
    EXPECT_EQ(1u, app.count("-s"));
    EXPECT_EQ(1u, app.count("--string"));
    EXPECT_EQ(str, "mystring");
}

TEST_F(TApp, OneStringEqualVersionWindowsStyle) {
    std::string str;
    app.add_option("-s,--string", str);
    args = {"/string:mystring"};
    app.allow_windows_style_options();
    run();
    EXPECT_EQ(1u, app.count("-s"));
    EXPECT_EQ(1u, app.count("--string"));
    EXPECT_EQ(str, "mystring");
}

TEST_F(TApp, OneStringEqualVersionSingleString) {
    std::string str;
    app.add_option("-s,--string", str);
    app.parse("--string=mystring");
    EXPECT_EQ(1u, app.count("-s"));
    EXPECT_EQ(1u, app.count("--string"));
    EXPECT_EQ(str, "mystring");
}

TEST_F(TApp, OneStringEqualVersionSingleStringQuoted) {
    std::string str;
    app.add_option("-s,--string", str);
    app.parse(R"raw(--string="this is my quoted string")raw");
    EXPECT_EQ(1u, app.count("-s"));
    EXPECT_EQ(1u, app.count("--string"));
    EXPECT_EQ(str, "this is my quoted string");
}

TEST_F(TApp, OneStringEqualVersionSingleStringQuotedMultiple) {
    std::string str, str2, str3;
    app.add_option("-s,--string", str);
    app.add_option("-t,--tstr", str2);
    app.add_option("-m,--mstr", str3);
    app.parse(R"raw(--string="this is my quoted string" -t 'qstring 2' -m=`"quoted string"`)raw");
    EXPECT_EQ(str, "this is my quoted string");
    EXPECT_EQ(str2, "qstring 2");
    EXPECT_EQ(str3, "\"quoted string\"");
}

TEST_F(TApp, OneStringEqualVersionSingleStringEmbeddedEqual) {
    std::string str, str2, str3;
    app.add_option("-s,--string", str);
    app.add_option("-t,--tstr", str2);
    app.add_option("-m,--mstr", str3);
    app.parse(R"raw(--string="app=\"test1 b\" test2=\"frogs\"" -t 'qstring 2' -m=`"quoted string"`)raw");
    EXPECT_EQ(str, "app=\"test1 b\" test2=\"frogs\"");
    EXPECT_EQ(str2, "qstring 2");
    EXPECT_EQ(str3, "\"quoted string\"");

    app.parse(R"raw(--string="app='test1 b' test2='frogs'" -t 'qstring 2' -m=`"quoted string"`)raw");
    EXPECT_EQ(str, "app='test1 b' test2='frogs'");
    EXPECT_EQ(str2, "qstring 2");
    EXPECT_EQ(str3, "\"quoted string\"");
}

TEST_F(TApp, OneStringEqualVersionSingleStringEmbeddedEqualWindowsStyle) {
    std::string str, str2, str3;
    app.add_option("-s,--string", str);
    app.add_option("-t,--tstr", str2);
    app.add_option("--mstr", str3);
    app.allow_windows_style_options();
    app.parse(R"raw(/string:"app:\"test1 b\" test2:\"frogs\"" /t 'qstring 2' /mstr:`"quoted string"`)raw");
    EXPECT_EQ(str, "app:\"test1 b\" test2:\"frogs\"");
    EXPECT_EQ(str2, "qstring 2");
    EXPECT_EQ(str3, "\"quoted string\"");

    app.parse(R"raw(/string:"app:'test1 b' test2:'frogs'" /t 'qstring 2' /mstr:`"quoted string"`)raw");
    EXPECT_EQ(str, "app:'test1 b' test2:'frogs'");
    EXPECT_EQ(str2, "qstring 2");
    EXPECT_EQ(str3, "\"quoted string\"");
}

TEST_F(TApp, OneStringEqualVersionSingleStringQuotedMultipleMixedStyle) {
    std::string str, str2, str3;
    app.add_option("-s,--string", str);
    app.add_option("-t,--tstr", str2);
    app.add_option("-m,--mstr", str3);
    app.allow_windows_style_options();
    app.parse(R"raw(/string:"this is my quoted string" /t 'qstring 2' -m=`"quoted string"`)raw");
    EXPECT_EQ(str, "this is my quoted string");
    EXPECT_EQ(str2, "qstring 2");
    EXPECT_EQ(str3, "\"quoted string\"");
}

TEST_F(TApp, OneStringEqualVersionSingleStringQuotedMultipleInMiddle) {
    std::string str, str2, str3;
    app.add_option("-s,--string", str);
    app.add_option("-t,--tstr", str2);
    app.add_option("-m,--mstr", str3);
    app.parse(R"raw(--string="this is my quoted string" -t "qst\"ring 2" -m=`"quoted string"`)raw");
    EXPECT_EQ(str, "this is my quoted string");
    EXPECT_EQ(str2, "qst\"ring 2");
    EXPECT_EQ(str3, "\"quoted string\"");
}

TEST_F(TApp, OneStringEqualVersionSingleStringQuotedEscapedCharacters) {
    std::string str, str2, str3;
    app.add_option("-s,--string", str);
    app.add_option("-t,--tstr", str2);
    app.add_option("-m,--mstr", str3);
    app.parse(R"raw(--string="this is my \"quoted\" string" -t 'qst\'ring 2' -m=`"quoted\` string"`")raw");
    EXPECT_EQ(str, "this is my \"quoted\" string");
    EXPECT_EQ(str2, "qst\'ring 2");
    EXPECT_EQ(str3, "\"quoted` string\"");
}

TEST_F(TApp, OneStringEqualVersionSingleStringQuotedMultipleWithEqual) {
    std::string str, str2, str3, str4;
    app.add_option("-s,--string", str);
    app.add_option("-t,--tstr", str2);
    app.add_option("-m,--mstr", str3);
    app.add_option("-j,--jstr", str4);
    app.parse(R"raw(--string="this is my quoted string" -t 'qstring 2' -m=`"quoted string"` --jstr=Unquoted)raw");
    EXPECT_EQ(str, "this is my quoted string");
    EXPECT_EQ(str2, "qstring 2");
    EXPECT_EQ(str3, "\"quoted string\"");
    EXPECT_EQ(str4, "Unquoted");
}

TEST_F(TApp, OneStringEqualVersionSingleStringQuotedMultipleWithEqualAndProgram) {
    std::string str, str2, str3, str4;
    app.add_option("-s,--string", str);
    app.add_option("-t,--tstr", str2);
    app.add_option("-m,--mstr", str3);
    app.add_option("-j,--jstr", str4);
    app.parse(
        R"raw(program --string="this is my quoted string" -t 'qstring 2' -m=`"quoted string"` --jstr=Unquoted)raw",
        true);
    EXPECT_EQ(str, "this is my quoted string");
    EXPECT_EQ(str2, "qstring 2");
    EXPECT_EQ(str3, "\"quoted string\"");
    EXPECT_EQ(str4, "Unquoted");
}

TEST_F(TApp, OneStringFlagLike) {
    std::string str{"something"};
    app.add_option("-s,--string", str)->expected(0, 1);
    args = {"--string"};
    run();
    EXPECT_EQ(1u, app.count("-s"));
    EXPECT_EQ(1u, app.count("--string"));
    EXPECT_TRUE(str.empty());
}

TEST_F(TApp, OneIntFlagLike) {
    int val{0};
    auto opt = app.add_option("-i", val)->expected(0, 1);
    args = {"-i"};
    run();
    EXPECT_EQ(1u, app.count("-i"));
    opt->default_str("7");
    run();
    EXPECT_EQ(val, 7);

    opt->default_val(9);
    run();
    EXPECT_EQ(val, 9);
}

TEST_F(TApp, TogetherInt) {
    int i{0};
    app.add_option("-i,--int", i);
    args = {"-i4"};
    run();
    EXPECT_EQ(1u, app.count("--int"));
    EXPECT_EQ(1u, app.count("-i"));
    EXPECT_EQ(i, 4);
    EXPECT_EQ(app["-i"]->as<std::string>(), "4");
    EXPECT_EQ(app["--int"]->as<double>(), 4.0);
}

TEST_F(TApp, SepInt) {
    int i{0};
    app.add_option("-i,--int", i);
    args = {"-i", "4"};
    run();
    EXPECT_EQ(1u, app.count("--int"));
    EXPECT_EQ(1u, app.count("-i"));
    EXPECT_EQ(i, 4);
}

TEST_F(TApp, OneStringAgain) {
    std::string str;
    app.add_option("-s,--string", str);
    args = {"--string", "mystring"};
    run();
    EXPECT_EQ(1u, app.count("-s"));
    EXPECT_EQ(1u, app.count("--string"));
    EXPECT_EQ(str, "mystring");
}

TEST_F(TApp, OneStringFunction) {
    std::string str;
    app.add_option_function<std::string>("-s,--string", [&str](const std::string &val) { str = val; });
    args = {"--string", "mystring"};
    run();
    EXPECT_EQ(1u, app.count("-s"));
    EXPECT_EQ(1u, app.count("--string"));
    EXPECT_EQ(str, "mystring");
}

TEST_F(TApp, doubleFunction) {
    double res{0.0};
    app.add_option_function<double>("--val", [&res](double val) { res = std::abs(val + 54); });
    args = {"--val", "-354.356"};
    run();
    EXPECT_EQ(res, 300.356);
    // get the original value as entered as an integer
    EXPECT_EQ(app["--val"]->as<float>(), -354.356f);
}

TEST_F(TApp, doubleFunctionFail) {
    double res;
    app.add_option_function<double>("--val", [&res](double val) { res = std::abs(val + 54); });
    args = {"--val", "not_double"};
    EXPECT_THROW(run(), CLI::ConversionError);
}

TEST_F(TApp, doubleVectorFunction) {
    std::vector<double> res;
    app.add_option_function<std::vector<double>>("--val", [&res](const std::vector<double> &val) {
        res = val;
        std::transform(res.begin(), res.end(), res.begin(), [](double v) { return v + 5.0; });
    });
    args = {"--val", "5", "--val", "6", "--val", "7"};
    run();
    EXPECT_EQ(res.size(), 3u);
    EXPECT_EQ(res[0], 10.0);
    EXPECT_EQ(res[2], 12.0);
}

TEST_F(TApp, doubleVectorFunctionFail) {
    std::vector<double> res;
    std::string vstring = "--val";
    app.add_option_function<std::vector<double>>(vstring, [&res](const std::vector<double> &val) {
        res = val;
        std::transform(res.begin(), res.end(), res.begin(), [](double v) { return v + 5.0; });
    });
    args = {"--val", "five", "--val", "nine", "--val", "7"};
    EXPECT_THROW(run(), CLI::ConversionError);
    // check that getting the results through the results function generates the same error
    EXPECT_THROW(app[vstring]->results(res), CLI::ConversionError);
    auto strvec = app[vstring]->as<std::vector<std::string>>();
    EXPECT_EQ(strvec.size(), 3u);
}

TEST_F(TApp, doubleVectorFunctionRunCallbackOnDefault) {
    std::vector<double> res;
    auto opt = app.add_option_function<std::vector<double>>("--val", [&res](const std::vector<double> &val) {
        res = val;
        std::transform(res.begin(), res.end(), res.begin(), [](double v) { return v + 5.0; });
    });
    args = {"--val", "5", "--val", "6", "--val", "7"};
    run();
    EXPECT_EQ(res.size(), 3u);
    EXPECT_EQ(res[0], 10.0);
    EXPECT_EQ(res[2], 12.0);
    EXPECT_FALSE(opt->get_run_callback_for_default());
    opt->run_callback_for_default();
    opt->default_val(std::vector<int>{2, 1, -2});
    EXPECT_EQ(res[0], 7.0);
    EXPECT_EQ(res[2], 3.0);

    EXPECT_THROW(opt->default_val("this is a string"), CLI::ConversionError);
    auto vec = opt->as<std::vector<double>>();
    ASSERT_EQ(vec.size(), 3U);
    EXPECT_EQ(vec[0], 5.0);
    EXPECT_EQ(vec[2], 7.0);
    opt->check(CLI::Number);
    opt->run_callback_for_default(false);
    EXPECT_THROW(opt->default_val("this is a string"), CLI::ValidationError);
}

TEST_F(TApp, DefaultStringAgain) {
    std::string str = "previous";
    app.add_option("-s,--string", str);
    run();
    EXPECT_EQ(0u, app.count("-s"));
    EXPECT_EQ(0u, app.count("--string"));
    EXPECT_EQ(str, "previous");
}

TEST_F(TApp, DefaultStringAgainEmpty) {
    std::string str = "previous";
    app.add_option("-s,--string", str);
    app.parse("   ");
    EXPECT_EQ(0u, app.count("-s"));
    EXPECT_EQ(0u, app.count("--string"));
    EXPECT_EQ(str, "previous");
}

TEST_F(TApp, DualOptions) {

    std::string str = "previous";
    std::vector<std::string> vstr = {"previous"};
    std::vector<std::string> ans = {"one", "two"};
    app.add_option("-s,--string", str);
    app.add_option("-v,--vector", vstr);

    args = {"--vector=one", "--vector=two"};
    run();
    EXPECT_EQ(ans, vstr);

    args = {"--string=one", "--string=two"};
    EXPECT_THROW(run(), CLI::ArgumentMismatch);
}

TEST_F(TApp, LotsOfFlags) {

    app.add_flag("-a");
    app.add_flag("-A");
    app.add_flag("-b");

    args = {"-a", "-b", "-aA"};
    run();
    EXPECT_EQ(2u, app.count("-a"));
    EXPECT_EQ(1u, app.count("-b"));
    EXPECT_EQ(1u, app.count("-A"));
    EXPECT_EQ(app.count_all(), 4u);
}

TEST_F(TApp, NumberFlags) {

    int val{0};
    app.add_flag("-1{1},-2{2},-3{3},-4{4},-5{5},-6{6}, -7{7}, -8{8}, -9{9}", val);

    args = {"-7"};
    run();
    EXPECT_EQ(1u, app.count("-1"));
    EXPECT_EQ(val, 7);
}

TEST_F(TApp, DisableFlagOverrideTest) {

    int val{0};
    auto opt = app.add_flag("--1{1},--2{2},--3{3},--4{4},--5{5},--6{6}, --7{7}, --8{8}, --9{9}", val);
    EXPECT_FALSE(opt->get_disable_flag_override());
    opt->disable_flag_override();
    args = {"--7=5"};
    EXPECT_THROW(run(), CLI::ArgumentMismatch);
    EXPECT_TRUE(opt->get_disable_flag_override());
    opt->disable_flag_override(false);
    EXPECT_FALSE(opt->get_disable_flag_override());
    EXPECT_NO_THROW(run());
    EXPECT_EQ(val, 5);
    opt->disable_flag_override();
    args = {"--7=7"};
    EXPECT_NO_THROW(run());
}

TEST_F(TApp, LotsOfFlagsSingleString) {

    app.add_flag("-a");
    app.add_flag("-A");
    app.add_flag("-b");

    app.parse("-a -b -aA");
    EXPECT_EQ(2u, app.count("-a"));
    EXPECT_EQ(1u, app.count("-b"));
    EXPECT_EQ(1u, app.count("-A"));
}

TEST_F(TApp, LotsOfFlagsSingleStringExtraSpace) {

    app.add_flag("-a");
    app.add_flag("-A");
    app.add_flag("-b");

    app.parse("  -a    -b    -aA   ");
    EXPECT_EQ(2u, app.count("-a"));
    EXPECT_EQ(1u, app.count("-b"));
    EXPECT_EQ(1u, app.count("-A"));
}

TEST_F(TApp, BoolAndIntFlags) {

    bool bflag{false};
    int iflag{0};
    unsigned int uflag{0};

    app.add_flag("-b", bflag);
    app.add_flag("-i", iflag);
    app.add_flag("-u", uflag);

    args = {"-b", "-i", "-u"};
    run();
    EXPECT_TRUE(bflag);
    EXPECT_EQ(1, iflag);
    EXPECT_EQ((unsigned int)1, uflag);

    args = {"-b", "-b"};
    ASSERT_NO_THROW(run());
    EXPECT_TRUE(bflag);

    bflag = false;

    args = {"-iiiuu"};
    run();
    EXPECT_FALSE(bflag);
    EXPECT_EQ(3, iflag);
    EXPECT_EQ((unsigned int)2, uflag);
}

TEST_F(TApp, FlagLikeOption) {
    bool val{false};
    auto opt = app.add_option("--flag", val)->type_size(0)->default_str("true");
    args = {"--flag"};
    run();
    EXPECT_EQ(1u, app.count("--flag"));
    EXPECT_TRUE(val);
    val = false;
    opt->type_size(0, 0);  // should be the same as above
    EXPECT_EQ(opt->get_type_size_min(), 0);
    EXPECT_EQ(opt->get_type_size_max(), 0);
    run();
    EXPECT_EQ(1u, app.count("--flag"));
    EXPECT_TRUE(val);
}

TEST_F(TApp, FlagLikeIntOption) {
    int val{-47};
    auto opt = app.add_option("--flag", val)->expected(0, 1);
    // normally some default value should be set, but this test is for some paths in the validators checks to skip
    // validation on empty string if nothing is expected
    opt->check(CLI::PositiveNumber);
    args = {"--flag"};
    EXPECT_TRUE(opt->as<std::string>().empty());
    run();
    EXPECT_EQ(1u, app.count("--flag"));
    EXPECT_NE(val, -47);
    args = {"--flag", "12"};
    run();

    EXPECT_EQ(val, 12);
    args.clear();
    run();
    EXPECT_TRUE(opt->as<std::string>().empty());
}

TEST_F(TApp, BoolOnlyFlag) {
    bool bflag{false};
    app.add_flag("-b", bflag)->multi_option_policy(CLI::MultiOptionPolicy::Throw);

    args = {"-b"};
    ASSERT_NO_THROW(run());
    EXPECT_TRUE(bflag);

    args = {"-b", "-b"};
    EXPECT_THROW(run(), CLI::ArgumentMismatch);
}

TEST_F(TApp, BoolOption) {
    bool bflag{false};
    app.add_option("-b", bflag);

    args = {"-b", "false"};
    run();
    EXPECT_FALSE(bflag);

    args = {"-b", "1"};
    run();
    EXPECT_TRUE(bflag);

    args = {"-b", "-7"};
    run();
    EXPECT_FALSE(bflag);

    // cause an out of bounds error internally
    args = {"-b", "751615654161688126132138844896646748852"};
    run();
    EXPECT_TRUE(bflag);

    args = {"-b", "-751615654161688126132138844896646748852"};
    run();
    EXPECT_FALSE(bflag);
}

TEST_F(TApp, ShortOpts) {

    unsigned long long funnyint{0};
    std::string someopt;
    app.add_flag("-z", funnyint);
    app.add_option("-y", someopt);

    args = {
        "-zzyzyz",
    };

    run();

    EXPECT_EQ(2u, app.count("-z"));
    EXPECT_EQ(1u, app.count("-y"));
    EXPECT_EQ((unsigned long long)2, funnyint);
    EXPECT_EQ("zyz", someopt);
    EXPECT_EQ(app.count_all(), 3u);
}

TEST_F(TApp, TwoParamTemplateOpts) {

    double funnyint{0.0};
    auto opt = app.add_option<double, unsigned int>("-y", funnyint);

    args = {"-y", "32"};

    run();

    EXPECT_EQ(32.0, funnyint);

    args = {"-y", "32.3"};
    EXPECT_THROW(run(), CLI::ConversionError);

    args = {"-y", "-19"};
    EXPECT_THROW(run(), CLI::ConversionError);

    opt->capture_default_str();
    EXPECT_TRUE(opt->get_default_str().empty());
}

TEST_F(TApp, DefaultOpts) {

    int i{3};
    std::string s = "HI";

    app.add_option("-i,i", i);
    app.add_option("-s,s", s)->capture_default_str();  //  Used to be different

    args = {"-i2", "9"};

    run();

    EXPECT_EQ(1u, app.count("i"));
    EXPECT_EQ(1u, app.count("-s"));
    EXPECT_EQ(2, i);
    EXPECT_EQ("9", s);
}

TEST_F(TApp, TakeLastOpt) {

    std::string str;
    app.add_option("--str", str)->multi_option_policy(CLI::MultiOptionPolicy::TakeLast);

    args = {"--str=one", "--str=two"};

    run();

    EXPECT_EQ(str, "two");
}

TEST_F(TApp, TakeLastOpt2) {

    std::string str;
    app.add_option("--str", str)->take_last();

    args = {"--str=one", "--str=two"};

    run();

    EXPECT_EQ(str, "two");
}

TEST_F(TApp, TakeFirstOpt) {

    std::string str;
    app.add_option("--str", str)->multi_option_policy(CLI::MultiOptionPolicy::TakeFirst);

    args = {"--str=one", "--str=two"};

    run();

    EXPECT_EQ(str, "one");
}

TEST_F(TApp, TakeFirstOpt2) {

    std::string str;
    app.add_option("--str", str)->take_first();

    args = {"--str=one", "--str=two"};

    run();

    EXPECT_EQ(str, "one");
}

TEST_F(TApp, JoinOpt) {

    std::string str;
    app.add_option("--str", str)->multi_option_policy(CLI::MultiOptionPolicy::Join);

    args = {"--str=one", "--str=two"};

    run();

    EXPECT_EQ(str, "one\ntwo");
}

TEST_F(TApp, JoinOpt2) {

    std::string str;
    app.add_option("--str", str)->join();

    args = {"--str=one", "--str=two"};

    run();

    EXPECT_EQ(str, "one\ntwo");
}

TEST_F(TApp, TakeLastOptMulti) {
    std::vector<int> vals;
    app.add_option("--long", vals)->expected(2)->take_last();

    args = {"--long", "1", "2", "3"};

    run();

    EXPECT_EQ(vals, std::vector<int>({2, 3}));
}

TEST_F(TApp, vectorDefaults) {
    std::vector<int> vals{4, 5};
    auto opt = app.add_option("--long", vals, "", true);

    args = {"--long", "[1,2,3]"};

    run();

    EXPECT_EQ(vals, std::vector<int>({1, 2, 3}));

    args.clear();
    run();
    auto res = app["--long"]->as<std::vector<int>>();
    EXPECT_EQ(res, std::vector<int>({4, 5}));

    app.clear();
    opt->expected(1)->take_last();
    res = app["--long"]->as<std::vector<int>>();
    EXPECT_EQ(res, std::vector<int>({5}));
    opt->take_first();
    res = app["--long"]->as<std::vector<int>>();
    EXPECT_EQ(res, std::vector<int>({4}));

    opt->expected(0, 1)->take_last();
    run();

    EXPECT_EQ(res, std::vector<int>({4}));
    res = app["--long"]->as<std::vector<int>>();
    EXPECT_EQ(res, std::vector<int>({5}));
}

TEST_F(TApp, TakeLastOptMulti_alternative_path) {
    std::vector<int> vals;
    app.add_option("--long", vals)->expected(2, -1)->take_last();

    args = {"--long", "1", "2", "3"};

    run();

    EXPECT_EQ(vals, std::vector<int>({2, 3}));
}

TEST_F(TApp, TakeLastOptMultiCheck) {
    std::vector<int> vals;
    auto opt = app.add_option("--long", vals)->expected(-2)->take_last();

    opt->check(CLI::Validator(CLI::PositiveNumber).application_index(0));
    opt->check((!CLI::PositiveNumber).application_index(1));
    args = {"--long", "-1", "2", "-3"};

    EXPECT_NO_THROW(run());

    EXPECT_EQ(vals, std::vector<int>({2, -3}));
}

TEST_F(TApp, TakeFirstOptMulti) {
    std::vector<int> vals;
    app.add_option("--long", vals)->expected(2)->take_first();

    args = {"--long", "1", "2", "3"};

    run();

    EXPECT_EQ(vals, std::vector<int>({1, 2}));
}

TEST_F(TApp, ComplexOptMulti) {
    std::complex<double> val;
    app.add_complex("--long", val)->take_first()->allow_extra_args();

    args = {"--long", "1", "2", "3", "4"};

    run();

    EXPECT_DOUBLE_EQ(val.real(), 1);
    EXPECT_DOUBLE_EQ(val.imag(), 2);
}

TEST_F(TApp, MissingValueNonRequiredOpt) {
    int count{0};
    app.add_option("-c,--count", count);

    args = {"-c"};
    EXPECT_THROW(run(), CLI::ArgumentMismatch);

    args = {"--count"};
    EXPECT_THROW(run(), CLI::ArgumentMismatch);
}

TEST_F(TApp, MissingValueMoreThan) {
    std::vector<int> vals1;
    std::vector<int> vals2;
    app.add_option("-v", vals1)->expected(-2);
    app.add_option("--vals", vals2)->expected(-2);

    args = {"-v", "2"};
    EXPECT_THROW(run(), CLI::ArgumentMismatch);

    args = {"--vals", "4"};
    EXPECT_THROW(run(), CLI::ArgumentMismatch);
}

TEST_F(TApp, NoMissingValueMoreThan) {
    std::vector<int> vals1;
    std::vector<int> vals2;
    app.add_option("-v", vals1)->expected(-2);
    app.add_option("--vals", vals2)->expected(-2);

    args = {"-v", "2", "3", "4"};
    run();
    EXPECT_EQ(vals1, std::vector<int>({2, 3, 4}));

    args = {"--vals", "2", "3", "4"};
    run();
    EXPECT_EQ(vals2, std::vector<int>({2, 3, 4}));
}

TEST_F(TApp, NotRequiredOptsSingle) {

    std::string str;
    app.add_option("--str", str);

    args = {"--str"};

    EXPECT_THROW(run(), CLI::ArgumentMismatch);
}

TEST_F(TApp, NotRequiredOptsSingleShort) {

    std::string str;
    app.add_option("-s", str);

    args = {"-s"};

    EXPECT_THROW(run(), CLI::ArgumentMismatch);
}

TEST_F(TApp, RequiredOptsSingle) {

    std::string str;
    app.add_option("--str", str)->required();

    args = {"--str"};

    EXPECT_THROW(run(), CLI::ArgumentMismatch);
}

TEST_F(TApp, RequiredOptsSingleShort) {

    std::string str;
    app.add_option("-s", str)->required();

    args = {"-s"};

    EXPECT_THROW(run(), CLI::ArgumentMismatch);
}

TEST_F(TApp, RequiredOptsDouble) {

    std::vector<std::string> strs;
    app.add_option("--str", strs)->required()->expected(2);

    args = {"--str", "one"};

    EXPECT_THROW(run(), CLI::ArgumentMismatch);

    args = {"--str", "one", "two"};

    run();

    EXPECT_EQ(strs, std::vector<std::string>({"one", "two"}));
}

TEST_F(TApp, RequiredOptsDoubleShort) {

    std::vector<std::string> strs;
    app.add_option("-s", strs)->required()->expected(2);

    args = {"-s", "one"};

    EXPECT_THROW(run(), CLI::ArgumentMismatch);

    args = {"-s", "one", "-s", "one", "-s", "one"};

    EXPECT_THROW(run(), CLI::ArgumentMismatch);
}

TEST_F(TApp, RequiredOptsDoubleNeg) {
    std::vector<std::string> strs;
    app.add_option("-s", strs)->required()->expected(-2);

    args = {"-s", "one"};

    EXPECT_THROW(run(), CLI::ArgumentMismatch);

    args = {"-s", "one", "two", "-s", "three"};

    ASSERT_NO_THROW(run());
    EXPECT_EQ(strs, std::vector<std::string>({"one", "two", "three"}));

    args = {"-s", "one", "two"};
    ASSERT_NO_THROW(run());
    EXPECT_EQ(strs, std::vector<std::string>({"one", "two"}));
}

// This makes sure unlimited option priority is
// correct for space vs. no space #90
TEST_F(TApp, PositionalNoSpace) {
    std::vector<std::string> options;
    std::string foo, bar;

    app.add_option("-O", options);
    app.add_option("foo", foo)->required();
    app.add_option("bar", bar)->required();

    args = {"-O", "Test", "param1", "param2"};
    run();

    EXPECT_EQ(options.size(), 1u);
    EXPECT_EQ(options.at(0), "Test");

    args = {"-OTest", "param1", "param2"};
    run();

    EXPECT_EQ(options.size(), 1u);
    EXPECT_EQ(options.at(0), "Test");
}

// Tests positionals at end
TEST_F(TApp, PositionalAtEnd) {
    std::string options;
    std::string foo;

    app.add_option("-O", options);
    app.add_option("foo", foo);
    app.positionals_at_end();
    EXPECT_TRUE(app.get_positionals_at_end());
    args = {"-O", "Test", "param1"};
    run();

    EXPECT_EQ(options, "Test");
    EXPECT_EQ(foo, "param1");

    args = {"param2", "-O", "Test"};
    EXPECT_THROW(run(), CLI::ExtrasError);
}

// Tests positionals at end
TEST_F(TApp, RequiredPositionals) {
    std::vector<std::string> sources;
    std::string dest;
    app.add_option("src", sources);
    app.add_option("dest", dest)->required();
    app.positionals_at_end();

    args = {"1", "2", "3"};
    run();

    EXPECT_EQ(sources.size(), 2u);
    EXPECT_EQ(dest, "3");

    args = {"a"};
    sources.clear();
    run();

    EXPECT_EQ(sources.size(), 0u);
    EXPECT_EQ(dest, "a");
}

TEST_F(TApp, RequiredPositionalVector) {
    std::string d1;
    std::string d2;
    std::string d3;
    std::vector<std::string> sources;

    app.add_option("dest1", d1);
    app.add_option("dest2", d2);
    app.add_option("dest3", d3);
    app.add_option("src", sources)->required();

    app.positionals_at_end();

    args = {"1", "2", "3"};
    run();

    EXPECT_EQ(sources.size(), 1u);
    EXPECT_EQ(d1, "1");
    EXPECT_EQ(d2, "2");
    EXPECT_TRUE(d3.empty());
    args = {"a"};
    sources.clear();
    run();

    EXPECT_EQ(sources.size(), 1u);
}

// Tests positionals at end
TEST_F(TApp, RequiredPositionalValidation) {
    std::vector<std::string> sources;
    int dest;  // required
    std::string d2;
    app.add_option("src", sources);
    app.add_option("dest", dest)->required()->check(CLI::PositiveNumber);
    app.add_option("dest2", d2)->required();
    app.positionals_at_end()->validate_positionals();

    args = {"1", "2", "string", "3"};
    run();

    EXPECT_EQ(sources.size(), 2u);
    EXPECT_EQ(dest, 3);
    EXPECT_EQ(d2, "string");
}

// Tests positionals at end
TEST_F(TApp, PositionalValidation) {
    std::string options;
    std::string foo;

    app.add_option("bar", options)->check(CLI::Number.name("valbar"));
    // disable the check on foo
    app.add_option("foo", foo)->check(CLI::Number.active(false));
    app.validate_positionals();
    args = {"1", "param1"};
    run();

    EXPECT_EQ(options, "1");
    EXPECT_EQ(foo, "param1");

    args = {"param1", "1"};
    EXPECT_NO_THROW(run());

    EXPECT_EQ(options, "1");
    EXPECT_EQ(foo, "param1");

    EXPECT_NE(app.get_option("bar")->get_validator("valbar"), nullptr);
}

TEST_F(TApp, PositionalNoSpaceLong) {
    std::vector<std::string> options;
    std::string foo, bar;

    app.add_option("--option", options);
    app.add_option("foo", foo)->required();
    app.add_option("bar", bar)->required();

    args = {"--option", "Test", "param1", "param2"};
    run();

    EXPECT_EQ(options.size(), 1u);
    EXPECT_EQ(options.at(0), "Test");

    args = {"--option=Test", "param1", "param2"};
    run();

    EXPECT_EQ(options.size(), 1u);
    EXPECT_EQ(options.at(0), "Test");
}

TEST_F(TApp, RequiredOptsUnlimited) {

    std::vector<std::string> strs;
    app.add_option("--str", strs)->required();

    args = {"--str"};
    EXPECT_THROW(run(), CLI::ArgumentMismatch);

    args = {"--str", "one", "--str", "two"};
    run();
    EXPECT_EQ(strs, std::vector<std::string>({"one", "two"}));

    args = {"--str", "one", "two"};
    run();
    EXPECT_EQ(strs, std::vector<std::string>({"one", "two"}));

    // It's better to feed a hungry option than to feed allow_extras
    app.allow_extras();
    run();
    EXPECT_EQ(strs, std::vector<std::string>({"one", "two"}));
    EXPECT_EQ(app.remaining(), std::vector<std::string>({}));

    app.allow_extras(false);
    std::vector<std::string> remain;
    auto popt = app.add_option("positional", remain);
    run();
    EXPECT_EQ(strs, std::vector<std::string>({"one", "two"}));
    EXPECT_EQ(remain, std::vector<std::string>());

    args = {"--str", "one", "--", "two"};

    run();
    EXPECT_EQ(strs, std::vector<std::string>({"one"}));
    EXPECT_EQ(remain, std::vector<std::string>({"two"}));

    args = {"one", "--str", "two"};

    run();
    EXPECT_EQ(strs, std::vector<std::string>({"two"}));
    EXPECT_EQ(remain, std::vector<std::string>({"one"}));

    args = {"--str", "one", "two"};
    popt->required();
    run();
    EXPECT_EQ(strs, std::vector<std::string>({"one"}));
    EXPECT_EQ(remain, std::vector<std::string>({"two"}));
}

TEST_F(TApp, RequiredOptsUnlimitedShort) {

    std::vector<std::string> strs;
    app.add_option("-s", strs)->required();

    args = {"-s"};
    EXPECT_THROW(run(), CLI::ArgumentMismatch);

    args = {"-s", "one", "-s", "two"};
    run();
    EXPECT_EQ(strs, std::vector<std::string>({"one", "two"}));

    args = {"-s", "one", "two"};
    run();
    EXPECT_EQ(strs, std::vector<std::string>({"one", "two"}));

    // It's better to feed a hungry option than to feed allow_extras
    app.allow_extras();
    run();
    EXPECT_EQ(strs, std::vector<std::string>({"one", "two"}));
    EXPECT_EQ(app.remaining(), std::vector<std::string>({}));

    app.allow_extras(false);
    std::vector<std::string> remain;
    app.add_option("positional", remain);
    run();
    EXPECT_EQ(strs, std::vector<std::string>({"one", "two"}));
    EXPECT_EQ(remain, std::vector<std::string>());

    args = {"-s", "one", "--", "two"};

    run();
    EXPECT_EQ(strs, std::vector<std::string>({"one"}));
    EXPECT_EQ(remain, std::vector<std::string>({"two"}));

    args = {"one", "-s", "two"};

    run();
    EXPECT_EQ(strs, std::vector<std::string>({"two"}));
    EXPECT_EQ(remain, std::vector<std::string>({"one"}));
}

TEST_F(TApp, OptsUnlimitedEnd) {
    std::vector<std::string> strs;
    app.add_option("-s,--str", strs);
    app.allow_extras();

    args = {"one", "-s", "two", "three", "--", "four"};

    run();

    EXPECT_EQ(strs, std::vector<std::string>({"two", "three"}));
    EXPECT_EQ(app.remaining(), std::vector<std::string>({"one", "four"}));
}

TEST_F(TApp, RequireOptPriority) {

    std::vector<std::string> strs;
    app.add_option("--str", strs);
    std::vector<std::string> remain;
    app.add_option("positional", remain)->expected(2)->required();

    args = {"--str", "one", "two", "three"};
    run();

    EXPECT_EQ(strs, std::vector<std::string>({"one"}));
    EXPECT_EQ(remain, std::vector<std::string>({"two", "three"}));

    args = {"two", "three", "--str", "one", "four"};
    run();

    EXPECT_EQ(strs, std::vector<std::string>({"one", "four"}));
    EXPECT_EQ(remain, std::vector<std::string>({"two", "three"}));
}

TEST_F(TApp, RequireOptPriorityShort) {

    std::vector<std::string> strs;
    app.add_option("-s", strs)->required();
    std::vector<std::string> remain;
    app.add_option("positional", remain)->expected(2)->required();

    args = {"-s", "one", "two", "three"};
    run();

    EXPECT_EQ(strs, std::vector<std::string>({"one"}));
    EXPECT_EQ(remain, std::vector<std::string>({"two", "three"}));

    args = {"two", "three", "-s", "one", "four"};
    run();

    EXPECT_EQ(strs, std::vector<std::string>({"one", "four"}));
    EXPECT_EQ(remain, std::vector<std::string>({"two", "three"}));
}

TEST_F(TApp, NotRequiredExpectedDouble) {

    std::vector<std::string> strs;
    app.add_option("--str", strs)->expected(2);

    args = {"--str", "one"};

    EXPECT_THROW(run(), CLI::ArgumentMismatch);
}

TEST_F(TApp, NotRequiredExpectedDoubleShort) {

    std::vector<std::string> strs;
    app.add_option("-s", strs)->expected(2);

    args = {"-s", "one"};

    EXPECT_THROW(run(), CLI::ArgumentMismatch);
}

TEST_F(TApp, RequiredFlags) {
    app.add_flag("-a")->required();
    app.add_flag("-b")->mandatory();  // Alternate term

    EXPECT_THROW(run(), CLI::RequiredError);

    args = {"-a"};
    EXPECT_THROW(run(), CLI::RequiredError);

    args = {"-b"};
    EXPECT_THROW(run(), CLI::RequiredError);

    args = {"-a", "-b"};
    run();
}

TEST_F(TApp, CallbackFlags) {

    std::int64_t value{0};

    auto func = [&value](std::int64_t x) { value = x; };

    app.add_flag_function("-v", func);

    run();
    EXPECT_EQ(value, 0u);

    args = {"-v"};
    run();
    EXPECT_EQ(value, 1u);

    args = {"-vv"};
    run();
    EXPECT_EQ(value, 2u);

    EXPECT_THROW(app.add_flag_function("hi", func), CLI::IncorrectConstruction);
}

TEST_F(TApp, CallbackBoolFlags) {

    bool value{false};

    auto func = [&value]() { value = true; };

    auto cback = app.add_flag_callback("--val", func);
    args = {"--val"};
    run();
    EXPECT_TRUE(value);
    value = false;
    args = {"--val=false"};
    run();
    EXPECT_FALSE(value);

    EXPECT_THROW(app.add_flag_callback("hi", func), CLI::IncorrectConstruction);
    cback->multi_option_policy(CLI::MultiOptionPolicy::Throw);
    args = {"--val", "--val=false"};
    EXPECT_THROW(run(), CLI::ArgumentMismatch);
}

TEST_F(TApp, CallbackFlagsFalse) {
    std::int64_t value = 0;

    auto func = [&value](std::int64_t x) { value = x; };

    app.add_flag_function("-v,-f{false},--val,--fval{false}", func);

    run();
    EXPECT_EQ(value, 0);

    args = {"-f"};
    run();
    EXPECT_EQ(value, -1);

    args = {"-vfv"};
    run();
    EXPECT_EQ(value, 1);

    args = {"--fval"};
    run();
    EXPECT_EQ(value, -1);

    args = {"--fval=2"};
    run();
    EXPECT_EQ(value, -2);

    EXPECT_THROW(app.add_flag_function("hi", func), CLI::IncorrectConstruction);
}

TEST_F(TApp, CallbackFlagsFalseShortcut) {
    std::int64_t value = 0;

    auto func = [&value](std::int64_t x) { value = x; };

    app.add_flag_function("-v,!-f,--val,!--fval", func);

    run();
    EXPECT_EQ(value, 0);

    args = {"-f"};
    run();
    EXPECT_EQ(value, -1);

    args = {"-vfv"};
    run();
    EXPECT_EQ(value, 1);

    args = {"--fval"};
    run();
    EXPECT_EQ(value, -1);

    args = {"--fval=2"};
    run();
    EXPECT_EQ(value, -2);

    EXPECT_THROW(app.add_flag_function("hi", func), CLI::IncorrectConstruction);
}

#if __cplusplus >= 201402L || _MSC_VER >= 1900
TEST_F(TApp, CallbackFlagsAuto) {

    std::int64_t value{0};

    auto func = [&value](std::int64_t x) { value = x; };

    app.add_flag("-v", func);

    run();
    EXPECT_EQ(value, 0u);

    args = {"-v"};
    run();
    EXPECT_EQ(value, 1u);

    args = {"-vv"};
    run();
    EXPECT_EQ(value, 2u);

    EXPECT_THROW(app.add_flag("hi", func), CLI::IncorrectConstruction);
}
#endif

TEST_F(TApp, Positionals) {

    std::string posit1;
    std::string posit2;
    app.add_option("posit1", posit1);
    app.add_option("posit2", posit2);

    args = {"thing1", "thing2"};

    run();

    EXPECT_EQ(1u, app.count("posit1"));
    EXPECT_EQ(1u, app.count("posit2"));
    EXPECT_EQ("thing1", posit1);
    EXPECT_EQ("thing2", posit2);
}

TEST_F(TApp, ForcedPositional) {
    std::vector<std::string> posit;
    auto one = app.add_flag("--one");
    app.add_option("posit", posit);

    args = {"--one", "two", "three"};
    run();
    std::vector<std::string> answers1 = {"two", "three"};
    EXPECT_TRUE(one->count());
    EXPECT_EQ(answers1, posit);

    args = {"--", "--one", "two", "three"};
    std::vector<std::string> answers2 = {"--one", "two", "three"};
    run();

    EXPECT_FALSE(one->count());
    EXPECT_EQ(answers2, posit);
}

TEST_F(TApp, MixedPositionals) {

    int positional_int{0};
    std::string positional_string;
    app.add_option("posit1,--posit1", positional_int, "");
    app.add_option("posit2,--posit2", positional_string, "");

    args = {"--posit2", "thing2", "7"};

    run();

    EXPECT_EQ(1u, app.count("posit2"));
    EXPECT_EQ(1u, app.count("--posit1"));
    EXPECT_EQ(7, positional_int);
    EXPECT_EQ("thing2", positional_string);
}

TEST_F(TApp, BigPositional) {
    std::vector<std::string> vec;
    app.add_option("pos", vec);

    args = {"one"};

    run();
    EXPECT_EQ(args, vec);

    args = {"one", "two"};
    run();

    EXPECT_EQ(args, vec);
}

TEST_F(TApp, Reset) {

    app.add_flag("--simple");
    double doub{0.0};
    app.add_option("-d,--double", doub);

    args = {"--simple", "--double", "1.2"};

    run();

    EXPECT_EQ(1u, app.count("--simple"));
    EXPECT_EQ(1u, app.count("-d"));
    EXPECT_DOUBLE_EQ(1.2, doub);

    app.clear();

    EXPECT_EQ(0u, app.count("--simple"));
    EXPECT_EQ(0u, app.count("-d"));

    run();

    EXPECT_EQ(1u, app.count("--simple"));
    EXPECT_EQ(1u, app.count("-d"));
    EXPECT_DOUBLE_EQ(1.2, doub);
}

TEST_F(TApp, RemoveOption) {
    app.add_flag("--one");
    auto opt = app.add_flag("--two");

    EXPECT_TRUE(app.remove_option(opt));
    EXPECT_FALSE(app.remove_option(opt));

    args = {"--two"};

    EXPECT_THROW(run(), CLI::ExtrasError);
}

TEST_F(TApp, RemoveNeedsLinks) {
    auto one = app.add_flag("--one");
    auto two = app.add_flag("--two");

    two->needs(one);
    one->needs(two);

    EXPECT_TRUE(app.remove_option(one));

    args = {"--two"};

    run();
}

TEST_F(TApp, RemoveExcludesLinks) {
    auto one = app.add_flag("--one");
    auto two = app.add_flag("--two");

    two->excludes(one);
    one->excludes(two);

    EXPECT_TRUE(app.remove_option(one));

    args = {"--two"};

    run();  // Mostly hoping it does not crash
}

TEST_F(TApp, FileNotExists) {
    std::string myfile{"TestNonFileNotUsed.txt"};
    ASSERT_NO_THROW(CLI::NonexistentPath(myfile));

    std::string filename;
    auto opt = app.add_option("--file", filename)->check(CLI::NonexistentPath, "path_check");
    args = {"--file", myfile};

    run();
    EXPECT_EQ(myfile, filename);

    bool ok = static_cast<bool>(std::ofstream(myfile.c_str()).put('a'));  // create file
    EXPECT_TRUE(ok);
    EXPECT_THROW(run(), CLI::ValidationError);
    // deactivate the check, so it should run now
    opt->get_validator("path_check")->active(false);
    EXPECT_NO_THROW(run());
    std::remove(myfile.c_str());
    EXPECT_FALSE(CLI::ExistingFile(myfile).empty());
}

TEST_F(TApp, FileExists) {
    std::string myfile{"TestNonFileNotUsed.txt"};
    EXPECT_FALSE(CLI::ExistingFile(myfile).empty());

    std::string filename = "Failed";
    app.add_option("--file", filename)->check(CLI::ExistingFile);
    args = {"--file", myfile};

    EXPECT_THROW(run(), CLI::ValidationError);

    bool ok = static_cast<bool>(std::ofstream(myfile.c_str()).put('a'));  // create file
    EXPECT_TRUE(ok);
    run();
    EXPECT_EQ(myfile, filename);

    std::remove(myfile.c_str());
    EXPECT_FALSE(CLI::ExistingFile(myfile).empty());
}

TEST_F(TApp, NotFileExists) {
    std::string myfile{"TestNonFileNotUsed.txt"};
    EXPECT_FALSE(CLI::ExistingFile(myfile).empty());

    std::string filename = "Failed";
    app.add_option("--file", filename)->check(!CLI::ExistingFile);
    args = {"--file", myfile};

    EXPECT_NO_THROW(run());

    bool ok = static_cast<bool>(std::ofstream(myfile.c_str()).put('a'));  // create file
    EXPECT_TRUE(ok);
    EXPECT_THROW(run(), CLI::ValidationError);

    std::remove(myfile.c_str());
    EXPECT_FALSE(CLI::ExistingFile(myfile).empty());
}

TEST_F(TApp, pair_check) {
    std::string myfile{"pair_check_file.txt"};
    bool ok = static_cast<bool>(std::ofstream(myfile.c_str()).put('a'));  // create file
    EXPECT_TRUE(ok);

    EXPECT_TRUE(CLI::ExistingFile(myfile).empty());
    std::pair<std::string, int> findex;

    auto v0 = CLI::ExistingFile;
    v0.application_index(0);
    auto v1 = CLI::PositiveNumber;
    v1.application_index(1);
    app.add_option("--file", findex)->check(v0)->check(v1);

    args = {"--file", myfile, "2"};

    EXPECT_NO_THROW(run());

    EXPECT_EQ(findex.first, myfile);
    EXPECT_EQ(findex.second, 2);

    args = {"--file", myfile, "-3"};

    EXPECT_THROW(run(), CLI::ValidationError);

    args = {"--file", myfile, "2"};
    std::remove(myfile.c_str());
    EXPECT_THROW(run(), CLI::ValidationError);
}

// this will require that modifying the multi-option policy for tuples be allowed which it isn't at present

TEST_F(TApp, pair_check_take_first) {
    std::string myfile{"pair_check_file2.txt"};
    bool ok = static_cast<bool>(std::ofstream(myfile.c_str()).put('a'));  // create file
    EXPECT_TRUE(ok);

    EXPECT_TRUE(CLI::ExistingFile(myfile).empty());
    std::pair<std::string, int> findex;

    auto opt = app.add_option("--file", findex)->check(CLI::ExistingFile)->check(CLI::PositiveNumber);
    EXPECT_THROW(opt->get_validator(3), CLI::OptionNotFound);
    opt->get_validator(0)->application_index(0);
    opt->get_validator(1)->application_index(1);
    opt->multi_option_policy(CLI::MultiOptionPolicy::TakeLast);
    args = {"--file", "not_a_file.txt", "-16", "--file", myfile, "2"};
    // should only check the last one
    EXPECT_NO_THROW(run());

    EXPECT_EQ(findex.first, myfile);
    EXPECT_EQ(findex.second, 2);

    opt->multi_option_policy(CLI::MultiOptionPolicy::TakeFirst);

    EXPECT_THROW(run(), CLI::ValidationError);
}

TEST_F(TApp, VectorFixedString) {
    std::vector<std::string> strvec;
    std::vector<std::string> answer{"mystring", "mystring2", "mystring3"};

    CLI::Option *opt = app.add_option("-s,--string", strvec)->expected(3);
    EXPECT_EQ(3, opt->get_expected());

    args = {"--string", "mystring", "mystring2", "mystring3"};
    run();
    EXPECT_EQ(3u, app.count("--string"));
    EXPECT_EQ(answer, strvec);
}

TEST_F(TApp, VectorDefaultedFixedString) {
    std::vector<std::string> strvec{"one"};
    std::vector<std::string> answer{"mystring", "mystring2", "mystring3"};

    CLI::Option *opt = app.add_option("-s,--string", strvec, "")->expected(3)->capture_default_str();
    EXPECT_EQ(3, opt->get_expected());

    args = {"--string", "mystring", "mystring2", "mystring3"};
    run();
    EXPECT_EQ(3u, app.count("--string"));
    EXPECT_EQ(answer, strvec);
}

TEST_F(TApp, VectorIndexedValidator) {
    std::vector<int> vvec;

    CLI::Option *opt = app.add_option("-v", vvec);

    args = {"-v", "1", "-1", "-v", "3", "-v", "-976"};
    run();
    EXPECT_EQ(4u, app.count("-v"));
    EXPECT_EQ(4u, vvec.size());
    opt->check(CLI::PositiveNumber.application_index(0));
    opt->check((!CLI::PositiveNumber).application_index(1));
    EXPECT_NO_THROW(run());
    EXPECT_EQ(4u, vvec.size());
    // v[3] would be negative
    opt->check(CLI::PositiveNumber.application_index(3));
    EXPECT_THROW(run(), CLI::ValidationError);
}

TEST_F(TApp, DefaultedResult) {
    std::string sval = "NA";
    int ival{0};
    auto opts = app.add_option("--string", sval)->capture_default_str();
    auto optv = app.add_option("--val", ival);
    args = {};
    run();
    EXPECT_EQ(sval, "NA");
    std::string nString;
    opts->results(nString);
    EXPECT_EQ(nString, "NA");
    int newIval;
    // EXPECT_THROW(optv->results(newIval), CLI::ConversionError);
    optv->default_str("442");
    optv->results(newIval);
    EXPECT_EQ(newIval, 442);
}

TEST_F(TApp, VectorUnlimString) {
    std::vector<std::string> strvec;
    std::vector<std::string> answer{"mystring", "mystring2", "mystring3"};

    CLI::Option *opt = app.add_option("-s,--string", strvec);
    EXPECT_EQ(1, opt->get_expected());
    EXPECT_EQ(CLI::detail::expected_max_vector_size, opt->get_expected_max());

    args = {"--string", "mystring", "mystring2", "mystring3"};
    run();
    EXPECT_EQ(3u, app.count("--string"));
    EXPECT_EQ(answer, strvec);

    args = {"-s", "mystring", "mystring2", "mystring3"};
    run();
    EXPECT_EQ(3u, app.count("--string"));
    EXPECT_EQ(answer, strvec);
}

// From https://github.com/CLIUtils/CLI11/issues/420
TEST_F(TApp, stringLikeTests) {
    struct nType {
        explicit nType(const std::string &a_value) : m_value{a_value} {}

        explicit operator std::string() const { return std::string{"op str"}; }

        std::string m_value;
    };

    nType m_type{"abc"};
    app.add_option("--type", m_type, "type")->capture_default_str();
    run();

    EXPECT_EQ(app["--type"]->as<std::string>(), "op str");
    args = {"--type", "bca"};
    run();
    EXPECT_EQ(std::string(m_type), "op str");
    EXPECT_EQ(m_type.m_value, "bca");
}

TEST_F(TApp, VectorExpectedRange) {
    std::vector<std::string> strvec;

    CLI::Option *opt = app.add_option("--string", strvec);
    opt->expected(2, 4)->multi_option_policy(CLI::MultiOptionPolicy::Throw);

    args = {"--string", "mystring", "mystring2", "mystring3"};
    run();
    EXPECT_EQ(3u, app.count("--string"));

    args = {"--string", "mystring"};
    EXPECT_THROW(run(), CLI::ArgumentMismatch);

    args = {"--string", "mystring", "mystring2", "string2", "--string", "string4", "string5"};
    EXPECT_THROW(run(), CLI::ArgumentMismatch);

    EXPECT_EQ(opt->get_expected_max(), 4);
    EXPECT_EQ(opt->get_expected_min(), 2);
    opt->expected(4, 2);  // just test the handling of reversed arguments
    EXPECT_EQ(opt->get_expected_max(), 4);
    EXPECT_EQ(opt->get_expected_min(), 2);
    opt->expected(-5);
    EXPECT_EQ(opt->get_expected_max(), 5);
    EXPECT_EQ(opt->get_expected_min(), 5);
    opt->expected(-5, 7);
    EXPECT_EQ(opt->get_expected_max(), 7);
    EXPECT_EQ(opt->get_expected_min(), 5);
}

TEST_F(TApp, VectorFancyOpts) {
    std::vector<std::string> strvec;
    std::vector<std::string> answer{"mystring", "mystring2", "mystring3"};

    CLI::Option *opt = app.add_option("-s,--string", strvec)->required()->expected(3);
    EXPECT_EQ(3, opt->get_expected());

    args = {"--string", "mystring", "mystring2", "mystring3"};
    run();
    EXPECT_EQ(3u, app.count("--string"));
    EXPECT_EQ(answer, strvec);

    args = {"one", "two"};
    EXPECT_THROW(run(), CLI::RequiredError);

    EXPECT_THROW(run(), CLI::ParseError);
}

TEST_F(TApp, OriginalOrder) {
    std::vector<int> st1;
    CLI::Option *op1 = app.add_option("-a", st1);
    std::vector<int> st2;
    CLI::Option *op2 = app.add_option("-b", st2);

    args = {"-a", "1", "-b", "2", "-a3", "-a", "4"};

    run();

    EXPECT_EQ(st1, std::vector<int>({1, 3, 4}));
    EXPECT_EQ(st2, std::vector<int>({2}));

    EXPECT_EQ(app.parse_order(), std::vector<CLI::Option *>({op1, op2, op1, op1}));
}

TEST_F(TApp, NeedsFlags) {
    CLI::Option *opt = app.add_flag("-s,--string");
    app.add_flag("--both")->needs(opt);

    run();

    args = {"-s"};
    run();

    args = {"-s", "--both"};
    run();

    args = {"--both"};
    EXPECT_THROW(run(), CLI::RequiresError);

    EXPECT_NO_THROW(opt->needs(opt));
}

TEST_F(TApp, ExcludesFlags) {
    CLI::Option *opt = app.add_flag("-s,--string");
    app.add_flag("--nostr")->excludes(opt);

    run();

    args = {"-s"};
    run();

    args = {"--nostr"};
    run();

    args = {"--nostr", "-s"};
    EXPECT_THROW(run(), CLI::ExcludesError);

    args = {"--string", "--nostr"};
    EXPECT_THROW(run(), CLI::ExcludesError);

    EXPECT_THROW(opt->excludes(opt), CLI::IncorrectConstruction);
}

TEST_F(TApp, ExcludesMixedFlags) {
    CLI::Option *opt1 = app.add_flag("--opt1");
    app.add_flag("--opt2");
    CLI::Option *opt3 = app.add_flag("--opt3");
    app.add_flag("--no")->excludes(opt1, "--opt2", opt3);

    run();

    args = {"--no"};
    run();

    args = {"--opt2"};
    run();

    args = {"--no", "--opt1"};
    EXPECT_THROW(run(), CLI::ExcludesError);

    args = {"--no", "--opt2"};
    EXPECT_THROW(run(), CLI::ExcludesError);
}

TEST_F(TApp, NeedsMultiFlags) {
    CLI::Option *opt1 = app.add_flag("--opt1");
    CLI::Option *opt2 = app.add_flag("--opt2");
    CLI::Option *opt3 = app.add_flag("--opt3");
    app.add_flag("--optall")->needs(opt1, opt2, opt3);

    run();

    args = {"--opt1"};
    run();

    args = {"--opt2"};
    run();

    args = {"--optall"};
    EXPECT_THROW(run(), CLI::RequiresError);

    args = {"--optall", "--opt1"};
    EXPECT_THROW(run(), CLI::RequiresError);

    args = {"--optall", "--opt2", "--opt1"};
    EXPECT_THROW(run(), CLI::RequiresError);

    args = {"--optall", "--opt1", "--opt2", "--opt3"};
    run();
}

TEST_F(TApp, NeedsMixedFlags) {
    CLI::Option *opt1 = app.add_flag("--opt1");
    app.add_flag("--opt2");
    app.add_flag("--opt3");
    app.add_flag("--optall")->needs(opt1, "--opt2", "--opt3");

    run();

    args = {"--opt1"};
    run();

    args = {"--opt2"};
    run();

    args = {"--optall"};
    EXPECT_THROW(run(), CLI::RequiresError);

    args = {"--optall", "--opt1"};
    EXPECT_THROW(run(), CLI::RequiresError);

    args = {"--optall", "--opt2", "--opt1"};
    EXPECT_THROW(run(), CLI::RequiresError);

    args = {"--optall", "--opt1", "--opt2", "--opt3"};
    run();
}

TEST_F(TApp, NeedsChainedFlags) {
    CLI::Option *opt1 = app.add_flag("--opt1");
    CLI::Option *opt2 = app.add_flag("--opt2")->needs(opt1);
    app.add_flag("--opt3")->needs(opt2);

    run();

    args = {"--opt1"};
    run();

    args = {"--opt2"};
    EXPECT_THROW(run(), CLI::RequiresError);

    args = {"--opt3"};
    EXPECT_THROW(run(), CLI::RequiresError);

    args = {"--opt3", "--opt2"};
    EXPECT_THROW(run(), CLI::RequiresError);

    args = {"--opt3", "--opt1"};
    EXPECT_THROW(run(), CLI::RequiresError);

    args = {"--opt2", "--opt1"};
    run();

    args = {"--opt1", "--opt2", "--opt3"};
    run();
}

TEST_F(TApp, Env) {

    put_env("CLI11_TEST_ENV_TMP", "2");

    int val{1};
    CLI::Option *vopt = app.add_option("--tmp", val)->envname("CLI11_TEST_ENV_TMP");

    run();

    EXPECT_EQ(2, val);
    EXPECT_EQ(1u, vopt->count());

    vopt->required();
    run();

    unset_env("CLI11_TEST_ENV_TMP");
    EXPECT_THROW(run(), CLI::RequiredError);
}

TEST_F(TApp, RangeInt) {
    int x{0};
    app.add_option("--one", x)->check(CLI::Range(3, 6));

    args = {"--one=1"};
    EXPECT_THROW(run(), CLI::ValidationError);

    args = {"--one=7"};
    EXPECT_THROW(run(), CLI::ValidationError);

    args = {"--one=3"};
    run();

    args = {"--one=5"};
    run();

    args = {"--one=6"};
    run();
}

TEST_F(TApp, RangeDouble) {

    double x{0.0};
    /// Note that this must be a double in Range, too
    app.add_option("--one", x)->check(CLI::Range(3.0, 6.0));

    args = {"--one=1"};
    EXPECT_THROW(run(), CLI::ValidationError);

    args = {"--one=7"};
    EXPECT_THROW(run(), CLI::ValidationError);

    args = {"--one=3"};
    run();

    args = {"--one=5"};
    run();

    args = {"--one=6"};
    run();
}

// Check to make sure programmatic access to left over is available
TEST_F(TApp, AllowExtras) {

    app.allow_extras();

    bool val{true};
    app.add_flag("-f", val);

    args = {"-x", "-f"};

    ASSERT_NO_THROW(run());
    EXPECT_TRUE(val);
    EXPECT_EQ(app.remaining(), std::vector<std::string>({"-x"}));
}

TEST_F(TApp, AllowExtrasOrder) {

    app.allow_extras();

    args = {"-x", "-f"};
    ASSERT_NO_THROW(run());
    EXPECT_EQ(app.remaining(), std::vector<std::string>({"-x", "-f"}));

    std::vector<std::string> left_over = app.remaining();
    app.parse(left_over);
    EXPECT_EQ(app.remaining(), std::vector<std::string>({"-f", "-x"}));
    EXPECT_EQ(app.remaining_for_passthrough(), left_over);
}

TEST_F(TApp, AllowExtrasCascade) {

    app.allow_extras();

    args = {"-x", "45", "-f", "27"};
    ASSERT_NO_THROW(run());
    EXPECT_EQ(app.remaining(), std::vector<std::string>({"-x", "45", "-f", "27"}));

    std::vector<std::string> left_over = app.remaining_for_passthrough();

    CLI::App capp{"cascade_program"};
    int v1 = 0;
    int v2 = 0;
    capp.add_option("-x", v1);
    capp.add_option("-f", v2);

    capp.parse(left_over);
    EXPECT_EQ(v1, 45);
    EXPECT_EQ(v2, 27);
}
// makes sure the error throws on the rValue version of the parse
TEST_F(TApp, ExtrasErrorRvalueParse) {

    args = {"-x", "45", "-f", "27"};
    EXPECT_THROW(app.parse(std::vector<std::string>({"-x", "45", "-f", "27"})), CLI::ExtrasError);
}

TEST_F(TApp, AllowExtrasCascadeDirect) {

    app.allow_extras();

    args = {"-x", "45", "-f", "27"};
    ASSERT_NO_THROW(run());
    EXPECT_EQ(app.remaining(), std::vector<std::string>({"-x", "45", "-f", "27"}));

    CLI::App capp{"cascade_program"};
    int v1{0};
    int v2{0};
    capp.add_option("-x", v1);
    capp.add_option("-f", v2);

    capp.parse(app.remaining_for_passthrough());
    EXPECT_EQ(v1, 45);
    EXPECT_EQ(v2, 27);
}

TEST_F(TApp, AllowExtrasArgModify) {

    int v1{0};
    int v2{0};
    app.allow_extras();
    app.add_option("-f", v2);
    args = {"27", "-f", "45", "-x"};
    auto cargs = args;
    app.parse(args);
    EXPECT_EQ(args, std::vector<std::string>({"45", "-x"}));

    CLI::App capp{"cascade_program"};

    capp.add_option("-x", v1);

    capp.parse(args);
    EXPECT_EQ(v1, 45);
    EXPECT_EQ(v2, 27);
}

// Test horrible error
TEST_F(TApp, CheckShortFail) {
    args = {"--two"};

    EXPECT_THROW(CLI::detail::AppFriend::parse_arg(&app, args, CLI::detail::Classifier::SHORT), CLI::HorribleError);
}

// Test horrible error
TEST_F(TApp, CheckLongFail) {
    args = {"-t"};

    EXPECT_THROW(CLI::detail::AppFriend::parse_arg(&app, args, CLI::detail::Classifier::LONG), CLI::HorribleError);
}

// Test horrible error
TEST_F(TApp, CheckWindowsFail) {
    args = {"-t"};

    EXPECT_THROW(CLI::detail::AppFriend::parse_arg(&app, args, CLI::detail::Classifier::WINDOWS), CLI::HorribleError);
}

// Test horrible error
TEST_F(TApp, CheckOtherFail) {
    args = {"-t"};

    EXPECT_THROW(CLI::detail::AppFriend::parse_arg(&app, args, CLI::detail::Classifier::NONE), CLI::HorribleError);
}

// Test horrible error
TEST_F(TApp, CheckSubcomFail) {
    args = {"subcom"};

    EXPECT_THROW(CLI::detail::AppFriend::parse_subcommand(&app, args), CLI::HorribleError);
}

TEST_F(TApp, FallthroughParentFail) {
    EXPECT_THROW(CLI::detail::AppFriend::get_fallthrough_parent(&app), CLI::HorribleError);
}

TEST_F(TApp, FallthroughParents) {
    auto sub = app.add_subcommand("test");
    EXPECT_EQ(CLI::detail::AppFriend::get_fallthrough_parent(sub), &app);

    auto ssub = sub->add_subcommand("sub2");
    EXPECT_EQ(CLI::detail::AppFriend::get_fallthrough_parent(ssub), sub);

    auto og1 = app.add_option_group("g1");
    auto og2 = og1->add_option_group("g2");
    auto og3 = og2->add_option_group("g3");
    EXPECT_EQ(CLI::detail::AppFriend::get_fallthrough_parent(og3), &app);

    auto ogb1 = sub->add_option_group("g1");
    auto ogb2 = ogb1->add_option_group("g2");
    auto ogb3 = ogb2->add_option_group("g3");
    EXPECT_EQ(CLI::detail::AppFriend::get_fallthrough_parent(ogb3), sub);

    ogb2->name("groupb");
    EXPECT_EQ(CLI::detail::AppFriend::get_fallthrough_parent(ogb3), ogb2);
}

TEST_F(TApp, OptionWithDefaults) {
    int someint{2};
    app.add_option("-a", someint)->capture_default_str();

    args = {"-a1", "-a2"};

    EXPECT_THROW(run(), CLI::ArgumentMismatch);
}

// Added to test ->transform
TEST_F(TApp, OrderedModifyingTransforms) {
    std::vector<std::string> val;
    auto m = app.add_option("-m", val);
    m->transform([](std::string x) { return x + "1"; });
    m->transform([](std::string x) { return x + "2"; });

    args = {"-mone", "-mtwo"};

    run();

    EXPECT_EQ(val, std::vector<std::string>({"one21", "two21"}));
}

TEST_F(TApp, ThrowingTransform) {
    std::string val;
    auto m = app.add_option("-m,--mess", val);
    m->transform([](std::string) -> std::string { throw CLI::ValidationError("My Message"); });

    ASSERT_NO_THROW(run());

    args = {"-mone"};

    ASSERT_THROW(run(), CLI::ValidationError);

    try {
        run();
    } catch(const CLI::ValidationError &e) {
        EXPECT_EQ(e.what(), std::string("--mess: My Message"));
    }
}

// This was added to make running a simple function on each item easier
TEST_F(TApp, EachItem) {

    std::vector<std::string> results;
    std::vector<std::string> dummy;

    auto opt = app.add_option("--vec", dummy);

    opt->each([&results](std::string item) { results.push_back(item); });

    args = {"--vec", "one", "two", "three"};

    run();

    EXPECT_EQ(results, dummy);
}

// #87
TEST_F(TApp, CustomDoubleOption) {

    std::pair<int, double> custom_opt;

    auto opt = app.add_option("posit", [&custom_opt](CLI::results_t vals) {
        custom_opt = {stol(vals.at(0)), stod(vals.at(1))};
        return true;
    });
    opt->type_name("INT FLOAT")->type_size(2);

    args = {"12", "1.5"};

    run();
    EXPECT_EQ(custom_opt.first, 12);
    EXPECT_DOUBLE_EQ(custom_opt.second, 1.5);
}

// now with tuple support this is possible
TEST_F(TApp, CustomDoubleOptionAlt) {

    std::pair<int, double> custom_opt;

    app.add_option("posit", custom_opt);

    args = {"12", "1.5"};

    run();
    EXPECT_EQ(custom_opt.first, 12);
    EXPECT_DOUBLE_EQ(custom_opt.second, 1.5);
}

// now with independent type sizes and expected this is possible
TEST_F(TApp, vectorPair) {

    std::vector<std::pair<int, std::string>> custom_opt;

    auto opt = app.add_option("--dict", custom_opt);

    args = {"--dict", "1", "str1", "--dict", "3", "str3"};

    run();
    EXPECT_EQ(custom_opt.size(), 2u);
    EXPECT_EQ(custom_opt[0].first, 1);
    EXPECT_EQ(custom_opt[1].second, "str3");

    args = {"--dict", "1", "str1", "--dict", "3", "str3", "--dict", "-1", "str4"};
    run();
    EXPECT_EQ(custom_opt.size(), 3u);
    EXPECT_EQ(custom_opt[2].first, -1);
    EXPECT_EQ(custom_opt[2].second, "str4");
    opt->check(CLI::PositiveNumber.application_index(0));

    EXPECT_THROW(run(), CLI::ValidationError);
}

TEST_F(TApp, vectorPairFail) {

    std::vector<std::pair<int, std::string>> custom_opt;

    app.add_option("--dict", custom_opt);

    args = {"--dict", "1", "str1", "--dict", "str3", "1"};

    EXPECT_THROW(run(), CLI::ConversionError);
}

TEST_F(TApp, vectorPairTypeRange) {

    std::vector<std::pair<int, std::string>> custom_opt;

    auto opt = app.add_option("--dict", custom_opt);

    opt->type_size(2, 1);  // just test switched arguments
    EXPECT_EQ(opt->get_type_size_min(), 1);
    EXPECT_EQ(opt->get_type_size_max(), 2);

    args = {"--dict", "1", "str1", "--dict", "3", "str3"};

    run();
    EXPECT_EQ(custom_opt.size(), 2u);
    EXPECT_EQ(custom_opt[0].first, 1);
    EXPECT_EQ(custom_opt[1].second, "str3");

    args = {"--dict", "1", "str1", "--dict", "3", "--dict", "-1", "str4"};
    run();
    EXPECT_EQ(custom_opt.size(), 3u);
    EXPECT_TRUE(custom_opt[1].second.empty());
    EXPECT_EQ(custom_opt[2].first, -1);
    EXPECT_EQ(custom_opt[2].second, "str4");

    opt->type_size(-2, -1);  // test negative arguments
    EXPECT_EQ(opt->get_type_size_min(), 1);
    EXPECT_EQ(opt->get_type_size_max(), 2);
    // this type size spec should run exactly as before
    run();
    EXPECT_EQ(custom_opt.size(), 3u);
    EXPECT_TRUE(custom_opt[1].second.empty());
    EXPECT_EQ(custom_opt[2].first, -1);
    EXPECT_EQ(custom_opt[2].second, "str4");
}

// now with independent type sizes and expected this is possible
TEST_F(TApp, vectorTuple) {

    std::vector<std::tuple<int, std::string, double>> custom_opt;

    auto opt = app.add_option("--dict", custom_opt);

    args = {"--dict", "1", "str1", "4.3", "--dict", "3", "str3", "2.7"};

    run();
    EXPECT_EQ(custom_opt.size(), 2u);
    EXPECT_EQ(std::get<0>(custom_opt[0]), 1);
    EXPECT_EQ(std::get<1>(custom_opt[1]), "str3");
    EXPECT_EQ(std::get<2>(custom_opt[1]), 2.7);

    args = {"--dict", "1", "str1", "4.3", "--dict", "3", "str3", "2.7", "--dict", "-1", "str4", "-1.87"};
    run();
    EXPECT_EQ(custom_opt.size(), 3u);
    EXPECT_EQ(std::get<0>(custom_opt[2]), -1);
    EXPECT_EQ(std::get<1>(custom_opt[2]), "str4");
    EXPECT_EQ(std::get<2>(custom_opt[2]), -1.87);
    opt->check(CLI::PositiveNumber.application_index(0));

    EXPECT_THROW(run(), CLI::ValidationError);

    args.back() = "haha";
    args[9] = "45";
    EXPECT_THROW(run(), CLI::ConversionError);
}

// now with independent type sizes and expected this is possible
TEST_F(TApp, vectorVector) {

    std::vector<std::vector<int>> custom_opt;

    auto opt = app.add_option("--dict", custom_opt);

    args = {"--dict", "1", "2", "4", "--dict", "3", "1"};

    run();
    EXPECT_EQ(custom_opt.size(), 2u);
    EXPECT_EQ(custom_opt[0].size(), 3u);
    EXPECT_EQ(custom_opt[1].size(), 2u);

    args = {"--dict", "1", "2", "4", "--dict", "3", "1", "--dict", "3", "--dict",
            "3",      "3", "3", "3", "3",      "3", "3", "3",      "3", "-3"};
    run();
    EXPECT_EQ(custom_opt.size(), 4u);
    EXPECT_EQ(custom_opt[0].size(), 3u);
    EXPECT_EQ(custom_opt[1].size(), 2u);
    EXPECT_EQ(custom_opt[2].size(), 1u);
    EXPECT_EQ(custom_opt[3].size(), 10u);
    opt->check(CLI::PositiveNumber.application_index(9));

    EXPECT_THROW(run(), CLI::ValidationError);
    args.pop_back();
    EXPECT_NO_THROW(run());

    args.back() = "haha";
    EXPECT_THROW(run(), CLI::ConversionError);
}

// #128
TEST_F(TApp, RepeatingMultiArgumentOptions) {
    std::vector<std::string> entries;
    app.add_option("--entry", entries, "set a key and value")->type_name("KEY VALUE")->type_size(-2);

    args = {"--entry", "key1", "value1", "--entry", "key2", "value2"};
    ASSERT_NO_THROW(run());
    EXPECT_EQ(entries, std::vector<std::string>({"key1", "value1", "key2", "value2"}));

    args.pop_back();
    ASSERT_THROW(run(), CLI::ArgumentMismatch);
}

// #122
TEST_F(TApp, EmptyOptionEach) {
    std::string q;
    app.add_option("--each")->each([&q](std::string s) { q = s; });

    args = {"--each", "that"};
    run();

    EXPECT_EQ(q, "that");
}

// #122
TEST_F(TApp, EmptyOptionFail) {
    std::string q;
    app.add_option("--each");

    args = {"--each", "that"};
    run();
}

TEST_F(TApp, BeforeRequirements) {
    app.add_flag_function("-a", [](std::int64_t) { throw CLI::Success(); });
    app.add_flag_function("-b", [](std::int64_t) { throw CLI::CallForHelp(); });

    args = {"extra"};
    EXPECT_THROW(run(), CLI::ExtrasError);

    args = {"-a", "extra"};
    EXPECT_THROW(run(), CLI::Success);

    args = {"-b", "extra"};
    EXPECT_THROW(run(), CLI::CallForHelp);

    // These run in definition order.
    args = {"-a", "-b", "extra"};
    EXPECT_THROW(run(), CLI::Success);

    // Currently, the original order is not preserved when calling callbacks
    // args = {"-b", "-a", "extra"};
    // EXPECT_THROW(run(), CLI::CallForHelp);
}

// #209
TEST_F(TApp, CustomUserSepParse) {

    std::vector<int> vals{1, 2, 3};
    args = {"--idx", "1,2,3"};
    auto opt = app.add_option("--idx", vals)->delimiter(',');
    run();
    EXPECT_EQ(vals, std::vector<int>({1, 2, 3}));
    std::vector<int> vals2;
    // check that the results vector gets the results in the same way
    opt->results(vals2);
    EXPECT_EQ(vals2, vals);

    app.remove_option(opt);

    app.add_option("--idx", vals)->delimiter(',')->capture_default_str();
    run();
    EXPECT_EQ(vals, std::vector<int>({1, 2, 3}));
}

// #209
TEST_F(TApp, DefaultUserSepParse) {

    std::vector<std::string> vals;
    args = {"--idx", "1 2 3", "4 5 6"};
    auto opt = app.add_option("--idx", vals, "");
    run();
    EXPECT_EQ(vals, std::vector<std::string>({"1 2 3", "4 5 6"}));
    opt->delimiter(',');
    run();
    EXPECT_EQ(vals, std::vector<std::string>({"1 2 3", "4 5 6"}));
}

// #209
TEST_F(TApp, BadUserSepParse) {

    std::vector<int> vals;
    app.add_option("--idx", vals);

    args = {"--idx", "1,2,3"};

    EXPECT_THROW(run(), CLI::ConversionError);
}

// #209
TEST_F(TApp, CustomUserSepParse2) {

    std::vector<int> vals{1, 2, 3};
    args = {"--idx", "1,2,"};
    auto opt = app.add_option("--idx", vals)->delimiter(',');
    run();
    EXPECT_EQ(vals, std::vector<int>({1, 2}));

    app.remove_option(opt);

    app.add_option("--idx", vals, "")->delimiter(',')->capture_default_str();
    run();
    EXPECT_EQ(vals, std::vector<int>({1, 2}));
}

TEST_F(TApp, CustomUserSepParseFunction) {

    std::vector<int> vals{1, 2, 3};
    args = {"--idx", "1,2,3"};
    app.add_option_function<std::vector<int>>("--idx", [&vals](std::vector<int> v) { vals = std::move(v); })
        ->delimiter(',');
    run();
    EXPECT_EQ(vals, std::vector<int>({1, 2, 3}));
}

// delimiter removal
TEST_F(TApp, CustomUserSepParseToggle) {

    std::vector<std::string> vals;
    args = {"--idx", "1,2,3"};
    auto opt = app.add_option("--idx", vals)->delimiter(',');
    run();
    EXPECT_EQ(vals, std::vector<std::string>({"1", "2", "3"}));
    opt->delimiter('\0');
    run();
    EXPECT_EQ(vals, std::vector<std::string>({"1,2,3"}));
    opt->delimiter(',');
    run();
    EXPECT_EQ(vals, std::vector<std::string>({"1", "2", "3"}));
}

// #209
TEST_F(TApp, CustomUserSepParse3) {

    std::vector<int> vals = {1, 2, 3};
    args = {"--idx",
            "1",
            ","
            "2"};
    auto opt = app.add_option("--idx", vals)->delimiter(',');
    run();
    EXPECT_EQ(vals, std::vector<int>({1, 2}));
    app.remove_option(opt);

    app.add_option("--idx", vals, "", false)->delimiter(',');
    run();
    EXPECT_EQ(vals, std::vector<int>({1, 2}));
}

// #209
TEST_F(TApp, CustomUserSepParse4) {

    std::vector<int> vals;
    args = {"--idx", "1,    2"};
    auto opt = app.add_option("--idx", vals)->delimiter(',')->capture_default_str();
    run();
    EXPECT_EQ(vals, std::vector<int>({1, 2}));

    app.remove_option(opt);

    app.add_option("--idx", vals)->delimiter(',');
    run();
    EXPECT_EQ(vals, std::vector<int>({1, 2}));
}

// #218
TEST_F(TApp, CustomUserSepParse5) {

    std::vector<std::string> bar;
    args = {"this", "is", "a", "test"};
    auto opt = app.add_option("bar", bar, "bar");
    run();
    EXPECT_EQ(bar, std::vector<std::string>({"this", "is", "a", "test"}));

    app.remove_option(opt);
    args = {"this", "is", "a", "test"};
    app.add_option("bar", bar, "bar")->capture_default_str();
    run();
    EXPECT_EQ(bar, std::vector<std::string>({"this", "is", "a", "test"}));
}
