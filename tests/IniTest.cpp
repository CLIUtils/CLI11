#include "app_helper.hpp"

#include <cstdio>
#include <sstream>
#include "gmock/gmock.h"

using ::testing::HasSubstr;
using ::testing::Not;

TEST(StringBased, First) {
    std::stringstream ofile;

    ofile << "one=three" << std::endl;
    ofile << "two=four" << std::endl;

    ofile.seekg(0, std::ios::beg);

    std::vector<CLI::detail::ini_ret_t> output = CLI::detail::parse_ini(ofile);

    EXPECT_EQ((size_t)2, output.size());
    EXPECT_EQ("one", output.at(0).name());
    EXPECT_EQ((size_t)1, output.at(0).inputs.size());
    EXPECT_EQ("three", output.at(0).inputs.at(0));
    EXPECT_EQ("two", output.at(1).name());
    EXPECT_EQ((size_t)1, output.at(1).inputs.size());
    EXPECT_EQ("four", output.at(1).inputs.at(0));
}

TEST(StringBased, FirstWithComments) {
    std::stringstream ofile;

    ofile << ";this is a comment" << std::endl;
    ofile << "one=three" << std::endl;
    ofile << "two=four" << std::endl;
    ofile << "; and another one" << std::endl;

    ofile.seekg(0, std::ios::beg);

    auto output = CLI::detail::parse_ini(ofile);

    EXPECT_EQ((size_t)2, output.size());
    EXPECT_EQ("one", output.at(0).name());
    EXPECT_EQ((size_t)1, output.at(0).inputs.size());
    EXPECT_EQ("three", output.at(0).inputs.at(0));
    EXPECT_EQ("two", output.at(1).name());
    EXPECT_EQ((size_t)1, output.at(1).inputs.size());
    EXPECT_EQ("four", output.at(1).inputs.at(0));
}

TEST(StringBased, Quotes) {
    std::stringstream ofile;

    ofile << R"(one = "three")" << std::endl;
    ofile << R"(two = 'four')" << std::endl;
    ofile << R"(five = "six and seven")" << std::endl;

    ofile.seekg(0, std::ios::beg);

    auto output = CLI::detail::parse_ini(ofile);

    EXPECT_EQ((size_t)3, output.size());
    EXPECT_EQ("one", output.at(0).name());
    EXPECT_EQ((size_t)1, output.at(0).inputs.size());
    EXPECT_EQ("three", output.at(0).inputs.at(0));
    EXPECT_EQ("two", output.at(1).name());
    EXPECT_EQ((size_t)1, output.at(1).inputs.size());
    EXPECT_EQ("four", output.at(1).inputs.at(0));
    EXPECT_EQ("five", output.at(2).name());
    EXPECT_EQ((size_t)1, output.at(2).inputs.size());
    EXPECT_EQ("six and seven", output.at(2).inputs.at(0));
}

TEST(StringBased, Vector) {
    std::stringstream ofile;

    ofile << "one = three" << std::endl;
    ofile << "two = four" << std::endl;
    ofile << "five = six and seven" << std::endl;

    ofile.seekg(0, std::ios::beg);

    auto output = CLI::detail::parse_ini(ofile);

    EXPECT_EQ((size_t)3, output.size());
    EXPECT_EQ("one", output.at(0).name());
    EXPECT_EQ((size_t)1, output.at(0).inputs.size());
    EXPECT_EQ("three", output.at(0).inputs.at(0));
    EXPECT_EQ("two", output.at(1).name());
    EXPECT_EQ((size_t)1, output.at(1).inputs.size());
    EXPECT_EQ("four", output.at(1).inputs.at(0));
    EXPECT_EQ("five", output.at(2).name());
    EXPECT_EQ((size_t)3, output.at(2).inputs.size());
    EXPECT_EQ("six", output.at(2).inputs.at(0));
    EXPECT_EQ("and", output.at(2).inputs.at(1));
    EXPECT_EQ("seven", output.at(2).inputs.at(2));
}

TEST(StringBased, Spaces) {
    std::stringstream ofile;

    ofile << "one = three" << std::endl;
    ofile << "two = four" << std::endl;

    ofile.seekg(0, std::ios::beg);

    auto output = CLI::detail::parse_ini(ofile);

    EXPECT_EQ((size_t)2, output.size());
    EXPECT_EQ("one", output.at(0).name());
    EXPECT_EQ((size_t)1, output.at(0).inputs.size());
    EXPECT_EQ("three", output.at(0).inputs.at(0));
    EXPECT_EQ("two", output.at(1).name());
    EXPECT_EQ((size_t)1, output.at(1).inputs.size());
    EXPECT_EQ("four", output.at(1).inputs.at(0));
}

TEST(StringBased, Sections) {
    std::stringstream ofile;

    ofile << "one=three" << std::endl;
    ofile << "[second]" << std::endl;
    ofile << "  two=four" << std::endl;

    ofile.seekg(0, std::ios::beg);

    auto output = CLI::detail::parse_ini(ofile);

    EXPECT_EQ((size_t)2, output.size());
    EXPECT_EQ("one", output.at(0).name());
    EXPECT_EQ((size_t)1, output.at(0).inputs.size());
    EXPECT_EQ("three", output.at(0).inputs.at(0));
    EXPECT_EQ("two", output.at(1).name());
    EXPECT_EQ("second", output.at(1).parent());
    EXPECT_EQ((size_t)1, output.at(1).inputs.size());
    EXPECT_EQ("four", output.at(1).inputs.at(0));
}

