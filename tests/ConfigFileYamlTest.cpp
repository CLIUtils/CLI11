// Copyright (c) 2017-2022, University of Cincinnati, developed by Henry Schreiner
// under NSF AWARD 1414736 and by the respective contributors.
// All rights reserved.
//
// SPDX-License-Identifier: BSD-3-Clause

#if CLI11_ENABLE_YAML

#include "app_helper.hpp"

#include <cstdio>
#include <sstream>

using Catch::Matchers::Contains;

TEST_CASE("YamlStringBased: First", "[config]") {
    std::stringstream ofile;

    ofile << "one: three\n";
    ofile << "two: four\n";

    ofile.seekg(0, std::ios::beg);

    std::vector<CLI::ConfigItem> output = CLI::ConfigYAML().from_config(ofile);

    CHECK(output.size() == 2u);
    CHECK(output.at(0).name == "one");
    CHECK(output.at(0).inputs.size() == 1u);
    CHECK(output.at(0).inputs.at(0) == "three");
    CHECK(output.at(1).name == "two");
    CHECK(output.at(1).inputs.size() == 1u);
    CHECK(output.at(1).inputs.at(0) == "four");
}

TEST_CASE("YamlStringBased: FirstWithComments", "[config]") {
    std::stringstream ofile;

    ofile << "# this is a comment\n";
    ofile << "one: three\n";
    ofile << "two: four\n";
    ofile << "--- and another one\n";

    ofile.seekg(0, std::ios::beg);

    std::vector<CLI::ConfigItem> output = CLI::ConfigYAML().from_config(ofile);

    CHECK(output.size() == 2u);
    CHECK(output.at(0).name == "one");
    CHECK(output.at(0).inputs.size() == 1u);
    CHECK(output.at(0).inputs.at(0) == "three");
    CHECK(output.at(1).name == "two");
    CHECK(output.at(1).inputs.size() == 1u);
    CHECK(output.at(1).inputs.at(0) == "four");
}

TEST_CASE("YamlStringBased: Quotes", "[config]") {
    std::stringstream ofile;

    ofile << R"(one:  "three")" << '\n';
    ofile << R"(two: 'four')" << '\n';
    ofile << R"(five: "six and seven")" << '\n';

    ofile.seekg(0, std::ios::beg);

    std::vector<CLI::ConfigItem> output = CLI::ConfigYAML().from_config(ofile);

    CHECK(output.size() == 3u);
    CHECK(output.at(0).name == "one");
    CHECK(output.at(0).inputs.size() == 1u);
    CHECK(output.at(0).inputs.at(0) == "three");
    CHECK(output.at(1).name == "two");
    CHECK(output.at(1).inputs.size() == 1u);
    CHECK(output.at(1).inputs.at(0) == "four");
    CHECK(output.at(2).name == "five");
    CHECK(output.at(2).inputs.size() == 1u);
    CHECK(output.at(2).inputs.at(0) == "six and seven");
}

TEST_CASE("YamlStringBased: Vector", "[config]") {
    std::stringstream ofile;

    ofile << "one: three\n";
    ofile << "two: four\n";
    ofile << "five: [six, and, seven]\n";

    ofile.seekg(0, std::ios::beg);

    std::vector<CLI::ConfigItem> output = CLI::ConfigYAML().from_config(ofile);

    CHECK(output.size() == 3u);
    CHECK(output.at(0).name == "one");
    CHECK(output.at(0).inputs.size() == 1u);
    CHECK(output.at(0).inputs.at(0) == "three");
    CHECK(output.at(1).name == "two");
    CHECK(output.at(1).inputs.size() == 1u);
    CHECK(output.at(1).inputs.at(0) == "four");
    CHECK(output.at(2).name == "five");
    CHECK(output.at(2).inputs.size() == 3u);
    CHECK(output.at(2).inputs.at(0) == "six");
    CHECK(output.at(2).inputs.at(1) == "and");
    CHECK(output.at(2).inputs.at(2) == "seven");
}

TEST_CASE("YamlStringBased: Sections", "[config]") {
    std::stringstream ofile;

    ofile << "one: three\n";
    ofile << "second:\n";
    ofile << "  two: four\n";

    ofile.seekg(0, std::ios::beg);

    std::vector<CLI::ConfigItem> output = CLI::ConfigYAML().from_config(ofile);

    CHECK(output.size() == 4u);
    CHECK(output.at(0).name == "one");
    CHECK(output.at(0).inputs.size() == 1u);
    CHECK(output.at(0).inputs.at(0) == "three");
    CHECK(output.at(2).name == "two");
    CHECK(output.at(2).parents.at(0) == "second");
    CHECK(output.at(2).inputs.size() == 1u);
    CHECK(output.at(2).inputs.at(0) == "four");
    CHECK(output.at(2).fullname() == "second.two");
}

TEST_CASE_METHOD(TApp, "YamlVector", "[config]") {

    TempFile tempYaml{"TestYamlTmp.yaml"};

    app.config_formatter(std::make_shared<CLI::ConfigYAML>());
    app.set_config("--config", tempYaml);

    {
        std::ofstream out{tempYaml};
        out << "two:\n"
               " - 2\n"
               " - 3\n"
               "three:\n"
               " - 1\n"
               " - 2\n"
               " - 3\n"
               "four: [1, 2, 3, 4]\n"
               "five: \"[1, 2, 3, 4, 5]\"\n";
    }

    std::vector<int> two, three, four, five;
    app.add_option("--two", two)->expected(2)->required();
    app.add_option("--three", three)->required();
    app.add_option("--four", four)->required();
    app.add_option("--five", five)->required();

    run();

    CHECK(two == std::vector<int>({2, 3}));
    CHECK(three == std::vector<int>({1, 2, 3}));
    CHECK(four == std::vector<int>({1, 2, 3, 4}));
    CHECK(five == std::vector<int>({1, 2, 3, 4, 5}));
}

TEST_CASE_METHOD(TApp, "YamlVectorMultiple", "[config]") {

    TempFile tempYaml{"TestYamlTmp.yaml"};

    app.config_formatter(std::make_shared<CLI::ConfigYAML>());

    app.set_config("--config", tempYaml);

    {
        std::ofstream out{tempYaml};
        out << "#this is a comment line\n"
               "three: 1\n"
               "two: 2\n"
               "three: 2\n"
               "two: 3\n"
               "three: 3\n";
    }

    std::vector<int> two, three;
    app.add_option("--two", two)->expected(2)->required();
    app.add_option("--three", three)->required();

    run();

    CHECK(two == std::vector<int>({2, 3}));
    CHECK(three == std::vector<int>({1, 2, 3}));
}

TEST_CASE_METHOD(TApp, "YamlLayered", "[config]") {

    TempFile tempYaml{"TestYamlTmp.yaml"};

    app.config_formatter(std::make_shared<CLI::ConfigYAML>());
    app.set_config("--config", tempYaml);

    {
        std::ofstream out{tempYaml};
        out << "val: 1\n"
               "subcom:\n"
               " - val: 2\n"
               " - subsubcom:\n"
               "   - val: 3\n";
    }

    int one{0}, two{0}, three{0};
    app.add_option("--val", one);
    auto subcom = app.add_subcommand("subcom");
    subcom->add_option("--val", two);
    auto subsubcom = subcom->add_subcommand("subsubcom");
    subsubcom->add_option("--val", three);

    run();

    CHECK(one == 1);
    CHECK(two == 2);
    CHECK(three == 3);

    CHECK(0U == subcom->count());
    CHECK(!*subcom);
}

TEST_CASE_METHOD(TApp, "YamlLayeredStream", "[config]") {

    TempFile tempYaml{"TestYamlTmp.yaml"};

    app.config_formatter(std::make_shared<CLI::ConfigYAML>());
    app.set_config("--config", tempYaml);

    {
        std::ofstream out{tempYaml};
        out << "val: 1\n"
               "subcom:\n"
               " - val: 2\n"
               " - subsubcom:\n"
               "   - val: 3\n";
    }

    int one{0}, two{0}, three{0};
    app.add_option("--val", one);
    auto subcom = app.add_subcommand("subcom");
    subcom->add_option("--val", two);
    auto subsubcom = subcom->add_subcommand("subsubcom");
    subsubcom->add_option("--val", three);

    std::ifstream in{tempYaml};
    app.parse_from_stream(in);

    CHECK(one == 1);
    CHECK(two == 2);
    CHECK(three == 3);

    CHECK(0U == subcom->count());
    CHECK(!*subcom);
}

