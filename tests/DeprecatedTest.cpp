#include "app_helper.hpp"

TEST(Deprecated, Emtpy) {
    // No deprecated features at this time.
    EXPECT_TRUE(true);
}

// Classic sets

TEST_F(TApp, SetWithDefaults) {
    int someint = 2;
    app.add_set("-a", someint, {1, 2, 3, 4}, "", true);

    args = {"-a1", "-a2"};

    EXPECT_THROW(run(), CLI::ArgumentMismatch);
}

TEST_F(TApp, SetWithDefaultsConversion) {
    int someint = 2;
    app.add_set("-a", someint, {1, 2, 3, 4}, "", true);

    args = {"-a", "hi"};

    EXPECT_THROW(run(), CLI::ValidationError);
}

TEST_F(TApp, SetWithDefaultsIC) {
    std::string someint = "ho";
    app.add_set_ignore_case("-a", someint, {"Hi", "Ho"}, "", true);

    args = {"-aHi", "-aHo"};

    EXPECT_THROW(run(), CLI::ArgumentMismatch);
}

TEST_F(TApp, InSet) {

    std::string choice;
    app.add_set("-q,--quick", choice, {"one", "two", "three"});

    args = {"--quick", "two"};

    run();
    EXPECT_EQ("two", choice);

    args = {"--quick", "four"};
    EXPECT_THROW(run(), CLI::ValidationError);
}

TEST_F(TApp, InSetWithDefault) {

    std::string choice = "one";
    app.add_set("-q,--quick", choice, {"one", "two", "three"}, "", true);

    run();
    EXPECT_EQ("one", choice);

    args = {"--quick", "two"};

    run();
    EXPECT_EQ("two", choice);

    args = {"--quick", "four"};
    EXPECT_THROW(run(), CLI::ValidationError);
}

TEST_F(TApp, InCaselessSetWithDefault) {

    std::string choice = "one";
    app.add_set_ignore_case("-q,--quick", choice, {"one", "two", "three"}, "", true);

    run();
    EXPECT_EQ("one", choice);

    args = {"--quick", "tWo"};

    run();
    EXPECT_EQ("two", choice);

    args = {"--quick", "four"};
    EXPECT_THROW(run(), CLI::ValidationError);
}

TEST_F(TApp, InIntSet) {

    int choice;
    app.add_set("-q,--quick", choice, {1, 2, 3});

    args = {"--quick", "2"};

    run();
    EXPECT_EQ(2, choice);

    args = {"--quick", "4"};
    EXPECT_THROW(run(), CLI::ValidationError);
}

TEST_F(TApp, InIntSetWindows) {

    int choice;
    app.add_set("-q,--quick", choice, {1, 2, 3});
    app.allow_windows_style_options();
    args = {"/q", "2"};

    run();
    EXPECT_EQ(2, choice);

    args = {"/q", "4"};
    EXPECT_THROW(run(), CLI::ValidationError);

    args = {"/q4"};
    EXPECT_THROW(run(), CLI::ExtrasError);
}

TEST_F(TApp, FailSet) {

    int choice;
    app.add_set("-q,--quick", choice, {1, 2, 3});

    args = {"--quick", "3", "--quick=2"};
    EXPECT_THROW(run(), CLI::ArgumentMismatch);

    args = {"--quick=hello"};
    EXPECT_THROW(run(), CLI::ValidationError);
}

TEST_F(TApp, FailMutableSet) {

    int choice;
    std::set<int> vals{1, 2, 3};
    app.add_mutable_set("-q,--quick", choice, vals);
    app.add_mutable_set("-s,--slow", choice, vals, "", true);

    args = {"--quick=hello"};
    EXPECT_THROW(run(), CLI::ValidationError);

    args = {"--slow=hello"};
    EXPECT_THROW(run(), CLI::ValidationError);
}

TEST_F(TApp, InSetIgnoreCase) {

    std::string choice;
    app.add_set_ignore_case("-q,--quick", choice, {"one", "Two", "THREE"});

    args = {"--quick", "One"};
    run();
    EXPECT_EQ("one", choice);

    args = {"--quick", "two"};
    run();
    EXPECT_EQ("Two", choice); // Keeps caps from set

    args = {"--quick", "ThrEE"};
    run();
    EXPECT_EQ("THREE", choice); // Keeps caps from set

    args = {"--quick", "four"};
    EXPECT_THROW(run(), CLI::ValidationError);

    args = {"--quick=one", "--quick=two"};
    EXPECT_THROW(run(), CLI::ArgumentMismatch);
}

TEST_F(TApp, InSetIgnoreCaseMutableValue) {

    std::set<std::string> options{"one", "Two", "THREE"};
    std::string choice;
    app.add_mutable_set_ignore_case("-q,--quick", choice, options);

    args = {"--quick", "One"};
    run();
    EXPECT_EQ("one", choice);

    args = {"--quick", "two"};
    run();
    EXPECT_EQ("Two", choice); // Keeps caps from set

    args = {"--quick", "ThrEE"};
    run();
    EXPECT_EQ("THREE", choice); // Keeps caps from set

    options.clear();
    args = {"--quick", "ThrEE"};
    EXPECT_THROW(run(), CLI::ValidationError);
}

