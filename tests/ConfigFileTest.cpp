#include "app_helper.hpp"

#include "gmock/gmock.h"
#include <cstdio>
#include <sstream>

using ::testing::HasSubstr;
using ::testing::Not;

TEST(StringBased, convert_arg_for_ini) {

    EXPECT_EQ(CLI::detail::convert_arg_for_ini(std::string{}), "\"\"");

    EXPECT_EQ(CLI::detail::convert_arg_for_ini("true"), "true");

    EXPECT_EQ(CLI::detail::convert_arg_for_ini("nan"), "nan");

    EXPECT_EQ(CLI::detail::convert_arg_for_ini("happy hippo"), "\"happy hippo\"");

    EXPECT_EQ(CLI::detail::convert_arg_for_ini("47"), "47");

    EXPECT_EQ(CLI::detail::convert_arg_for_ini("47.365225"), "47.365225");

    EXPECT_EQ(CLI::detail::convert_arg_for_ini("+3.28e-25"), "+3.28e-25");
    EXPECT_EQ(CLI::detail::convert_arg_for_ini("-22E14"), "-22E14");

    EXPECT_EQ(CLI::detail::convert_arg_for_ini("a"), "'a'");
    // hex
    EXPECT_EQ(CLI::detail::convert_arg_for_ini("0x5461FAED"), "0x5461FAED");
    // hex fail
    EXPECT_EQ(CLI::detail::convert_arg_for_ini("0x5461FAEG"), "\"0x5461FAEG\"");

    // octal
    EXPECT_EQ(CLI::detail::convert_arg_for_ini("0o546123567"), "0o546123567");
    // octal fail
    EXPECT_EQ(CLI::detail::convert_arg_for_ini("0o546123587"), "\"0o546123587\"");

    // binary
    EXPECT_EQ(CLI::detail::convert_arg_for_ini("0b01101110010"), "0b01101110010");
    // binary fail
    EXPECT_EQ(CLI::detail::convert_arg_for_ini("0b01102110010"), "\"0b01102110010\"");
}

TEST(StringBased, IniJoin) {
    std::vector<std::string> items = {"one", "two", "three four"};
    std::string result = "\"one\" \"two\" \"three four\"";

    EXPECT_EQ(CLI::detail::ini_join(items, ' ', '\0', '\0'), result);

    result = "[\"one\", \"two\", \"three four\"]";

    EXPECT_EQ(CLI::detail::ini_join(items), result);

    result = "{\"one\"; \"two\"; \"three four\"}";

    EXPECT_EQ(CLI::detail::ini_join(items, ';', '{', '}'), result);
}

TEST(StringBased, First) {
    std::stringstream ofile;

    ofile << "one=three\n";
    ofile << "two=four\n";

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

    ofile << ";this is a comment\n";
    ofile << "one=three\n";
    ofile << "two=four\n";
    ofile << "; and another one\n";

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

    ofile << R"(one = "three")" << '\n';
    ofile << R"(two = 'four')" << '\n';
    ofile << R"(five = "six and seven")" << '\n';

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

    ofile << "one = three\n";
    ofile << "two = four\n";
    ofile << "five = six and seven\n";

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

    ofile << "one = three\n";
    ofile << "two = four";

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

    ofile << "one=three\n";
    ofile << "[second]\n";
    ofile << "  two=four\n";

    ofile.seekg(0, std::ios::beg);

    std::vector<CLI::ConfigItem> output = CLI::ConfigINI().from_config(ofile);

    EXPECT_EQ(4u, output.size());
    EXPECT_EQ("one", output.at(0).name);
    EXPECT_EQ(1u, output.at(0).inputs.size());
    EXPECT_EQ("three", output.at(0).inputs.at(0));
    EXPECT_EQ("two", output.at(2).name);
    EXPECT_EQ("second", output.at(2).parents.at(0));
    EXPECT_EQ(1u, output.at(2).inputs.size());
    EXPECT_EQ("four", output.at(2).inputs.at(0));
    EXPECT_EQ("second.two", output.at(2).fullname());
}