TEST_CASE_METHOD(TApp, "YamlLayeredDotSection", "[config]") {

    TempFile tempYaml{"TestYamlTmp.yaml"};

    app.config_formatter(std::make_shared<CLI::ConfigYAML>());
    app.set_config("--config", tempYaml);

    {
        std::ofstream out{tempYaml};
        out << "val: 1" << std::endl;
        out << "subcom: " << std::endl;
        out << "  - val: 2" << std::endl;
        out << "  - subsubcom:" << std::endl;
        out << "    - val: 3" << std::endl;
    }

    int one{0}, two{0}, three{0};
    app.add_option("--val", one);
    auto subcom = app.add_subcommand("subcom");
    subcom->add_option("--val", two);
    auto subsubcom = subcom->add_subcommand("subsubcom");
    subsubcom->add_option("--val", three);

    run();

    CHECK(one == 1);
    CHECK(two == 2);
    CHECK(three == 3);

    CHECK(0U == subcom->count());
    CHECK(!*subcom);

    three = 0;
    // check maxlayers
    app.get_config_formatter_base()->maxLayers(1);
    run();
    CHECK(three == 0);
}

TEST_CASE_METHOD(TApp, "YamlLayeredOptionGroupAlias", "[config]") {

    TempFile tempYaml{"TestYamlTmp.yaml"};

    app.config_formatter(std::make_shared<CLI::ConfigYAML>());
    app.set_config("--config", tempYaml);

    {
        std::ofstream out{tempYaml};
        out << "val: 1" << std::endl;
        out << "ogroup: " << std::endl;
        out << "  val2: 2" << std::endl;
    }
    int one{0}, two{0};
    app.add_option("--val", one);
    auto subcom = app.add_option_group("ogroup")->alias("ogroup");
    subcom->add_option("--val2", two);

    run();

    CHECK(one == 1);
    CHECK(two == 2);
}

TEST_CASE_METHOD(TApp, "YamlSubcommandConfigurable", "[config]") {

    TempFile tempYaml{"TestYamlTmp.yaml"};

    app.config_formatter(std::make_shared<CLI::ConfigYAML>());

    app.set_config("--config", tempYaml);

    {
        std::ofstream out{tempYaml};
        out << "val: 1" << std::endl;
        out << "subcom:" << std::endl;
        out << "  val: 2" << std::endl;
        out << "  subsubcom:" << std::endl;
        out << "    val: 3" << std::endl;
    }

    int one{0}, two{0}, three{0};
    app.add_option("--val", one);
    auto subcom = app.add_subcommand("subcom");
    subcom->configurable();
    subcom->add_option("--val", two);
    auto subsubcom = subcom->add_subcommand("subsubcom");
    subsubcom->add_option("--val", three);

    run();

    CHECK(one == 1);
    CHECK(two == 2);
    CHECK(three == 3);

    CHECK(1U == subcom->count());
    CHECK(*subcom);
    CHECK(app.got_subcommand(subcom));
}

TEST_CASE_METHOD(TApp, "YamlSubcommandConfigurablePreParse", "[config]") {

    TempFile tempYaml{"TestYamlTmp.yaml"};

    app.config_formatter(std::make_shared<CLI::ConfigYAML>());
    app.set_config("--config", tempYaml);

    {
        std::ofstream out{tempYaml};
        out << "val: 1" << std::endl;
        out << "subcom:" << std::endl;
        out << "  val: 2" << std::endl;
        out << "  subsubcom:" << std::endl;
        out << "    val: 3" << std::endl;
    }

    int one{0}, two{0}, three{0}, four{0};
    app.add_option("--val", one);
    auto subcom = app.add_subcommand("subcom");
    auto subcom2 = app.add_subcommand("subcom2");
    subcom->configurable();
    std::vector<std::size_t> parse_c;
    subcom->preparse_callback([&parse_c](std::size_t cnt) { parse_c.push_back(cnt); });
    subcom->add_option("--val", two);
    subcom2->add_option("--val", four);
    subcom2->preparse_callback([&parse_c](std::size_t cnt) { parse_c.push_back(cnt + 2623); });
    auto subsubcom = subcom->add_subcommand("subsubcom");
    subsubcom->add_option("--val", three);

    run();

    CHECK(one == 1);
    CHECK(two == 2);
    CHECK(three == 3);
    CHECK(four == 0);

    CHECK(1U == parse_c.size());
    CHECK(2U == parse_c[0]);

    CHECK(0U == subcom2->count());
}

//TEST_CASE_METHOD(TApp, "YamlSection", "[config]") {
//
//    TempFile tempYaml{"TestYamlTmp.yaml"};
//
//    app.config_formatter(std::make_shared<CLI::ConfigYAML>());
//    app.set_config("--config", tempYaml);
//    app.get_config_formatter_base()->section("config");
//
//    {
//        std::ofstream out{tempYaml};
//        out << "config:" << std::endl;
//        out << "  val: 2" << std::endl;
//        out << "  subsubcom:" << std::endl;
//        out << "    val: 3" << std::endl;
//        out << "val: 1" << std::endl;
//    }
//
//    int val{0};
//    app.add_option("--val", val);
//
//    run();
//
//    CHECK(2 == val);
//}

//TEST_CASE_METHOD(TApp, "YamlSection2", "[config]") {
//
//    TempFile tempYaml{"TestYamlTmp.yaml"};
//
//    app.config_formatter(std::make_shared<CLI::ConfigYAML>());
//    app.set_config("--config", tempYaml);
//    app.get_config_formatter_base()->section("config");
//
//    {
//        std::ofstream out{tempYaml};
//        out << "val: 1" << std::endl;
//        out << "config:" << std::endl;
//        out << "  val: 2" << std::endl;
//        out << "  subsubcom:" << std::endl;
//        out << "    val: 3" << std::endl;
//    }
//
//    int val{0};
//    app.add_option("--val", val);
//
//    run();
//
//    CHECK(2 == val);
//}


//TEST_CASE_METHOD(TApp, "IniSubcommandConfigurableParseComplete", "[config]") {
//
//    TempFile tempYaml{"TestYamlTmp.yaml"};
//
//    app.config_formatter(std::make_shared<CLI::ConfigYAML>());
//    app.set_config("--config", tempYaml);
//
//    {
//        std::ofstream out{tempYaml};
//        out << "val: 1" << std::endl;
//        out << "subcom:" << std::endl;
//        out << "  val: 2" << std::endl;
//        out << "  subsubcom:" << std::endl;
//        out << "    val: 3" << std::endl;
//    }
//
//    int one{0}, two{0}, three{0}, four{0};
//    app.add_option("--val", one);
//    auto subcom = app.add_subcommand("subcom");
//    auto subcom2 = app.add_subcommand("subcom2");
//    subcom->configurable();
//    std::vector<std::size_t> parse_c;
//    subcom->parse_complete_callback([&parse_c]() { parse_c.push_back(58); });
//    subcom->add_option("--val", two);
//    subcom2->add_option("--val", four);
//    subcom2->parse_complete_callback([&parse_c]() { parse_c.push_back(2623); });
//    auto subsubcom = subcom->add_subcommand("subsubcom");
//    // configurable should be inherited
//    subsubcom->parse_complete_callback([&parse_c]() { parse_c.push_back(68); });
//    subsubcom->add_option("--val", three);
//
//    run();
//
//    CHECK(one == 1);
//    CHECK(two == 2);
//    CHECK(three == 3);
//    CHECK(four == 0);
//
//    REQUIRE(2u == parse_c.size());
//    CHECK(68U == parse_c[0]);
//    CHECK(58U == parse_c[1]);
//    CHECK(1u == subsubcom->count());
//    CHECK(0u == subcom2->count());
//}

