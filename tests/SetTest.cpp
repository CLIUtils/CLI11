#include "app_helper.hpp"
#include <map>

static_assert(CLI::is_shared_ptr<std::shared_ptr<int>>::value == true, "is_shared_ptr should work on shared pointers");
static_assert(CLI::is_shared_ptr<int *>::value == false, "is_shared_ptr should work on pointers");
static_assert(CLI::is_shared_ptr<int>::value == false, "is_shared_ptr should work on non-pointers");
static_assert(CLI::is_shared_ptr<const std::shared_ptr<int>>::value == true,
              "is_shared_ptr should work on const shared pointers");
static_assert(CLI::is_shared_ptr<const int *>::value == false, "is_shared_ptr should work on const pointers");
static_assert(CLI::is_shared_ptr<const int &>::value == false, "is_shared_ptr should work on const references");
static_assert(CLI::is_shared_ptr<int &>::value == false, "is_shared_ptr should work on non-const references");

static_assert(CLI::is_copyable_ptr<std::shared_ptr<int>>::value == true,
              "is_copyable_ptr should work on shared pointers");
static_assert(CLI::is_copyable_ptr<int *>::value == true, "is_copyable_ptr should work on pointers");
static_assert(CLI::is_copyable_ptr<int>::value == false, "is_copyable_ptr should work on non-pointers");
static_assert(CLI::is_copyable_ptr<const std::shared_ptr<int>>::value == true,
              "is_copyable_ptr should work on const shared pointers");
static_assert(CLI::is_copyable_ptr<const int *>::value == true, "is_copyable_ptr should work on const pointers");
static_assert(CLI::is_copyable_ptr<const int &>::value == false, "is_copyable_ptr should work on const references");
static_assert(CLI::is_copyable_ptr<int &>::value == false, "is_copyable_ptr should work on non-const references");

static_assert(CLI::detail::pair_adaptor<std::set<int>>::value == false, "Should not have pairs");
static_assert(CLI::detail::pair_adaptor<std::vector<std::string>>::value == false, "Should not have pairs");
static_assert(CLI::detail::pair_adaptor<std::map<int, int>>::value == true, "Should have pairs");
static_assert(CLI::detail::pair_adaptor<std::vector<std::pair<int, int>>>::value == true, "Should have pairs");

TEST_F(TApp, SimpleMaps) {
    int value;
    std::map<std::string, int> map = {{"one", 1}, {"two", 2}};
    auto opt = app.add_option("-s,--set", value)->transform(CLI::Transformer(map));
    args = {"-s", "one"};
    run();
    EXPECT_EQ(1u, app.count("-s"));
    EXPECT_EQ(1u, app.count("--set"));
    EXPECT_EQ(1u, opt->count());
    EXPECT_EQ(value, 1);
}

TEST_F(TApp, StringStringMap) {
    std::string value;
    std::map<std::string, std::string> map = {{"a", "b"}, {"b", "c"}};
    app.add_option("-s,--set", value)->transform(CLI::CheckedTransformer(map));
    args = {"-s", "a"};
    run();
    EXPECT_EQ(value, "b");

    args = {"-s", "b"};
    run();
    EXPECT_EQ(value, "c");

    args = {"-s", "c"};
    EXPECT_EQ(value, "c");
}

TEST_F(TApp, StringStringMapNoModify) {
    std::string value;
    std::map<std::string, std::string> map = {{"a", "b"}, {"b", "c"}};
    app.add_option("-s,--set", value)->check(CLI::IsMember(map));
    args = {"-s", "a"};
    run();
    EXPECT_EQ(value, "a");

    args = {"-s", "b"};
    run();
    EXPECT_EQ(value, "b");

    args = {"-s", "c"};
    EXPECT_THROW(run(), CLI::ValidationError);
}

enum SimpleEnum { SE_one = 1, SE_two = 2 };

TEST_F(TApp, EnumMap) {
    SimpleEnum value;
    std::map<std::string, SimpleEnum> map = {{"one", SE_one}, {"two", SE_two}};
    auto opt = app.add_option("-s,--set", value)->transform(CLI::Transformer(map));
    args = {"-s", "one"};
    run();
    EXPECT_EQ(1u, app.count("-s"));
    EXPECT_EQ(1u, app.count("--set"));
    EXPECT_EQ(1u, opt->count());
    EXPECT_EQ(value, SE_one);
}