TEST(StringBased, SpacesSections) {
    std::stringstream ofile;

    ofile << "one=three\n\n";
    ofile << "[second]   \n";
    ofile << "   \n";
    ofile << "  two=four\n";

    ofile.seekg(0, std::ios::beg);

    std::vector<CLI::ConfigItem> output = CLI::ConfigINI().from_config(ofile);

    EXPECT_EQ(4u, output.size());
    EXPECT_EQ("one", output.at(0).name);
    EXPECT_EQ(1u, output.at(0).inputs.size());
    EXPECT_EQ("three", output.at(0).inputs.at(0));
    EXPECT_EQ("second", output.at(1).parents.at(0));
    EXPECT_EQ("++", output.at(1).name);
    EXPECT_EQ("two", output.at(2).name);
    EXPECT_EQ(1u, output.at(2).parents.size());
    EXPECT_EQ("second", output.at(2).parents.at(0));
    EXPECT_EQ(1u, output.at(2).inputs.size());
    EXPECT_EQ("four", output.at(2).inputs.at(0));
    EXPECT_EQ("second", output.at(3).parents.at(0));
    EXPECT_EQ("--", output.at(3).name);
}

// check function to make sure that open sections match close sections
bool checkSections(const std::vector<CLI::ConfigItem> &output) {
    std::set<std::string> open;
    for(auto &ci : output) {
        if(ci.name == "++") {
            auto nm = ci.fullname();
            nm.pop_back();
            nm.pop_back();
            auto rv = open.insert(nm);
            if(!rv.second) {
                return false;
            }
        }
        if(ci.name == "--") {
            auto nm = ci.fullname();
            nm.pop_back();
            nm.pop_back();
            auto rv = open.erase(nm);
            if(rv != 1U) {
                return false;
            }
        }
    }
    return open.empty();
}
TEST(StringBased, Layers) {
    std::stringstream ofile;

    ofile << "simple = true\n\n";
    ofile << "[other]\n";
    ofile << "[other.sub2]\n";
    ofile << "[other.sub2.sub-level2]\n";
    ofile << "[other.sub2.sub-level2.sub-level3]\n";
    ofile << "absolute_newest = true\n";
    ofile.seekg(0, std::ios::beg);

    std::vector<CLI::ConfigItem> output = CLI::ConfigINI().from_config(ofile);

    // 2 flags and 4 openings and 4 closings
    EXPECT_EQ(10u, output.size());
    EXPECT_TRUE(checkSections(output));
}

TEST(StringBased, LayersSkip) {
    std::stringstream ofile;

    ofile << "simple = true\n\n";
    ofile << "[other.sub2]\n";
    ofile << "[other.sub2.sub-level2.sub-level3]\n";
    ofile << "absolute_newest = true\n";
    ofile.seekg(0, std::ios::beg);

    std::vector<CLI::ConfigItem> output = CLI::ConfigINI().from_config(ofile);

    // 2 flags and 4 openings and 4 closings
    EXPECT_EQ(10u, output.size());
    EXPECT_TRUE(checkSections(output));
}

TEST(StringBased, LayersSkipOrdered) {
    std::stringstream ofile;

    ofile << "simple = true\n\n";
    ofile << "[other.sub2.sub-level2.sub-level3]\n";
    ofile << "[other.sub2]\n";
    ofile << "absolute_newest = true\n";
    ofile.seekg(0, std::ios::beg);

    std::vector<CLI::ConfigItem> output = CLI::ConfigINI().from_config(ofile);

    // 2 flags and 4 openings and 4 closings
    EXPECT_EQ(12u, output.size());
    EXPECT_TRUE(checkSections(output));
}

TEST(StringBased, LayersChange) {
    std::stringstream ofile;

    ofile << "simple = true\n\n";
    ofile << "[other.sub2]\n";
    ofile << "[other.sub3]\n";
    ofile << "absolute_newest = true\n";
    ofile.seekg(0, std::ios::beg);

    std::vector<CLI::ConfigItem> output = CLI::ConfigINI().from_config(ofile);

    // 2 flags and 3 openings and 3 closings
    EXPECT_EQ(8u, output.size());
    EXPECT_TRUE(checkSections(output));
}

TEST(StringBased, Layers2LevelChange) {
    std::stringstream ofile;

    ofile << "simple = true\n\n";
    ofile << "[other.sub2.cmd]\n";
    ofile << "[other.sub3.cmd]\n";
    ofile << "absolute_newest = true\n";
    ofile.seekg(0, std::ios::beg);

    std::vector<CLI::ConfigItem> output = CLI::ConfigINI().from_config(ofile);

    // 2 flags and 5 openings and 5 closings
    EXPECT_EQ(12u, output.size());
    EXPECT_TRUE(checkSections(output));
}

