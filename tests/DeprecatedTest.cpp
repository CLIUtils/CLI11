#include "app_helper.hpp"

#include "gmock/gmock.h"

using ::testing::HasSubstr;
using ::testing::Not;

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

TEST(THelp, Defaults) {
    CLI::App app{"My prog"};

    int one{1}, two{2};
    app.add_option("--one", one, "Help for one", true);
    app.add_option("--set", two, "Help for set", true)->check(CLI::IsMember({2, 3, 4}));

    std::string help = app.help();

    EXPECT_THAT(help, HasSubstr("--one"));
    EXPECT_THAT(help, HasSubstr("--set"));
    EXPECT_THAT(help, HasSubstr("1"));
    EXPECT_THAT(help, HasSubstr("=2"));
    EXPECT_THAT(help, HasSubstr("2,3,4"));
}

TEST(THelp, VectorOpts) {
    CLI::App app{"My prog"};
    std::vector<int> x = {1, 2};
    app.add_option("-q,--quick", x, "", true);

    std::string help = app.help();

    EXPECT_THAT(help, HasSubstr("INT=[1,2] ..."));
}

TEST(THelp, SetLower) {
    CLI::App app{"My prog"};

    std::string def{"One"};
    app.add_option("--set", def, "Help for set", true)->check(CLI::IsMember({"oNe", "twO", "THREE"}));

    std::string help = app.help();

    EXPECT_THAT(help, HasSubstr("--set"));
    EXPECT_THAT(help, HasSubstr("=One"));
    EXPECT_THAT(help, HasSubstr("oNe"));
    EXPECT_THAT(help, HasSubstr("twO"));
    EXPECT_THAT(help, HasSubstr("THREE"));
}

TEST(THelp, ChangingSetDefaulted) {
    CLI::App app;

    std::set<int> vals{1, 2, 3};
    int val = 2;
    app.add_option("--val", val, "", true)->check(CLI::IsMember(&vals));

    std::string help = app.help();

    EXPECT_THAT(help, HasSubstr("1"));
    EXPECT_THAT(help, Not(HasSubstr("4")));

    vals.insert(4);
    vals.erase(1);

    help = app.help();

    EXPECT_THAT(help, Not(HasSubstr("1")));
    EXPECT_THAT(help, HasSubstr("4"));
}

TEST(THelp, ChangingCaselessSetDefaulted) {
    CLI::App app;

    std::set<std::string> vals{"1", "2", "3"};
    std::string val = "2";
    app.add_option("--val", val, "", true)->check(CLI::IsMember(&vals, CLI::ignore_case));

    std::string help = app.help();

    EXPECT_THAT(help, HasSubstr("1"));
    EXPECT_THAT(help, Not(HasSubstr("4")));

    vals.insert("4");
    vals.erase("1");

    help = app.help();

    EXPECT_THAT(help, Not(HasSubstr("1")));
    EXPECT_THAT(help, HasSubstr("4"));
}

TEST_F(TApp, DefaultOpts) {

    int i = 3;
    std::string s = "HI";

    app.add_option("-i,i", i, "", false);
    app.add_option("-s,s", s, "", true);

    args = {"-i2", "9"};

    run();

    EXPECT_EQ(1u, app.count("i"));
    EXPECT_EQ(1u, app.count("-s"));
    EXPECT_EQ(2, i);
    EXPECT_EQ("9", s);
}

TEST_F(TApp, VectorDefaultedFixedString) {
    std::vector<std::string> strvec{"one"};
    std::vector<std::string> answer{"mystring", "mystring2", "mystring3"};

    CLI::Option *opt = app.add_option("-s,--string", strvec, "", true)->expected(3);
    EXPECT_EQ(3, opt->get_expected());

    args = {"--string", "mystring", "mystring2", "mystring3"};
    run();
    EXPECT_EQ(3u, app.count("--string"));
    EXPECT_EQ(answer, strvec);
}

TEST_F(TApp, DefaultedResult) {
    std::string sval = "NA";
    int ival;
    auto opts = app.add_option("--string", sval, "", true);
    auto optv = app.add_option("--val", ival);
    args = {};
    run();
    EXPECT_EQ(sval, "NA");
    std::string nString;
    opts->results(nString);
    EXPECT_EQ(nString, "NA");
    int newIval;
    EXPECT_THROW(optv->results(newIval), CLI::ConversionError);
    optv->default_str("442");
    optv->results(newIval);
    EXPECT_EQ(newIval, 442);
}

TEST_F(TApp, OptionWithDefaults) {
    int someint = 2;
    app.add_option("-a", someint, "", true);

    args = {"-a1", "-a2"};

    EXPECT_THROW(run(), CLI::ArgumentMismatch);
}

// #209
TEST_F(TApp, CustomUserSepParse) {

    std::vector<int> vals = {1, 2, 3};
    args = {"--idx", "1,2,3"};
    auto opt = app.add_option("--idx", vals)->delimiter(',');
    run();
    EXPECT_EQ(vals, std::vector<int>({1, 2, 3}));
    std::vector<int> vals2;
    // check that the results vector gets the results in the same way
    opt->results(vals2);
    EXPECT_EQ(vals2, vals);

    app.remove_option(opt);

    app.add_option("--idx", vals, "", true)->delimiter(',');
    run();
    EXPECT_EQ(vals, std::vector<int>({1, 2, 3}));
}

// #209
TEST_F(TApp, CustomUserSepParse2) {

    std::vector<int> vals = {1, 2, 3};
    args = {"--idx", "1,2,"};
    auto opt = app.add_option("--idx", vals)->delimiter(',');
    run();
    EXPECT_EQ(vals, std::vector<int>({1, 2}));

    app.remove_option(opt);

    app.add_option("--idx", vals, "", true)->delimiter(',');
    run();
    EXPECT_EQ(vals, std::vector<int>({1, 2}));
}
//
// #209
TEST_F(TApp, CustomUserSepParse4) {

    std::vector<int> vals;
    args = {"--idx", "1,    2"};
    auto opt = app.add_option("--idx", vals, "", true)->delimiter(',');
    run();
    EXPECT_EQ(vals, std::vector<int>({1, 2}));

    app.remove_option(opt);

    app.add_option("--idx", vals)->delimiter(',');
    run();
    EXPECT_EQ(vals, std::vector<int>({1, 2}));
}

// #218
TEST_F(TApp, CustomUserSepParse5) {

    std::vector<std::string> bar;
    args = {"this", "is", "a", "test"};
    auto opt = app.add_option("bar", bar, "bar");
    run();
    EXPECT_EQ(bar, std::vector<std::string>({"this", "is", "a", "test"}));

    app.remove_option(opt);
    args = {"this", "is", "a", "test"};
    app.add_option("bar", bar, "bar", true);
    run();
    EXPECT_EQ(bar, std::vector<std::string>({"this", "is", "a", "test"}));
}
