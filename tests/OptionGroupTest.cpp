#include "app_helper.hpp"

#include "gmock/gmock.h"
#include "gtest/gtest.h"

using ::testing::HasSubstr;
using ::testing::Not;

using vs_t = std::vector<std::string>;

TEST_F(TApp, BasicOptionGroup) {
    auto ogroup = app.add_option_group("clusters");
    int res;
    ogroup->add_option("--test1", res);
    ogroup->add_option("--test2", res);
    ogroup->add_option("--test3", res);

    args = {"--test1", "5"};
    run();
    EXPECT_EQ(res, 5);
}

TEST_F(TApp, BasicOptionGroupExact) {
    auto ogroup = app.add_option_group("clusters");
    int res;
    ogroup->add_option("--test1", res);
    ogroup->add_option("--test2", res);
    ogroup->add_option("--test3", res);
    int val2;
    app.add_option("--option", val2);
    ogroup->require_option(1);
    args = {"--test1", "5"};
    run();
    EXPECT_EQ(res, 5);

    args = {"--test1", "5", "--test2", "4"};
    EXPECT_THROW(run(), CLI::RequiredError);

    args = {"--option", "9"};
    EXPECT_THROW(run(), CLI::RequiredError);

    std::string help = ogroup->help();
    auto exactloc = help.find("[Exactly 1");
    EXPECT_NE(exactloc, std::string::npos);
}

TEST_F(TApp, BasicOptionGroupExactTooMany) {
    auto ogroup = app.add_option_group("clusters");
    int res;
    ogroup->add_option("--test1", res);
    ogroup->add_option("--test2", res);
    ogroup->add_option("--test3", res);
    int val2;
    app.add_option("--option", val2);
    ogroup->require_option(10);
    args = {"--test1", "5"};
    EXPECT_THROW(run(), CLI::InvalidError);
}

TEST_F(TApp, BasicOptionGroupMinMax) {
    auto ogroup = app.add_option_group("clusters");
    int res;
    ogroup->add_option("--test1", res);
    ogroup->add_option("--test2", res);
    ogroup->add_option("--test3", res);
    int val2;
    app.add_option("--option", val2);
    ogroup->require_option(1, 1);
    args = {"--test1", "5"};
    run();
    EXPECT_EQ(res, 5);

    args = {"--test1", "5", "--test2", "4"};
    EXPECT_THROW(run(), CLI::RequiredError);

    args = {"--option", "9"};
    EXPECT_THROW(run(), CLI::RequiredError);

    std::string help = ogroup->help();
    auto exactloc = help.find("[Exactly 1");
    EXPECT_NE(exactloc, std::string::npos);
}

TEST_F(TApp, BasicOptionGroupMinMaxDifferent) {
    auto ogroup = app.add_option_group("clusters");
    int res;
    ogroup->add_option("--test1", res);
    ogroup->add_option("--test2", res);
    ogroup->add_option("--test3", res);
    int val2;
    app.add_option("--option", val2);
    ogroup->require_option(1, 2);
    args = {"--test1", "5"};
    run();
    EXPECT_EQ(res, 5);

    args = {"--test1", "5", "--test2", "4"};
    EXPECT_NO_THROW(run());

    args = {"--option", "9"};
    EXPECT_THROW(run(), CLI::RequiredError);

    args = {"--test1", "5", "--test2", "4", "--test3=5"};
    EXPECT_THROW(run(), CLI::RequiredError);

    std::string help = ogroup->help();
    auto exactloc = help.find("[Between 1 and 2");
    EXPECT_NE(exactloc, std::string::npos);
}

