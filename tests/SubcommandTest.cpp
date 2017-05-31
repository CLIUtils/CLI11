#include "app_helper.hpp"

TEST_F(TApp, BasicSubcommands) {
    auto sub1 = app.add_subcommand("sub1");
    auto sub2 = app.add_subcommand("sub2");

    EXPECT_EQ(sub1, app.get_subcommand(sub1));
    EXPECT_EQ(sub1, app.get_subcommand("sub1"));
    EXPECT_THROW(app.get_subcommand("sub3"), CLI::OptionNotFound);

    run();
    EXPECT_EQ((size_t)0, app.get_subcommands().size());

    app.reset();
    args = {"sub1"};
    run();
    EXPECT_EQ(sub1, app.get_subcommands().at(0));

    app.reset();
    EXPECT_EQ((size_t)0, app.get_subcommands().size());

    args = {"sub2"};
    run();
    EXPECT_EQ(sub2, app.get_subcommands().at(0));

    app.reset();
    args = {"SUb2"};
    EXPECT_THROW(run(), CLI::ExtrasError);
}

TEST_F(TApp, MultiSubFallthrough) {

    // No explicit fallthrough
    auto sub1 = app.add_subcommand("sub1");
    auto sub2 = app.add_subcommand("sub2");

    args = {"sub1", "sub2"};
    run();
    EXPECT_TRUE(app.got_subcommand("sub1"));
    EXPECT_TRUE(app.got_subcommand(sub1));
    EXPECT_TRUE(*sub1);
    EXPECT_TRUE(sub1->parsed());

    EXPECT_TRUE(app.got_subcommand("sub2"));
    EXPECT_TRUE(app.got_subcommand(sub2));
    EXPECT_TRUE(*sub2);

    app.reset();
    app.require_subcommand();

    run();

    app.reset();
    app.require_subcommand(2);

    run();

    app.reset();
    app.require_subcommand(1);

    EXPECT_THROW(run(), CLI::RequiredError);

    app.reset();
    args = {"sub1"};
    run();

    EXPECT_TRUE(app.got_subcommand("sub1"));
    EXPECT_FALSE(app.got_subcommand("sub2"));

    EXPECT_TRUE(*sub1);
    EXPECT_FALSE(*sub2);
    EXPECT_FALSE(sub2->parsed());

    EXPECT_THROW(app.got_subcommand("sub3"), CLI::OptionNotFound);
}

TEST_F(TApp, Callbacks) {
    auto sub1 = app.add_subcommand("sub1");
    sub1->set_callback([]() { throw CLI::Success(); });
    auto sub2 = app.add_subcommand("sub2");
    bool val = false;
    sub2->set_callback([&val]() { val = true; });

    args = {"sub2"};
    EXPECT_FALSE(val);
    run();
    EXPECT_TRUE(val);
}

TEST_F(TApp, NoFallThroughOpts) {
    int val = 1;
    app.add_option("--val", val);

    app.add_subcommand("sub");

    args = {"sub", "--val", "2"};
    EXPECT_THROW(run(), CLI::ExtrasError);
}

TEST_F(TApp, NoFallThroughPositionals) {
    int val = 1;
    app.add_option("val", val);

    app.add_subcommand("sub");

    args = {"sub", "2"};
    EXPECT_THROW(run(), CLI::ExtrasError);
}

TEST_F(TApp, FallThroughRegular) {
    app.fallthrough();
    int val = 1;
    app.add_option("--val", val);

    app.add_subcommand("sub");

    args = {"sub", "--val", "2"};
    // Should not throw
    run();
}

TEST_F(TApp, FallThroughShort) {
    app.fallthrough();
    int val = 1;
    app.add_option("-v", val);

    app.add_subcommand("sub");

    args = {"sub", "-v", "2"};
    // Should not throw
    run();
}

TEST_F(TApp, FallThroughPositional) {
    app.fallthrough();
    int val = 1;
    app.add_option("val", val);

    app.add_subcommand("sub");

    args = {"sub", "2"};
    // Should not throw
    run();
}

TEST_F(TApp, FallThroughEquals) {
    app.fallthrough();
    int val = 1;
    app.add_option("--val", val);

    app.add_subcommand("sub");

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
    sub->set_callback([&val, &sub_val]() { sub_val = val; });

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

    run();
}

TEST_F(TApp, Required1SubCom) {
    app.require_subcommand(1);
    app.add_subcommand("sub1");
    app.add_subcommand("sub2");
    app.add_subcommand("sub3");

    EXPECT_THROW(run(), CLI::RequiredError);

    app.reset();
    args = {"sub1"};
    run();

    app.reset();
    args = {"sub1", "sub2"};
    EXPECT_THROW(run(), CLI::RequiredError);
}

TEST_F(TApp, BadSubcomSearch) {

    auto one = app.add_subcommand("one");
    auto two = one->add_subcommand("two");

    EXPECT_THROW(app.get_subcommand(two), CLI::OptionNotFound);
}

struct SubcommandProgram : public TApp {

    CLI::App *start;
    CLI::App *stop;

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

    run();

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

    run();
    EXPECT_EQ((size_t)2, app.get_subcommands().size());
    EXPECT_EQ(1, dummy);
    EXPECT_EQ("filename", file);
}

TEST_F(SubcommandProgram, MultipleOtherOrder) {
    args = {"start", "-d", "-ffilename", "stop"};

    EXPECT_THROW(run(), CLI::ExtrasError);
}

TEST_F(SubcommandProgram, MultipleArgs) {
    args = {"start", "stop"};

    run();

    EXPECT_EQ((size_t)2, app.get_subcommands().size());
}

TEST_F(SubcommandProgram, CaseCheck) {
    args = {"Start"};
    EXPECT_THROW(run(), CLI::ExtrasError);

    app.reset();
    args = {"start"};
    run();

    app.reset();
    start->ignore_case();
    run();

    app.reset();
    args = {"Start"};
    run();
}

TEST_F(TApp, SubcomInheritCaseCheck) {
    app.ignore_case();
    auto sub1 = app.add_subcommand("sub1");
    auto sub2 = app.add_subcommand("sub2");

    run();
    EXPECT_EQ((size_t)0, app.get_subcommands().size());

    app.reset();
    args = {"SuB1"};
    run();
    EXPECT_EQ(sub1, app.get_subcommands().at(0));

    app.reset();
    EXPECT_EQ((size_t)0, app.get_subcommands().size());

    args = {"sUb2"};
    run();
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

TEST_F(SubcommandProgram, Callbacks) {

    start->set_callback([]() { throw CLI::Success(); });

    run();

    app.reset();

    args = {"start"};

    EXPECT_THROW(run(), CLI::Success);
}
