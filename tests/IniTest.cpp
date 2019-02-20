#include "app_helper.hpp"

#include "gmock/gmock.h"
#include <cstdio>
#include <sstream>

using ::testing::HasSubstr;
using ::testing::Not;

TEST(StringBased, IniJoin) {
    std::vector<std::string> items = {"one", "two", "three four"};
    std::string result = "one two \"three four\"";

    EXPECT_EQ(CLI::detail::ini_join(items), result);
}

TEST(StringBased, First) {
    std::stringstream ofile;

    ofile << "one=three" << std::endl;
    ofile << "two=four" << std::endl;

    ofile.seekg(0, std::ios::beg);

    std::vector<CLI::ConfigItem> output = CLI::ConfigINI().from_config(ofile);

    EXPECT_EQ(2u, output.size());
    EXPECT_EQ("one", output.at(0).name);
    EXPECT_EQ(1u, output.at(0).inputs.size());
    EXPECT_EQ("three", output.at(0).inputs.at(0));
    EXPECT_EQ("two", output.at(1).name);
    EXPECT_EQ(1u, output.at(1).inputs.size());
    EXPECT_EQ("four", output.at(1).inputs.at(0));
}

TEST(StringBased, FirstWithComments) {
    std::stringstream ofile;

    ofile << ";this is a comment" << std::endl;
    ofile << "one=three" << std::endl;
    ofile << "two=four" << std::endl;
    ofile << "; and another one" << std::endl;

    ofile.seekg(0, std::ios::beg);

    std::vector<CLI::ConfigItem> output = CLI::ConfigINI().from_config(ofile);

    EXPECT_EQ(2u, output.size());
    EXPECT_EQ("one", output.at(0).name);
    EXPECT_EQ(1u, output.at(0).inputs.size());
    EXPECT_EQ("three", output.at(0).inputs.at(0));
    EXPECT_EQ("two", output.at(1).name);
    EXPECT_EQ(1u, output.at(1).inputs.size());
    EXPECT_EQ("four", output.at(1).inputs.at(0));
}

TEST(StringBased, Quotes) {
    std::stringstream ofile;

    ofile << R"(one = "three")" << std::endl;
    ofile << R"(two = 'four')" << std::endl;
    ofile << R"(five = "six and seven")" << std::endl;

    ofile.seekg(0, std::ios::beg);

    std::vector<CLI::ConfigItem> output = CLI::ConfigINI().from_config(ofile);

    EXPECT_EQ(3u, output.size());
    EXPECT_EQ("one", output.at(0).name);
    EXPECT_EQ(1u, output.at(0).inputs.size());
    EXPECT_EQ("three", output.at(0).inputs.at(0));
    EXPECT_EQ("two", output.at(1).name);
    EXPECT_EQ(1u, output.at(1).inputs.size());
    EXPECT_EQ("four", output.at(1).inputs.at(0));
    EXPECT_EQ("five", output.at(2).name);
    EXPECT_EQ(1u, output.at(2).inputs.size());
    EXPECT_EQ("six and seven", output.at(2).inputs.at(0));
}

TEST(StringBased, Vector) {
    std::stringstream ofile;

    ofile << "one = three" << std::endl;
    ofile << "two = four" << std::endl;
    ofile << "five = six and seven" << std::endl;

    ofile.seekg(0, std::ios::beg);

    std::vector<CLI::ConfigItem> output = CLI::ConfigINI().from_config(ofile);

    EXPECT_EQ(3u, output.size());
    EXPECT_EQ("one", output.at(0).name);
    EXPECT_EQ(1u, output.at(0).inputs.size());
    EXPECT_EQ("three", output.at(0).inputs.at(0));
    EXPECT_EQ("two", output.at(1).name);
    EXPECT_EQ(1u, output.at(1).inputs.size());
    EXPECT_EQ("four", output.at(1).inputs.at(0));
    EXPECT_EQ("five", output.at(2).name);
    EXPECT_EQ(3u, output.at(2).inputs.size());
    EXPECT_EQ("six", output.at(2).inputs.at(0));
    EXPECT_EQ("and", output.at(2).inputs.at(1));
    EXPECT_EQ("seven", output.at(2).inputs.at(2));
}

