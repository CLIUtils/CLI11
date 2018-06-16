#include "app_helper.hpp"
#include <cstdlib>

TEST_F(TApp, AddingExistingShort) {
    CLI::Option *opt = app.add_flag("-c,--count");
    EXPECT_EQ(opt->get_lnames(), std::vector<std::string>({"count"}));
    EXPECT_EQ(opt->get_snames(), std::vector<std::string>({"c"}));

    EXPECT_THROW(app.add_flag("--cat,-c"), CLI::OptionAlreadyAdded);
}

TEST_F(TApp, AddingExistingLong) {
    app.add_flag("-q,--count");
    EXPECT_THROW(app.add_flag("--count,-c"), CLI::OptionAlreadyAdded);
}

TEST_F(TApp, AddingExistingShortNoCase) {
    app.add_flag("-C,--count")->ignore_case();
    EXPECT_THROW(app.add_flag("--cat,-c"), CLI::OptionAlreadyAdded);
}

TEST_F(TApp, AddingExistingLongNoCase) {
    app.add_flag("-q,--count")->ignore_case();
    EXPECT_THROW(app.add_flag("--Count,-c"), CLI::OptionAlreadyAdded);
}

TEST_F(TApp, AddingExistingNoCaseReversed) {
    app.add_flag("-c,--count")->ignore_case();
    EXPECT_THROW(app.add_flag("--cat,-C"), CLI::OptionAlreadyAdded);
}

TEST_F(TApp, AddingExistingWithCase) {
    app.add_flag("-c,--count");
    EXPECT_NO_THROW(app.add_flag("--Cat,-C"));
}

TEST_F(TApp, AddingExistingWithCaseAfter) {
    auto count = app.add_flag("-c,--count");
    app.add_flag("--Cat,-C");

    EXPECT_THROW(count->ignore_case(), CLI::OptionAlreadyAdded);
}

TEST_F(TApp, AddingExistingWithCaseAfter2) {
    app.add_flag("-c,--count");
    auto cat = app.add_flag("--Cat,-C");

    EXPECT_THROW(cat->ignore_case(), CLI::OptionAlreadyAdded);
}

TEST_F(TApp, AddingMultipleInfPositionals) {
    std::vector<std::string> one, two;
    app.add_option("one", one);
    app.add_option("two", two);

    EXPECT_THROW(run(), CLI::InvalidError);
}

TEST_F(TApp, AddingMultipleInfPositionalsSubcom) {
    std::vector<std::string> one, two;
    CLI::App *below = app.add_subcommand("below");
    below->add_option("one", one);
    below->add_option("two", two);

    EXPECT_THROW(run(), CLI::InvalidError);
}

TEST_F(TApp, MultipleSubcomMatching) {
    app.add_subcommand("first");
    app.add_subcommand("second");
    app.add_subcommand("Second");
    EXPECT_THROW(app.add_subcommand("first"), CLI::OptionAlreadyAdded);
}

TEST_F(TApp, RecoverSubcommands) {
    CLI::App *app1 = app.add_subcommand("app1");
    CLI::App *app2 = app.add_subcommand("app2");
    CLI::App *app3 = app.add_subcommand("app3");
    CLI::App *app4 = app.add_subcommand("app4");

    EXPECT_EQ(app.get_subcommands({}), std::vector<CLI::App *>({app1, app2, app3, app4}));
}

TEST_F(TApp, MultipleSubcomMatchingWithCase) {
    app.add_subcommand("first")->ignore_case();
    EXPECT_THROW(app.add_subcommand("fIrst"), CLI::OptionAlreadyAdded);
}

TEST_F(TApp, MultipleSubcomMatchingWithCaseFirst) {
    app.ignore_case();
    app.add_subcommand("first");
    EXPECT_THROW(app.add_subcommand("fIrst"), CLI::OptionAlreadyAdded);
}

TEST_F(TApp, MultipleSubcomMatchingWithCaseInplace) {
    app.add_subcommand("first");
    auto first = app.add_subcommand("fIrst");

    EXPECT_THROW(first->ignore_case(), CLI::OptionAlreadyAdded);
}

TEST_F(TApp, MultipleSubcomMatchingWithCaseInplace2) {
    auto first = app.add_subcommand("first");
    app.add_subcommand("fIrst");

    EXPECT_THROW(first->ignore_case(), CLI::OptionAlreadyAdded);
}