enum class SimpleEnumC { one = 1, two = 2 };

TEST_F(TApp, EnumCMap) {
    SimpleEnumC value;
    std::map<std::string, SimpleEnumC> map = {{"one", SimpleEnumC::one}, {"two", SimpleEnumC::two}};
    auto opt = app.add_option("-s,--set", value)->transform(CLI::Transformer(map));
    args = {"-s", "one"};
    run();
    EXPECT_EQ(1u, app.count("-s"));
    EXPECT_EQ(1u, app.count("--set"));
    EXPECT_EQ(1u, opt->count());
    EXPECT_EQ(value, SimpleEnumC::one);
}

TEST_F(TApp, structMap) {
    struct tstruct {
        int val2;
        double val3;
        std::string v4;
    };
    std::string struct_name;
    std::map<std::string, struct tstruct> map = {{"sone", {4, 32.4, "foo"}}, {"stwo", {5, 99.7, "bar"}}};
    auto opt = app.add_option("-s,--set", struct_name)->check(CLI::IsMember(map));
    args = {"-s", "sone"};
    run();
    EXPECT_EQ(1u, app.count("-s"));
    EXPECT_EQ(1u, app.count("--set"));
    EXPECT_EQ(1u, opt->count());
    EXPECT_EQ(struct_name, "sone");

    args = {"-s", "sthree"};
    EXPECT_THROW(run(), CLI::ValidationError);
}

TEST_F(TApp, structMapChange) {
    struct tstruct {
        int val2;
        double val3;
        std::string v4;
    };
    std::string struct_name;
    std::map<std::string, struct tstruct> map = {{"sone", {4, 32.4, "foo"}}, {"stwo", {5, 99.7, "bar"}}};
    auto opt = app.add_option("-s,--set", struct_name)
                   ->transform(CLI::IsMember(map, CLI::ignore_case, CLI::ignore_underscore, CLI::ignore_space));
    args = {"-s", "s one"};
    run();
    EXPECT_EQ(1u, app.count("-s"));
    EXPECT_EQ(1u, app.count("--set"));
    EXPECT_EQ(1u, opt->count());
    EXPECT_EQ(struct_name, "sone");

    args = {"-s", "sthree"};
    EXPECT_THROW(run(), CLI::ValidationError);

    args = {"-s", "S_t_w_o"};
    run();
    EXPECT_EQ(struct_name, "stwo");
    args = {"-s", "S two"};
    run();
    EXPECT_EQ(struct_name, "stwo");
}

TEST_F(TApp, structMapNoChange) {
    struct tstruct {
        int val2;
        double val3;
        std::string v4;
    };
    std::string struct_name;
    std::map<std::string, struct tstruct> map = {{"sone", {4, 32.4, "foo"}}, {"stwo", {5, 99.7, "bar"}}};
    auto opt = app.add_option("-s,--set", struct_name)
                   ->check(CLI::IsMember(map, CLI::ignore_case, CLI::ignore_underscore, CLI::ignore_space));
    args = {"-s", "SONE"};
    run();
    EXPECT_EQ(1u, app.count("-s"));
    EXPECT_EQ(1u, app.count("--set"));
    EXPECT_EQ(1u, opt->count());
    EXPECT_EQ(struct_name, "SONE");

    args = {"-s", "sthree"};
    EXPECT_THROW(run(), CLI::ValidationError);

    args = {"-s", "S_t_w_o"};
    run();
    EXPECT_EQ(struct_name, "S_t_w_o");

    args = {"-s", "S two"};
    run();
    EXPECT_EQ(struct_name, "S two");
}

TEST_F(TApp, NonCopyableMap) {

    std::string map_name;
    std::map<std::string, std::unique_ptr<double>> map;
    map["e1"] = std::unique_ptr<double>(new double(5.7));
    map["e3"] = std::unique_ptr<double>(new double(23.8));
    auto opt = app.add_option("-s,--set", map_name)->check(CLI::IsMember(&map));
    args = {"-s", "e1"};
    run();
    EXPECT_EQ(1u, app.count("-s"));
    EXPECT_EQ(1u, app.count("--set"));
    EXPECT_EQ(1u, opt->count());
    EXPECT_EQ(map_name, "e1");

    args = {"-s", "e45"};
    EXPECT_THROW(run(), CLI::ValidationError);
}