TEST(StringBased, Spaces) {
    std::stringstream ofile;

    ofile << "one = three" << std::endl;
    ofile << "two = four" << std::endl;

    ofile.seekg(0, std::ios::beg);

    std::vector<CLI::ConfigItem> output = CLI::ConfigINI().from_config(ofile);

    EXPECT_EQ(2u, output.size());
    EXPECT_EQ("one", output.at(0).name);
    EXPECT_EQ(1u, output.at(0).inputs.size());
    EXPECT_EQ("three", output.at(0).inputs.at(0));
    EXPECT_EQ("two", output.at(1).name);
    EXPECT_EQ(1u, output.at(1).inputs.size());
    EXPECT_EQ("four", output.at(1).inputs.at(0));
}

TEST(StringBased, Sections) {
    std::stringstream ofile;

    ofile << "one=three" << std::endl;
    ofile << "[second]" << std::endl;
    ofile << "  two=four" << std::endl;

    ofile.seekg(0, std::ios::beg);

    std::vector<CLI::ConfigItem> output = CLI::ConfigINI().from_config(ofile);

    EXPECT_EQ(2u, output.size());
    EXPECT_EQ("one", output.at(0).name);
    EXPECT_EQ(1u, output.at(0).inputs.size());
    EXPECT_EQ("three", output.at(0).inputs.at(0));
    EXPECT_EQ("two", output.at(1).name);
    EXPECT_EQ("second", output.at(1).parents.at(0));
    EXPECT_EQ(1u, output.at(1).inputs.size());
    EXPECT_EQ("four", output.at(1).inputs.at(0));
    EXPECT_EQ("second.two", output.at(1).fullname());
}

TEST(StringBased, SpacesSections) {
    std::stringstream ofile;

    ofile << "one=three" << std::endl;
    ofile << std::endl;
    ofile << "[second]" << std::endl;
    ofile << "   " << std::endl;
    ofile << "  two=four" << std::endl;

    ofile.seekg(0, std::ios::beg);

    std::vector<CLI::ConfigItem> output = CLI::ConfigINI().from_config(ofile);

    EXPECT_EQ(2u, output.size());
    EXPECT_EQ("one", output.at(0).name);
    EXPECT_EQ(1u, output.at(0).inputs.size());
    EXPECT_EQ("three", output.at(0).inputs.at(0));
    EXPECT_EQ("two", output.at(1).name);
    EXPECT_EQ(1u, output.at(1).parents.size());
    EXPECT_EQ("second", output.at(1).parents.at(0));
    EXPECT_EQ(1u, output.at(1).inputs.size());
    EXPECT_EQ("four", output.at(1).inputs.at(0));
}

TEST_F(TApp, IniNotRequired) {

    TempFile tmpini{"TestIniTmp.ini"};

    app.set_config("--config", tmpini);

    {
        std::ofstream out{tmpini};
        out << "[default]" << std::endl;
        out << "two=99" << std::endl;
        out << "three=3" << std::endl;
    }

    int one = 0, two = 0, three = 0;
    app.add_option("--one", one);
    app.add_option("--two", two);
    app.add_option("--three", three);

    args = {"--one=1"};

    run();

    EXPECT_EQ(1, one);
    EXPECT_EQ(99, two);
    EXPECT_EQ(3, three);

    one = two = three = 0;
    args = {"--one=1", "--two=2"};

    run();

    EXPECT_EQ(1, one);
    EXPECT_EQ(2, two);
    EXPECT_EQ(3, three);
}

TEST_F(TApp, IniSuccessOnUnknownOption) {
    TempFile tmpini{"TestIniTmp.ini"};

    app.set_config("--config", tmpini);
    app.allow_config_extras(true);

    {
        std::ofstream out{tmpini};
        out << "three=3" << std::endl;
        out << "two=99" << std::endl;
    }

    int two = 0;
    app.add_option("--two", two);
    run();
    EXPECT_EQ(99, two);
}

TEST_F(TApp, IniGetRemainingOption) {
    TempFile tmpini{"TestIniTmp.ini"};

    app.set_config("--config", tmpini);
    app.allow_config_extras(true);

    std::string ExtraOption = "three";
    std::string ExtraOptionValue = "3";
    {
        std::ofstream out{tmpini};
        out << ExtraOption << "=" << ExtraOptionValue << std::endl;
        out << "two=99" << std::endl;
    }

    int two = 0;
    app.add_option("--two", two);
    ASSERT_NO_THROW(run());
    std::vector<std::string> ExpectedRemaining = {ExtraOption};
    EXPECT_EQ(app.remaining(), ExpectedRemaining);
}