TEST_F(TApp, MultipleSubcomNoMatchingInplace2) {
    auto first = app.add_subcommand("first");
    auto second = app.add_subcommand("second");

    EXPECT_NO_THROW(first->ignore_case());
    EXPECT_NO_THROW(second->ignore_case());
}

TEST_F(TApp, IncorrectConstructionFlagPositional1) { EXPECT_THROW(app.add_flag("cat"), CLI::IncorrectConstruction); }

TEST_F(TApp, IncorrectConstructionFlagPositional2) {
    int x;
    EXPECT_THROW(app.add_flag("cat", x), CLI::IncorrectConstruction);
}

TEST_F(TApp, IncorrectConstructionFlagPositional3) {
    bool x;
    EXPECT_THROW(app.add_flag("cat", x), CLI::IncorrectConstruction);
}

TEST_F(TApp, IncorrectConstructionFlagExpected) {
    auto cat = app.add_flag("--cat");
    EXPECT_THROW(cat->expected(0), CLI::IncorrectConstruction);
    EXPECT_THROW(cat->expected(1), CLI::IncorrectConstruction);
}

TEST_F(TApp, IncorrectConstructionOptionAsFlag) {
    int x;
    auto cat = app.add_option("--cat", x);
    EXPECT_NO_THROW(cat->expected(1));
    EXPECT_THROW(cat->expected(0), CLI::IncorrectConstruction);
    EXPECT_THROW(cat->expected(2), CLI::IncorrectConstruction);
}

TEST_F(TApp, IncorrectConstructionOptionAsVector) {
    int x;
    auto cat = app.add_option("--cat", x);
    EXPECT_THROW(cat->expected(2), CLI::IncorrectConstruction);
}

TEST_F(TApp, IncorrectConstructionVectorAsFlag) {
    std::vector<int> x;
    auto cat = app.add_option("--cat", x);
    EXPECT_THROW(cat->expected(0), CLI::IncorrectConstruction);
}

TEST_F(TApp, IncorrectConstructionVectorTakeLast) {
    std::vector<int> vec;
    auto cat = app.add_option("--vec", vec);
    EXPECT_THROW(cat->multi_option_policy(CLI::MultiOptionPolicy::TakeLast), CLI::IncorrectConstruction);
}

TEST_F(TApp, IncorrectConstructionTakeLastExpected) {
    std::vector<int> vec;
    auto cat = app.add_option("--vec", vec);
    cat->expected(1);
    ASSERT_NO_THROW(cat->multi_option_policy(CLI::MultiOptionPolicy::TakeLast));
    EXPECT_THROW(cat->expected(2), CLI::IncorrectConstruction);
}

TEST_F(TApp, IncorrectConstructionNeedsCannotFind) {
    auto cat = app.add_flag("--cat");
    EXPECT_THROW(cat->needs("--nothing"), CLI::IncorrectConstruction);
}

TEST_F(TApp, IncorrectConstructionExcludesCannotFind) {
    auto cat = app.add_flag("--cat");
    EXPECT_THROW(cat->excludes("--nothing"), CLI::IncorrectConstruction);
}

TEST_F(TApp, IncorrectConstructionDuplicateNeeds) {
    auto cat = app.add_flag("--cat");
    auto other = app.add_flag("--other");
    ASSERT_NO_THROW(cat->needs(other));
    EXPECT_THROW(cat->needs(other), CLI::OptionAlreadyAdded);
}

TEST_F(TApp, IncorrectConstructionDuplicateNeedsTxt) {
    auto cat = app.add_flag("--cat");
    app.add_flag("--other");
    ASSERT_NO_THROW(cat->needs("--other"));
    EXPECT_THROW(cat->needs("--other"), CLI::OptionAlreadyAdded);
}

// Now allowed
TEST_F(TApp, CorrectConstructionDuplicateExcludes) {
    auto cat = app.add_flag("--cat");
    auto other = app.add_flag("--other");
    ASSERT_NO_THROW(cat->excludes(other));
    ASSERT_NO_THROW(other->excludes(cat));
}

// Now allowed
TEST_F(TApp, CorrectConstructionDuplicateExcludesTxt) {
    auto cat = app.add_flag("--cat");
    auto other = app.add_flag("--other");
    ASSERT_NO_THROW(cat->excludes("--other"));
    ASSERT_NO_THROW(other->excludes("--cat"));
}

