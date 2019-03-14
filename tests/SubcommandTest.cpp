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
    EXPECT_EQ(0u, app.get_subcommands().size());

    args = {"sub1"};
    run();
    EXPECT_EQ(sub1, app.get_subcommands().at(0));
    EXPECT_EQ(1u, app.get_subcommands().size());

    app.clear();
    EXPECT_EQ(0u, app.get_subcommands().size());

    args = {"sub2"};
    run();
    EXPECT_EQ(1u, app.get_subcommands().size());
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
    EXPECT_EQ(sub1->count(), 1u);

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
    EXPECT_EQ(sub2->count(), 0u);

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

TEST_F(TApp, DuplicateSubcommands) {

    auto foo = app.add_subcommand("foo");

    args = {"foo", "foo"};
    run();
    EXPECT_TRUE(*foo);
    EXPECT_EQ(foo->count(), 2u);

    args = {"foo", "foo", "foo"};
    run();
    EXPECT_TRUE(*foo);
    EXPECT_EQ(foo->count(), 3u);
}

TEST_F(TApp, DuplicateSubcommandCallbacks) {

    auto foo = app.add_subcommand("foo");
    int count = 0;
    foo->callback([&count]() { ++count; });
    foo->immediate_callback();
    EXPECT_TRUE(foo->get_immediate_callback());
    args = {"foo", "foo"};
    run();
    EXPECT_EQ(count, 2);
    count = 0;
    args = {"foo", "foo", "foo"};
    run();
    EXPECT_EQ(count, 3);
}