TEST_F(TApp, InSetIgnoreCasePointer) {

    auto options = std::make_shared<std::set<std::string>>(std::initializer_list<std::string>{"one", "Two", "THREE"});
    std::string choice;
    app.add_set_ignore_case("-q,--quick", choice, *options);

    args = {"--quick", "One"};
    run();
    EXPECT_EQ("one", choice);

    args = {"--quick", "two"};
    run();
    EXPECT_EQ("Two", choice); // Keeps caps from set

    args = {"--quick", "ThrEE"};
    run();
    EXPECT_EQ("THREE", choice); // Keeps caps from set

    options.reset();
    args = {"--quick", "ThrEE"};
    run();
    EXPECT_EQ("THREE", choice); // this does not throw a segfault

    args = {"--quick", "four"};
    EXPECT_THROW(run(), CLI::ValidationError);

    args = {"--quick=one", "--quick=two"};
    EXPECT_THROW(run(), CLI::ArgumentMismatch);
}

TEST_F(TApp, InSetIgnoreUnderscore) {

    std::string choice;
    app.add_set_ignore_underscore("-q,--quick", choice, {"option_one", "option_two", "optionthree"});

    args = {"--quick", "option_one"};
    run();
    EXPECT_EQ("option_one", choice);

    args = {"--quick", "optiontwo"};
    run();
    EXPECT_EQ("option_two", choice); // Keeps underscore from set

    args = {"--quick", "_option_thr_ee"};
    run();
    EXPECT_EQ("optionthree", choice); // no underscore

    args = {"--quick", "Option4"};
    EXPECT_THROW(run(), CLI::ValidationError);

    args = {"--quick=option_one", "--quick=option_two"};
    EXPECT_THROW(run(), CLI::ArgumentMismatch);
}

TEST_F(TApp, InSetIgnoreCaseUnderscore) {

    std::string choice;
    app.add_set_ignore_case_underscore("-q,--quick", choice, {"Option_One", "option_two", "OptionThree"});

    args = {"--quick", "option_one"};
    run();
    EXPECT_EQ("Option_One", choice);

    args = {"--quick", "OptionTwo"};
    run();
    EXPECT_EQ("option_two", choice); // Keeps underscore and case from set

    args = {"--quick", "_OPTION_thr_ee"};
    run();
    EXPECT_EQ("OptionThree", choice); // no underscore

    args = {"--quick", "Option4"};
    EXPECT_THROW(run(), CLI::ValidationError);

    args = {"--quick=option_one", "--quick=option_two"};
    EXPECT_THROW(run(), CLI::ArgumentMismatch);
}

// #113
TEST_F(TApp, AddRemoveSetItems) {
    std::set<std::string> items{"TYPE1", "TYPE2", "TYPE3", "TYPE4", "TYPE5"};

    std::string type1, type2;
    app.add_mutable_set("--type1", type1, items);
    app.add_mutable_set("--type2", type2, items, "", true);

    args = {"--type1", "TYPE1", "--type2", "TYPE2"};

    run();
    EXPECT_EQ(type1, "TYPE1");
    EXPECT_EQ(type2, "TYPE2");

    items.insert("TYPE6");
    items.insert("TYPE7");

    items.erase("TYPE1");
    items.erase("TYPE2");

    args = {"--type1", "TYPE6", "--type2", "TYPE7"};
    run();
    EXPECT_EQ(type1, "TYPE6");
    EXPECT_EQ(type2, "TYPE7");

    args = {"--type1", "TYPE1"};
    EXPECT_THROW(run(), CLI::ValidationError);

    args = {"--type2", "TYPE2"};
    EXPECT_THROW(run(), CLI::ValidationError);
}

TEST_F(TApp, AddRemoveSetItemsNoCase) {
    std::set<std::string> items{"TYPE1", "TYPE2", "TYPE3", "TYPE4", "TYPE5"};

    std::string type1, type2;
    app.add_mutable_set_ignore_case("--type1", type1, items);
    app.add_mutable_set_ignore_case("--type2", type2, items, "", true);

    args = {"--type1", "TYPe1", "--type2", "TyPE2"};

    run();
    EXPECT_EQ(type1, "TYPE1");
    EXPECT_EQ(type2, "TYPE2");

    items.insert("TYPE6");
    items.insert("TYPE7");

    items.erase("TYPE1");
    items.erase("TYPE2");

    args = {"--type1", "TyPE6", "--type2", "tYPE7"};
    run();
    EXPECT_EQ(type1, "TYPE6");
    EXPECT_EQ(type2, "TYPE7");

    args = {"--type1", "TYPe1"};
    EXPECT_THROW(run(), CLI::ValidationError);

    args = {"--type2", "TYpE2"};
    EXPECT_THROW(run(), CLI::ValidationError);
}