TEST_F(TApp, CheckName) {
    auto long1 = app.add_flag("--long1");
    auto long2 = app.add_flag("--Long2");
    auto short1 = app.add_flag("-a");
    auto short2 = app.add_flag("-B");
    int x, y;
    auto pos1 = app.add_option("pos1", x);
    auto pos2 = app.add_option("pOs2", y);

    EXPECT_TRUE(long1->check_name("--long1"));
    EXPECT_FALSE(long1->check_name("--lonG1"));

    EXPECT_TRUE(long2->check_name("--Long2"));
    EXPECT_FALSE(long2->check_name("--long2"));

    EXPECT_TRUE(short1->check_name("-a"));
    EXPECT_FALSE(short1->check_name("-A"));

    EXPECT_TRUE(short2->check_name("-B"));
    EXPECT_FALSE(short2->check_name("-b"));

    EXPECT_TRUE(pos1->check_name("pos1"));
    EXPECT_FALSE(pos1->check_name("poS1"));

    EXPECT_TRUE(pos2->check_name("pOs2"));
    EXPECT_FALSE(pos2->check_name("pos2"));
}

TEST_F(TApp, CheckNameNoCase) {
    auto long1 = app.add_flag("--long1")->ignore_case();
    auto long2 = app.add_flag("--Long2")->ignore_case();
    auto short1 = app.add_flag("-a")->ignore_case();
    auto short2 = app.add_flag("-B")->ignore_case();
    int x, y;
    auto pos1 = app.add_option("pos1", x)->ignore_case();
    auto pos2 = app.add_option("pOs2", y)->ignore_case();

    EXPECT_TRUE(long1->check_name("--long1"));
    EXPECT_TRUE(long1->check_name("--lonG1"));

    EXPECT_TRUE(long2->check_name("--Long2"));
    EXPECT_TRUE(long2->check_name("--long2"));

    EXPECT_TRUE(short1->check_name("-a"));
    EXPECT_TRUE(short1->check_name("-A"));

    EXPECT_TRUE(short2->check_name("-B"));
    EXPECT_TRUE(short2->check_name("-b"));

    EXPECT_TRUE(pos1->check_name("pos1"));
    EXPECT_TRUE(pos1->check_name("poS1"));

    EXPECT_TRUE(pos2->check_name("pOs2"));
    EXPECT_TRUE(pos2->check_name("pos2"));
}

TEST_F(TApp, PreSpaces) {
    int x;
    auto myapp = app.add_option(" -a, --long, other", x);

    EXPECT_TRUE(myapp->check_lname("long"));
    EXPECT_TRUE(myapp->check_sname("a"));
    EXPECT_TRUE(myapp->check_name("other"));
}

TEST_F(TApp, AllSpaces) {
    int x;
    auto myapp = app.add_option(" -a , --long , other ", x);

    EXPECT_TRUE(myapp->check_lname("long"));
    EXPECT_TRUE(myapp->check_sname("a"));
    EXPECT_TRUE(myapp->check_name("other"));
}

TEST_F(TApp, OptionFromDefaults) {
    app.option_defaults()->required();

    // Options should remember defaults
    int x;
    auto opt = app.add_option("--simple", x);
    EXPECT_TRUE(opt->get_required());

    // Flags cannot be required
    auto flag = app.add_flag("--other");
    EXPECT_FALSE(flag->get_required());

    app.option_defaults()->required(false);
    auto opt2 = app.add_option("--simple2", x);
    EXPECT_FALSE(opt2->get_required());

    app.option_defaults()->required()->ignore_case();

    auto opt3 = app.add_option("--simple3", x);
    EXPECT_TRUE(opt3->get_required());
    EXPECT_TRUE(opt3->get_ignore_case());
}

TEST_F(TApp, OptionFromDefaultsSubcommands) {
    // Initial defaults
    EXPECT_FALSE(app.option_defaults()->get_required());
    EXPECT_EQ(app.option_defaults()->get_multi_option_policy(), CLI::MultiOptionPolicy::Throw);
    EXPECT_FALSE(app.option_defaults()->get_ignore_case());
    EXPECT_TRUE(app.option_defaults()->get_configurable());
    EXPECT_EQ(app.option_defaults()->get_group(), "Options");

    app.option_defaults()
        ->required()
        ->multi_option_policy(CLI::MultiOptionPolicy::TakeLast)
        ->ignore_case()
        ->configurable(false)
        ->group("Something");

    auto app2 = app.add_subcommand("app2");

    EXPECT_TRUE(app2->option_defaults()->get_required());
    EXPECT_EQ(app2->option_defaults()->get_multi_option_policy(), CLI::MultiOptionPolicy::TakeLast);
    EXPECT_TRUE(app2->option_defaults()->get_ignore_case());
    EXPECT_FALSE(app2->option_defaults()->get_configurable());
    EXPECT_EQ(app2->option_defaults()->get_group(), "Something");
}