TEST_F(TApp, IniGetNoRemaining) {
    TempFile tmpini{"TestIniTmp.ini"};

    app.set_config("--config", tmpini);
    app.allow_config_extras(true);

    {
        std::ofstream out{tmpini};
        out << "two=99" << std::endl;
    }

    int two = 0;
    app.add_option("--two", two);
    ASSERT_NO_THROW(run());
    EXPECT_EQ(app.remaining().size(), 0u);
}

TEST_F(TApp, IniNotRequiredNotDefault) {

    TempFile tmpini{"TestIniTmp.ini"};
    TempFile tmpini2{"TestIniTmp2.ini"};

    app.set_config("--config", tmpini);

    {
        std::ofstream out{tmpini};
        out << "[default]" << std::endl;
        out << "two=99" << std::endl;
        out << "three=3" << std::endl;
    }

    {
        std::ofstream out{tmpini2};
        out << "[default]" << std::endl;
        out << "two=98" << std::endl;
        out << "three=4" << std::endl;
    }

    int one = 0, two = 0, three = 0;
    app.add_option("--one", one);
    app.add_option("--two", two);
    app.add_option("--three", three);

    run();

    EXPECT_EQ(99, two);
    EXPECT_EQ(3, three);

    args = {"--config", tmpini2};
    run();

    EXPECT_EQ(98, two);
    EXPECT_EQ(4, three);
}

TEST_F(TApp, IniRequiredNotFound) {

    std::string noini = "TestIniNotExist.ini";
    app.set_config("--config", noini, "", true);

    EXPECT_THROW(run(), CLI::FileError);
}

TEST_F(TApp, IniNotRequiredPassedNotFound) {

    std::string noini = "TestIniNotExist.ini";
    app.set_config("--config", "", "", false);

    args = {"--config", noini};
    EXPECT_THROW(run(), CLI::FileError);
}

TEST_F(TApp, IniOverwrite) {

    TempFile tmpini{"TestIniTmp.ini"};
    {
        std::ofstream out{tmpini};
        out << "[default]" << std::endl;
        out << "two=99" << std::endl;
    }

    std::string orig = "filename_not_exist.ini";
    std::string next = "TestIniTmp.ini";
    app.set_config("--config", orig);
    // Make sure this can be overwritten
    app.set_config("--conf", next);
    int two = 7;
    app.add_option("--two", two);

    run();

    EXPECT_EQ(99, two);
}

TEST_F(TApp, IniRequired) {

    TempFile tmpini{"TestIniTmp.ini"};

    app.set_config("--config", tmpini, "", true);

    {
        std::ofstream out{tmpini};
        out << "[default]" << std::endl;
        out << "two=99" << std::endl;
        out << "three=3" << std::endl;
    }

    int one = 0, two = 0, three = 0;
    app.add_option("--one", one)->required();
    app.add_option("--two", two)->required();
    app.add_option("--three", three)->required();

    args = {"--one=1"};

    run();

    one = two = three = 0;
    args = {"--one=1", "--two=2"};

    run();

    args = {};

    EXPECT_THROW(run(), CLI::RequiredError);

    args = {"--two=2"};

    EXPECT_THROW(run(), CLI::RequiredError);
}

TEST_F(TApp, IniVector) {

    TempFile tmpini{"TestIniTmp.ini"};

    app.set_config("--config", tmpini);

    {
        std::ofstream out{tmpini};
        out << "[default]" << std::endl;
        out << "two=2 3" << std::endl;
        out << "three=1 2 3" << std::endl;
    }

    std::vector<int> two, three;
    app.add_option("--two", two)->expected(2)->required();
    app.add_option("--three", three)->required();

    run();

    EXPECT_EQ(std::vector<int>({2, 3}), two);
    EXPECT_EQ(std::vector<int>({1, 2, 3}), three);
}

TEST_F(TApp, IniLayered) {

    TempFile tmpini{"TestIniTmp.ini"};

    app.set_config("--config", tmpini);

    {
        std::ofstream out{tmpini};
        out << "[default]" << std::endl;
        out << "val=1" << std::endl;
        out << "[subcom]" << std::endl;
        out << "val=2" << std::endl;
        out << "subsubcom.val=3" << std::endl;
    }

    int one = 0, two = 0, three = 0;
    app.add_option("--val", one);
    auto subcom = app.add_subcommand("subcom");
    subcom->add_option("--val", two);
    auto subsubcom = subcom->add_subcommand("subsubcom");
    subsubcom->add_option("--val", three);

    run();

    EXPECT_EQ(1, one);
    EXPECT_EQ(2, two);
    EXPECT_EQ(3, three);
}

