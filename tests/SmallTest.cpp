#include "CLI.hpp"
#include "gtest/gtest.h"
#include <cstdio>
#include <fstream>


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

TEST(Validators, FileExists) {
    std::string myfile{"TestFileNotUsed.txt"};
    EXPECT_FALSE(CLI::_ExistingFile(myfile));
    bool ok = static_cast<bool>(std::ofstream(myfile.c_str()).put('a')); // create file
    EXPECT_TRUE(ok);
    EXPECT_TRUE(CLI::_ExistingFile(myfile));

    std::remove(myfile.c_str());
    EXPECT_FALSE(CLI::_ExistingFile(myfile));
}

TEST(Validators, FileNotExists) {
    std::string myfile{"TestFileNotUsed.txt"};
    EXPECT_TRUE(CLI::_NonexistentPath(myfile));
    bool ok = static_cast<bool>(std::ofstream(myfile.c_str()).put('a')); // create file
    EXPECT_TRUE(ok);
    EXPECT_FALSE(CLI::_NonexistentPath(myfile));

    std::remove(myfile.c_str());
    EXPECT_TRUE(CLI::_NonexistentPath(myfile));
}