//TEST_CASE_METHOD(TApp, "YamlSubcommandMultipleSections", "[config]") {
//
//    TempFile tempYaml{"TestYamlTmp.yaml"};
//
//    app.config_formatter(std::make_shared<CLI::ConfigYAML>());
//    app.set_config("--config", tempYaml);
//
//    {
//        std::ofstream out{tempYaml};
//        out << "val: 1" << std::endl;
//        out << "subcom:" << std::endl;
//        out << "  val: 2" << std::endl;
//        out << "  subsubcom:" << std::endl;
//        out << "    val: 3" << std::endl;
//        out << "subcom2:" << std::endl;
//        out << "  val: 4" << std::endl;
//    }
//
//    int one{0}, two{0}, three{0}, four{0};
//    app.add_option("--val", one);
//    auto subcom = app.add_subcommand("subcom");
//    auto subcom2 = app.add_subcommand("subcom2");
//    subcom->configurable();
//    std::vector<std::size_t> parse_c;
//    subcom->parse_complete_callback([&parse_c]() { parse_c.push_back(58); });
//    subcom->add_option("--val", two);
//    subcom2->add_option("--val", four);
//    subcom2->parse_complete_callback([&parse_c]() { parse_c.push_back(2623); });
//    subcom2->configurable(false);
//    auto subsubcom = subcom->add_subcommand("subsubcom");
//    // configurable should be inherited
//    subsubcom->parse_complete_callback([&parse_c]() { parse_c.push_back(68); });
//    subsubcom->add_option("--val", three);
//
//    run();
//
//    CHECK(one == 1);
//    CHECK(two == 2);
//    CHECK(three == 3);
//    CHECK(four == 4);
//
//    REQUIRE(2u == parse_c.size());
//    CHECK(68U == parse_c[0]);
//    CHECK(58U == parse_c[1]);
//    CHECK(1u == subsubcom->count());
//    CHECK(0u == subcom2->count());
//}

//TEST_CASE_METHOD(TApp, "YamlDuplicateSubcommandCallbacks", "[config]") {
//
//    TempFile tempYaml{"TestYamlTmp.yaml"};
//
//    app.config_formatter(std::make_shared<CLI::ConfigYAML>());
//    app.set_config("--config", tempYaml);
//
//    {
//        std::ofstream out{tempYaml};
//        out << "foo:" << std::endl;
//        out << "foo:" << std::endl;
//        out << "foo:" << std::endl;
//    }
//
//    auto foo = app.add_subcommand("foo");
//    int count{0};
//    foo->callback([&count]() { ++count; });
//    foo->immediate_callback();
//    CHECK(foo->get_immediate_callback());
//    foo->configurable();
//
//    run();
//    CHECK(3 == count);
//}

TEST_CASE_METHOD(TApp, "YamlFailure", "[config]") {

    TempFile tempYaml{"TestYamlTmp.yaml"};

    app.config_formatter(std::make_shared<CLI::ConfigYAML>());
    app.set_config("--config", tempYaml);
    app.allow_config_extras(false);
    {
        std::ofstream out{tempYaml};
        out << "val: 1" << std::endl;
    }

    CHECK_THROWS_AS(run(), CLI::ConfigError);
}

TEST_CASE_METHOD(TApp, "YamlConfigurable", "[config]") {

    TempFile tempYaml{"TestYamlTmp.yaml"};

    app.config_formatter(std::make_shared<CLI::ConfigYAML>());
    app.set_config("--config", tempYaml);
    bool value{false};
    app.add_flag("--val", value)->configurable(true);

    {
        std::ofstream out{tempYaml};
        out << "val: 1" << std::endl;
    }

    REQUIRE_NOTHROW(run());
    CHECK(value);
}

TEST_CASE_METHOD(TApp, "YamlNotConfigurable", "[config]") {

    TempFile tempYaml{"TestYamlTmp.yaml"};

    app.config_formatter(std::make_shared<CLI::ConfigYAML>());
    app.set_config("--config", tempYaml);
    bool value{false};
    app.add_flag("--val", value)->configurable(false);

    {
        std::ofstream out{tempYaml};
        out << "val: true" << std::endl;
    }

    CHECK_THROWS_AS(run(), CLI::ConfigError);
}

TEST_CASE_METHOD(TApp, "YamlSubFailure", "[config]") {

    TempFile tempYaml{"TestYamlTmp.yaml"};

    app.config_formatter(std::make_shared<CLI::ConfigYAML>());
    app.set_config("--config", tempYaml);

    app.add_subcommand("other");
    app.set_config("--config", tempYaml);
    app.allow_config_extras(false);
    {
        std::ofstream out{tempYaml};
        out << "other:" << std::endl;
        out << " - val: 1" << std::endl;
    }

    CHECK_THROWS_AS(run(), CLI::ConfigError);
}

TEST_CASE_METHOD(TApp, "YamlNoSubFailure", "[config]") {

    TempFile tempYaml{"TestYamlTmp.yaml"};

    app.config_formatter(std::make_shared<CLI::ConfigYAML>());
    app.set_config("--config", tempYaml);
    app.allow_config_extras(CLI::config_extras_mode::error);
    {
        std::ofstream out{tempYaml};
        out << "other:" << std::endl;
        out << " - val: 1" << std::endl;
    }

    CHECK_THROWS_AS(run(), CLI::ConfigError);
}

TEST_CASE_METHOD(TApp, "YamlFlagConvertFailure", "[config]") {

    TempFile tempYaml{"TestYamlTmp.yaml"};

    app.config_formatter(std::make_shared<CLI::ConfigYAML>());
    app.add_flag("--flag");
    app.set_config("--config", tempYaml);

    {
        std::ofstream out{tempYaml};
        out << "flag: moobook" << std::endl;
    }
    run();
    bool result{false};
    auto *opt = app.get_option("--flag");
    CHECK_THROWS_AS(opt->results(result), CLI::ConversionError);
    std::string res;
    opt->results(res);
    CHECK("moobook" == res);
}

TEST_CASE_METHOD(TApp, "YamlFlagNumbers", "[config]") {

    TempFile tempYaml{"TestYamlTmp.yaml"};

    app.config_formatter(std::make_shared<CLI::ConfigYAML>());

    bool boo{false};
    app.add_flag("--flag", boo);
    app.set_config("--config", tempYaml);

    {
        std::ofstream out{tempYaml};
        out << "flag: 3" << std::endl;
    }

    REQUIRE_NOTHROW(run());
    CHECK(boo);
}

TEST_CASE_METHOD(TApp, "YamlFlagDual", "[config]") {

    TempFile tempYaml{"TestYamlTmp.yaml"};

    bool boo{false};
    app.config_formatter(std::make_shared<CLI::ConfigYAML>());
    app.add_flag("--flag", boo);
    app.set_config("--config", tempYaml);

    {
        std::ofstream out{tempYaml};
        out << "flag: [1, 1]" << std::endl;
    }

    CHECK_THROWS_AS(run(), CLI::ConversionError);
}

TEST_CASE_METHOD(TApp, "Yaml IniShort", "[config]") {

    TempFile tempYaml{"TestYamlTmp.yaml"};

    int key{0};
    app.config_formatter(std::make_shared<CLI::ConfigYAML>());
    app.add_option("--flag,-f", key);
    app.set_config("--config", tempYaml);

    {
        std::ofstream out{tempYaml};
        out << "f: 3" << std::endl;
    }

    REQUIRE_NOTHROW(run());
    CHECK(3 == key);
}

TEST_CASE_METHOD(TApp, "YamlDefaultPath", "[config]") {

    TempFile tempYaml{"../TestYamlTmp.yaml"};

    app.config_formatter(std::make_shared<CLI::ConfigYAML>());

    int key{0};
    app.add_option("--flag,-f", key);
    app.set_config("--config", "TestYamlTmp.yaml")->transform(CLI::FileOnDefaultPath("../"));

    {
        std::ofstream out{tempYaml};
        out << "f: 3" << std::endl;
    }

    REQUIRE_NOTHROW(run());
    CHECK(3 == key);
}