TEST_F(TApp, IniFailure) {

    TempFile tmpini{"TestIniTmp.ini"};

    app.set_config("--config", tmpini);

    {
        std::ofstream out{tmpini};
        out << "[default]" << std::endl;
        out << "val=1" << std::endl;
    }

    EXPECT_THROW(run(), CLI::ConfigError);
}

TEST_F(TApp, IniConfigurable) {

    TempFile tmpini{"TestIniTmp.ini"};

    app.set_config("--config", tmpini);
    bool value;
    app.add_flag("--val", value)->configurable(true);

    {
        std::ofstream out{tmpini};
        out << "[default]" << std::endl;
        out << "val=1" << std::endl;
    }

    ASSERT_NO_THROW(run());
    EXPECT_TRUE(value);
}

TEST_F(TApp, IniNotConfigurable) {

    TempFile tmpini{"TestIniTmp.ini"};

    app.set_config("--config", tmpini);
    bool value;
    app.add_flag("--val", value)->configurable(false);

    {
        std::ofstream out{tmpini};
        out << "[default]" << std::endl;
        out << "val=1" << std::endl;
    }

    EXPECT_THROW(run(), CLI::ConfigError);
}

TEST_F(TApp, IniSubFailure) {

    TempFile tmpini{"TestIniTmp.ini"};

    app.add_subcommand("other");
    app.set_config("--config", tmpini);

    {
        std::ofstream out{tmpini};
        out << "[other]" << std::endl;
        out << "val=1" << std::endl;
    }

    EXPECT_THROW(run(), CLI::ConfigError);
}

TEST_F(TApp, IniNoSubFailure) {

    TempFile tmpini{"TestIniTmp.ini"};

    app.set_config("--config", tmpini);

    {
        std::ofstream out{tmpini};
        out << "[other]" << std::endl;
        out << "val=1" << std::endl;
    }

    EXPECT_THROW(run(), CLI::ConfigError);
}

TEST_F(TApp, IniFlagConvertFailure) {

    TempFile tmpini{"TestIniTmp.ini"};

    app.add_flag("--flag");
    app.set_config("--config", tmpini);

    {
        std::ofstream out{tmpini};
        out << "flag=moobook" << std::endl;
    }
    run();
    bool result;
    auto *opt = app.get_option("--flag");
    EXPECT_THROW(opt->results(result), CLI::ConversionError);
    std::string res;
    opt->results(res);
    EXPECT_EQ(res, "moobook");
}

TEST_F(TApp, IniFlagNumbers) {

    TempFile tmpini{"TestIniTmp.ini"};

    bool boo;
    app.add_flag("--flag", boo);
    app.set_config("--config", tmpini);

    {
        std::ofstream out{tmpini};
        out << "flag=3" << std::endl;
    }

    ASSERT_NO_THROW(run());
    EXPECT_TRUE(boo);
}

TEST_F(TApp, IniFlagDual) {

    TempFile tmpini{"TestIniTmp.ini"};

    bool boo;
    app.add_flag("--flag", boo);
    app.set_config("--config", tmpini);

    {
        std::ofstream out{tmpini};
        out << "flag=1 1" << std::endl;
    }

    EXPECT_THROW(run(), CLI::ConversionError);
}

TEST_F(TApp, IniFlagText) {

    TempFile tmpini{"TestIniTmp.ini"};

    bool flag1, flag2, flag3, flag4;
    app.add_flag("--flag1", flag1);
    app.add_flag("--flag2", flag2);
    app.add_flag("--flag3", flag3);
    app.add_flag("--flag4", flag4);
    app.set_config("--config", tmpini);

    {
        std::ofstream out{tmpini};
        out << "flag1=true" << std::endl;
        out << "flag2=on" << std::endl;
        out << "flag3=off" << std::endl;
        out << "flag4=1" << std::endl;
    }

    run();

    EXPECT_TRUE(flag1);
    EXPECT_TRUE(flag2);
    EXPECT_FALSE(flag3);
    EXPECT_TRUE(flag4);
}

