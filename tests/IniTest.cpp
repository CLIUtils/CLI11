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