TEST_F(TApp, BasicOptionGroupMinMaxDifferentReversed) {
    auto ogroup = app.add_option_group("clusters");
    int res;
    ogroup->add_option("--test1", res);
    ogroup->add_option("--test2", res);
    ogroup->add_option("--test3", res);
    int val2;
    app.add_option("--option", val2);
    ogroup->require_option(2, 1);
    EXPECT_EQ(ogroup->get_require_option_min(), 2);
    EXPECT_EQ(ogroup->get_require_option_max(), 1);
    args = {"--test1", "5"};
    EXPECT_THROW(run(), CLI::InvalidError);
    ogroup->require_option(1, 2);
    EXPECT_NO_THROW(run());
    EXPECT_EQ(res, 5);
    EXPECT_EQ(ogroup->get_require_option_min(), 1);
    EXPECT_EQ(ogroup->get_require_option_max(), 2);
    args = {"--test1", "5", "--test2", "4"};
    EXPECT_NO_THROW(run());

    args = {"--option", "9"};
    EXPECT_THROW(run(), CLI::RequiredError);

    args = {"--test1", "5", "--test2", "4", "--test3=5"};
    EXPECT_THROW(run(), CLI::RequiredError);

    std::string help = ogroup->help();
    auto exactloc = help.find("[Between 1 and 2");
    EXPECT_NE(exactloc, std::string::npos);
}

TEST_F(TApp, BasicOptionGroupMax) {
    auto ogroup = app.add_option_group("clusters");
    int res;
    ogroup->add_option("--test1", res);
    ogroup->add_option("--test2", res);
    ogroup->add_option("--test3", res);
    int val2;
    app.add_option("--option", val2);
    ogroup->require_option(-2);
    args = {"--test1", "5"};
    run();
    EXPECT_EQ(res, 5);

    args = {"--option", "9"};
    EXPECT_NO_THROW(run());

    args = {"--test1", "5", "--test2", "4", "--test3=5"};
    EXPECT_THROW(run(), CLI::RequiredError);

    std::string help = ogroup->help();
    auto exactloc = help.find("[At most 2");
    EXPECT_NE(exactloc, std::string::npos);
}

TEST_F(TApp, BasicOptionGroupMax1) {
    auto ogroup = app.add_option_group("clusters");
    int res;
    ogroup->add_option("--test1", res);
    ogroup->add_option("--test2", res);
    ogroup->add_option("--test3", res);
    int val2;
    app.add_option("--option", val2);
    ogroup->require_option(-1);
    args = {"--test1", "5"};
    run();
    EXPECT_EQ(res, 5);

    args = {"--option", "9"};
    EXPECT_NO_THROW(run());

    args = {"--test1", "5", "--test2", "4"};
    EXPECT_THROW(run(), CLI::RequiredError);

    std::string help = ogroup->help();
    auto exactloc = help.find("[At most 1");
    EXPECT_NE(exactloc, std::string::npos);
}

TEST_F(TApp, BasicOptionGroupMin) {
    auto ogroup = app.add_option_group("clusters");
    int res;
    ogroup->add_option("--test1", res);
    ogroup->add_option("--test2", res);
    ogroup->add_option("--test3", res);
    int val2;
    app.add_option("--option", val2);
    ogroup->require_option();

    args = {"--option", "9"};
    EXPECT_THROW(run(), CLI::RequiredError);

    args = {"--test1", "5", "--test2", "4", "--test3=5"};
    EXPECT_NO_THROW(run());

    std::string help = ogroup->help();
    auto exactloc = help.find("[At least 1");
    EXPECT_NE(exactloc, std::string::npos);
}

TEST_F(TApp, BasicOptionGroupExact2) {
    auto ogroup = app.add_option_group("clusters");
    int res;
    ogroup->add_option("--test1", res);
    ogroup->add_option("--test2", res);
    ogroup->add_option("--test3", res);
    int val2;
    app.add_option("--option", val2);
    ogroup->require_option(2);

    args = {"--option", "9"};
    EXPECT_THROW(run(), CLI::RequiredError);

    args = {"--test1", "5", "--test2", "4", "--test3=5"};
    EXPECT_THROW(run(), CLI::RequiredError);

    args = {"--test1", "5", "--test3=5"};
    EXPECT_NO_THROW(run());

    std::string help = ogroup->help();
    auto exactloc = help.find("[Exactly 2");
    EXPECT_NE(exactloc, std::string::npos);
}