TEST_F(TApp, IniFlags) {
    TempFile tmpini{"TestIniTmp.ini"};
    app.set_config("--config", tmpini);

    {
        std::ofstream out{tmpini};
        out << "[default]" << std::endl;
        out << "two=2" << std::endl;
        out << "three=true" << std::endl;
        out << "four=on" << std::endl;
        out << "five" << std::endl;
    }

    int two;
    bool three, four, five;
    app.add_flag("--two", two);
    app.add_flag("--three", three);
    app.add_flag("--four", four);
    app.add_flag("--five", five);

    run();

    EXPECT_EQ(2, two);
    EXPECT_TRUE(three);
    EXPECT_TRUE(four);
    EXPECT_TRUE(five);
}

TEST_F(TApp, IniFalseFlags) {
    TempFile tmpini{"TestIniTmp.ini"};
    app.set_config("--config", tmpini);

    {
        std::ofstream out{tmpini};
        out << "[default]" << std::endl;
        out << "two=-2" << std::endl;
        out << "three=false" << std::endl;
        out << "four=1" << std::endl;
        out << "five" << std::endl;
    }

    int two;
    bool three, four, five;
    app.add_flag("--two", two);
    app.add_flag("--three", three);
    app.add_flag("--four", four);
    app.add_flag("--five", five);

    run();

    EXPECT_EQ(-2, two);
    EXPECT_FALSE(three);
    EXPECT_TRUE(four);
    EXPECT_TRUE(five);
}

TEST_F(TApp, IniFalseFlagsDef) {
    TempFile tmpini{"TestIniTmp.ini"};
    app.set_config("--config", tmpini);

    {
        std::ofstream out{tmpini};
        out << "[default]" << std::endl;
        out << "two=2" << std::endl;
        out << "three=true" << std::endl;
        out << "four=on" << std::endl;
        out << "five" << std::endl;
    }

    int two;
    bool three, four, five;
    app.add_flag("--two{false}", two);
    app.add_flag("--three", three);
    app.add_flag("!--four", four);
    app.add_flag("--five", five);

    run();

    EXPECT_EQ(-2, two);
    EXPECT_TRUE(three);
    EXPECT_FALSE(four);
    EXPECT_TRUE(five);
}

TEST_F(TApp, IniFalseFlagsDefDisableOverrideError) {
    TempFile tmpini{"TestIniTmp.ini"};
    app.set_config("--config", tmpini);

    {
        std::ofstream out{tmpini};
        out << "[default]" << std::endl;
        out << "two=2" << std::endl;
        out << "four=on" << std::endl;
        out << "five" << std::endl;
    }

    int two;
    bool four, five;
    app.add_flag("--two{false}", two)->disable_flag_override();
    app.add_flag("!--four", four);
    app.add_flag("--five", five);

    EXPECT_THROW(run(), CLI::ArgumentMismatch);
}

TEST_F(TApp, IniFalseFlagsDefDisableOverrideSuccess) {
    TempFile tmpini{"TestIniTmp.ini"};
    app.set_config("--config", tmpini);

    {
        std::ofstream out{tmpini};
        out << "[default]" << std::endl;
        out << "two=2" << std::endl;
        out << "four={}" << std::endl;
        out << "val=15" << std::endl;
    }

    int two, four, val;
    app.add_flag("--two{2}", two)->disable_flag_override();
    app.add_flag("--four{4}", four)->disable_flag_override();
    app.add_flag("--val", val);

    run();

    EXPECT_EQ(2, two);
    EXPECT_EQ(4, four);
    EXPECT_EQ(15, val);
}

TEST_F(TApp, IniOutputSimple) {

    int v;
    app.add_option("--simple", v);

    args = {"--simple=3"};

    run();

    std::string str = app.config_to_str();
    EXPECT_EQ("simple=3\n", str);
}

TEST_F(TApp, IniOutputNoConfigurable) {

    int v1, v2;
    app.add_option("--simple", v1);
    app.add_option("--noconf", v2)->configurable(false);

    args = {"--simple=3", "--noconf=2"};

    run();

    std::string str = app.config_to_str();
    EXPECT_EQ("simple=3\n", str);
}

TEST_F(TApp, IniOutputShortSingleDescription) {
    std::string flag = "some_flag";
    const std::string description = "Some short description.";
    app.add_flag("--" + flag, description);

    run();

    std::string str = app.config_to_str(true, true);
    EXPECT_THAT(str, HasSubstr("; " + description + "\n" + flag + "=false\n"));
}

