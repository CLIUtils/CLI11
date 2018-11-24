#include "app_helper.hpp"

#include "gmock/gmock.h"
#include "gtest/gtest.h"

using ::testing::HasSubstr;
using ::testing::Not;

using vs_t = std::vector<std::string>;

TEST_F(TApp, BasicSubcommands) {
    auto sub1 = app.add_subcommand("sub1");
    auto sub2 = app.add_subcommand("sub2");

    EXPECT_EQ(sub1->get_parent(), &app);

    EXPECT_EQ(sub1, app.get_subcommand(sub1));
    EXPECT_EQ(sub1, app.get_subcommand("sub1"));
    EXPECT_THROW(app.get_subcommand("sub3"), CLI::OptionNotFound);

    run();
    EXPECT_EQ((size_t)0, app.get_subcommands().size());

    args = {"sub1"};
    run();
    EXPECT_EQ(sub1, app.get_subcommands().at(0));
    EXPECT_EQ((size_t)1, app.get_subcommands().size());

    app.clear();
    EXPECT_EQ((size_t)0, app.get_subcommands().size());

    args = {"sub2"};
    run();
    EXPECT_EQ((size_t)1, app.get_subcommands().size());
    EXPECT_EQ(sub2, app.get_subcommands().at(0));

    args = {"SUb2"};
    EXPECT_THROW(run(), CLI::ExtrasError);

    args = {"SUb2"};
    try {
        run();
    } catch(const CLI::ExtrasError &e) {
        EXPECT_THAT(e.what(), HasSubstr("SUb2"));
    }

    args = {"sub1", "extra"};
    try {
        run();
    } catch(const CLI::ExtrasError &e) {
        EXPECT_THAT(e.what(), HasSubstr("extra"));
    }
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

    app.require_subcommand();

    run();

    app.require_subcommand(2);

    run();

    app.require_subcommand(1);

    EXPECT_THROW(run(), CLI::ExtrasError);

    args = {"sub1"};
    run();

    EXPECT_TRUE(app.got_subcommand("sub1"));
    EXPECT_FALSE(app.got_subcommand("sub2"));

    EXPECT_TRUE(*sub1);
    EXPECT_FALSE(*sub2);
    EXPECT_FALSE(sub2->parsed());

    EXPECT_THROW(app.got_subcommand("sub3"), CLI::OptionNotFound);
}

TEST_F(TApp, RequiredAndSubcoms) { // #23

    std::string baz;
    app.add_option("baz", baz, "Baz Description", true)->required();
    auto foo = app.add_subcommand("foo");
    auto bar = app.add_subcommand("bar");

    args = {"bar", "foo"};
    ASSERT_NO_THROW(run());
    EXPECT_TRUE(*foo);
    EXPECT_FALSE(*bar);
    EXPECT_EQ(baz, "bar");

    args = {"foo"};
    ASSERT_NO_THROW(run());
    EXPECT_FALSE(*foo);
    EXPECT_EQ(baz, "foo");

    args = {"foo", "foo"};
    ASSERT_NO_THROW(run());
    EXPECT_TRUE(*foo);
    EXPECT_EQ(baz, "foo");

    args = {"foo", "other"};
    EXPECT_THROW(run(), CLI::ExtrasError);
}

TEST_F(TApp, RequiredAndSubcomFallthrough) {

    std::string baz;
    app.add_option("baz", baz)->required();
    app.add_subcommand("foo");
    auto bar = app.add_subcommand("bar");
    app.fallthrough();

    args = {"other", "bar"};
    run();
    EXPECT_TRUE(bar);
    EXPECT_EQ(baz, "other");

    args = {"bar", "other2"};
    EXPECT_THROW(run(), CLI::ExtrasError);
}

TEST_F(TApp, FooFooProblem) {

    std::string baz_str, other_str;
    auto baz = app.add_option("baz", baz_str);
    auto foo = app.add_subcommand("foo");
    auto other = foo->add_option("other", other_str);

    args = {"foo", "foo"};
    run();
    EXPECT_TRUE(*foo);
    EXPECT_FALSE(*baz);
    EXPECT_TRUE(*other);
    EXPECT_EQ(baz_str, "");
    EXPECT_EQ(other_str, "foo");

    baz_str = "";
    other_str = "";
    baz->required();
    run();
    EXPECT_TRUE(*foo);
    EXPECT_TRUE(*baz);
    EXPECT_FALSE(*other);
    EXPECT_EQ(baz_str, "foo");
    EXPECT_EQ(other_str, "");
}

