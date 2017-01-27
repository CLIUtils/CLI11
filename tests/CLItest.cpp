
#include "CLI.hpp"
#include "gtest/gtest.h"


typedef std::vector<std::string> input_t;

TEST(Basic, Empty) {

    {
    CLI::App app;
    input_t simpleput;
    app.parse(simpleput);
    }
    {
    CLI::App app;
    input_t spare = {"spare"};
    EXPECT_THROW(app.parse(spare), CLI::ExtraPositionalsError);
    }
    {
    CLI::App app;
    input_t simpleput;
    app.parse(simpleput);
    }
}

struct TApp : public ::testing::Test {
    CLI::App app{"My Test Program"};
    input_t args;

    void run() {
        input_t newargs = args;
        std::reverse(std::begin(newargs), std::end(newargs));
        app.parse(newargs);
    }

};

TEST_F(TApp, OneFlagShort) {
    app.add_flag("c,count");
    args = {"-c"};
    run();
    EXPECT_EQ(1, app.count("c"));
    EXPECT_EQ(1, app.count("count"));
}

TEST_F(TApp, OneFlagLong) {
    app.add_flag("c,count");
    args = {"--count"};
    run();
    EXPECT_EQ(1, app.count("c"));
    EXPECT_EQ(1, app.count("count"));
}

TEST_F(TApp, OneFlagRef) {
    int ref;
    app.add_flag("c,count", ref);
    args = {"--count"};
    run();
    EXPECT_EQ(1, app.count("c"));
    EXPECT_EQ(1, app.count("count"));
    EXPECT_EQ(1, ref);
}

TEST_F(TApp, OneString) {
    std::string str;
    app.add_option("s,string", str);
    args = {"--string", "mystring"};
    run();
    EXPECT_EQ(1, app.count("s"));
    EXPECT_EQ(1, app.count("string"));
    EXPECT_EQ(str, "mystring");
}


TEST_F(TApp, TogetherInt) {
    int i;
    app.add_option("i,int", i);
    args = {"-i4"};
    run();
    EXPECT_EQ(1, app.count("int"));
    EXPECT_EQ(1, app.count("i"));
    EXPECT_EQ(i, 4);
}

TEST_F(TApp, SepInt) {
    int i;
    app.add_option("i,int", i);
    args = {"-i","4"};
    run();
    EXPECT_EQ(1, app.count("int"));
    EXPECT_EQ(1, app.count("i"));
    EXPECT_EQ(i, 4);
}

TEST_F(TApp, OneStringAgain) {
    std::string str;
    app.add_option("s,string", str);
    args = {"--string", "mystring"};
    run();
    EXPECT_EQ(1, app.count("s"));
    EXPECT_EQ(1, app.count("string"));
    EXPECT_EQ(str, "mystring");
}


TEST_F(TApp, DefaultStringAgain) {
    std::string str = "previous";
    app.add_option("s,string", str);
    run();
    EXPECT_EQ(0, app.count("s"));
    EXPECT_EQ(0, app.count("string"));
    EXPECT_EQ(str, "previous");
}

TEST_F(TApp, LotsOfFlags) {

    app.add_flag("a");
    app.add_flag("A");
    app.add_flag("b");

    args = {"-a","-b","-aA"};
    run();
    EXPECT_EQ(2, app.count("a"));
    EXPECT_EQ(1, app.count("b"));
    EXPECT_EQ(1, app.count("A"));
}
