#include "CLI.hpp"
#include "gtest/gtest.h"
#include <cstdio>
#include <fstream>


TEST(Split, GoodStrings) {
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

    EXPECT_THROW(CLI::split("a,,boo"), CLI::BadNameString);
    EXPECT_THROW(CLI::split("a,b,c"), CLI::BadNameString);
    EXPECT_THROW(CLI::split("ssd,sfd"), CLI::BadNameString);
    EXPECT_THROW(CLI::split("-a"), CLI::BadNameString);
    EXPECT_THROW(CLI::split(""), CLI::BadNameString);
    EXPECT_THROW(CLI::split(","), CLI::BadNameString);
    EXPECT_THROW(CLI::split("one two"), CLI::BadNameString);
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

TEST(Split, StringList) {

    std::vector<std::string> results {"a", "long", "--lone", "-q"};
    EXPECT_EQ(results, CLI::split_names("a,long,--lone,-q"));

    EXPECT_EQ(std::vector<std::string>({"one"}), CLI::split_names("one"));
}

TEST(RegEx, Shorts) {
    std::string name, value;

    EXPECT_TRUE(CLI::split_short("-a", name, value));
    EXPECT_EQ("a", name);
    EXPECT_EQ("", value);

    EXPECT_TRUE(CLI::split_short("-B", name, value));
    EXPECT_EQ("B", name);
    EXPECT_EQ("", value);

    EXPECT_TRUE(CLI::split_short("-cc", name, value));
    EXPECT_EQ("c", name);
    EXPECT_EQ("c", value);

    EXPECT_TRUE(CLI::split_short("-simple", name, value));
    EXPECT_EQ("s", name);
    EXPECT_EQ("imple", value);

    EXPECT_FALSE(CLI::split_short("--a", name, value));
    EXPECT_FALSE(CLI::split_short("--thing", name, value));
    EXPECT_FALSE(CLI::split_short("--", name, value));
    EXPECT_FALSE(CLI::split_short("something", name, value));
    EXPECT_FALSE(CLI::split_short("s", name, value));
}

TEST(RegEx, Longs) {
    std::string name, value;

    EXPECT_TRUE(CLI::split_long("--a", name, value));
    EXPECT_EQ("a", name);
    EXPECT_EQ("", value);

    EXPECT_TRUE(CLI::split_long("--thing", name, value));
    EXPECT_EQ("thing", name);
    EXPECT_EQ("", value);

    EXPECT_TRUE(CLI::split_long("--some=thing", name, value));
    EXPECT_EQ("some", name);
    EXPECT_EQ("thing", value);

    EXPECT_FALSE(CLI::split_long("-a", name, value));
    EXPECT_FALSE(CLI::split_long("-things", name, value));
    EXPECT_FALSE(CLI::split_long("Q", name, value));
    EXPECT_FALSE(CLI::split_long("--", name, value));

}
