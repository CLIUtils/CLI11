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

    EXPECT_EQ(2, output.size());
    EXPECT_EQ("one", output.at(0).name());
    EXPECT_EQ(1, output.at(0).inputs.size());  
    EXPECT_EQ("three", output.at(0).inputs.at(0));
    EXPECT_EQ("two", output.at(1).name());
    EXPECT_EQ(1, output.at(1).inputs.size());  
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

    EXPECT_EQ(2, output.size());
    EXPECT_EQ("one", output.at(0).name());
    EXPECT_EQ(1, output.at(0).inputs.size());  
    EXPECT_EQ("three", output.at(0).inputs.at(0));
    EXPECT_EQ("two", output.at(1).name());
    EXPECT_EQ(1, output.at(1).inputs.size());  
    EXPECT_EQ("four", output.at(1).inputs.at(0));
}

TEST(StringBased, Quotes) {
    std::stringstream ofile;

    ofile << "one = \"three\"" << std::endl;
    ofile << "two = \'four\'" << std::endl;
    ofile << "five = \"six and seven\"" << std::endl;

    ofile.seekg(0, std::ios::beg);

    auto output = CLI::detail::parse_ini(ofile);

    EXPECT_EQ(3, output.size());
    EXPECT_EQ("one", output.at(0).name());
    EXPECT_EQ(1, output.at(0).inputs.size());  
    EXPECT_EQ("three", output.at(0).inputs.at(0));
    EXPECT_EQ("two", output.at(1).name());
    EXPECT_EQ(1, output.at(1).inputs.size());  
    EXPECT_EQ("four", output.at(1).inputs.at(0));
    EXPECT_EQ("five", output.at(2).name());
    EXPECT_EQ(1, output.at(2).inputs.size());  
    EXPECT_EQ("six and seven", output.at(2).inputs.at(0));
}

TEST(StringBased, Vector) {
    std::stringstream ofile;

    ofile << "one = three" << std::endl;
    ofile << "two = four" << std::endl;
    ofile << "five = six and seven" << std::endl;

    ofile.seekg(0, std::ios::beg);

    auto output = CLI::detail::parse_ini(ofile);

    EXPECT_EQ(3, output.size());
    EXPECT_EQ("one", output.at(0).name());
    EXPECT_EQ(1, output.at(0).inputs.size());  
    EXPECT_EQ("three", output.at(0).inputs.at(0));
    EXPECT_EQ("two", output.at(1).name());
    EXPECT_EQ(1, output.at(1).inputs.size());  
    EXPECT_EQ("four", output.at(1).inputs.at(0));
    EXPECT_EQ("five", output.at(2).name());
    EXPECT_EQ(3, output.at(2).inputs.size());  
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

    EXPECT_EQ(2, output.size());
    EXPECT_EQ("one", output.at(0).name());
    EXPECT_EQ(1, output.at(0).inputs.size());  
    EXPECT_EQ("three", output.at(0).inputs.at(0));
    EXPECT_EQ("two", output.at(1).name());
    EXPECT_EQ(1, output.at(1).inputs.size());  
    EXPECT_EQ("four", output.at(1).inputs.at(0));
}

TEST(StringBased, Sections) {
    std::stringstream ofile;

    ofile << "one=three" << std::endl;
    ofile << "[second]" << std::endl;
    ofile << "  two=four" << std::endl;

    ofile.seekg(0, std::ios::beg);

    auto output = CLI::detail::parse_ini(ofile);
    
    EXPECT_EQ(2, output.size());
    EXPECT_EQ("one", output.at(0).name());
    EXPECT_EQ(1, output.at(0).inputs.size());  
    EXPECT_EQ("three", output.at(0).inputs.at(0));
    EXPECT_EQ("two", output.at(1).name());
    EXPECT_EQ("second", output.at(1).parent());
    EXPECT_EQ(1, output.at(1).inputs.size());  
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

    EXPECT_EQ(2, output.size());
    EXPECT_EQ("one", output.at(0).name());
    EXPECT_EQ(1, output.at(0).inputs.size());  
    EXPECT_EQ("three", output.at(0).inputs.at(0));
    EXPECT_EQ("two", output.at(1).name());
    EXPECT_EQ("second", output.at(1).parent());
    EXPECT_EQ(1, output.at(1).inputs.size());  
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

TEST_F(TApp, IniRequiredNotFound) {

    std::string noini = "TestIniNotExist.ini";
    app.add_config("--config", noini, "", true);

    EXPECT_THROW(run(), CLI::FileError);
    
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

    int one=0, two=0, three=0;
    app.add_option("--one", one)->required();
    app.add_option("--two", two)->required();
    app.add_option("--three", three)->required();

    args = {"--one=1"};

    run();

    app.reset();
    one=two=three=0;
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

    EXPECT_EQ(std::vector<int>({2,3}), two);
    EXPECT_EQ(std::vector<int>({1,2,3}), three);

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

/// Flags should not show up in config file
TEST_F(TApp, IniOutputFlag) {

    int v;
    app.add_option("--simple", v);
    app.add_flag("--nothing");

    args = {"--simple=3", "--nothing"};

    run();

    std::string str = app.config_to_str();
    EXPECT_THAT(str, HasSubstr("simple=3"));
    EXPECT_THAT(str, Not(HasSubstr("nothing=")));

}