TEST_F(TApp, NonCopyableMapWithFunction) {

    std::string map_name;
    std::map<std::string, std::unique_ptr<double>> map;
    map["e1"] = std::unique_ptr<double>(new double(5.7));
    map["e3"] = std::unique_ptr<double>(new double(23.8));
    auto opt = app.add_option("-s,--set", map_name)->transform(CLI::IsMember(&map, CLI::ignore_underscore));
    args = {"-s", "e_1"};
    run();
    EXPECT_EQ(1u, app.count("-s"));
    EXPECT_EQ(1u, app.count("--set"));
    EXPECT_EQ(1u, opt->count());
    EXPECT_EQ(map_name, "e1");

    args = {"-s", "e45"};
    EXPECT_THROW(run(), CLI::ValidationError);
}

TEST_F(TApp, NonCopyableMapNonStringMap) {

    std::string map_name;
    std::map<int, std::unique_ptr<double>> map;
    map[4] = std::unique_ptr<double>(new double(5.7));
    map[17] = std::unique_ptr<double>(new double(23.8));
    auto opt = app.add_option("-s,--set", map_name)->check(CLI::IsMember(&map));
    args = {"-s", "4"};
    run();
    EXPECT_EQ(1u, app.count("-s"));
    EXPECT_EQ(1u, app.count("--set"));
    EXPECT_EQ(1u, opt->count());
    EXPECT_EQ(map_name, "4");

    args = {"-s", "e45"};
    EXPECT_THROW(run(), CLI::ValidationError);
}

TEST_F(TApp, CopyableMapMove) {

    std::string map_name;
    std::map<int, double> map;
    map[4] = 5.7;
    map[17] = 23.8;
    auto opt = app.add_option("-s,--set", map_name)->check(CLI::IsMember(std::move(map)));
    args = {"-s", "4"};
    run();
    EXPECT_EQ(1u, app.count("-s"));
    EXPECT_EQ(1u, app.count("--set"));
    EXPECT_EQ(1u, opt->count());
    EXPECT_EQ(map_name, "4");

    args = {"-s", "e45"};
    EXPECT_THROW(run(), CLI::ValidationError);
}

TEST_F(TApp, SimpleSets) {
    std::string value;
    auto opt = app.add_option("-s,--set", value)->check(CLI::IsMember{std::set<std::string>({"one", "two", "three"})});
    args = {"-s", "one"};
    run();
    EXPECT_EQ(1u, app.count("-s"));
    EXPECT_EQ(1u, app.count("--set"));
    EXPECT_EQ(1u, opt->count());
    EXPECT_EQ(value, "one");
}

TEST_F(TApp, SimpleSetsPtrs) {
    auto set = std::shared_ptr<std::set<std::string>>(new std::set<std::string>{"one", "two", "three"});
    std::string value;
    auto opt = app.add_option("-s,--set", value)->check(CLI::IsMember{set});
    args = {"-s", "one"};
    run();
    EXPECT_EQ(1u, app.count("-s"));
    EXPECT_EQ(1u, app.count("--set"));
    EXPECT_EQ(1u, opt->count());
    EXPECT_EQ(value, "one");

    set->insert("four");

    args = {"-s", "four"};
    run();
    EXPECT_EQ(1u, app.count("-s"));
    EXPECT_EQ(1u, app.count("--set"));
    EXPECT_EQ(1u, opt->count());
    EXPECT_EQ(value, "four");
}