TEST_F(TApp, GetNameCheck) {
    int x;
    auto a = app.add_flag("--that");
    auto b = app.add_flag("-x");
    auto c = app.add_option("pos", x);
    auto d = app.add_option("one,-o,--other", x);

    EXPECT_EQ(a->get_name(false, true), "--that");
    EXPECT_EQ(b->get_name(false, true), "-x");
    EXPECT_EQ(c->get_name(false, true), "pos");

    EXPECT_EQ(d->get_name(), "--other");
    EXPECT_EQ(d->get_name(false, false), "--other");
    EXPECT_EQ(d->get_name(false, true), "-o,--other");
    EXPECT_EQ(d->get_name(true, true), "one,-o,--other");
    EXPECT_EQ(d->get_name(true, false), "one");
}

TEST_F(TApp, SubcommandDefaults) {
    // allow_extras, prefix_command, ignore_case, fallthrough, group, min/max subcommand

    // Initial defaults
    EXPECT_FALSE(app.get_allow_extras());
    EXPECT_FALSE(app.get_prefix_command());
    EXPECT_FALSE(app.get_ignore_case());
    EXPECT_FALSE(app.get_fallthrough());
    EXPECT_EQ(app.get_footer(), "");
    EXPECT_EQ(app.get_group(), "Subcommands");
    EXPECT_EQ(app.get_require_subcommand_min(), (size_t)0);
    EXPECT_EQ(app.get_require_subcommand_max(), (size_t)0);

    app.allow_extras();
    app.prefix_command();
    app.ignore_case();
    app.fallthrough();
    app.footer("footy");
    app.group("Stuff");
    app.require_subcommand(2, 3);

    auto app2 = app.add_subcommand("app2");

    // Initial defaults
    EXPECT_TRUE(app2->get_allow_extras());
    EXPECT_TRUE(app2->get_prefix_command());
    EXPECT_TRUE(app2->get_ignore_case());
    EXPECT_TRUE(app2->get_fallthrough());
    EXPECT_EQ(app2->get_footer(), "footy");
    EXPECT_EQ(app2->get_group(), "Stuff");
    EXPECT_EQ(app2->get_require_subcommand_min(), (size_t)0);
    EXPECT_EQ(app2->get_require_subcommand_max(), (size_t)3);
}

TEST_F(TApp, SubcommandMinMax) {

    EXPECT_EQ(app.get_require_subcommand_min(), (size_t)0);
    EXPECT_EQ(app.get_require_subcommand_max(), (size_t)0);

    app.require_subcommand();

    EXPECT_EQ(app.get_require_subcommand_min(), (size_t)1);
    EXPECT_EQ(app.get_require_subcommand_max(), (size_t)0);

    app.require_subcommand(2);

    EXPECT_EQ(app.get_require_subcommand_min(), (size_t)2);
    EXPECT_EQ(app.get_require_subcommand_max(), (size_t)2);

    app.require_subcommand(0);

    EXPECT_EQ(app.get_require_subcommand_min(), (size_t)0);
    EXPECT_EQ(app.get_require_subcommand_max(), (size_t)0);

    app.require_subcommand(-2);

    EXPECT_EQ(app.get_require_subcommand_min(), (size_t)0);
    EXPECT_EQ(app.get_require_subcommand_max(), (size_t)2);

    app.require_subcommand(3, 7);

    EXPECT_EQ(app.get_require_subcommand_min(), (size_t)3);
    EXPECT_EQ(app.get_require_subcommand_max(), (size_t)7);
}

TEST_F(TApp, GetOptionList) {
    int two;
    auto flag = app.add_flag("--one");
    auto opt = app.add_option("--two", two);

    auto opt_list = app.get_options();

    ASSERT_EQ(opt_list.size(), static_cast<size_t>(3));
    EXPECT_EQ(opt_list.at(1), flag);
    EXPECT_EQ(opt_list.at(2), opt);
}