TEST_CASE_METHOD(TApp, "YamlMultipleDefaultPath", "[config]") {

    TempFile tempYaml{"../TestYamlTmp.yaml"};

    app.config_formatter(std::make_shared<CLI::ConfigYAML>());

    int key{0};
    app.add_option("--flag,-f", key);
    auto *cfgOption = app.set_config("--config", "doesnotexist.yaml")
                          ->transform(CLI::FileOnDefaultPath("../"))
                          ->transform(CLI::FileOnDefaultPath("../other", false));

    {
        std::ofstream out{tempYaml};
        out << "f: 3" << std::endl;
    }

    args = {"--config", "TestYamlTmp.yaml"};
    REQUIRE_NOTHROW(run());
    CHECK(3 == key);
    CHECK(cfgOption->as<std::string>() == "../TestYamlTmp.yaml");
}

TEST_CASE_METHOD(TApp, "YamlMultipleDefaultPathAlternate", "[config]") {

    TempFile tempYaml{"../TestYamlTmp.yaml"};

    app.config_formatter(std::make_shared<CLI::ConfigYAML>());

    int key{0};
    app.add_option("--flag,-f", key);
    auto *cfgOption = app.set_config("--config", "doesnotexist.yaml")
                          ->transform(CLI::FileOnDefaultPath("../other") | CLI::FileOnDefaultPath("../"));

    {
        std::ofstream out{tempYaml};
        out << "f: 3" << std::endl;
    }

    args = {"--config", "TestYamlTmp.yaml"};
    REQUIRE_NOTHROW(run());
    CHECK(3 == key);
    CHECK(cfgOption->as<std::string>() == "../TestYamlTmp.yaml");
}

TEST_CASE_METHOD(TApp, "IniPositional", "[config]") {

    TempFile tempYaml{"TestYamlTmp.yaml"};

    app.config_formatter(std::make_shared<CLI::ConfigYAML>());

    int key{0};
    app.add_option("key", key);
    app.set_config("--config", tempYaml);

    {
        std::ofstream out{tempYaml};
        out << "key: 3" << std::endl;
    }

    REQUIRE_NOTHROW(run());
    CHECK(3 == key);
}

TEST_CASE_METHOD(TApp, "IniEnvironmental", "[config]") {

    TempFile tempYaml{"TestYamlTmp.yaml"};

    app.config_formatter(std::make_shared<CLI::ConfigYAML>());

    int key{0};
    app.add_option("key", key)->envname("CLI11_TEST_ENV_KEY_TMP");
    app.set_config("--config", tempYaml);

    {
        std::ofstream out{tempYaml};
        out << "CLI11_TEST_ENV_KEY_TMP: 3" << std::endl;
    }

    REQUIRE_NOTHROW(run());
    CHECK(3 == key);
}

TEST_CASE_METHOD(TApp, "YamlFlagText", "[config]") {

    TempFile tempYaml{"TestYamlTmp.yaml"};

    app.config_formatter(std::make_shared<CLI::ConfigYAML>());

    bool flag1{false}, flag2{false}, flag3{false}, flag4{false};
    app.add_flag("--flag1", flag1);
    app.add_flag("--flag2", flag2);
    app.add_flag("--flag3", flag3);
    app.add_flag("--flag4", flag4);
    app.set_config("--config", tempYaml);

    {
        std::ofstream out{tempYaml};
        out << "flag1: true\n"
               "flag2: on\n"
               "flag3: off\n"
               "flag4: 1" << std::endl;
    }

    run();

    CHECK(flag1);
    CHECK(flag2);
    CHECK(!flag3);
    CHECK(flag4);
}

TEST_CASE_METHOD(TApp, "Yaml IniFlags", "[config]") {
    TempFile tempYaml{"TestYamlTmp.yaml"};

    app.config_formatter(std::make_shared<CLI::ConfigYAML>());
    app.set_config("--config", tempYaml);

    {
        std::ofstream out{tempYaml};
        out << "two: 2\n"
               "three: true\n"
               "four: on\n"
               "five\n";
    }

    int two{0};
    bool three{false}, four{false}, five{false};
    app.add_flag("--two", two);
    app.add_flag("--three", three);
    app.add_flag("--four", four);
    app.add_flag("--five", five);

    run();

    CHECK(two == 2);
    CHECK(three);
    CHECK(four);
    CHECK(five);
}

TEST_CASE_METHOD(TApp, "Yaml IniFalseFlags", "[config]") {
    TempFile tempYaml{"TestYamlTmp.yaml"};

    app.config_formatter(std::make_shared<CLI::ConfigYAML>());
    app.set_config("--config", tempYaml);

    {
        std::ofstream out{tempYaml};
        out << "two: -2\n"
               "three: false\n"
               "four: 1\n"
               "five\n";
    }

    int two{0};
    bool three{false}, four{false}, five{false};
    app.add_flag("--two", two);
    app.add_flag("--three", three);
    app.add_flag("--four", four);
    app.add_flag("--five", five);

    run();

    CHECK(two == -2);
    CHECK(!three);
    CHECK(four);
    CHECK(five);
}

TEST_CASE_METHOD(TApp, "YamlFalseFlagsDef", "[config]") {
    TempFile tempYaml{"TestYamlTmp.yaml"};

    app.config_formatter(std::make_shared<CLI::ConfigYAML>());
    app.set_config("--config", tempYaml);

    {
        std::ofstream out{tempYaml};
        out << "two: 2\n"
               "three: true\n"
               "four: on\n"
               "five" << std::endl;
    }

    int two{0};
    bool three{false}, four{false}, five{false};
    app.add_flag("--two{false}", two);
    app.add_flag("--three", three);
    app.add_flag("!--four", four);
    app.add_flag("--five", five);

    run();

    CHECK(two == -2);
    CHECK(three);
    CHECK(!four);
    CHECK(five);
}

TEST_CASE_METHOD(TApp, "YamlFalseFlagsDefDisableOverrideError", "[config]") {
    TempFile tempYaml{"TestYamlTmp.yaml"};

    app.config_formatter(std::make_shared<CLI::ConfigYAML>());
    app.set_config("--config", tempYaml);

    {
        std::ofstream out{tempYaml};
        out << "two: 2" << std::endl;
        out << "four: on" << std::endl;
        out << "five" << std::endl;
    }

    int two{0};
    bool four{false}, five{false};
    app.add_flag("--two{false}", two)->disable_flag_override();
    app.add_flag("!--four", four);
    app.add_flag("--five", five);

    CHECK_THROWS_AS(run(), CLI::ArgumentMismatch);
}

TEST_CASE_METHOD(TApp, "YamlFalseFlagsDefDisableOverrideSuccess", "[config]") {
    TempFile tempYaml{"TestYamlTmp.yaml"};

    app.config_formatter(std::make_shared<CLI::ConfigYAML>());
    app.set_config("--config", tempYaml);

    {
        std::ofstream out{tempYaml};
        out << "two: 2" << std::endl;
        out << "four: []" << std::endl;
        out << "val: 15" << std::endl;
    }

    int two{0}, four{0}, val{0};
    app.add_flag("--two{2}", two)->disable_flag_override();
    app.add_flag("--four{4}", four)->disable_flag_override();
    app.add_flag("--val", val);

    run();

    CHECK(two == 2);
    CHECK(four == 4);
    CHECK(val == 15);
}

TEST_CASE_METHOD(TApp, "YamlOutputSimple", "[config]") {

    app.config_formatter(std::make_shared<CLI::ConfigYAML>());

    int v{0};
    app.add_option("--simple", v);

    args = {"--simple=3"};

    run();

    std::string str = app.config_to_str();
    CHECK(str == "simple: 3\n");
}

TEST_CASE_METHOD(TApp, "YamlOutputShort", "[config]") {

    app.config_formatter(std::make_shared<CLI::ConfigYAML>());

    int v{0};
    app.add_option("-s", v);

    args = {"-s3"};

    run();

    std::string str = app.config_to_str();
    CHECK(str == "s: 3\n");
}

TEST_CASE_METHOD(TApp, "YamlOutputPositional", "[config]") {

    app.config_formatter(std::make_shared<CLI::ConfigYAML>());

    int v{0};
    app.add_option("pos", v);

    args = {"3"};

    run();

    std::string str = app.config_to_str();
    CHECK(str == "pos: 3\n");
}

