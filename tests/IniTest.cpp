#include "app_helper.hpp"

#include <cstdio>
#include <sstream>
#include "gmock/gmock.h"
 
using ::testing::HasSubstr;

TEST(StringBased, First) {
    std::stringstream ofile;

    ofile << "one=three" << std::endl;
    ofile << "two=four" << std::endl;

    ofile.seekg(0, std::ios::beg);

    std::vector<std::string> output = CLI::detail::parse_ini(ofile);

    std::vector<std::string> answer = {"--one=three", "--two=four"};

    EXPECT_EQ(answer, output);
    
}


TEST(StringBased, Sections) {
    std::stringstream ofile;

    ofile << "one=three" << std::endl;
    ofile << "[second]" << std::endl;
    ofile << "  two=four" << std::endl;

    ofile.seekg(0, std::ios::beg);

    std::vector<std::string> output = CLI::detail::parse_ini(ofile);

    std::vector<std::string> answer = {"--one=three", "--second.two=four"};

    EXPECT_EQ(answer, output);
    
}


TEST(StringBased, SpacesSections) {
    std::stringstream ofile;

    ofile << "one=three" << std::endl;
    ofile << std::endl;
    ofile << "[second]" << std::endl;
    ofile << "   " << std::endl;
    ofile << "  two=four" << std::endl;

    ofile.seekg(0, std::ios::beg);

    std::vector<std::string> output = CLI::detail::parse_ini(ofile);

    std::vector<std::string> answer = {"--one=three", "--second.two=four"};

    EXPECT_EQ(answer, output);
    
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

    int one=0, two=0, three=0;
    app.add_option("--one", one);
    app.add_option("--two", two);
    app.add_option("--three", three);

    args = {"--one=1"};

    run();

    EXPECT_EQ(1, one);
    EXPECT_EQ(99, two);
    EXPECT_EQ(3, three);

    app.reset();
    one=two=three=0;
    args = {"--one=1", "--two=2"};

    run();

    EXPECT_EQ(1, one);
    EXPECT_EQ(2, two);
    EXPECT_EQ(3, three);

}

TEST_F(TApp, IniRequired) {

    TempFile tmpini{"TestIniTmp.ini"};

    app.add_config("--config", tmpini);

    {
        std::ofstream out{tmpini};
        out << "[default]" << std::endl;
        out << "two=99" << std::endl;
        out << "three=3" << std::endl;
    }

    int one=0, two=0, three=0;
    app.add_option("--one", one)->required();
    app.add_option("--two", two)->required();
    app.add_option("--three", three)->required();

    args = {"--one=1"};

    EXPECT_NO_THROW(run());

    app.reset();
    one=two=three=0;
    args = {"--one=1", "--two=2"};

    EXPECT_NO_THROW(run());

    app.reset();
    args = {};

    EXPECT_THROW(run(), CLI::RequiredError);

    app.reset();
    args = {"--two=2"};

    EXPECT_THROW(run(), CLI::RequiredError);

}

TEST_F(TApp, IniOutputSimple) {

    int v;
    app.add_option("--simple", v);

    args = {"--simple=3"};

    run();

    std::string str = app.config_to_str();
    EXPECT_EQ("simple=3\n", str);

}


TEST_F(TApp, IniOutputFlag) {

    int v;
    app.add_option("--simple", v);
    app.add_flag("--nothing");

    args = {"--simple=3", "--nothing"};

    run();

    std::string str = app.config_to_str();
    EXPECT_THAT(str, HasSubstr("simple=3"));
    EXPECT_THAT(str, HasSubstr("nothing="));

}
