#include "app_helper.hpp"

TEST_F(TApp, BasicSubcommands) {
    auto sub1 = app.add_subcommand("sub1");
    auto sub2 = app.add_subcommand("sub2");

    EXPECT_NO_THROW(run());
    EXPECT_EQ(0, app.get_subcommands().size());
    
    app.reset();
    args = {"sub1"};
    EXPECT_NO_THROW(run());
    EXPECT_EQ(sub1, app.get_subcommands().at(0));

    app.reset();
    EXPECT_EQ(0, app.get_subcommands().size());

    args = {"sub2"};
    EXPECT_NO_THROW(run());
    EXPECT_EQ(sub2, app.get_subcommands().at(0));

    app.reset();
    args = {"SUb2"};
    EXPECT_THROW(run(), CLI::ExtrasError);
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
    
    args = {"sub2"};
    EXPECT_FALSE(val);
    EXPECT_NO_THROW(run());
    EXPECT_TRUE(val);

}

TEST_F(TApp, FallThroughRegular) {
    app.fallthrough();
    int val = 1;
    app.add_option("--val", val);

    auto sub = app.add_subcommand("sub");
    
    args = {"sub", "--val", "2"};
    // Should not throw
    run();
}

TEST_F(TApp, FallThroughEquals) {
    app.fallthrough();
    int val = 1;
    app.add_option("--val", val);

    auto sub = app.add_subcommand("sub");
    
    args = {"sub", "--val=2"};
    // Should not throw
    run();
}


TEST_F(TApp, EvilParseFallthrough) {
    app.fallthrough();
    int val1 = 0, val2 = 0;
    app.add_option("--val1", val1);

    auto sub = app.add_subcommand("sub");
    sub->add_option("val2", val2);
    
    args = {"sub", "--val1", "1", "2"};
    // Should not throw
    run();

    EXPECT_EQ(1, val1);
    EXPECT_EQ(2, val2);
}

TEST_F(TApp, CallbackOrdering) {
    app.fallthrough();
    int val = 1, sub_val = 0;
    app.add_option("--val", val);

    auto sub = app.add_subcommand("sub");
    sub->set_callback([&val, &sub_val](){
            sub_val = val;
            });
    
    args = {"sub", "--val=2"};
    run();
    EXPECT_EQ(2, val);
    EXPECT_EQ(2, sub_val);

    app.reset();
    args = {"--val=2", "sub"};
    run();
    EXPECT_EQ(2, val);
    EXPECT_EQ(2, sub_val);

}

TEST_F(TApp, RequiredSubCom) {
    app.add_subcommand("sub1");
    app.add_subcommand("sub2");

    app.require_subcommand();

    EXPECT_THROW(run(), CLI::RequiredError);

    app.reset();

    args = {"sub1"};

    EXPECT_NO_THROW(run());

}

TEST_F(TApp, Required1SubCom) {
    app.require_subcommand(1);
    app.add_subcommand("sub1");
    app.add_subcommand("sub2");
    app.add_subcommand("sub3");

    EXPECT_THROW(run(), CLI::RequiredError);

    app.reset();
    args = {"sub1"};
    EXPECT_NO_THROW(run());

    app.reset();
    args = {"sub1", "sub2"};
    EXPECT_THROW(run(), CLI::RequiredError);
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
    EXPECT_EQ(start, app.get_subcommands().at(0));
    EXPECT_EQ("filename", file);
}


TEST_F(SubcommandProgram, Spare) {
    args = {"extra", "-d", "start", "-ffilename"};

    EXPECT_THROW(run(), CLI::ExtrasError);
}

TEST_F(SubcommandProgram, SpareSub) {
    args = {"-d", "start", "spare", "-ffilename"};

    EXPECT_THROW(run(), CLI::ExtrasError);
}

TEST_F(SubcommandProgram, Multiple) {
    args = {"-d", "start", "-ffilename", "stop"};

    EXPECT_NO_THROW(run());
    EXPECT_EQ(2, app.get_subcommands().size());
    EXPECT_EQ(1, dummy);
    EXPECT_EQ("filename", file);
}

TEST_F(SubcommandProgram, MultipleOtherOrder) {
    args = {"start", "-d", "-ffilename", "stop"};

    EXPECT_THROW(run(), CLI::ExtrasError);
}

TEST_F(SubcommandProgram, MultipleArgs) {
    args = {"start", "stop"};

    EXPECT_NO_THROW(run());

    EXPECT_EQ(2, app.get_subcommands().size());

}

TEST_F(SubcommandProgram, CaseCheck) {
    args = {"Start"};
    EXPECT_THROW(run(), CLI::ExtrasError);


    app.reset();
    args = {"start"};
    EXPECT_NO_THROW(run());


    app.reset();
    start->ignore_case();
    EXPECT_NO_THROW(run());

    app.reset();
    args = {"Start"};
    EXPECT_NO_THROW(run());
}

TEST_F(TApp, SubcomInheritCaseCheck) {
    app.ignore_case();
    auto sub1 = app.add_subcommand("sub1");
    auto sub2 = app.add_subcommand("sub2");

    EXPECT_NO_THROW(run());
    EXPECT_EQ(0, app.get_subcommands().size());
    
    app.reset();
    args = {"SuB1"};
    EXPECT_NO_THROW(run());
    EXPECT_EQ(sub1, app.get_subcommands().at(0));

    app.reset();
    EXPECT_EQ(0, app.get_subcommands().size());

    args = {"sUb2"};
    EXPECT_NO_THROW(run());
    EXPECT_EQ(sub2, app.get_subcommands().at(0));
}

TEST_F(SubcommandProgram, HelpOrder) {

    args = {"-h"};
    EXPECT_THROW(run(), CLI::CallForHelp);

    args = {"start", "-h"};
    EXPECT_THROW(run(), CLI::CallForHelp);

    args = {"-h", "start"};
    EXPECT_THROW(run(), CLI::CallForHelp);
}