TEST_F(TApp, BasicOptionGroupMin2) {
    auto ogroup = app.add_option_group("clusters");
    int res;
    ogroup->add_option("--test1", res);
    ogroup->add_option("--test2", res);
    ogroup->add_option("--test3", res);
    int val2;
    app.add_option("--option", val2);
    ogroup->require_option(2, 0);

    args = {"--option", "9"};
    EXPECT_THROW(run(), CLI::RequiredError);

    args = {"--test1", "5", "--test2", "4", "--test3=5"};
    EXPECT_NO_THROW(run());

    std::string help = ogroup->help();
    auto exactloc = help.find("[At least 2");
    EXPECT_NE(exactloc, std::string::npos);
}

TEST_F(TApp, BasicOptionGroupMinMoved) {

    int res;
    auto opt1 = app.add_option("--test1", res);
    auto opt2 = app.add_option("--test2", res);
    auto opt3 = app.add_option("--test3", res);
    int val2;
    app.add_option("--option", val2);

    auto ogroup = app.add_option_group("clusters");
    ogroup->require_option();
    ogroup->add_option(opt1);
    ogroup->add_option(opt2);
    ogroup->add_option(opt3);

    args = {"--option", "9"};
    EXPECT_THROW(run(), CLI::RequiredError);

    args = {"--test1", "5", "--test2", "4", "--test3=5"};
    EXPECT_NO_THROW(run());

    std::string help = app.help();
    auto exactloc = help.find("[At least 1");
    auto oloc = help.find("--test1");
    EXPECT_NE(exactloc, std::string::npos);
    EXPECT_NE(oloc, std::string::npos);
    EXPECT_LT(exactloc, oloc);
}

TEST_F(TApp, BasicOptionGroupMinMovedAsGroup) {

    int res;
    auto opt1 = app.add_option("--test1", res);
    auto opt2 = app.add_option("--test2", res);
    auto opt3 = app.add_option("--test3", res);
    int val2;
    app.add_option("--option", val2);

    auto ogroup = app.add_option_group("clusters");
    ogroup->require_option();
    ogroup->add_options(opt1, opt2, opt3);

    EXPECT_THROW(ogroup->add_options(opt1), CLI::OptionNotFound);
    args = {"--option", "9"};
    EXPECT_THROW(run(), CLI::RequiredError);

    args = {"--test1", "5", "--test2", "4", "--test3=5"};
    EXPECT_NO_THROW(run());

    std::string help = app.help();
    auto exactloc = help.find("[At least 1");
    auto oloc = help.find("--test1");
    EXPECT_NE(exactloc, std::string::npos);
    EXPECT_NE(oloc, std::string::npos);
    EXPECT_LT(exactloc, oloc);
}

TEST_F(TApp, BasicOptionGroupAddFailures) {

    int res;
    auto opt1 = app.add_option("--test1", res);
    app.set_config("--config");
    int val2;
    app.add_option("--option", val2);

    auto ogroup = app.add_option_group("clusters");
    EXPECT_THROW(ogroup->add_options(app.get_config_ptr()), CLI::OptionAlreadyAdded);
    EXPECT_THROW(ogroup->add_options(app.get_help_ptr()), CLI::OptionAlreadyAdded);

    auto sub = app.add_subcommand("sub", "subcommand");
    auto opt2 = sub->add_option("--option2", val2);

    EXPECT_THROW(ogroup->add_option(opt2), CLI::OptionNotFound);

    EXPECT_THROW(ogroup->add_options(nullptr), CLI::OptionNotFound);

    ogroup->add_option(opt1);

    auto opt3 = app.add_option("--test1", res);

    EXPECT_THROW(ogroup->add_option(opt3), CLI::OptionAlreadyAdded);
}

TEST_F(TApp, BasicOptionGroupScrewedUpMove) {

    int res;
    auto opt1 = app.add_option("--test1", res);
    auto opt2 = app.add_option("--test2", res);
    int val2;
    app.add_option("--option", val2);

    auto ogroup = app.add_option_group("clusters");
    ogroup->require_option();
    auto ogroup2 = ogroup->add_option_group("clusters2");
    EXPECT_THROW(ogroup2->add_options(opt1, opt2), CLI::OptionNotFound);

    CLI::Option_group EmptyGroup("description", "new group", nullptr);

    EXPECT_THROW(EmptyGroup.add_option(opt2), CLI::OptionNotFound);
    EXPECT_THROW(app._move_option(opt2, ogroup2), CLI::OptionNotFound);
}
/*
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
*/