TEST_F(TApp, DuplicateSubcommandCallbacksValues) {

    auto foo = app.add_subcommand("foo");
    int val;
    foo->add_option("--val", val);
    std::vector<int> vals;
    foo->callback([&vals, &val]() { vals.push_back(val); });
    foo->immediate_callback();
    args = {"foo", "--val=45", "foo", "--val=27"};
    run();
    EXPECT_EQ(vals.size(), 2u);
    EXPECT_EQ(vals[0], 45);
    EXPECT_EQ(vals[1], 27);
    vals.clear();
    args = {"foo", "--val=45", "foo", "--val=27", "foo", "--val=36"};
    run();
    EXPECT_EQ(vals.size(), 3u);
    EXPECT_EQ(vals[0], 45);
    EXPECT_EQ(vals[1], 27);
    EXPECT_EQ(vals[2], 36);
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

TEST_F(TApp, NoFallThroughOptsWithTerminator) {
    int val = 1;
    app.add_option("--val", val);

    app.add_subcommand("sub");

    args = {"sub", "++", "--val", "2"};
    run();
    EXPECT_EQ(val, 2);
}

TEST_F(TApp, NoFallThroughPositionalsWithTerminator) {
    int val = 1;
    app.add_option("val", val);

    app.add_subcommand("sub");

    args = {"sub", "++", "2"};
    run();
    EXPECT_EQ(val, 2);

    // try with positional only mark
    args = {"sub", "--", "3"};
    run();
    EXPECT_EQ(val, 3);
}

TEST_F(TApp, NamelessSubComPositionals) {

    auto sub = app.add_subcommand();
    int val = 1;
    sub->add_option("val", val);

    args = {"2"};
    run();
    EXPECT_EQ(val, 2);
}

TEST_F(TApp, NamelessSubWithSub) {

    auto sub = app.add_subcommand();
    auto subsub = sub->add_subcommand("val");

    args = {"val"};
    run();
    EXPECT_TRUE(subsub->parsed());
    EXPECT_TRUE(app.got_subcommand("val"));
}

TEST_F(TApp, NamelessSubWithMultipleSub) {

    auto sub1 = app.add_subcommand();
    auto sub2 = app.add_subcommand();
    auto sub1sub1 = sub1->add_subcommand("val1");
    auto sub1sub2 = sub1->add_subcommand("val2");
    auto sub2sub1 = sub2->add_subcommand("val3");
    auto sub2sub2 = sub2->add_subcommand("val4");
    args = {"val1"};
    run();
    EXPECT_TRUE(sub1sub1->parsed());
    EXPECT_TRUE(app.got_subcommand("val1"));

    args = {"val2"};
    run();
    EXPECT_TRUE(sub1sub2->parsed());
    EXPECT_TRUE(app.got_subcommand("val2"));

    args = {"val3"};
    run();
    EXPECT_TRUE(sub2sub1->parsed());
    EXPECT_TRUE(app.got_subcommand("val3"));

    args = {"val4"};
    run();
    EXPECT_TRUE(sub2sub2->parsed());
    EXPECT_TRUE(app.got_subcommand("val4"));

    args = {"val4", "val1"};
    run();
    EXPECT_TRUE(sub2sub2->parsed());
    EXPECT_TRUE(app.got_subcommand("val4"));
    EXPECT_TRUE(sub1sub1->parsed());
    EXPECT_TRUE(app.got_subcommand("val1"));
}

TEST_F(TApp, Nameless4LayerDeep) {

    auto sub = app.add_subcommand();
    auto ssub = sub->add_subcommand();
    auto sssub = ssub->add_subcommand();

    auto ssssub = sssub->add_subcommand();
    auto sssssub = ssssub->add_subcommand("val");

    args = {"val"};
    run();
    EXPECT_TRUE(sssssub->parsed());
    EXPECT_TRUE(app.got_subcommand("val"));
}

/// Put subcommands in some crazy pattern and make everything still works
TEST_F(TApp, Nameless4LayerDeepMulit) {

    auto sub1 = app.add_subcommand();
    auto sub2 = app.add_subcommand();
    auto ssub1 = sub1->add_subcommand();
    auto ssub2 = sub2->add_subcommand();

    auto sssub1 = ssub1->add_subcommand();
    auto sssub2 = ssub2->add_subcommand();
    sssub1->add_subcommand("val1");
    ssub2->add_subcommand("val2");
    sub2->add_subcommand("val3");
    ssub1->add_subcommand("val4");
    sssub2->add_subcommand("val5");
    args = {"val1"};
    run();
    EXPECT_TRUE(app.got_subcommand("val1"));

    args = {"val2"};
    run();
    EXPECT_TRUE(app.got_subcommand("val2"));

    args = {"val3"};
    run();
    EXPECT_TRUE(app.got_subcommand("val3"));

    args = {"val4"};
    run();
    EXPECT_TRUE(app.got_subcommand("val4"));
    args = {"val5"};
    run();
    EXPECT_TRUE(app.got_subcommand("val5"));

    args = {"val4", "val1", "val5"};
    run();
    EXPECT_TRUE(app.got_subcommand("val4"));
    EXPECT_TRUE(app.got_subcommand("val1"));
    EXPECT_TRUE(app.got_subcommand("val5"));
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

TEST_F(TApp, CallbackOrderingImmediate) {
    app.fallthrough();
    int val = 1, sub_val = 0;
    app.add_option("--val", val);

    auto sub = app.add_subcommand("sub")->immediate_callback();
    sub->callback([&val, &sub_val]() { sub_val = val; });

    args = {"sub", "--val=2"};
    run();
    EXPECT_EQ(2, val);
    EXPECT_EQ(1, sub_val);

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
    EXPECT_THROW(app.get_subcommand_ptr(two), CLI::OptionNotFound);
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

    EXPECT_EQ(app.remaining_size(), 0u);
    EXPECT_EQ(app.remaining_size(true), 3u);
    EXPECT_EQ(subc->remaining(), std::vector<std::string>({"other", "--simple", "--mine"}));
}

TEST_F(TApp, InheritHelpAllFlag) {
    app.set_help_all_flag("--help-all");
    auto subc = app.add_subcommand("subc");
    auto help_opt_list = subc->get_options([](const CLI::Option *opt) { return opt->get_name() == "--help-all"; });
    EXPECT_EQ(help_opt_list.size(), 1u);
}

TEST_F(TApp, RequiredPosInSubcommand) {
    app.require_subcommand();
    std::string bar;

    CLI::App *fooApp = app.add_subcommand("foo", "Foo a bar");
    fooApp->add_option("bar", bar, "A bar to foo")->required();

    CLI::App *bazApp = app.add_subcommand("baz", "Baz a bar");
    bazApp->add_option("bar", bar, "A bar a baz")->required();

    args = {"foo", "abc"};
    run();
    EXPECT_EQ(bar, "abc");
    args = {"baz", "cba"};
    run();
    EXPECT_EQ(bar, "cba");

    args = {};
    EXPECT_THROW(run(), CLI::RequiredError);
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
    EXPECT_EQ(2u, app.get_subcommands().size());
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

    EXPECT_EQ(2u, app.get_subcommands().size());
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
    EXPECT_EQ(0u, app.get_subcommands().size());
    EXPECT_EQ(2u, app.get_subcommands({}).size());
    EXPECT_EQ(1u, app.get_subcommands([](const CLI::App *s) { return s->get_name() == "sub1"; }).size());

    args = {"SuB1"};
    run();
    EXPECT_EQ(sub1, app.get_subcommands().at(0));
    EXPECT_EQ(1u, app.get_subcommands().size());

    app.clear();
    EXPECT_EQ(0u, app.get_subcommands().size());

    args = {"sUb2"};
    run();
    EXPECT_EQ(sub2, app.get_subcommands().at(0));
}

TEST_F(SubcommandProgram, UnderscoreCheck) {
    args = {"start_"};
    EXPECT_THROW(run(), CLI::ExtrasError);

    args = {"start"};
    run();

    start->ignore_underscore();
    run();

    args = {"_start_"};
    run();
}

TEST_F(TApp, SubcomInheritUnderscoreCheck) {
    app.ignore_underscore();
    auto sub1 = app.add_subcommand("sub_option1");
    auto sub2 = app.add_subcommand("sub_option2");

    run();
    EXPECT_EQ(0u, app.get_subcommands().size());
    EXPECT_EQ(2u, app.get_subcommands({}).size());
    EXPECT_EQ(1u, app.get_subcommands([](const CLI::App *s) { return s->get_name() == "sub_option1"; }).size());

    args = {"suboption1"};
    run();
    EXPECT_EQ(sub1, app.get_subcommands().at(0));
    EXPECT_EQ(1u, app.get_subcommands().size());

    app.clear();
    EXPECT_EQ(0u, app.get_subcommands().size());

    args = {"_suboption2"};
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

    EXPECT_EQ(app.remaining(), std::vector<std::string>({"one", "two", "four"}));
    EXPECT_EQ(start->remaining(), std::vector<std::string>({"three"}));
    EXPECT_EQ(app.remaining(true), std::vector<std::string>({"one", "two", "four", "three"}));
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

TEST_F(SubcommandProgram, CallbackOrderImmediate) {
    std::vector<int> callback_order;
    start->callback([&callback_order]() { callback_order.push_back(1); })->immediate_callback();
    stop->callback([&callback_order]() { callback_order.push_back(2); });

    args = {"start", "stop", "start"};
    run();
    EXPECT_EQ(callback_order, std::vector<int>({1, 1, 2}));

    callback_order.clear();

    args = {"stop", "start", "stop", "start"};
    run();
    EXPECT_EQ(callback_order, std::vector<int>({1, 1, 2}));
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

TEST_F(ManySubcommands, RemoveSub) {
    run();
    EXPECT_EQ(app.remaining_size(true), 0u);
    app.remove_subcommand(sub1);
    app.allow_extras();
    run();
    EXPECT_EQ(app.remaining_size(true), 1u);
}

TEST_F(ManySubcommands, RemoveSubFail) {
    auto sub_sub = sub1->add_subcommand("subsub");
    EXPECT_FALSE(app.remove_subcommand(sub_sub));
    EXPECT_TRUE(sub1->remove_subcommand(sub_sub));
    EXPECT_FALSE(app.remove_subcommand(nullptr));
}

TEST_F(ManySubcommands, manyIndexQuery) {
    auto s1 = app.get_subcommand(0);
    auto s2 = app.get_subcommand(1);
    auto s3 = app.get_subcommand(2);
    auto s4 = app.get_subcommand(3);
    EXPECT_EQ(s1, sub1);
    EXPECT_EQ(s2, sub2);
    EXPECT_EQ(s3, sub3);
    EXPECT_EQ(s4, sub4);
    EXPECT_THROW(app.get_subcommand(4), CLI::OptionNotFound);
    auto s0 = app.get_subcommand();
    EXPECT_EQ(s0, sub1);
}

TEST_F(ManySubcommands, manyIndexQueryPtr) {
    auto s1 = app.get_subcommand_ptr(0);
    auto s2 = app.get_subcommand_ptr(1);
    auto s3 = app.get_subcommand_ptr(2);
    auto s4 = app.get_subcommand_ptr(3);
    EXPECT_EQ(s1.get(), sub1);
    EXPECT_EQ(s2.get(), sub2);
    EXPECT_EQ(s3.get(), sub3);
    EXPECT_EQ(s4.get(), sub4);
    EXPECT_THROW(app.get_subcommand_ptr(4), CLI::OptionNotFound);
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

TEST_F(ManySubcommands, HelpFlags) {

    args = {"-h"};

    EXPECT_THROW(run(), CLI::CallForHelp);

    args = {"sub2", "-h"};

    EXPECT_THROW(run(), CLI::CallForHelp);

    args = {"-h", "sub2"};

    EXPECT_THROW(run(), CLI::CallForHelp);
}

TEST_F(ManySubcommands, MaxCommands) {

    app.require_subcommand(2);

    args = {"sub1", "sub2"};
    EXPECT_NO_THROW(run());

    // The extra subcommand counts as an extra
    args = {"sub1", "sub2", "sub3"};
    EXPECT_NO_THROW(run());
    EXPECT_EQ(sub2->remaining().size(), 1u);
    EXPECT_EQ(app.count_all(), 2u);

    // Currently, setting sub2 to throw causes an extras error
    // In the future, would passing on up to app's extras be better?

    app.allow_extras(false);
    sub1->allow_extras(false);
    sub2->allow_extras(false);

    args = {"sub1", "sub2"};

    EXPECT_NO_THROW(run());

    args = {"sub1", "sub2", "sub3"};
    EXPECT_THROW(run(), CLI::ExtrasError);
}

TEST_F(ManySubcommands, SubcommandExclusion) {

    sub1->excludes(sub3);
    sub2->excludes(sub3);
    args = {"sub1", "sub2"};
    EXPECT_NO_THROW(run());

    args = {"sub1", "sub2", "sub3"};
    EXPECT_THROW(run(), CLI::ExcludesError);

    args = {"sub1", "sub2", "sub4"};
    EXPECT_NO_THROW(run());
    EXPECT_EQ(app.count_all(), 3u);

    args = {"sub3", "sub4"};
    EXPECT_NO_THROW(run());
}

TEST_F(ManySubcommands, SubcommandOptionExclusion) {

    auto excluder_flag = app.add_flag("--exclude");
    sub1->excludes(excluder_flag)->fallthrough();
    sub2->excludes(excluder_flag)->fallthrough();
    sub3->fallthrough();
    sub4->fallthrough();
    args = {"sub3", "sub4", "--exclude"};
    EXPECT_NO_THROW(run());

    args = {"sub1", "sub3", "--exclude"};
    EXPECT_THROW(run(), CLI::ExcludesError);
    EXPECT_TRUE(sub1->remove_excludes(excluder_flag));
    EXPECT_NO_THROW(run());
    EXPECT_FALSE(sub1->remove_excludes(excluder_flag));

    args = {"--exclude", "sub2", "sub4"};
    EXPECT_THROW(run(), CLI::ExcludesError);
    EXPECT_EQ(sub1->excludes(excluder_flag), sub1);
    args = {"sub1", "--exclude", "sub2", "sub4"};
    try {
        run();
    } catch(const CLI::ExcludesError &ee) {
        EXPECT_NE(std::string(ee.what()).find("sub1"), std::string::npos);
    }
}

TEST_F(ManySubcommands, SubcommandRequired) {

    sub1->required();
    args = {"sub1", "sub2"};
    EXPECT_NO_THROW(run());

    args = {"sub1", "sub2", "sub3"};
    EXPECT_NO_THROW(run());

    args = {"sub3", "sub4"};
    EXPECT_THROW(run(), CLI::RequiredError);
}

TEST_F(ManySubcommands, SubcommandDisabled) {

    sub3->disabled();
    args = {"sub1", "sub2"};
    EXPECT_NO_THROW(run());

    args = {"sub1", "sub2", "sub3"};
    app.allow_extras(false);
    sub2->allow_extras(false);
    EXPECT_THROW(run(), CLI::ExtrasError);
    args = {"sub3", "sub4"};
    EXPECT_THROW(run(), CLI::ExtrasError);
    sub3->disabled(false);
    args = {"sub3", "sub4"};
    EXPECT_NO_THROW(run());
}

TEST_F(ManySubcommands, SubcommandTriggeredOff) {

    app.allow_extras(false);
    sub1->allow_extras(false);
    sub2->allow_extras(false);
    CLI::TriggerOff(sub1, sub2);
    args = {"sub1", "sub2"};
    EXPECT_THROW(run(), CLI::ExtrasError);

    args = {"sub2", "sub1", "sub3"};
    EXPECT_NO_THROW(run());
    CLI::TriggerOff(sub1, {sub3, sub4});
    EXPECT_THROW(run(), CLI::ExtrasError);
    args = {"sub1", "sub2", "sub4"};
    EXPECT_THROW(run(), CLI::ExtrasError);
}

TEST_F(ManySubcommands, SubcommandTriggeredOn) {

    app.allow_extras(false);
    sub1->allow_extras(false);
    sub2->allow_extras(false);
    CLI::TriggerOn(sub1, sub2);
    args = {"sub1", "sub2"};
    EXPECT_NO_THROW(run());

    args = {"sub2", "sub1", "sub4"};
    EXPECT_THROW(run(), CLI::ExtrasError);
    CLI::TriggerOn(sub1, {sub3, sub4});
    sub2->disabled_by_default(false);
    sub2->disabled(false);
    EXPECT_NO_THROW(run());
    args = {"sub3", "sub1", "sub2"};
    EXPECT_THROW(run(), CLI::ExtrasError);
}

TEST_F(TApp, UnnamedSub) {
    double val;
    auto sub = app.add_subcommand("", "empty name");
    auto opt = sub->add_option("-v,--value", val);
    args = {"-v", "4.56"};

    run();
    EXPECT_EQ(val, 4.56);
    // make sure unnamed sub options can be found from the main app
    auto opt2 = app.get_option("-v");
    EXPECT_EQ(opt, opt2);

    EXPECT_THROW(app.get_option("--vvvv"), CLI::OptionNotFound);
    // now test in the constant context
    const auto &appC = app;
    auto opt3 = appC.get_option("-v");
    EXPECT_EQ(opt3->get_name(), "--value");
    EXPECT_THROW(appC.get_option("--vvvv"), CLI::OptionNotFound);
}

TEST_F(TApp, UnnamedSubMix) {
    double val, val2, val3;
    app.add_option("-t", val2);
    auto sub1 = app.add_subcommand("", "empty name");
    sub1->add_option("-v,--value", val);
    auto sub2 = app.add_subcommand("", "empty name2");
    sub2->add_option("-m,--mix", val3);
    args = {"-m", "4.56", "-t", "5.93", "-v", "-3"};

    run();
    EXPECT_EQ(val, -3.0);
    EXPECT_EQ(val2, 5.93);
    EXPECT_EQ(val3, 4.56);
    EXPECT_EQ(app.count_all(), 3u);
}

TEST_F(TApp, UnnamedSubMixExtras) {
    double val, val2;
    app.add_option("-t", val2);
    auto sub = app.add_subcommand("", "empty name");
    sub->add_option("-v,--value", val);
    args = {"-m", "4.56", "-t", "5.93", "-v", "-3"};
    app.allow_extras();
    run();
    EXPECT_EQ(val, -3.0);
    EXPECT_EQ(val2, 5.93);
    EXPECT_EQ(app.remaining_size(), 2u);
    EXPECT_EQ(sub->remaining_size(), 0u);
}

TEST_F(TApp, UnnamedSubNoExtras) {
    double val, val2;
    app.add_option("-t", val2);
    auto sub = app.add_subcommand();
    sub->add_option("-v,--value", val);
    args = {"-t", "5.93", "-v", "-3"};
    run();
    EXPECT_EQ(val, -3.0);
    EXPECT_EQ(val2, 5.93);
    EXPECT_EQ(app.remaining_size(), 0u);
    EXPECT_EQ(sub->remaining_size(), 0u);
}

TEST(SharedSubTests, SharedSubcommand) {
    double val, val2, val3, val4;
    CLI::App app1{"test program1"};

    app1.add_option("-t", val2);
    auto sub = app1.add_subcommand("", "empty name");
    sub->add_option("-v,--value", val);
    sub->add_option("-g", val4);
    CLI::App app2{"test program2"};
    app2.add_option("-m", val3);
    // extract an owning ptr from app1 and add it to app2
    auto subown = app1.get_subcommand_ptr(sub);
    // add the extracted subcommand to a different app
    app2.add_subcommand(std::move(subown));
    EXPECT_THROW(app2.add_subcommand(CLI::App_p{}), CLI::IncorrectConstruction);
    input_t args1 = {"-m", "4.56", "-t", "5.93", "-v", "-3"};
    input_t args2 = {"-m", "4.56", "-g", "8.235"};
    std::reverse(std::begin(args1), std::end(args1));
    std::reverse(std::begin(args2), std::end(args2));
    app1.allow_extras();
    app1.parse(args1);

    app2.parse(args2);

    EXPECT_EQ(val, -3.0);
    EXPECT_EQ(val2, 5.93);
    EXPECT_EQ(val3, 4.56);
    EXPECT_EQ(val4, 8.235);
}

TEST(SharedSubTests, SharedSubIndependent) {
    double val, val2, val4;
    CLI::App_p app1 = std::make_shared<CLI::App>("test program1");
    app1->allow_extras();
    app1->add_option("-t", val2);
    auto sub = app1->add_subcommand("", "empty name");
    sub->add_option("-v,--value", val);
    sub->add_option("-g", val4);

    // extract an owning ptr from app1 and add it to app2
    auto subown = app1->get_subcommand_ptr(sub);

    input_t args1 = {"-m", "4.56", "-t", "5.93", "-v", "-3"};
    input_t args2 = {"-m", "4.56", "-g", "8.235"};
    std::reverse(std::begin(args1), std::end(args1));
    std::reverse(std::begin(args2), std::end(args2));

    app1->parse(args1);
    // destroy the first parser
    app1 = nullptr;
    // parse with the extracted subcommand
    subown->parse(args2);

    EXPECT_EQ(val, -3.0);
    EXPECT_EQ(val2, 5.93);
    EXPECT_EQ(val4, 8.235);
}

TEST(SharedSubTests, SharedSubIndependentReuse) {
    double val, val2, val4;
    CLI::App_p app1 = std::make_shared<CLI::App>("test program1");
    app1->allow_extras();
    app1->add_option("-t", val2);
    auto sub = app1->add_subcommand("", "empty name");
    sub->add_option("-v,--value", val);
    sub->add_option("-g", val4);

    // extract an owning ptr from app1 and add it to app2
    auto subown = app1->get_subcommand_ptr(sub);

    input_t args1 = {"-m", "4.56", "-t", "5.93", "-v", "-3"};
    std::reverse(std::begin(args1), std::end(args1));
    auto args2 = args1;
    app1->parse(args1);

    // parse with the extracted subcommand
    subown->parse("program1 -m 4.56 -g 8.235", true);

    EXPECT_EQ(val, -3.0);
    EXPECT_EQ(val2, 5.93);
    EXPECT_EQ(val4, 8.235);
    val = 0.0;
    val2 = 0.0;
    EXPECT_EQ(subown->get_name(), "program1");
    // this tests the name reset in subcommand since it was automatic
    app1->parse(args2);
    EXPECT_EQ(val, -3.0);
    EXPECT_EQ(val2, 5.93);
}

TEST_F(ManySubcommands, getSubtests) {
    CLI::App_p sub2p = app.get_subcommand_ptr(sub2);
    EXPECT_EQ(sub2p.get(), sub2);
    EXPECT_THROW(app.get_subcommand_ptr(nullptr), CLI::OptionNotFound);
    EXPECT_THROW(app.get_subcommand(nullptr), CLI::OptionNotFound);
    CLI::App_p sub3p = app.get_subcommand_ptr(2);
    EXPECT_EQ(sub3p.get(), sub3);
}

TEST_F(ManySubcommands, defaultDisabledSubcommand) {

    sub1->fallthrough();
    sub2->disabled_by_default();
    run();
    auto rem = app.remaining();
    EXPECT_EQ(rem.size(), 1u);
    EXPECT_EQ(rem[0], "sub2");
    EXPECT_TRUE(sub2->get_disabled_by_default());
    sub2->disabled(false);
    EXPECT_FALSE(sub2->get_disabled());
    run();
    // this should disable it again even though it was disabled
    rem = app.remaining();
    EXPECT_EQ(rem.size(), 1u);
    EXPECT_EQ(rem[0], "sub2");
    EXPECT_TRUE(sub2->get_disabled_by_default());
    EXPECT_TRUE(sub2->get_disabled());
}

TEST_F(ManySubcommands, defaultEnabledSubcommand) {

    sub2->enabled_by_default();
    run();
    auto rem = app.remaining();
    EXPECT_EQ(rem.size(), 0u);
    EXPECT_TRUE(sub2->get_enabled_by_default());
    sub2->disabled();
    EXPECT_TRUE(sub2->get_disabled());
    run();
    // this should disable it again even though it was disabled
    rem = app.remaining();
    EXPECT_EQ(rem.size(), 0u);
    EXPECT_TRUE(sub2->get_enabled_by_default());
    EXPECT_FALSE(sub2->get_disabled());
}
