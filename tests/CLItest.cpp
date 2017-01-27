
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

TEST_F(TApp, ShortOpts) {

    unsigned long long funnyint;
    std::string someopt;
    app.add_flag("z", funnyint);
    app.add_option("y", someopt);

    args = {"-zzyzyz",};

    run();

    EXPECT_EQ(2, app.count("z"));
    EXPECT_EQ(1, app.count("y"));
    EXPECT_EQ((unsigned long long) 2, funnyint);
    EXPECT_EQ("zyz", someopt);
}

TEST_F(TApp, Reset) {

    app.add_flag("simple");
    double doub;
    app.add_option("d,double", doub);

    args = {"--simple", "--double", "1.2"};

    run();

    EXPECT_EQ(1, app.count("simple"));
    EXPECT_EQ(1, app.count("d"));
    EXPECT_FLOAT_EQ(1.2, doub);

    app.reset();

    EXPECT_EQ(0, app.count("simple"));
    EXPECT_EQ(0, app.count("d"));
    
    run();

    EXPECT_EQ(1, app.count("simple"));
    EXPECT_EQ(1, app.count("d"));
    EXPECT_FLOAT_EQ(1.2, doub);

}


TEST_F(TApp, Basic) {
    auto sub1 = app.add_subcommand("sub1");
    auto sub2 = app.add_subcommand("sub2");

    run();
    EXPECT_EQ(nullptr, app.get_subcommand());
    
    app.reset();
    args = {"sub1"};
    run();
    EXPECT_EQ(sub1, app.get_subcommand());

    app.reset();
    EXPECT_EQ(nullptr, app.get_subcommand());

    args = {"sub2"};
    run();
    EXPECT_EQ(sub2, app.get_subcommand());
}


struct SubcommandProgram : public TApp {

    CLI::App* start;
    CLI::App* stop;

    int dummy;
    std::string file;
    int count;

    SubcommandProgram() {
        start = app.add_subcommand("start", "Start prog");
        stop = app.add_subcommand("stop", "Stop prog");
            
        app.add_flag("d", dummy, "My dummy var");
        start->add_option("f,file", file, "File name");
        stop->add_flag("c,count", count, "Some flag opt");
    }
};

TEST_F(SubcommandProgram, Working) {
    args = {"-d", "start", "-ffilename"};

    run();

    EXPECT_EQ(1, dummy);
    EXPECT_EQ(start, app.get_subcommand());
    EXPECT_EQ("filename", file);
}


TEST_F(SubcommandProgram, Spare) {
    args = {"extra", "-d", "start", "-ffilename"};

    EXPECT_THROW(run(), CLI::ExtraPositionalsError);
}

TEST_F(SubcommandProgram, SpareSub) {
    args = {"-d", "start", "spare", "-ffilename"};

    EXPECT_THROW(run(), CLI::ExtraPositionalsError);
}

// TODO: Add positionals
// TODO: Add vector arguments
// TODO: Maybe add function to call on subcommand parse?
// TODO: Check help output
// TODO: Add default/type info to help
// TODO: Add set checking
// TODO: Try all of the options together