TEST_F(TApp, SimiShortcutSets) {
    std::string value;
    auto opt = app.add_option("--set", value)->check(CLI::IsMember({"one", "two", "three"}));
    args = {"--set", "one"};
    run();
    EXPECT_EQ(1u, app.count("--set"));
    EXPECT_EQ(1u, opt->count());
    EXPECT_EQ(value, "one");

    std::string value2;
    auto opt2 = app.add_option("--set2", value2)->transform(CLI::IsMember({"One", "two", "three"}, CLI::ignore_case));
    args = {"--set2", "onE"};
    run();
    EXPECT_EQ(1u, app.count("--set2"));
    EXPECT_EQ(1u, opt2->count());
    EXPECT_EQ(value2, "One");

    std::string value3;
    auto opt3 = app.add_option("--set3", value3)
                    ->transform(CLI::IsMember({"O_ne", "two", "three"}, CLI::ignore_case, CLI::ignore_underscore));
    args = {"--set3", "onE"};
    run();
    EXPECT_EQ(1u, app.count("--set3"));
    EXPECT_EQ(1u, opt3->count());
    EXPECT_EQ(value3, "O_ne");
}

TEST_F(TApp, SetFromCharStarArrayVector) {
    constexpr const char *names[] = {"one", "two", "three"};
    std::string value;
    auto opt = app.add_option("-s,--set", value)
                   ->check(CLI::IsMember{std::vector<std::string>(std::begin(names), std::end(names))});
    args = {"-s", "one"};
    run();
    EXPECT_EQ(1u, app.count("-s"));
    EXPECT_EQ(1u, app.count("--set"));
    EXPECT_EQ(1u, opt->count());
    EXPECT_EQ(value, "one");
}

TEST_F(TApp, OtherTypeSets) {
    int value;
    std::vector<int> set = {2, 3, 4};
    auto opt = app.add_option("--set", value)->check(CLI::IsMember(set));
    args = {"--set", "3"};
    run();
    EXPECT_EQ(1u, app.count("--set"));
    EXPECT_EQ(1u, opt->count());
    EXPECT_EQ(value, 3);

    args = {"--set", "5"};
    EXPECT_THROW(run(), CLI::ValidationError);

    std::vector<int> set2 = {-2, 3, 4};
    auto opt2 = app.add_option("--set2", value)->transform(CLI::IsMember(set2, [](int x) { return std::abs(x); }));
    args = {"--set2", "-3"};
    run();
    EXPECT_EQ(1u, app.count("--set2"));
    EXPECT_EQ(1u, opt2->count());
    EXPECT_EQ(value, 3);

    args = {"--set2", "-3"};
    run();
    EXPECT_EQ(1u, app.count("--set2"));
    EXPECT_EQ(1u, opt2->count());
    EXPECT_EQ(value, 3);

    args = {"--set2", "2"};
    run();
    EXPECT_EQ(1u, app.count("--set2"));
    EXPECT_EQ(1u, opt2->count());
    EXPECT_EQ(value, -2);
}

TEST_F(TApp, NumericalSets) {
    int value;
    auto opt = app.add_option("-s,--set", value)->check(CLI::IsMember{std::set<int>({1, 2, 3})});
    args = {"-s", "1"};
    run();
    EXPECT_EQ(1u, app.count("-s"));
    EXPECT_EQ(1u, app.count("--set"));
    EXPECT_EQ(1u, opt->count());
    EXPECT_EQ(value, 1);
}

// Converted original set tests

TEST_F(TApp, SetWithDefaults) {
    int someint = 2;
    app.add_option("-a", someint, "", true)->check(CLI::IsMember({1, 2, 3, 4}));

    args = {"-a1", "-a2"};

    EXPECT_THROW(run(), CLI::ArgumentMismatch);
}

TEST_F(TApp, SetWithDefaultsConversion) {
    int someint = 2;
    app.add_option("-a", someint, "", true)->check(CLI::IsMember({1, 2, 3, 4}));

    args = {"-a", "hi"};

    EXPECT_THROW(run(), CLI::ValidationError);
}

TEST_F(TApp, SetWithDefaultsIC) {
    std::string someint = "ho";
    app.add_option("-a", someint, "", true)->check(CLI::IsMember({"Hi", "Ho"}));

    args = {"-aHi", "-aHo"};

    EXPECT_THROW(run(), CLI::ArgumentMismatch);
}

TEST_F(TApp, InSet) {

    std::string choice;
    app.add_option("-q,--quick", choice)->check(CLI::IsMember({"one", "two", "three"}));

    args = {"--quick", "two"};

    run();
    EXPECT_EQ("two", choice);

    args = {"--quick", "four"};
    EXPECT_THROW(run(), CLI::ValidationError);
}