TEST(StringBased, Layers3LevelChange) {
    std::stringstream ofile;

    ofile << "[other.sub2.subsub.cmd]\n";
    ofile << "[other.sub3.subsub.cmd]\n";
    ofile << "absolute_newest = true\n";
    ofile.seekg(0, std::ios::beg);

    std::vector<CLI::ConfigItem> output = CLI::ConfigINI().from_config(ofile);

    // 1 flags and 7 openings and 7 closings
    EXPECT_EQ(15u, output.size());
    EXPECT_TRUE(checkSections(output));
}

TEST(StringBased, newSegment) {
    std::stringstream ofile;

    ofile << "[other.sub2.subsub.cmd]\n";
    ofile << "flag = true\n";
    ofile << "[another]\n";
    ofile << "absolute_newest = true\n";
    ofile.seekg(0, std::ios::beg);

    std::vector<CLI::ConfigItem> output = CLI::ConfigINI().from_config(ofile);

    // 2 flags and 5 openings and 5 closings
    EXPECT_EQ(12u, output.size());
    EXPECT_TRUE(checkSections(output));
}

TEST(StringBased, LayersDirect) {
    std::stringstream ofile;

    ofile << "simple = true\n\n";
    ofile << "[other.sub2.sub-level2.sub-level3]\n";
    ofile << "absolute_newest = true\n";

    ofile.seekg(0, std::ios::beg);

    std::vector<CLI::ConfigItem> output = CLI::ConfigINI().from_config(ofile);

    // 2 flags and 4 openings and 4 closings
    EXPECT_EQ(10u, output.size());
    EXPECT_TRUE(checkSections(output));
}

TEST(StringBased, LayersComplex) {
    std::stringstream ofile;

    ofile << "simple = true\n\n";
    ofile << "[other.sub2.sub-level2.sub-level3]\n";
    ofile << "absolute_newest = true\n";
    ofile << "[other.sub2.sub-level2]\n";
    ofile << "still_newer = true\n";
    ofile << "[other.sub2]\n";
    ofile << "newest = true\n";

    ofile.seekg(0, std::ios::beg);

    std::vector<CLI::ConfigItem> output = CLI::ConfigINI().from_config(ofile);

    // 4 flags and 6 openings and 6 closings
    EXPECT_EQ(16u, output.size());
    EXPECT_TRUE(checkSections(output));
}

TEST(StringBased, file_error) {
    EXPECT_THROW(std::vector<CLI::ConfigItem> output = CLI::ConfigINI().from_file("nonexist_file"), CLI::FileError);
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
    EXPECT_EQ(app["--config"]->as<std::string>(), "TestIniTmp.ini");
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

    int two{0};
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

    int two{0};
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

    int two{0};
    app.add_option("--two", two);
    ASSERT_NO_THROW(run());
    EXPECT_EQ(app.remaining().size(), 0u);
}

TEST_F(TApp, IniRequiredNoDefault) {

    app.set_config("--config")->required();

    int two{0};
    app.add_option("--two", two);
    ASSERT_THROW(run(), CLI::FileError);
}

TEST_F(TApp, IniNotRequiredNoDefault) {

    app.set_config("--config");

    int two{0};
    app.add_option("--two", two);
    ASSERT_NO_THROW(run());
}

/// Define a class for testing purposes that does bad things
class EvilConfig : public CLI::Config {
  public:
    EvilConfig() = default;
    virtual std::string to_config(const CLI::App *, bool, bool, std::string) const { throw CLI::FileError("evil"); }

    virtual std::vector<CLI::ConfigItem> from_config(std::istream &) const { throw CLI::FileError("evil"); }
};

TEST_F(TApp, IniRequiredbadConfigurator) {

    TempFile tmpini{"TestIniTmp.ini"};

    {
        std::ofstream out{tmpini};
        out << "[default]" << std::endl;
        out << "two=99" << std::endl;
        out << "three=3" << std::endl;
    }

    app.set_config("--config", tmpini)->required();
    app.config_formatter(std::make_shared<EvilConfig>());
    int two{0};
    app.add_option("--two", two);
    ASSERT_THROW(run(), CLI::FileError);
}