TEST_F(TApp, Callbacks) {
    auto sub1 = app.add_subcommand("sub1");
    sub1->callback([]() { throw CLI::Success(); });
    auto sub2 = app.add_subcommand("sub2");
    bool val = false;
    sub2->callback([&val]() { val = true; });

    args = {"sub2"};
    EXPECT_FALSE(val);
    run();
    EXPECT_TRUE(val);
}

TEST_F(TApp, RuntimeErrorInCallback) {
    auto sub1 = app.add_subcommand("sub1");
    sub1->callback([]() { throw CLI::RuntimeError(); });
    auto sub2 = app.add_subcommand("sub2");
    sub2->callback([]() { throw CLI::RuntimeError(2); });

    args = {"sub1"};
    EXPECT_THROW(run(), CLI::RuntimeError);

    args = {"sub1"};
    try {
        run();
    } catch(const CLI::RuntimeError &e) {
        EXPECT_EQ(1, e.get_exit_code());
    }

    args = {"sub2"};
    EXPECT_THROW(run(), CLI::RuntimeError);

    args = {"sub2"};
    try {
        run();
    } catch(const CLI::RuntimeError &e) {
        EXPECT_EQ(2, e.get_exit_code());
    }
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
    sub->callback([&val, &sub_val]() { sub_val = val; });

    args = {"sub", "--val=2"};
    run();
    EXPECT_EQ(2, val);
    EXPECT_EQ(2, sub_val);

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

    args = {"sub1"};
    run();
}

TEST_F(TApp, SubComExtras) {
    app.allow_extras();
    auto sub = app.add_subcommand("sub");

    args = {"extra", "sub"};
    run();
    EXPECT_EQ(app.remaining(), std::vector<std::string>({"extra"}));
    EXPECT_EQ(sub->remaining(), std::vector<std::string>());

    args = {"extra1", "extra2", "sub"};
    run();
    EXPECT_EQ(app.remaining(), std::vector<std::string>({"extra1", "extra2"}));
    EXPECT_EQ(sub->remaining(), std::vector<std::string>());

    args = {"sub", "extra1", "extra2"};
    run();
    EXPECT_EQ(app.remaining(), std::vector<std::string>());
    EXPECT_EQ(sub->remaining(), std::vector<std::string>({"extra1", "extra2"}));

    args = {"extra1", "extra2", "sub", "extra3", "extra4"};
    run();
    EXPECT_EQ(app.remaining(), std::vector<std::string>({"extra1", "extra2"}));
    EXPECT_EQ(app.remaining(true), std::vector<std::string>({"extra1", "extra2", "extra3", "extra4"}));
    EXPECT_EQ(sub->remaining(), std::vector<std::string>({"extra3", "extra4"}));
}

TEST_F(TApp, Required1SubCom) {
    app.require_subcommand(1);
    app.add_subcommand("sub1");
    app.add_subcommand("sub2");
    app.add_subcommand("sub3");

    EXPECT_THROW(run(), CLI::RequiredError);

    args = {"sub1"};
    run();

    args = {"sub1", "sub2"};
    EXPECT_THROW(run(), CLI::ExtrasError);
}

TEST_F(TApp, BadSubcomSearch) {

    auto one = app.add_subcommand("one");
    auto two = one->add_subcommand("two");

    EXPECT_THROW(app.get_subcommand(two), CLI::OptionNotFound);
}

TEST_F(TApp, PrefixProgram) {

    app.prefix_command();

    app.add_flag("--simple");

    args = {"--simple", "other", "--simple", "--mine"};
    run();

    EXPECT_EQ(app.remaining(), std::vector<std::string>({"other", "--simple", "--mine"}));
}

TEST_F(TApp, PrefixNoSeparation) {

    app.prefix_command();

    std::vector<int> vals;
    app.add_option("--vals", vals);

    args = {"--vals", "1", "2", "3", "other"};

    EXPECT_THROW(run(), CLI::ConversionError);
}