TEST(StringBased, SpacesSections) {
    std::stringstream ofile;

    ofile << "one=three" << std::endl;
    ofile << std::endl;
    ofile << "[second]" << std::endl;
    ofile << "   " << std::endl;
    ofile << "  two=four" << std::endl;

    ofile.seekg(0, std::ios::beg);

    auto output = CLI::detail::parse_ini(ofile);

    EXPECT_EQ((size_t)2, output.size());
    EXPECT_EQ("one", output.at(0).name());
    EXPECT_EQ((size_t)1, output.at(0).inputs.size());
    EXPECT_EQ("three", output.at(0).inputs.at(0));
    EXPECT_EQ("two", output.at(1).name());
    EXPECT_EQ("second", output.at(1).parent());
    EXPECT_EQ((size_t)1, output.at(1).inputs.size());
    EXPECT_EQ("four", output.at(1).inputs.at(0));
}

TEST_F(TApp, IniNotRequired) {

    TempFile tmpini{"TestIniTmp.ini"};

    app.add_config("--config", tmpini);

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

    app.reset();
    one = two = three = 0;
    args = {"--one=1", "--two=2"};

    run();

    EXPECT_EQ(1, one);
    EXPECT_EQ(2, two);
    EXPECT_EQ(3, three);
}

TEST_F(TApp, IniRequiredNotFound) {

    std::string noini = "TestIniNotExist.ini";
    app.add_config("--config", noini, "", true);

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
    app.add_config("--config", orig);
    // Make sure this can be overwritten
    app.add_config("--conf", next);
    int two = 7;
    app.add_option("--two", two);

    run();

    EXPECT_EQ(99, two);
}

TEST_F(TApp, IniRequired) {

    TempFile tmpini{"TestIniTmp.ini"};

    app.add_config("--config", tmpini, "", true);

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

    app.reset();
    one = two = three = 0;
    args = {"--one=1", "--two=2"};

    run();

    app.reset();
    args = {};

    EXPECT_THROW(run(), CLI::RequiredError);

    app.reset();
    args = {"--two=2"};

    EXPECT_THROW(run(), CLI::RequiredError);
}

TEST_F(TApp, IniVector) {

    TempFile tmpini{"TestIniTmp.ini"};

    app.add_config("--config", tmpini);

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

    app.add_config("--config", tmpini);

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

    ASSERT_NO_THROW(run());

    EXPECT_EQ(1, one);
    EXPECT_EQ(2, two);
    EXPECT_EQ(3, three);
}

TEST_F(TApp, IniFailure) {

    TempFile tmpini{"TestIniTmp.ini"};

    app.add_config("--config", tmpini);

    {
        std::ofstream out{tmpini};
        out << "[default]" << std::endl;
        out << "val=1" << std::endl;
    }

    EXPECT_THROW(run(), CLI::ExtrasINIError);
}

TEST_F(TApp, IniSubFailure) {

    TempFile tmpini{"TestIniTmp.ini"};

    app.add_subcommand("other");
    app.add_config("--config", tmpini);

    {
        std::ofstream out{tmpini};
        out << "[other]" << std::endl;
        out << "val=1" << std::endl;
    }

    EXPECT_THROW(run(), CLI::ExtrasINIError);
}

TEST_F(TApp, IniNoSubFailure) {

    TempFile tmpini{"TestIniTmp.ini"};

    app.add_config("--config", tmpini);

    {
        std::ofstream out{tmpini};
        out << "[other]" << std::endl;
        out << "val=1" << std::endl;
    }

    EXPECT_THROW(run(), CLI::ExtrasINIError);
}

TEST_F(TApp, IniFlagConvertFailure) {

    TempFile tmpini{"TestIniTmp.ini"};

    app.add_flag("--flag");
    app.add_config("--config", tmpini);

    {
        std::ofstream out{tmpini};
        out << "flag=moobook" << std::endl;
    }

    EXPECT_THROW(run(), CLI::ConversionError);
}

TEST_F(TApp, IniFlagNumbers) {

    TempFile tmpini{"TestIniTmp.ini"};

    bool boo;
    app.add_flag("--flag", boo);
    app.add_config("--config", tmpini);

    {
        std::ofstream out{tmpini};
        out << "flag=3" << std::endl;
    }

    EXPECT_THROW(run(), CLI::ConversionError);
}

TEST_F(TApp, IniFlagDual) {

    TempFile tmpini{"TestIniTmp.ini"};

    bool boo;
    app.add_flag("--flag", boo);
    app.add_config("--config", tmpini);

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
    app.add_config("--config", tmpini);

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
    app.add_config("--config", tmpini);

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
    EXPECT_EQ(true, three);
    EXPECT_EQ(true, four);
    EXPECT_EQ(true, five);
}

TEST_F(TApp, IniOutputSimple) {

    int v;
    app.add_option("--simple", v);

    args = {"--simple=3"};

    run();

    std::string str = app.config_to_str();
    EXPECT_EQ("simple=3\n", str);
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
    app.add_set("--simple", v, {1, 2, 3});

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
