#include "app_helper.hpp"
#include <cstdlib>

TEST_F(TApp, AddingExistingShort) {
    app.add_flag("-c,--count");
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
    EXPECT_THROW(cat->expected(1), CLI::IncorrectConstruction);
}

TEST_F(TApp, IncorrectConstructionOptionAsFlag) {
    int x;
    auto cat = app.add_option("--cat", x);
    EXPECT_THROW(cat->expected(0), CLI::IncorrectConstruction);
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

TEST_F(TApp, IncorrectConstructionRequiresCannotFind) {
    auto cat = app.add_flag("--cat");
    EXPECT_THROW(cat->requires("--nothing"), CLI::IncorrectConstruction);
}

TEST_F(TApp, IncorrectConstructionExcludesCannotFind) {
    auto cat = app.add_flag("--cat");
    EXPECT_THROW(cat->excludes("--nothing"), CLI::IncorrectConstruction);
}

TEST_F(TApp, IncorrectConstructionDuplicateRequires) {
    auto cat = app.add_flag("--cat");
    auto other = app.add_flag("--other");
    ASSERT_NO_THROW(cat->requires(other));
    EXPECT_THROW(cat->requires(other), CLI::OptionAlreadyAdded);
}

TEST_F(TApp, IncorrectConstructionDuplicateRequiresTxt) {
    auto cat = app.add_flag("--cat");
    app.add_flag("--other");
    ASSERT_NO_THROW(cat->requires("--other"));
    EXPECT_THROW(cat->requires("--other"), CLI::OptionAlreadyAdded);
}

TEST_F(TApp, IncorrectConstructionDuplicateExcludes) {
    auto cat = app.add_flag("--cat");
    auto other = app.add_flag("--other");
    ASSERT_NO_THROW(cat->excludes(other));
    EXPECT_THROW(cat->excludes(other), CLI::OptionAlreadyAdded);
}

TEST_F(TApp, IncorrectConstructionDuplicateExcludesTxt) {
    auto cat = app.add_flag("--cat");
    app.add_flag("--other");
    ASSERT_NO_THROW(cat->excludes("--other"));
    EXPECT_THROW(cat->excludes("--other"), CLI::OptionAlreadyAdded);
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