TEST_F(TApp, PrefixSeparation) {

    app.prefix_command();

    std::vector<int> vals;
    app.add_option("--vals", vals);

    args = {"--vals", "1", "2", "3", "--", "other"};

    run();

    EXPECT_EQ(app.remaining(), std::vector<std::string>({"other"}));
    EXPECT_EQ(vals, std::vector<int>({1, 2, 3}));
}

TEST_F(TApp, PrefixSubcom) {
    auto subc = app.add_subcommand("subc");
    subc->prefix_command();

    app.add_flag("--simple");

    args = {"--simple", "subc", "other", "--simple", "--mine"};
    run();

    EXPECT_EQ(app.remaining_size(), (size_t)0);
    EXPECT_EQ(app.remaining_size(true), (size_t)3);
    EXPECT_EQ(subc->remaining(), std::vector<std::string>({"other", "--simple", "--mine"}));
}

TEST_F(TApp, InheritHelpAllFlag) {
    app.set_help_all_flag("--help-all");
    auto subc = app.add_subcommand("subc");
    auto help_opt_list = subc->get_options([](const CLI::Option *opt) { return opt->get_name() == "--help-all"; });
    EXPECT_EQ(help_opt_list.size(), (size_t)1);
}

struct SubcommandProgram : public TApp {

    CLI::App *start;
    CLI::App *stop;

    int dummy;
    std::string file;
    int count;