TEST_F(TApp, InSetWithDefault) {

    std::string choice = "one";
    app.add_option("-q,--quick", choice, "", true)->check(CLI::IsMember({"one", "two", "three"}));

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
    app.add_option("-q,--quick", choice, "", true)->transform(CLI::IsMember({"one", "two", "three"}, CLI::ignore_case));

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
    app.add_option("-q,--quick", choice)->check(CLI::IsMember({1, 2, 3}));

    args = {"--quick", "2"};

    run();
    EXPECT_EQ(2, choice);

    args = {"--quick", "4"};
    EXPECT_THROW(run(), CLI::ValidationError);
}

TEST_F(TApp, InIntSetWindows) {

    int choice;
    app.add_option("-q,--quick", choice)->check(CLI::IsMember({1, 2, 3}));
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
    app.add_option("-q,--quick", choice)->check(CLI::IsMember({1, 2, 3}));

    args = {"--quick", "3", "--quick=2"};
    EXPECT_THROW(run(), CLI::ArgumentMismatch);

    args = {"--quick=hello"};
    EXPECT_THROW(run(), CLI::ValidationError);
}

TEST_F(TApp, FailMutableSet) {

    int choice;
    auto vals = std::shared_ptr<std::set<int>>(new std::set<int>({1, 2, 3}));
    app.add_option("-q,--quick", choice)->check(CLI::IsMember(vals));
    app.add_option("-s,--slow", choice, "", true)->check(CLI::IsMember(vals));

    args = {"--quick=hello"};
    EXPECT_THROW(run(), CLI::ValidationError);

    args = {"--slow=hello"};
    EXPECT_THROW(run(), CLI::ValidationError);
}

TEST_F(TApp, InSetIgnoreCase) {

    std::string choice;
    app.add_option("-q,--quick", choice)->transform(CLI::IsMember({"one", "Two", "THREE"}, CLI::ignore_case));

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
    app.add_option("-q,--quick", choice)->transform(CLI::IsMember(&options, CLI::ignore_case));

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

    std::set<std::string> *options = new std::set<std::string>{"one", "Two", "THREE"};
    std::string choice;
    app.add_option("-q,--quick", choice)->transform(CLI::IsMember(*options, CLI::ignore_case));

    args = {"--quick", "One"};
    run();
    EXPECT_EQ("one", choice);

    args = {"--quick", "two"};
    run();
    EXPECT_EQ("Two", choice); // Keeps caps from set

    args = {"--quick", "ThrEE"};
    run();
    EXPECT_EQ("THREE", choice); // Keeps caps from set

    delete options;
    args = {"--quick", "ThrEE"};
    run();
    EXPECT_EQ("THREE", choice); // this does not throw a segfault

    args = {"--quick", "four"};
    EXPECT_THROW(run(), CLI::ValidationError);

    args = {"--quick=one", "--quick=two"};
    EXPECT_THROW(run(), CLI::ArgumentMismatch);
}

TEST_F(TApp, NotInSetIgnoreCasePointer) {

    std::set<std::string> *options = new std::set<std::string>{"one", "Two", "THREE"};
    std::string choice;
    app.add_option("-q,--quick", choice)->check(!CLI::IsMember(*options, CLI::ignore_case));

    args = {"--quick", "One"};
    EXPECT_THROW(run(), CLI::ValidationError);

    args = {"--quick", "four"};
    run();
    EXPECT_EQ(choice, "four");
}

TEST_F(TApp, InSetIgnoreUnderscore) {

    std::string choice;
    app.add_option("-q,--quick", choice)
        ->transform(CLI::IsMember({"option_one", "option_two", "optionthree"}, CLI::ignore_underscore));

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
    app.add_option("-q,--quick", choice)
        ->transform(
            CLI::IsMember({"Option_One", "option_two", "OptionThree"}, CLI::ignore_case, CLI::ignore_underscore));

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
    app.add_option("--type1", type1)->check(CLI::IsMember(&items));
    app.add_option("--type2", type2, "", true)->check(CLI::IsMember(&items));

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
    app.add_option("--type1", type1)->transform(CLI::IsMember(&items, CLI::ignore_case));
    app.add_option("--type2", type2, "", true)->transform(CLI::IsMember(&items, CLI::ignore_case));

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