TEST_F(TApp, IniNotRequiredbadConfigurator) {

    TempFile tmpini{"TestIniTmp.ini"};

    {
        std::ofstream out{tmpini};
        out << "[default]" << std::endl;
        out << "two=99" << std::endl;
        out << "three=3" << std::endl;
    }

    app.set_config("--config", tmpini);
    app.config_formatter(std::make_shared<EvilConfig>());
    int two{0};
    app.add_option("--two", two);
    ASSERT_NO_THROW(run());
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

    int one{0}, two{0}, three{0};
    app.add_option("--one", one);
    app.add_option("--two", two);
    app.add_option("--three", three);

    run();
    EXPECT_EQ(app["--config"]->as<std::string>(), tmpini.c_str());
    EXPECT_EQ(99, two);
    EXPECT_EQ(3, three);

    args = {"--config", tmpini2};
    run();

    EXPECT_EQ(98, two);
    EXPECT_EQ(4, three);
    EXPECT_EQ(app.get_config_ptr()->as<std::string>(), tmpini2.c_str());
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
    int two{7};
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

    int one{0}, two{0}, three{0};
    app.add_option("--one", one)->required();
    app.add_option("--two", two)->required();
    app.add_option("--three", three)->required();

    args = {"--one=1"};

    run();
    EXPECT_EQ(one, 1);
    EXPECT_EQ(two, 99);
    EXPECT_EQ(three, 3);

    one = two = three = 0;
    args = {"--one=1", "--two=2"};

    EXPECT_NO_THROW(run());
    EXPECT_EQ(one, 1);
    EXPECT_EQ(two, 2);
    EXPECT_EQ(three, 3);

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
TEST_F(TApp, TOMLVector) {

    TempFile tmpini{"TestIniTmp.ini"};

    app.set_config("--config", tmpini);

    {
        std::ofstream out{tmpini};
        out << "#this is a comment line\n";
        out << "[default]\n";
        out << "two=[2,3]\n";
        out << "three=[1,2,3]\n";
    }

    std::vector<int> two, three;
    app.add_option("--two", two)->expected(2)->required();
    app.add_option("--three", three)->required();

    run();

    EXPECT_EQ(std::vector<int>({2, 3}), two);
    EXPECT_EQ(std::vector<int>({1, 2, 3}), three);
}

TEST_F(TApp, ColonValueSep) {

    TempFile tmpini{"TestIniTmp.ini"};

    app.set_config("--config", tmpini);

    {
        std::ofstream out{tmpini};
        out << "#this is a comment line\n";
        out << "[default]\n";
        out << "two:2\n";
        out << "three:3\n";
    }

    int two{0}, three{0};
    app.add_option("--two", two);
    app.add_option("--three", three);

    app.get_config_formatter_base()->valueSeparator(':');

    run();

    EXPECT_EQ(2, two);
    EXPECT_EQ(3, three);
}

TEST_F(TApp, TOMLVectordirect) {

    TempFile tmpini{"TestIniTmp.ini"};

    app.set_config("--config", tmpini);

    app.config_formatter(std::make_shared<CLI::ConfigTOML>());

    {
        std::ofstream out{tmpini};
        out << "#this is a comment line\n";
        out << "[default]\n";
        out << "two=[2,3]\n";
        out << "three=[1,2,3]\n";
    }

    std::vector<int> two, three;
    app.add_option("--two", two)->expected(2)->required();
    app.add_option("--three", three)->required();

    run();

    EXPECT_EQ(std::vector<int>({2, 3}), two);
    EXPECT_EQ(std::vector<int>({1, 2, 3}), three);
}

TEST_F(TApp, IniVectorCsep) {

    TempFile tmpini{"TestIniTmp.ini"};

    app.set_config("--config", tmpini);

    {
        std::ofstream out{tmpini};
        out << "#this is a comment line\n";
        out << "[default]\n";
        out << "two=[2,3]\n";
        out << "three=1,2,3\n";
    }

    std::vector<int> two, three;
    app.add_option("--two", two)->expected(2)->required();
    app.add_option("--three", three)->required();

    run();

    EXPECT_EQ(std::vector<int>({2, 3}), two);
    EXPECT_EQ(std::vector<int>({1, 2, 3}), three);
}

TEST_F(TApp, IniVectorMultiple) {

    TempFile tmpini{"TestIniTmp.ini"};

    app.set_config("--config", tmpini);

    {
        std::ofstream out{tmpini};
        out << "#this is a comment line\n";
        out << "[default]\n";
        out << "two=2\n";
        out << "two=3\n";
        out << "three=1\n";
        out << "three=2\n";
        out << "three=3\n";
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

    int one{0}, two{0}, three{0};
    app.add_option("--val", one);
    auto subcom = app.add_subcommand("subcom");
    subcom->add_option("--val", two);
    auto subsubcom = subcom->add_subcommand("subsubcom");
    subsubcom->add_option("--val", three);

    run();

    EXPECT_EQ(1, one);
    EXPECT_EQ(2, two);
    EXPECT_EQ(3, three);

    EXPECT_EQ(subcom->count(), 0U);
    EXPECT_FALSE(*subcom);
}

TEST_F(TApp, IniLayeredDotSection) {

    TempFile tmpini{"TestIniTmp.ini"};

    app.set_config("--config", tmpini);

    {
        std::ofstream out{tmpini};
        out << "[default]" << std::endl;
        out << "val=1" << std::endl;
        out << "[subcom]" << std::endl;
        out << "val=2" << std::endl;
        out << "[subcom.subsubcom]" << std::endl;
        out << "val=3" << std::endl;
    }

    int one{0}, two{0}, three{0};
    app.add_option("--val", one);
    auto subcom = app.add_subcommand("subcom");
    subcom->add_option("--val", two);
    auto subsubcom = subcom->add_subcommand("subsubcom");
    subsubcom->add_option("--val", three);

    run();

    EXPECT_EQ(1, one);
    EXPECT_EQ(2, two);
    EXPECT_EQ(3, three);

    EXPECT_EQ(subcom->count(), 0U);
    EXPECT_FALSE(*subcom);
}

TEST_F(TApp, IniSubcommandConfigurable) {

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

    int one{0}, two{0}, three{0};
    app.add_option("--val", one);
    auto subcom = app.add_subcommand("subcom");
    subcom->configurable();
    subcom->add_option("--val", two);
    auto subsubcom = subcom->add_subcommand("subsubcom");
    subsubcom->add_option("--val", three);

    run();

    EXPECT_EQ(1, one);
    EXPECT_EQ(2, two);
    EXPECT_EQ(3, three);

    EXPECT_EQ(subcom->count(), 1U);
    EXPECT_TRUE(*subcom);
    EXPECT_TRUE(app.got_subcommand(subcom));
}

TEST_F(TApp, IniSubcommandConfigurablePreParse) {

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

    int one{0}, two{0}, three{0}, four{0};
    app.add_option("--val", one);
    auto subcom = app.add_subcommand("subcom");
    auto subcom2 = app.add_subcommand("subcom2");
    subcom->configurable();
    std::vector<std::size_t> parse_c;
    subcom->preparse_callback([&parse_c](std::size_t cnt) { parse_c.push_back(cnt); });
    subcom->add_option("--val", two);
    subcom2->add_option("--val", four);
    subcom2->preparse_callback([&parse_c](std::size_t cnt) { parse_c.push_back(cnt + 2623); });
    auto subsubcom = subcom->add_subcommand("subsubcom");
    subsubcom->add_option("--val", three);

    run();

    EXPECT_EQ(1, one);
    EXPECT_EQ(2, two);
    EXPECT_EQ(3, three);
    EXPECT_EQ(0, four);

    EXPECT_EQ(parse_c.size(), 1U);
    EXPECT_EQ(parse_c[0], 2U);

    EXPECT_EQ(subcom2->count(), 0U);
}

TEST_F(TApp, IniSubcommandConfigurableParseComplete) {

    TempFile tmpini{"TestIniTmp.ini"};

    app.set_config("--config", tmpini);

    {
        std::ofstream out{tmpini};
        out << "[default]" << std::endl;
        out << "val=1" << std::endl;
        out << "[subcom]" << std::endl;
        out << "val=2" << std::endl;
        out << "[subcom.subsubcom]" << std::endl;
        out << "val=3" << std::endl;
    }

    int one{0}, two{0}, three{0}, four{0};
    app.add_option("--val", one);
    auto subcom = app.add_subcommand("subcom");
    auto subcom2 = app.add_subcommand("subcom2");
    subcom->configurable();
    std::vector<std::size_t> parse_c;
    subcom->parse_complete_callback([&parse_c]() { parse_c.push_back(58); });
    subcom->add_option("--val", two);
    subcom2->add_option("--val", four);
    subcom2->parse_complete_callback([&parse_c]() { parse_c.push_back(2623); });
    auto subsubcom = subcom->add_subcommand("subsubcom");
    // configurable should be inherited
    subsubcom->parse_complete_callback([&parse_c]() { parse_c.push_back(68); });
    subsubcom->add_option("--val", three);

    run();

    EXPECT_EQ(1, one);
    EXPECT_EQ(2, two);
    EXPECT_EQ(3, three);
    EXPECT_EQ(0, four);

    ASSERT_EQ(parse_c.size(), 2u);
    EXPECT_EQ(parse_c[0], 68U);
    EXPECT_EQ(parse_c[1], 58U);
    EXPECT_EQ(subsubcom->count(), 1u);
    EXPECT_EQ(subcom2->count(), 0u);
}

TEST_F(TApp, IniSubcommandMultipleSections) {

    TempFile tmpini{"TestIniTmp.ini"};

    app.set_config("--config", tmpini);

    {
        std::ofstream out{tmpini};
        out << "[default]" << std::endl;
        out << "val=1" << std::endl;
        out << "[subcom]" << std::endl;
        out << "val=2" << std::endl;
        out << "[subcom.subsubcom]" << std::endl;
        out << "val=3" << std::endl;
        out << "[subcom2]" << std::endl;
        out << "val=4" << std::endl;
    }

    int one{0}, two{0}, three{0}, four{0};
    app.add_option("--val", one);
    auto subcom = app.add_subcommand("subcom");
    auto subcom2 = app.add_subcommand("subcom2");
    subcom->configurable();
    std::vector<std::size_t> parse_c;
    subcom->parse_complete_callback([&parse_c]() { parse_c.push_back(58); });
    subcom->add_option("--val", two);
    subcom2->add_option("--val", four);
    subcom2->parse_complete_callback([&parse_c]() { parse_c.push_back(2623); });
    subcom2->configurable(false);
    auto subsubcom = subcom->add_subcommand("subsubcom");
    // configurable should be inherited
    subsubcom->parse_complete_callback([&parse_c]() { parse_c.push_back(68); });
    subsubcom->add_option("--val", three);

    run();

    EXPECT_EQ(1, one);
    EXPECT_EQ(2, two);
    EXPECT_EQ(3, three);
    EXPECT_EQ(4, four);

    ASSERT_EQ(parse_c.size(), 2u);
    EXPECT_EQ(parse_c[0], 68U);
    EXPECT_EQ(parse_c[1], 58U);
    EXPECT_EQ(subsubcom->count(), 1u);
    EXPECT_EQ(subcom2->count(), 0u);  // not configurable but value is updated
}

TEST_F(TApp, DuplicateSubcommandCallbacks) {

    TempFile tmpini{"TestIniTmp.ini"};

    app.set_config("--config", tmpini);

    {
        std::ofstream out{tmpini};
        out << "[[foo]]" << std::endl;
        out << "[[foo]]" << std::endl;
        out << "[[foo]]" << std::endl;
    }

    auto foo = app.add_subcommand("foo");
    int count{0};
    foo->callback([&count]() { ++count; });
    foo->immediate_callback();
    EXPECT_TRUE(foo->get_immediate_callback());
    foo->configurable();

    run();
    EXPECT_EQ(count, 3);
}

TEST_F(TApp, IniFailure) {

    TempFile tmpini{"TestIniTmp.ini"};

    app.set_config("--config", tmpini);
    app.allow_config_extras(false);
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
    bool value{false};
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
    bool value{false};
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
    app.allow_config_extras(false);
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
    app.allow_config_extras(CLI::config_extras_mode::error);
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
    bool result{false};
    auto *opt = app.get_option("--flag");
    EXPECT_THROW(opt->results(result), CLI::ConversionError);
    std::string res;
    opt->results(res);
    EXPECT_EQ(res, "moobook");
}

TEST_F(TApp, IniFlagNumbers) {

    TempFile tmpini{"TestIniTmp.ini"};

    bool boo{false};
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

    bool boo{false};
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

    bool flag1{false}, flag2{false}, flag3{false}, flag4{false};
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

    int two{0};
    bool three{false}, four{false}, five{false};
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

    int two{0};
    bool three{false}, four{false}, five{false};
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

    int two{0};
    bool three{false}, four{false}, five{false};
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

    int two{0};
    bool four{false}, five{false};
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

    int two{0}, four{0}, val{0};
    app.add_flag("--two{2}", two)->disable_flag_override();
    app.add_flag("--four{4}", four)->disable_flag_override();
    app.add_flag("--val", val);

    run();

    EXPECT_EQ(2, two);
    EXPECT_EQ(4, four);
    EXPECT_EQ(15, val);
}

TEST_F(TApp, IniOutputSimple) {

    int v{0};
    app.add_option("--simple", v);

    args = {"--simple=3"};

    run();

    std::string str = app.config_to_str();
    EXPECT_EQ("simple=3\n", str);
}

TEST_F(TApp, IniOutputNoConfigurable) {

    int v1{0}, v2{0};
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
    EXPECT_THAT(
        str, HasSubstr("; " + description1 + "\n" + flag1 + "=false\n\n; " + description2 + "\n" + flag2 + "=false\n"));
}

TEST_F(TApp, IniOutputGroups) {
    std::string flag1 = "flagnr1";
    std::string flag2 = "flagnr2";
    const std::string description1 = "First description.";
    const std::string description2 = "Second description.";
    app.add_flag("--" + flag1, description1)->group("group1");
    app.add_flag("--" + flag2, description2)->group("group2");

    run();

    std::string str = app.config_to_str(true, true);
    EXPECT_THAT(str, HasSubstr("group1"));
    EXPECT_THAT(str, HasSubstr("group2"));
}

TEST_F(TApp, IniOutputHiddenOptions) {
    std::string flag1 = "flagnr1";
    std::string flag2 = "flagnr2";
    double val{12.7};
    const std::string description1 = "First description.";
    const std::string description2 = "Second description.";
    app.add_flag("--" + flag1, description1)->group("group1");
    app.add_flag("--" + flag2, description2)->group("group2");
    app.add_option("--dval", val, "", true)->group("");

    run();

    std::string str = app.config_to_str(true, true);
    EXPECT_THAT(str, HasSubstr("group1"));
    EXPECT_THAT(str, HasSubstr("group2"));
    EXPECT_THAT(str, HasSubstr("dval=12.7"));
    auto loc = str.find("dval=12.7");
    auto locg1 = str.find("group1");
    EXPECT_GT(locg1, loc);
    // make sure it doesn't come twice
    loc = str.find("dval=12.7", loc + 4);
    EXPECT_EQ(loc, std::string::npos);
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

TEST_F(TApp, IniOutputOptionGroup) {
    std::string flag1 = "flagnr1";
    std::string flag2 = "flagnr2";
    double val{12.7};
    const std::string description1 = "First description.";
    const std::string description2 = "Second description.";
    app.add_flag("--" + flag1, description1)->group("group1");
    app.add_flag("--" + flag2, description2)->group("group2");
    auto og = app.add_option_group("group3", "g3 desc");
    og->add_option("--dval", val, "", true)->group("");

    run();

    std::string str = app.config_to_str(true, true);
    EXPECT_THAT(str, HasSubstr("group1"));
    EXPECT_THAT(str, HasSubstr("group2"));
    EXPECT_THAT(str, HasSubstr("dval=12.7"));
    EXPECT_THAT(str, HasSubstr("group3"));
    EXPECT_THAT(str, HasSubstr("g3 desc"));
    auto loc = str.find("dval=12.7");
    auto locg1 = str.find("group1");
    auto locg3 = str.find("group3");
    EXPECT_LT(locg1, loc);
    // make sure it doesn't come twice
    loc = str.find("dval=12.7", loc + 4);
    EXPECT_EQ(loc, std::string::npos);
    EXPECT_GT(locg3, locg1);
}

TEST_F(TApp, IniOutputVector) {

    std::vector<int> v;
    app.add_option("--vector", v);

    args = {"--vector", "1", "2", "3"};

    run();

    std::string str = app.config_to_str();
    EXPECT_EQ("vector=1 2 3\n", str);
}

TEST_F(TApp, IniOutputVectorTOML) {

    std::vector<int> v;
    app.add_option("--vector", v);
    app.config_formatter(std::make_shared<CLI::ConfigTOML>());
    args = {"--vector", "1", "2", "3"};

    run();

    std::string str = app.config_to_str();
    EXPECT_EQ("vector=[1, 2, 3]\n", str);
}

TEST_F(TApp, IniOutputVectorCustom) {

    std::vector<int> v;
    app.add_option("--vector", v);
    auto V = std::make_shared<CLI::ConfigBase>();
    V->arrayBounds('{', '}')->arrayDelimiter(';')->valueSeparator(':');
    app.config_formatter(V);
    args = {"--vector", "1", "2", "3"};

    run();

    std::string str = app.config_to_str();
    EXPECT_EQ("vector:{1; 2; 3}\n", str);
}

TEST_F(TApp, IniOutputFlag) {

    int v{0}, q{0};
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
    EXPECT_THAT(str, HasSubstr("something=true true"));

    str = app.config_to_str(true);
    EXPECT_THAT(str, HasSubstr("nothing"));
}

TEST_F(TApp, IniOutputSet) {

    int v{0};
    app.add_option("--simple", v)->check(CLI::IsMember({1, 2, 3}));

    args = {"--simple=2"};

    run();

    std::string str = app.config_to_str();
    EXPECT_THAT(str, HasSubstr("simple=2"));
}

TEST_F(TApp, IniOutputDefault) {

    int v{7};
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

TEST_F(TApp, IniOutputSubcomConfigurable) {

    app.add_flag("--simple");
    auto subcom = app.add_subcommand("other")->configurable();
    subcom->add_flag("--newer");

    args = {"--simple", "other", "--newer"};
    run();

    std::string str = app.config_to_str();
    EXPECT_THAT(str, HasSubstr("simple=true"));
    EXPECT_THAT(str, HasSubstr("[other]"));
    EXPECT_THAT(str, HasSubstr("newer=true"));
    EXPECT_EQ(str.find("other.newer=true"), std::string::npos);
}

TEST_F(TApp, IniOutputSubsubcom) {

    app.add_flag("--simple");
    auto subcom = app.add_subcommand("other");
    subcom->add_flag("--newer");
    auto subsubcom = subcom->add_subcommand("sub2");
    subsubcom->add_flag("--newest");

    args = {"--simple", "other", "--newer", "sub2", "--newest"};
    run();

    std::string str = app.config_to_str();
    EXPECT_THAT(str, HasSubstr("simple=true"));
    EXPECT_THAT(str, HasSubstr("other.newer=true"));
    EXPECT_THAT(str, HasSubstr("other.sub2.newest=true"));
}

TEST_F(TApp, IniOutputSubsubcomConfigurable) {

    app.add_flag("--simple");
    auto subcom = app.add_subcommand("other")->configurable();
    subcom->add_flag("--newer");

    auto subsubcom = subcom->add_subcommand("sub2");
    subsubcom->add_flag("--newest");

    args = {"--simple", "other", "--newer", "sub2", "--newest"};
    run();

    std::string str = app.config_to_str();
    EXPECT_THAT(str, HasSubstr("simple=true"));
    EXPECT_THAT(str, HasSubstr("[other]"));
    EXPECT_THAT(str, HasSubstr("newer=true"));
    EXPECT_THAT(str, HasSubstr("[other.sub2]"));
    EXPECT_THAT(str, HasSubstr("newest=true"));
    EXPECT_EQ(str.find("sub2.newest=true"), std::string::npos);
}

TEST_F(TApp, IniOutputSubsubcomConfigurableDeep) {

    app.add_flag("--simple");
    auto subcom = app.add_subcommand("other")->configurable();
    subcom->add_flag("--newer");

    auto subsubcom = subcom->add_subcommand("sub2");
    subsubcom->add_flag("--newest");
    auto sssscom = subsubcom->add_subcommand("sub-level2");
    subsubcom->add_flag("--still_newer");
    auto s5com = sssscom->add_subcommand("sub-level3");
    s5com->add_flag("--absolute_newest");

    args = {"--simple", "other", "sub2", "sub-level2", "sub-level3", "--absolute_newest"};
    run();

    std::string str = app.config_to_str();
    EXPECT_THAT(str, HasSubstr("simple=true"));
    EXPECT_THAT(str, HasSubstr("[other.sub2.sub-level2.sub-level3]"));
    EXPECT_THAT(str, HasSubstr("absolute_newest=true"));
    EXPECT_EQ(str.find(".absolute_newest=true"), std::string::npos);
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

// #298
TEST_F(TApp, StopReadingConfigOnClear) {

    TempFile tmpini{"TestIniTmp.ini"};

    app.set_config("--config", tmpini);
    auto ptr = app.set_config();  // Should *not* read config file
    EXPECT_EQ(ptr, nullptr);

    {
        std::ofstream out{tmpini};
        out << "volume=1" << std::endl;
    }

    int volume{0};
    app.add_option("--volume", volume, "volume1");

    run();

    EXPECT_EQ(volume, 0);
}

TEST_F(TApp, ConfigWriteReadWrite) {

    TempFile tmpini{"TestIniTmp.ini"};

    app.add_flag("--flag");
    run();

    // Save config, with default values too
    std::string config1 = app.config_to_str(true, true);
    {
        std::ofstream out{tmpini};
        out << config1 << std::endl;
    }

    app.set_config("--config", tmpini, "Read an ini file", true);
    run();

    std::string config2 = app.config_to_str(true, true);

    EXPECT_EQ(config1, config2);
}