TEST_F(TApp, IniOutputShortDoubleDescription) {
    std::string flag1 = "flagnr1";
    std::string flag2 = "flagnr2";
    const std::string description1 = "First description.";
    const std::string description2 = "Second description.";
    app.add_flag("--" + flag1, description1);
    app.add_flag("--" + flag2, description2);

    run();

    std::string str = app.config_to_str(true, true);
    EXPECT_EQ(str, "; " + description1 + "\n" + flag1 + "=false\n\n; " + description2 + "\n" + flag2 + "=false\n");
}

TEST_F(TApp, IniOutputMultiLineDescription) {
    std::string flag = "some_flag";
    const std::string description = "Some short description.\nThat has lines.";
    app.add_flag("--" + flag, description);

    run();

    std::string str = app.config_to_str(true, true);
    EXPECT_THAT(str, HasSubstr("; Some short description.\n"));
    EXPECT_THAT(str, HasSubstr("; That has lines.\n"));
    EXPECT_THAT(str, HasSubstr(flag + "=false\n"));
}

TEST_F(TApp, IniOutputVector) {

    std::vector<int> v;
    app.add_option("--vector", v);

    args = {"--vector", "1", "2", "3"};

    run();

    std::string str = app.config_to_str();
    EXPECT_EQ("vector=1 2 3\n", str);
}

TEST_F(TApp, IniOutputFlag) {

    int v, q;
    app.add_option("--simple", v);
    app.add_flag("--nothing");
    app.add_flag("--onething");
    app.add_flag("--something", q);

    args = {"--simple=3", "--onething", "--something", "--something"};

    run();

    std::string str = app.config_to_str();
    EXPECT_THAT(str, HasSubstr("simple=3"));
    EXPECT_THAT(str, Not(HasSubstr("nothing")));
    EXPECT_THAT(str, HasSubstr("onething=true"));
    EXPECT_THAT(str, HasSubstr("something=2"));

    str = app.config_to_str(true);
    EXPECT_THAT(str, HasSubstr("nothing"));
}

TEST_F(TApp, IniOutputSet) {

    int v;
    app.add_option("--simple", v)->check(CLI::IsMember({1, 2, 3}));

    args = {"--simple=2"};

    run();

    std::string str = app.config_to_str();
    EXPECT_THAT(str, HasSubstr("simple=2"));
}

TEST_F(TApp, IniOutputDefault) {

    int v = 7;
    app.add_option("--simple", v, "", true);

    run();

    std::string str = app.config_to_str();
    EXPECT_THAT(str, Not(HasSubstr("simple=7")));

    str = app.config_to_str(true);
    EXPECT_THAT(str, HasSubstr("simple=7"));
}

TEST_F(TApp, IniOutputSubcom) {

    app.add_flag("--simple");
    auto subcom = app.add_subcommand("other");
    subcom->add_flag("--newer");

    args = {"--simple", "other", "--newer"};
    run();

    std::string str = app.config_to_str();
    EXPECT_THAT(str, HasSubstr("simple=true"));
    EXPECT_THAT(str, HasSubstr("other.newer=true"));
}

TEST_F(TApp, IniQuotedOutput) {

    std::string val1;
    app.add_option("--val1", val1);

    std::string val2;
    app.add_option("--val2", val2);

    args = {"--val1", "I am a string", "--val2", R"(I am a "confusing" string)"};

    run();

    EXPECT_EQ("I am a string", val1);
    EXPECT_EQ("I am a \"confusing\" string", val2);

    std::string str = app.config_to_str();
    EXPECT_THAT(str, HasSubstr("val1=\"I am a string\""));
    EXPECT_THAT(str, HasSubstr("val2='I am a \"confusing\" string'"));
}

TEST_F(TApp, DefaultsIniQuotedOutput) {

    std::string val1{"I am a string"};
    app.add_option("--val1", val1, "", true);

    std::string val2{R"(I am a "confusing" string)"};
    app.add_option("--val2", val2, "", true);

    run();

    std::string str = app.config_to_str(true);
    EXPECT_THAT(str, HasSubstr("val1=\"I am a string\""));
    EXPECT_THAT(str, HasSubstr("val2='I am a \"confusing\" string'"));
}