//// try the output with environmental only arguments
//TEST_CASE_METHOD(TApp, "TomlOutputEnvironmental", "[config]") {
//
//    put_env("CLI11_TEST_ENV_TMP", "2");
//
//    int val{1};
//    app.add_option(std::string{}, val)->envname("CLI11_TEST_ENV_TMP");
//
//    run();
//
//    CHECK(val == 2);
//    std::string str = app.config_to_str();
//    CHECK(str == "CLI11_TEST_ENV_TMP=2\n");
//
//    unset_env("CLI11_TEST_ENV_TMP");
//}
//
//TEST_CASE_METHOD(TApp, "TomlOutputNoConfigurable", "[config]") {
//
//    int v1{0}, v2{0};
//    app.add_option("--simple", v1);
//    app.add_option("--noconf", v2)->configurable(false);
//
//    args = {"--simple=3", "--noconf=2"};
//
//    run();
//
//    std::string str = app.config_to_str();
//    CHECK(str == "simple=3\n");
//}
//
//TEST_CASE_METHOD(TApp, "TomlOutputShortSingleDescription", "[config]") {
//    std::string flag = "some_flag";
//    const std::string description = "Some short description.";
//    app.add_flag("--" + flag, description);
//
//    run();
//
//    std::string str = app.config_to_str(true, true);
//    CHECK_THAT(str, Contains("# " + description + "\n" + flag + "=false\n"));
//}
//
//TEST_CASE_METHOD(TApp, "TomlOutputShortDoubleDescription", "[config]") {
//    std::string flag1 = "flagnr1";
//    std::string flag2 = "flagnr2";
//    const std::string description1 = "First description.";
//    const std::string description2 = "Second description.";
//    app.add_flag("--" + flag1, description1);
//    app.add_flag("--" + flag2, description2);
//
//    run();
//
//    std::string str = app.config_to_str(true, true);
//    std::string ans = "# " + description1 + "\n" + flag1 + "=false\n\n# " + description2 + "\n" + flag2 + "=false\n";
//    CHECK_THAT(str, Contains(ans));
//}
//
//TEST_CASE_METHOD(TApp, "TomlOutputGroups", "[config]") {
//    std::string flag1 = "flagnr1";
//    std::string flag2 = "flagnr2";
//    const std::string description1 = "First description.";
//    const std::string description2 = "Second description.";
//    app.add_flag("--" + flag1, description1)->group("group1");
//    app.add_flag("--" + flag2, description2)->group("group2");
//
//    run();
//
//    std::string str = app.config_to_str(true, true);
//    CHECK_THAT(str, Contains("group1"));
//    CHECK_THAT(str, Contains("group2"));
//}
//
//TEST_CASE_METHOD(TApp, "TomlOutputHiddenOptions", "[config]") {
//    std::string flag1 = "flagnr1";
//    std::string flag2 = "flagnr2";
//    double val{12.7};
//    const std::string description1 = "First description.";
//    const std::string description2 = "Second description.";
//    app.add_flag("--" + flag1, description1)->group("group1");
//    app.add_flag("--" + flag2, description2)->group("group2");
//    app.add_option("--dval", val)->capture_default_str()->group("");
//
//    run();
//
//    std::string str = app.config_to_str(true, true);
//    CHECK_THAT(str, Contains("group1"));
//    CHECK_THAT(str, Contains("group2"));
//    CHECK_THAT(str, Contains("dval=12.7"));
//    auto loc = str.find("dval=12.7");
//    auto locg1 = str.find("group1");
//    CHECK(loc < locg1);
//    // make sure it doesn't come twice
//    loc = str.find("dval=12.7", loc + 4);
//    CHECK(std::string::npos == loc);
//}
//
//TEST_CASE_METHOD(TApp, "TomlOutputAppMultiLineDescription", "[config]") {
//    app.description("Some short app description.\n"
//                    "That has multiple lines.");
//    run();
//
//    std::string str = app.config_to_str(true, true);
//    CHECK_THAT(str, Contains("# Some short app description.\n"));
//    CHECK_THAT(str, Contains("# That has multiple lines.\n"));
//}
//
//TEST_CASE_METHOD(TApp, "TomlOutputMultiLineDescription", "[config]") {
//    std::string flag = "some_flag";
//    const std::string description = "Some short description.\nThat has lines.";
//    app.add_flag("--" + flag, description);
//
//    run();
//
//    std::string str = app.config_to_str(true, true);
//    CHECK_THAT(str, Contains("# Some short description.\n"));
//    CHECK_THAT(str, Contains("# That has lines.\n"));
//    CHECK_THAT(str, Contains(flag + "=false\n"));
//}
//
//TEST_CASE_METHOD(TApp, "TomlOutputOptionGroupMultiLineDescription", "[config]") {
//    std::string flag = "flag";
//    const std::string description = "Short flag description.\n";
//    auto og = app.add_option_group("group");
//    og->description("Option group description.\n"
//                    "That has multiple lines.");
//    og->add_flag("--" + flag, description);
//    run();
//
//    std::string str = app.config_to_str(true, true);
//    CHECK_THAT(str, Contains("# Option group description.\n"));
//    CHECK_THAT(str, Contains("# That has multiple lines.\n"));
//}
//
//TEST_CASE_METHOD(TApp, "TomlOutputSubcommandMultiLineDescription", "[config]") {
//    std::string flag = "flag";
//    const std::string description = "Short flag description.\n";
//    auto subcom = app.add_subcommand("subcommand");
//    subcom->configurable();
//    subcom->description("Subcommand description.\n"
//                        "That has multiple lines.");
//    subcom->add_flag("--" + flag, description);
//    run();
//
//    std::string str = app.config_to_str(true, true);
//    CHECK_THAT(str, Contains("# Subcommand description.\n"));
//    CHECK_THAT(str, Contains("# That has multiple lines.\n"));
//}
//
//TEST_CASE_METHOD(TApp, "TomlOutputOptionGroup", "[config]") {
//    std::string flag1 = "flagnr1";
//    std::string flag2 = "flagnr2";
//    double val{12.7};
//    const std::string description1 = "First description.";
//    const std::string description2 = "Second description.";
//    app.add_flag("--" + flag1, description1)->group("group1");
//    app.add_flag("--" + flag2, description2)->group("group2");
//    auto og = app.add_option_group("group3", "g3 desc");
//    og->add_option("--dval", val)->capture_default_str()->group("");
//
//    run();
//
//    std::string str = app.config_to_str(true, true);
//    CHECK_THAT(str, Contains("group1"));
//    CHECK_THAT(str, Contains("group2"));
//    CHECK_THAT(str, Contains("dval=12.7"));
//    CHECK_THAT(str, Contains("group3"));
//    CHECK_THAT(str, Contains("g3 desc"));
//    auto loc = str.find("dval=12.7");
//    auto locg1 = str.find("group1");
//    auto locg3 = str.find("group3");
//    CHECK(loc > locg1);
//    // make sure it doesn't come twice
//    loc = str.find("dval=12.7", loc + 4);
//    CHECK(std::string::npos == loc);
//    CHECK(locg1 < locg3);
//}
//
//TEST_CASE_METHOD(TApp, "TomlOutputVector", "[config]") {
//
//    std::vector<int> v;
//    app.add_option("--vector", v);
//    app.config_formatter(std::make_shared<CLI::ConfigTOML>());
//    args = {"--vector", "1", "2", "3"};
//
//    run();
//
//    std::string str = app.config_to_str();
//    CHECK(str == "vector=[1, 2, 3]\n");
//}
//
//TEST_CASE_METHOD(TApp, "TomlOutputTuple", "[config]") {
//
//    std::tuple<double, double, double, double> t;
//    app.add_option("--tuple", t);
//    app.config_formatter(std::make_shared<CLI::ConfigTOML>());
//    args = {"--tuple", "1", "2", "3", "4"};
//
//    run();
//
//    std::string str = app.config_to_str();
//    CHECK(str == "tuple=[1, 2, 3, 4]\n");
//}
//
//TEST_CASE_METHOD(TApp, "ConfigOutputVectorCustom", "[config]") {
//
//    std::vector<int> v;
//    app.add_option("--vector", v);
//    auto V = std::make_shared<CLI::ConfigBase>();
//    V->arrayBounds('{', '}')->arrayDelimiter(';')->valueSeparator(':');
//    app.config_formatter(V);
//    args = {"--vector", "1", "2", "3"};
//
//    run();
//
//    std::string str = app.config_to_str();
//    CHECK(str == "vector:{1; 2; 3}\n");
//}
//
//TEST_CASE_METHOD(TApp, "TomlOutputFlag", "[config]") {
//
//    int v{0}, q{0};
//    app.add_option("--simple", v);
//    app.add_flag("--nothing");
//    app.add_flag("--onething");
//    app.add_flag("--something", q);
//
//    args = {"--simple=3", "--onething", "--something", "--something"};
//
//    run();
//
//    std::string str = app.config_to_str();
//    CHECK_THAT(str, Contains("simple=3"));
//    CHECK_THAT(str, !Contains("nothing"));
//    CHECK_THAT(str, Contains("onething=true"));
//    CHECK_THAT(str, Contains("something=2"));
//
//    str = app.config_to_str(true);
//    CHECK_THAT(str, Contains("nothing"));
//}
//
//TEST_CASE_METHOD(TApp, "TomlOutputSet", "[config]") {
//
//    int v{0};
//    app.add_option("--simple", v)->check(CLI::IsMember({1, 2, 3}));
//
//    args = {"--simple=2"};
//
//    run();
//
//    std::string str = app.config_to_str();
//    CHECK_THAT(str, Contains("simple=2"));
//}
//
//TEST_CASE_METHOD(TApp, "TomlOutputDefault", "[config]") {
//
//    int v{7};
//    app.add_option("--simple", v)->capture_default_str();
//
//    run();
//
//    std::string str = app.config_to_str();
//    CHECK_THAT(str, !Contains("simple=7"));
//
//    str = app.config_to_str(true);
//    CHECK_THAT(str, Contains("simple=7"));
//}
//
//TEST_CASE_METHOD(TApp, "TomlOutputSubcom", "[config]") {
//
//    app.add_flag("--simple");
//    auto subcom = app.add_subcommand("other");
//    subcom->add_flag("--newer");
//
//    args = {"--simple", "other", "--newer"};
//    run();
//
//    std::string str = app.config_to_str();
//    CHECK_THAT(str, Contains("simple=true"));
//    CHECK_THAT(str, Contains("other.newer=true"));
//}
//
//TEST_CASE_METHOD(TApp, "TomlOutputSubcomConfigurable", "[config]") {
//
//    app.add_flag("--simple");
//    auto subcom = app.add_subcommand("other")->configurable();
//    subcom->add_flag("--newer");
//
//    args = {"--simple", "other", "--newer"};
//    run();
//
//    std::string str = app.config_to_str();
//    CHECK_THAT(str, Contains("simple=true"));
//    CHECK_THAT(str, Contains("[other]"));
//    CHECK_THAT(str, Contains("newer=true"));
//    CHECK(std::string::npos == str.find("other.newer=true"));
//}
//
//TEST_CASE_METHOD(TApp, "TomlOutputSubsubcom", "[config]") {
//
//    app.add_flag("--simple");
//    auto subcom = app.add_subcommand("other");
//    subcom->add_flag("--newer");
//    auto subsubcom = subcom->add_subcommand("sub2");
//    subsubcom->add_flag("--newest");
//
//    args = {"--simple", "other", "--newer", "sub2", "--newest"};
//    run();
//
//    std::string str = app.config_to_str();
//    CHECK_THAT(str, Contains("simple=true"));
//    CHECK_THAT(str, Contains("other.newer=true"));
//    CHECK_THAT(str, Contains("other.sub2.newest=true"));
//}
//
//TEST_CASE_METHOD(TApp, "TomlOutputSubsubcomConfigurable", "[config]") {
//
//    app.add_flag("--simple");
//    auto subcom = app.add_subcommand("other")->configurable();
//    subcom->add_flag("--newer");
//
//    auto subsubcom = subcom->add_subcommand("sub2");
//    subsubcom->add_flag("--newest");
//
//    args = {"--simple", "other", "--newer", "sub2", "--newest"};
//    run();
//
//    std::string str = app.config_to_str();
//    CHECK_THAT(str, Contains("simple=true"));
//    CHECK_THAT(str, Contains("[other]"));
//    CHECK_THAT(str, Contains("newer=true"));
//    CHECK_THAT(str, Contains("[other.sub2]"));
//    CHECK_THAT(str, Contains("newest=true"));
//    CHECK(std::string::npos == str.find("sub2.newest=true"));
//}
//
//TEST_CASE_METHOD(TApp, "TomlOutputSubcomNonConfigurable", "[config]") {
//
//    app.add_flag("--simple");
//    auto subcom = app.add_subcommand("other", "other_descriptor")->configurable();
//    subcom->add_flag("--newer");
//
//    auto subcom2 = app.add_subcommand("sub2", "descriptor2");
//    subcom2->add_flag("--newest")->configurable(false);
//
//    args = {"--simple", "other", "--newer", "sub2", "--newest"};
//    run();
//
//    std::string str = app.config_to_str(true, true);
//    CHECK_THAT(str, Contains("other_descriptor"));
//    CHECK_THAT(str, Contains("simple=true"));
//    CHECK_THAT(str, Contains("[other]"));
//    CHECK_THAT(str, Contains("newer=true"));
//    CHECK_THAT(str, !Contains("newest"));
//    CHECK_THAT(str, !Contains("descriptor2"));
//}
//
//TEST_CASE_METHOD(TApp, "TomlOutputSubsubcomConfigurableDeep", "[config]") {
//
//    app.add_flag("--simple");
//    auto subcom = app.add_subcommand("other")->configurable();
//    subcom->add_flag("--newer");
//
//    auto subsubcom = subcom->add_subcommand("sub2");
//    subsubcom->add_flag("--newest");
//    auto sssscom = subsubcom->add_subcommand("sub-level2");
//    subsubcom->add_flag("--still_newer");
//    auto s5com = sssscom->add_subcommand("sub-level3");
//    s5com->add_flag("--absolute_newest");
//
//    args = {"--simple", "other", "sub2", "sub-level2", "sub-level3", "--absolute_newest"};
//    run();
//
//    std::string str = app.config_to_str();
//    CHECK_THAT(str, Contains("simple=true"));
//    CHECK_THAT(str, Contains("[other.sub2.sub-level2.sub-level3]"));
//    CHECK_THAT(str, Contains("absolute_newest=true"));
//    CHECK(std::string::npos == str.find(".absolute_newest=true"));
//}
//
//TEST_CASE_METHOD(TApp, "TomlOutputQuoted", "[config]") {
//
//    std::string val1;
//    app.add_option("--val1", val1);
//
//    std::string val2;
//    app.add_option("--val2", val2);
//
//    args = {"--val1", "I am a string", "--val2", R"(I am a "confusing" string)"};
//
//    run();
//
//    CHECK(val1 == "I am a string");
//    CHECK(val2 == "I am a \"confusing\" string");
//
//    std::string str = app.config_to_str();
//    CHECK_THAT(str, Contains("val1=\"I am a string\""));
//    CHECK_THAT(str, Contains("val2='I am a \"confusing\" string'"));
//}
//
//TEST_CASE_METHOD(TApp, "DefaultsTomlOutputQuoted", "[config]") {
//
//    std::string val1{"I am a string"};
//    app.add_option("--val1", val1)->capture_default_str();
//
//    std::string val2{R"(I am a "confusing" string)"};
//    app.add_option("--val2", val2)->capture_default_str();
//
//    run();
//
//    std::string str = app.config_to_str(true);
//    CHECK_THAT(str, Contains("val1=\"I am a string\""));
//    CHECK_THAT(str, Contains("val2='I am a \"confusing\" string'"));
//}
//
//// #298
//TEST_CASE_METHOD(TApp, "StopReadingConfigOnClear", "[config]") {
//
//    TempFile tmpini{"TestIniTmp.ini"};
//
//    app.set_config("--config", tmpini);
//    auto ptr = app.set_config();  // Should *not* read config file
//    CHECK(nullptr == ptr);
//
//    {
//        std::ofstream out{tmpini};
//        out << "volume=1" << std::endl;
//    }
//
//    int volume{0};
//    app.add_option("--volume", volume, "volume1");
//
//    run();
//
//    CHECK(0 == volume);
//}
//
//TEST_CASE_METHOD(TApp, "ConfigWriteReadWrite", "[config]") {
//
//    TempFile tmpini{"TestIniTmp.ini"};
//
//    app.add_flag("--flag");
//    run();
//
//    // Save config, with default values too
//    std::string config1 = app.config_to_str(true, true);
//    {
//        std::ofstream out{tmpini};
//        out << config1 << std::endl;
//    }
//
//    app.set_config("--config", tmpini, "Read an ini file", true);
//    run();
//
//    std::string config2 = app.config_to_str(true, true);
//
//    CHECK(config2 == config1);
//}
//
///////// YAML output tests

