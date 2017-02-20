#include "app_helper.hpp"

TEST_F(TApp, BasicSubcommands) {
    auto sub1 = app.add_subcommand("sub1");
    auto sub2 = app.add_subcommand("sub2");

    EXPECT_NO_THROW(run());
    EXPECT_EQ(nullptr, app.get_subcommand());
    
    app.reset();
    args = {"sub1"};
    EXPECT_NO_THROW(run());
    EXPECT_EQ(sub1, app.get_subcommand());

    app.reset();
    EXPECT_EQ(nullptr, app.get_subcommand());

    args = {"sub2"};
    EXPECT_NO_THROW(run());
    EXPECT_EQ(sub2, app.get_subcommand());
}


TEST_F(TApp, Callbacks) {
    auto sub1 = app.add_subcommand("sub1");
    sub1->set_callback([](){
            throw CLI::Success();
            });
    auto sub2 = app.add_subcommand("sub2");
    bool val = false;
    sub2->set_callback([&val](){
            val = true;
            });
    
    app.reset();
    args = {"sub2"};
    EXPECT_FALSE(val);
    EXPECT_NO_THROW(run());
    EXPECT_TRUE(val);

}

TEST_F(TApp, RequiredSubCom) {
    auto sub1 = app.add_subcommand("sub1");
    auto sub2 = app.add_subcommand("sub2");

    app.require_subcommand();

    EXPECT_THROW(run(), CLI::RequiredError);

    app.reset();

    args = {"sub1"};

    EXPECT_NO_THROW(run());

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
            
        app.add_flag("-d", dummy, "My dummy var");
        start->add_option("-f,--file", file, "File name");
        stop->add_flag("-c,--count", count, "Some flag opt");
    }
};

TEST_F(SubcommandProgram, Working) {
    args = {"-d", "start", "-ffilename"};

    EXPECT_NO_THROW(run());

    EXPECT_EQ(1, dummy);
    EXPECT_EQ(start, app.get_subcommand());
    EXPECT_EQ("filename", file);
}


TEST_F(SubcommandProgram, Spare) {
    args = {"extra", "-d", "start", "-ffilename"};

    EXPECT_THROW(run(), CLI::PositionalError);
}

TEST_F(SubcommandProgram, SpareSub) {
    args = {"-d", "start", "spare", "-ffilename"};

    EXPECT_THROW(run(), CLI::PositionalError);
}