    SubcommandProgram() {
        app.set_help_all_flag("--help-all");

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

    args = {"start"};
    run();

    start->ignore_case();
    run();

    args = {"Start"};
    run();
}

TEST_F(TApp, SubcomInheritCaseCheck) {
    app.ignore_case();
    auto sub1 = app.add_subcommand("sub1");
    auto sub2 = app.add_subcommand("sub2");

    run();
    EXPECT_EQ((size_t)0, app.get_subcommands().size());
    EXPECT_EQ((size_t)2, app.get_subcommands({}).size());
    EXPECT_EQ((size_t)1, app.get_subcommands([](const CLI::App *s) { return s->get_name() == "sub1"; }).size());

    args = {"SuB1"};
    run();
    EXPECT_EQ(sub1, app.get_subcommands().at(0));
    EXPECT_EQ((size_t)1, app.get_subcommands().size());

    app.clear();
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

TEST_F(SubcommandProgram, HelpAllOrder) {

    args = {"--help-all"};
    EXPECT_THROW(run(), CLI::CallForAllHelp);

    args = {"start", "--help-all"};
    EXPECT_THROW(run(), CLI::CallForAllHelp);

    args = {"--help-all", "start"};
    EXPECT_THROW(run(), CLI::CallForAllHelp);
}

TEST_F(SubcommandProgram, Callbacks) {

    start->callback([]() { throw CLI::Success(); });

    run();

    args = {"start"};

    EXPECT_THROW(run(), CLI::Success);
}

TEST_F(SubcommandProgram, Groups) {

    std::string help = app.help();
    EXPECT_THAT(help, Not(HasSubstr("More Commands:")));
    EXPECT_THAT(help, HasSubstr("Subcommands:"));

    start->group("More Commands");
    help = app.help();
    EXPECT_THAT(help, HasSubstr("More Commands:"));
    EXPECT_THAT(help, HasSubstr("Subcommands:"));

    // Case is ignored but for the first subcommand in a group.
    stop->group("more commands");
    help = app.help();
    EXPECT_THAT(help, HasSubstr("More Commands:"));
    EXPECT_THAT(help, Not(HasSubstr("Subcommands:")));
}

TEST_F(SubcommandProgram, ExtrasErrors) {

    args = {"one", "two", "start", "three", "four"};
    EXPECT_THROW(run(), CLI::ExtrasError);

    args = {"start", "three", "four"};
    EXPECT_THROW(run(), CLI::ExtrasError);

    args = {"one", "two"};
    EXPECT_THROW(run(), CLI::ExtrasError);
}

TEST_F(SubcommandProgram, OrderedExtras) {

    app.allow_extras();
    args = {"one", "two", "start", "three", "four"};
    EXPECT_THROW(run(), CLI::ExtrasError);

    start->allow_extras();

    run();

    EXPECT_EQ(app.remaining(), std::vector<std::string>({"one", "two"}));
    EXPECT_EQ(start->remaining(), std::vector<std::string>({"three", "four"}));
    EXPECT_EQ(app.remaining(true), std::vector<std::string>({"one", "two", "three", "four"}));

    args = {"one", "two", "start", "three", "--", "four"};

    run();

    EXPECT_EQ(app.remaining(), std::vector<std::string>({"one", "two"}));
    EXPECT_EQ(start->remaining(), std::vector<std::string>({"three", "--", "four"}));
    EXPECT_EQ(app.remaining(true), std::vector<std::string>({"one", "two", "three", "--", "four"}));
}

TEST_F(SubcommandProgram, MixedOrderExtras) {

    app.allow_extras();
    start->allow_extras();
    stop->allow_extras();

    args = {"one", "two", "start", "three", "four", "stop", "five", "six"};
    run();

    EXPECT_EQ(app.remaining(), std::vector<std::string>({"one", "two"}));
    EXPECT_EQ(start->remaining(), std::vector<std::string>({"three", "four"}));
    EXPECT_EQ(stop->remaining(), std::vector<std::string>({"five", "six"}));
    EXPECT_EQ(app.remaining(true), std::vector<std::string>({"one", "two", "three", "four", "five", "six"}));

    args = {"one", "two", "stop", "three", "four", "start", "five", "six"};
    run();

    EXPECT_EQ(app.remaining(), std::vector<std::string>({"one", "two"}));
    EXPECT_EQ(stop->remaining(), std::vector<std::string>({"three", "four"}));
    EXPECT_EQ(start->remaining(), std::vector<std::string>({"five", "six"}));
    EXPECT_EQ(app.remaining(true), std::vector<std::string>({"one", "two", "three", "four", "five", "six"}));
}

TEST_F(SubcommandProgram, CallbackOrder) {
    std::vector<int> callback_order;
    start->callback([&callback_order]() { callback_order.push_back(1); });
    stop->callback([&callback_order]() { callback_order.push_back(2); });

    args = {"start", "stop"};
    run();
    EXPECT_EQ(callback_order, std::vector<int>({1, 2}));

    callback_order.clear();

    args = {"stop", "start"};
    run();
    EXPECT_EQ(callback_order, std::vector<int>({2, 1}));
}

struct ManySubcommands : public TApp {

    CLI::App *sub1;
    CLI::App *sub2;
    CLI::App *sub3;
    CLI::App *sub4;

    ManySubcommands() {
        app.allow_extras();
        sub1 = app.add_subcommand("sub1");
        sub2 = app.add_subcommand("sub2");
        sub3 = app.add_subcommand("sub3");
        sub4 = app.add_subcommand("sub4");
        args = {"sub1", "sub2", "sub3"};
    }
};

TEST_F(ManySubcommands, Required1Exact) {
    app.require_subcommand(1);

    run();
    EXPECT_EQ(sub1->remaining(), vs_t({"sub2", "sub3"}));
    EXPECT_EQ(app.remaining(true), vs_t({"sub2", "sub3"}));
}

TEST_F(ManySubcommands, Required2Exact) {
    app.require_subcommand(2);

    run();
    EXPECT_EQ(sub2->remaining(), vs_t({"sub3"}));
}

TEST_F(ManySubcommands, Required4Failure) {
    app.require_subcommand(4);

    EXPECT_THROW(run(), CLI::RequiredError);
}

TEST_F(ManySubcommands, Required1Fuzzy) {

    app.require_subcommand(0, 1);

    run();
    EXPECT_EQ(sub1->remaining(), vs_t({"sub2", "sub3"}));

    app.require_subcommand(-1);

    run();
    EXPECT_EQ(sub1->remaining(), vs_t({"sub2", "sub3"}));
}

TEST_F(ManySubcommands, Required2Fuzzy) {
    app.require_subcommand(0, 2);

    run();
    EXPECT_EQ(sub2->remaining(), vs_t({"sub3"}));
    EXPECT_EQ(app.remaining(true), vs_t({"sub3"}));

    app.require_subcommand(-2);

    run();
    EXPECT_EQ(sub2->remaining(), vs_t({"sub3"}));
}

TEST_F(ManySubcommands, Unlimited) {
    run();
    EXPECT_EQ(app.remaining(true), vs_t());

    app.require_subcommand();

    run();
    EXPECT_EQ(app.remaining(true), vs_t());

    app.require_subcommand(2, 0); // 2 or more

    run();
    EXPECT_EQ(app.remaining(true), vs_t());
}
