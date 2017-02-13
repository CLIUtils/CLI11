#include "app_helper.hpp"

#include <cstdio>
#include <sstream>

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
        std::ofstream out = tmpini.ofstream();
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
