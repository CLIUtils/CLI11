#include "app_helper.hpp"
#include <stdlib.h>

TEST_F(TApp, AddingExisting) {
    app.add_flag("-c,--count");
    EXPECT_THROW(app.add_flag("--cat,-c"), CLI::OptionAlreadyAdded);
}

TEST_F(TApp, AddingExistingNoCase) {
    app.add_flag("-C,--count")->ignore_case();
    EXPECT_THROW(app.add_flag("--cat,-c"), CLI::OptionAlreadyAdded);
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
    CLI::App* below = app.add_subcommand("below");
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

TEST_F(TApp, IncorrectConstructionFlagPositional1) {
    EXPECT_THROW(app.add_flag("cat"), CLI::IncorrectConstruction);
}

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
    EXPECT_THROW(cat->requires("--nothing"),CLI::IncorrectConstruction);
}

TEST_F(TApp, IncorrectConstructionExcludesCannotFind) {
    auto cat = app.add_flag("--cat");
    EXPECT_THROW(cat->excludes("--nothing"),CLI::IncorrectConstruction);
}
