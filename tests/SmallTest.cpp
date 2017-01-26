#include "CLI.hpp"
#include "gtest/gtest.h"


TEST(Split, GoodStrings) {
    std::vector<std::string> test_strings = {"a,boo", ",coo", "d,", "Q,this-is", "s", "single"};

    std::string s, l;
        
    std::tie(s, l) = CLI::split("a,boo");
    EXPECT_EQ("a", s);
    EXPECT_EQ("boo", l);
    
    std::tie(s, l) = CLI::split(",coo");
    EXPECT_EQ("", s);
    EXPECT_EQ("coo", l);

    std::tie(s, l) = CLI::split("d,");
    EXPECT_EQ("d", s);
    EXPECT_EQ("", l);

    std::tie(s, l) = CLI::split("Q,this-is");
    EXPECT_EQ("Q", s);
    EXPECT_EQ("this-is", l);

    std::tie(s, l) = CLI::split("s");
    EXPECT_EQ("s", s);
    EXPECT_EQ("", l);

    std::tie(s, l) = CLI::split("single");
    EXPECT_EQ("", s);
    EXPECT_EQ("single", l);
    }

TEST(Split, BadStrings) {
    std::vector<std::string> test_fails= {"a,,boo", "a,b,c", "ssd,sfd", "-a", "", ",", "one two"};

    for(std::string name : test_fails) {
        EXPECT_THROW(CLI::split(name), CLI::BadNameString);
    }

    
}