TEST_CASE_METHOD(TApp, "YamlOutputNoConfigurable", "[config]") {

    int v1{0}, v2{0};
    app.add_option("--simple", v1);
    app.add_option("--noconf", v2)->configurable(false);
    app.config_formatter(std::make_shared<CLI::ConfigYAML>());
    args = {"--simple=3", "--noconf=2"};

    run();

    std::string str = app.config_to_str();
    CHECK(str == "simple: 3\n");
}

//TEST_CASE_METHOD(TApp, "IniOutputShortSingleDescription", "[config]") {
//    std::string flag = "some_flag";
//    const std::string description = "Some short description.";
//    app.add_flag("--" + flag, description);
//    app.config_formatter(std::make_shared<CLI::ConfigINI>());
//    run();
//
//    std::string str = app.config_to_str(true, true);
//    CHECK_THAT(str, Contains("; " + description + "\n" + flag + "=false\n"));
//}
//
//TEST_CASE_METHOD(TApp, "IniOutputShortDoubleDescription", "[config]") {
//    std::string flag1 = "flagnr1";
//    std::string flag2 = "flagnr2";
//    const std::string description1 = "First description.";
//    const std::string description2 = "Second description.";
//    app.add_flag("--" + flag1, description1);
//    app.add_flag("--" + flag2, description2);
//    app.config_formatter(std::make_shared<CLI::ConfigINI>());
//    run();
//
//    std::string str = app.config_to_str(true, true);
//    std::string ans = "; " + description1 + "\n" + flag1 + "=false\n\n; " + description2 + "\n" + flag2 + "=false\n";
//    CHECK_THAT(str, Contains(ans));
//}
//
//TEST_CASE_METHOD(TApp, "IniOutputGroups", "[config]") {
//    std::string flag1 = "flagnr1";
//    std::string flag2 = "flagnr2";
//    const std::string description1 = "First description.";
//    const std::string description2 = "Second description.";
//    app.add_flag("--" + flag1, description1)->group("group1");
//    app.add_flag("--" + flag2, description2)->group("group2");
//    app.config_formatter(std::make_shared<CLI::ConfigINI>());
//    run();
//
//    std::string str = app.config_to_str(true, true);
//    CHECK_THAT(str, Contains("group1"));
//    CHECK_THAT(str, Contains("group2"));
//}

//TEST_CASE_METHOD(TApp, "YamlOutputHiddenOptions", "[config]") {
//    std::string flag1 = "flagnr1";
//    std::string flag2 = "flagnr2";
//    double val{12.7};
//    const std::string description1 = "First description.";
//    const std::string description2 = "Second description.";
//    app.add_flag("--" + flag1, description1)->group("group1");
//    app.add_flag("--" + flag2, description2)->group("group2");
//    app.add_option("--dval", val)->capture_default_str()->group("");
//    app.config_formatter(std::make_shared<CLI::ConfigYAML>());
//    run();
//
//    std::string str = app.config_to_str(true, true);
//    CHECK_THAT(str, Contains("group1"));
//    CHECK_THAT(str, Contains("group2"));
//    CHECK_THAT(str, Contains("dval: 12.7"));
//    auto loc = str.find("dval: 12.7");
//    auto locg1 = str.find("group1");
//    CHECK(loc < locg1);
//    // make sure it doesn't come twice
//    loc = str.find("dval: 12.7", loc + 4);
//    CHECK(std::string::npos == loc);
//}

//TEST_CASE_METHOD(TApp, "IniOutputAppMultiLineDescription", "[config]") {
//    app.description("Some short app description.\n"
//                    "That has multiple lines.");
//    app.config_formatter(std::make_shared<CLI::ConfigINI>());
//    run();
//
//    std::string str = app.config_to_str(true, true);
//    CHECK_THAT(str, Contains("; Some short app description.\n"));
//    CHECK_THAT(str, Contains("; That has multiple lines.\n"));
//}
//
//TEST_CASE_METHOD(TApp, "IniOutputMultiLineDescription", "[config]") {
//    std::string flag = "some_flag";
//    const std::string description = "Some short description.\nThat has lines.";
//    app.add_flag("--" + flag, description);
//    app.config_formatter(std::make_shared<CLI::ConfigINI>());
//    run();
//
//    std::string str = app.config_to_str(true, true);
//    CHECK_THAT(str, Contains("; Some short description.\n"));
//    CHECK_THAT(str, Contains("; That has lines.\n"));
//    CHECK_THAT(str, Contains(flag + "=false\n"));
//}
//
//TEST_CASE_METHOD(TApp, "IniOutputOptionGroupMultiLineDescription", "[config]") {
//    std::string flag = "flag";
//    const std::string description = "Short flag description.\n";
//    auto og = app.add_option_group("group");
//    og->description("Option group description.\n"
//                    "That has multiple lines.");
//    og->add_flag("--" + flag, description);
//    app.config_formatter(std::make_shared<CLI::ConfigINI>());
//    run();
//
//    std::string str = app.config_to_str(true, true);
//    CHECK_THAT(str, Contains("; Option group description.\n"));
//    CHECK_THAT(str, Contains("; That has multiple lines.\n"));
//}
//
//TEST_CASE_METHOD(TApp, "IniOutputSubcommandMultiLineDescription", "[config]") {
//    std::string flag = "flag";
//    const std::string description = "Short flag description.\n";
//    auto subcom = app.add_subcommand("subcommand");
//    subcom->configurable();
//    subcom->description("Subcommand description.\n"
//                        "That has multiple lines.");
//    subcom->add_flag("--" + flag, description);
//    app.config_formatter(std::make_shared<CLI::ConfigINI>());
//    run();
//
//    std::string str = app.config_to_str(true, true);
//    CHECK_THAT(str, Contains("; Subcommand description.\n"));
//    CHECK_THAT(str, Contains("; That has multiple lines.\n"));
//}
//
//TEST_CASE_METHOD(TApp, "IniOutputOptionGroup", "[config]") {
//    std::string flag1 = "flagnr1";
//    std::string flag2 = "flagnr2";
//    double val{12.7};
//    const std::string description1 = "First description.";
//    const std::string description2 = "Second description.";
//    app.add_flag("--" + flag1, description1)->group("group1");
//    app.add_flag("--" + flag2, description2)->group("group2");
//    auto og = app.add_option_group("group3", "g3 desc");
//    og->add_option("--dval", val)->capture_default_str()->group("");
//    app.config_formatter(std::make_shared<CLI::ConfigINI>());
//    run();
//
//    std::string str = app.config_to_str(true, true);
//    CHECK_THAT(str, Contains("group1"));
//    CHECK_THAT(str, Contains("group2"));
//    CHECK_THAT(str, Contains("dval=12.7"));
//    CHECK_THAT(str, Contains("group3"));
//    CHECK_THAT(str, Contains("g3 desc"));
//    auto loc = str.find("dval=12.7");
//    auto locg1 = str.find("group1");
//    auto locg3 = str.find("group3");
//    CHECK(loc > locg1);
//    // make sure it doesn't come twice
//    loc = str.find("dval=12.7", loc + 4);
//    CHECK(std::string::npos == loc);
//    CHECK(locg1 < locg3);
//}

TEST_CASE_METHOD(TApp, "YamlOutputVector", "[config]") {
    std::vector<int> v;
    app.add_option("--vector", v);

    args = {"--vector", "1", "2", "3"};
    app.config_formatter(std::make_shared<CLI::ConfigYAML>());
    run();

    std::string str = app.config_to_str();
    CHECK(str == "vector:\n"
                 "  - 1\n"
                 "  - 2\n"
                 "  - 3\n");
}

TEST_CASE_METHOD(TApp, "YamlOutputFlag", "[config]") {

    int v{0}, q{0};
    app.add_option("--simple", v);
    app.add_flag("--nothing");
    app.add_flag("--onething");
    app.add_flag("--something", q);

    args = {"--simple=3", "--onething", "--something", "--something"};
    app.config_formatter(std::make_shared<CLI::ConfigYAML>());
    run();

    std::string str = app.config_to_str();
    CHECK_THAT(str, Contains("simple: 3"));
    CHECK_THAT(str, !Contains("nothing"));
    CHECK_THAT(str, Contains("onething: true"));
    CHECK_THAT(str, Contains("something: 2"));

    str = app.config_to_str(true);
    CHECK_THAT(str, Contains("nothing"));
}

TEST_CASE_METHOD(TApp, "YamlOutputSet", "[config]") {

    int v{0};
    app.add_option("--simple", v)->check(CLI::IsMember({1, 2, 3}));

    args = {"--simple=2"};
    app.config_formatter(std::make_shared<CLI::ConfigYAML>());
    run();

    std::string str = app.config_to_str();
    CHECK_THAT(str, Contains("simple: 2"));
}

TEST_CASE_METHOD(TApp, "YamlOutputDefault", "[config]") {

    int v{7};
    app.add_option("--simple", v)->capture_default_str();
    app.config_formatter(std::make_shared<CLI::ConfigYAML>());
    run();

    std::string str = app.config_to_str();
    CHECK_THAT(str, !Contains("simple: 7"));

    str = app.config_to_str(true);
    CHECK_THAT(str, Contains("simple: 7"));
}

TEST_CASE_METHOD(TApp, "YamlOutputSubcom", "[config]") {

    app.add_flag("--simple");
    auto subcom = app.add_subcommand("other");
    subcom->add_flag("--newer");
    app.config_formatter(std::make_shared<CLI::ConfigYAML>());
    args = {"--simple", "other", "--newer"};
    run();

    std::string str = app.config_to_str();
    CHECK_THAT(str, Contains("simple: true"));
    CHECK_THAT(str, Contains("other:\n"
                             "  newer: true"));
}

//TEST_CASE_METHOD(TApp, "IniOutputSubcomCustomSep", "[config]") {
//
//    app.add_flag("--simple");
//    auto subcom = app.add_subcommand("other");
//    subcom->add_flag("--newer");
//    app.config_formatter(std::make_shared<CLI::ConfigINI>());
//    app.get_config_formatter_base()->parentSeparator(':');
//    args = {"--simple", "other", "--newer"};
//    run();
//
//    std::string str = app.config_to_str();
//    CHECK_THAT(str, Contains("simple=true"));
//    CHECK_THAT(str, Contains("other:newer=true"));
//}

TEST_CASE_METHOD(TApp, "IniOutputSubcomConfigurable", "[config]") {

    app.add_flag("--simple");
    auto subcom = app.add_subcommand("other")->configurable();
    subcom->add_flag("--newer");
    app.config_formatter(std::make_shared<CLI::ConfigYAML>());
    args = {"--simple", "other", "--newer"};
    run();

    std::string str = app.config_to_str();
    CHECK_THAT(str, Contains("simple: true"));
    CHECK_THAT(str, Contains("other:"));
    CHECK_THAT(str, Contains("  newer: true"));
    //CHECK(std::string::npos == str.find("other.newer=true"));
}

TEST_CASE_METHOD(TApp, "YamlOutputSubsubcom", "[config]") {

    app.add_flag("--simple");
    auto subcom = app.add_subcommand("other");
    subcom->add_flag("--newer");
    auto subsubcom = subcom->add_subcommand("sub2");
    subsubcom->add_flag("--newest");
    app.config_formatter(std::make_shared<CLI::ConfigYAML>());
    args = {"--simple", "other", "--newer", "sub2", "--newest"};
    run();

    std::string str = app.config_to_str();
    CHECK_THAT(str, Contains("simple: true"));
    CHECK_THAT(str, Contains("other:\n"
                             "  newer: true"));
    CHECK_THAT(str, Contains("  sub2:\n"
                             "    newest: true"));
}

//TEST_CASE_METHOD(TApp, "IniOutputSubsubcomCustomSep", "[config]") {
//
//    app.add_flag("--simple");
//    auto subcom = app.add_subcommand("other");
//    subcom->add_flag("--newer");
//    auto subsubcom = subcom->add_subcommand("sub2");
//    subsubcom->add_flag("--newest");
//    app.config_formatter(std::make_shared<CLI::ConfigINI>());
//    app.get_config_formatter_base()->parentSeparator('|');
//    args = {"--simple", "other", "--newer", "sub2", "--newest"};
//    run();
//
//    std::string str = app.config_to_str();
//    CHECK_THAT(str, Contains("simple=true"));
//    CHECK_THAT(str, Contains("other|newer=true"));
//    CHECK_THAT(str, Contains("other|sub2|newest=true"));
//}

TEST_CASE_METHOD(TApp, "YamlOutputSubsubcomConfigurable", "[config]") {

    app.add_flag("--simple");
    auto subcom = app.add_subcommand("other")->configurable();
    subcom->add_flag("--newer");

    auto subsubcom = subcom->add_subcommand("sub2");
    subsubcom->add_flag("--newest");
    app.config_formatter(std::make_shared<CLI::ConfigYAML>());
    args = {"--simple", "other", "--newer", "sub2", "--newest"};
    run();

    std::string str = app.config_to_str();
    CHECK_THAT(str, Contains("simple: true"));
    CHECK_THAT(str, Contains("other:\n"));
    CHECK_THAT(str, Contains("newer: true"));
    CHECK_THAT(str, Contains("  sub2:\n"));
    CHECK_THAT(str, Contains("newest: true"));
    //CHECK(std::string::npos == str.find("sub2.newest=true"));
}

TEST_CASE_METHOD(TApp, "YamlOutputSubsubcomConfigurableDeep", "[config]") {

    app.add_flag("--simple");
    auto subcom = app.add_subcommand("other")->configurable();
    subcom->add_flag("--newer");

    auto subsubcom = subcom->add_subcommand("sub2");
    subsubcom->add_flag("--newest");
    auto sssscom = subsubcom->add_subcommand("sub-level2");
    subsubcom->add_flag("--still_newer");
    auto s5com = sssscom->add_subcommand("sub-level3");
    s5com->add_flag("--absolute_newest");
    app.config_formatter(std::make_shared<CLI::ConfigYAML>());
    args = {"--simple", "other", "sub2", "sub-level2", "sub-level3", "--absolute_newest"};
    run();

    std::string str = app.config_to_str();
    CHECK_THAT(str, Contains("simple: true"));
    CHECK_THAT(str, Contains("other:\n"
                             "  sub2:\n"
                             "    sub-level2:\n"
                             "      sub-level3:\n"));
    CHECK_THAT(str, Contains("absolute_newest: true"));
    //CHECK(std::string::npos == str.find(".absolute_newest=true"));
}

TEST_CASE_METHOD(TApp, "YamlOutputQuoted", "[config]") {

    std::string val1;
    app.add_option("--val1", val1);

    std::string val2;
    app.add_option("--val2", val2);
    app.config_formatter(std::make_shared<CLI::ConfigYAML>());
    args = {"--val1", "I am a string", "--val2", R"(I am a "confusing" string)"};

    run();

    CHECK(val1 == "I am a string");
    CHECK(val2 == R"(I am a "confusing" string)");

    std::string str = app.config_to_str();
    CHECK_THAT(str, Contains("val1: I am a string"));
    CHECK_THAT(str, Contains(R"(val2: I am a "confusing" string)"));
}

TEST_CASE_METHOD(TApp, "DefaultsYamlOutputQuoted", "[config]") {

    std::string val1{"I am a string"};
    app.add_option("--val1", val1)->capture_default_str();

    std::string val2{R"(I am a "confusing" string)"};
    app.add_option("--val2", val2)->capture_default_str();
    app.config_formatter(std::make_shared<CLI::ConfigYAML>());
    run();

    std::string str = app.config_to_str(true);
    CHECK_THAT(str, Contains("val1: I am a string"));
    CHECK_THAT(str, Contains(R"(val2: I am a "confusing" string)"));
}

#endif
