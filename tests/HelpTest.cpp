#ifdef CLI_SINGLE_FILE
#include "CLI11.hpp"
#else
#include "CLI/CLI.hpp"
#endif

#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include <fstream>

using ::testing::HasSubstr;
using ::testing::Not;

TEST(THelp, Basic) {
    CLI::App app{"My prog"};

    std::string help = app.help();

    EXPECT_THAT(help, HasSubstr("My prog"));
    EXPECT_THAT(help, HasSubstr("-h,--help"));
    EXPECT_THAT(help, HasSubstr("Options:"));
    EXPECT_THAT(help, HasSubstr("Usage:"));
}

TEST(THelp, OptionalPositional) {
    CLI::App app{"My prog"};

    std::string x;
    app.add_option("something", x, "My option here");

    std::string help = app.help();

    EXPECT_THAT(help, HasSubstr("My prog"));
    EXPECT_THAT(help, HasSubstr("-h,--help"));
    EXPECT_THAT(help, HasSubstr("Options:"));
    EXPECT_THAT(help, HasSubstr("Positionals:"));
    EXPECT_THAT(help, HasSubstr("something TEXT"));
    EXPECT_THAT(help, HasSubstr("My option here"));
    EXPECT_THAT(help, HasSubstr("Usage: program [OPTIONS] [something]"));
}

TEST(THelp, Hidden) {
    CLI::App app{"My prog"};

    std::string x;
    app.add_option("something", x, "My option here")->group("Hidden");
    std::string y;
    app.add_option("--another", y)->group("Hidden");

    std::string help = app.help();

    EXPECT_THAT(help, HasSubstr("My prog"));
    EXPECT_THAT(help, HasSubstr("-h,--help"));
    EXPECT_THAT(help, HasSubstr("Options:"));
    EXPECT_THAT(help, HasSubstr("[something]"));
    EXPECT_THAT(help, Not(HasSubstr("something ")));
    EXPECT_THAT(help, Not(HasSubstr("another")));
}

TEST(THelp, OptionalPositionalAndOptions) {
    CLI::App app{"My prog"};
    app.add_flag("-q,--quick");

    std::string x;
    app.add_option("something", x, "My option here");

    std::string help = app.help();

    EXPECT_THAT(help, HasSubstr("My prog"));
    EXPECT_THAT(help, HasSubstr("-h,--help"));
    EXPECT_THAT(help, HasSubstr("Options:"));
    EXPECT_THAT(help, HasSubstr("Usage: program [OPTIONS] [something]"));
}

TEST(THelp, RequiredPositionalAndOptions) {
    CLI::App app{"My prog"};
    app.add_flag("-q,--quick");

    std::string x;
    app.add_option("something", x, "My option here")->required();

    std::string help = app.help();

    EXPECT_THAT(help, HasSubstr("My prog"));
    EXPECT_THAT(help, HasSubstr("-h,--help"));
    EXPECT_THAT(help, HasSubstr("Options:"));
    EXPECT_THAT(help, HasSubstr("Positionals:"));
    EXPECT_THAT(help, HasSubstr("Usage: program [OPTIONS] something"));
}

TEST(THelp, MultiOpts) {
    CLI::App app{"My prog"};
    std::vector<int> x, y;
    app.add_option("-q,--quick", x, "Disc")->expected(2);
    app.add_option("-v,--vals", y, "Other");

    std::string help = app.help();

    EXPECT_THAT(help, HasSubstr("My prog"));
    EXPECT_THAT(help, Not(HasSubstr("Positionals:")));
    EXPECT_THAT(help, HasSubstr("Usage: program [OPTIONS]"));
    EXPECT_THAT(help, HasSubstr("INT x 2"));
    EXPECT_THAT(help, HasSubstr("INT ..."));
}

TEST(THelp, VectorOpts) {
    CLI::App app{"My prog"};
    std::vector<int> x = {1, 2};
    app.add_option("-q,--quick", x, "", true);

    std::string help = app.help();

    EXPECT_THAT(help, HasSubstr("INT=[1,2] ..."));
}

TEST(THelp, MultiPosOpts) {
    CLI::App app{"My prog"};
    std::vector<int> x, y;
    app.add_option("quick", x, "Disc")->expected(2);
    app.add_option("vals", y, "Other");

    std::string help = app.help();

    EXPECT_THAT(help, HasSubstr("My prog"));
    EXPECT_THAT(help, HasSubstr("Positionals:"));
    EXPECT_THAT(help, HasSubstr("Usage: program [OPTIONS]"));
    EXPECT_THAT(help, HasSubstr("INT x 2"));
    EXPECT_THAT(help, HasSubstr("INT ..."));
    EXPECT_THAT(help, HasSubstr("[quick(2x)]"));
    EXPECT_THAT(help, HasSubstr("[vals...]"));
}

TEST(THelp, EnvName) {
    CLI::App app{"My prog"};
    std::string input;
    app.add_option("--something", input)->envname("SOME_ENV");

    std::string help = app.help();

    EXPECT_THAT(help, HasSubstr("SOME_ENV"));
}

TEST(THelp, Requires) {
    CLI::App app{"My prog"};

    CLI::Option *op1 = app.add_flag("--op1");
    app.add_flag("--op2")->requires(op1);

    std::string help = app.help();

    EXPECT_THAT(help, HasSubstr("Requires: --op1"));
}

TEST(THelp, RequiresPositional) {
    CLI::App app{"My prog"};

    int x, y;

    CLI::Option *op1 = app.add_option("op1", x, "one");
    app.add_option("op2", y, "two")->requires(op1);

    std::string help = app.help();

    EXPECT_THAT(help, HasSubstr("Positionals:"));
    EXPECT_THAT(help, HasSubstr("Requires: op1"));
}

TEST(THelp, Excludes) {
    CLI::App app{"My prog"};

    CLI::Option *op1 = app.add_flag("--op1");
    app.add_flag("--op2")->excludes(op1);

    std::string help = app.help();

    EXPECT_THAT(help, HasSubstr("Excludes: --op1"));
}

TEST(THelp, ExcludesPositional) {
    CLI::App app{"My prog"};

    int x, y;

    CLI::Option *op1 = app.add_option("op1", x);
    app.add_option("op2", y)->excludes(op1);

    std::string help = app.help();

    EXPECT_THAT(help, HasSubstr("Positionals:"));
    EXPECT_THAT(help, HasSubstr("Excludes: op1"));
}

TEST(THelp, Subcom) {
    CLI::App app{"My prog"};

    auto sub1 = app.add_subcommand("sub1");
    app.add_subcommand("sub2");

    std::string help = app.help();
    EXPECT_THAT(help, HasSubstr("Usage: program [OPTIONS] [SUBCOMMAND]"));

    app.require_subcommand();

    help = app.help();
    EXPECT_THAT(help, HasSubstr("Usage: program [OPTIONS] SUBCOMMAND"));

    help = sub1->help();
    EXPECT_THAT(help, HasSubstr("Usage: sub1"));

    char x[] = "./myprogram";
    char y[] = "sub2";

    std::vector<char *> args = {x, y};
    app.parse((int)args.size(), args.data());

    help = app.help();
    EXPECT_THAT(help, HasSubstr("Usage: ./myprogram sub2"));
}

TEST(THelp, IntDefaults) {
    CLI::App app{"My prog"};

    int one{1}, two{2};
    app.add_option("--one", one, "Help for one", true);
    app.add_set("--set", two, {2, 3, 4}, "Help for set", true);

    std::string help = app.help();

    EXPECT_THAT(help, HasSubstr("--one"));
    EXPECT_THAT(help, HasSubstr("--set"));
    EXPECT_THAT(help, HasSubstr("1"));
    EXPECT_THAT(help, HasSubstr("=2"));
    EXPECT_THAT(help, HasSubstr("2,3,4"));
}

TEST(THelp, SetLower) {
    CLI::App app{"My prog"};

    std::string def{"One"};
    app.add_set_ignore_case("--set", def, {"oNe", "twO", "THREE"}, "Help for set", true);

    std::string help = app.help();

    EXPECT_THAT(help, HasSubstr("--set"));
    EXPECT_THAT(help, HasSubstr("=One"));
    EXPECT_THAT(help, HasSubstr("oNe"));
    EXPECT_THAT(help, HasSubstr("twO"));
    EXPECT_THAT(help, HasSubstr("THREE"));
}

TEST(Exit, ErrorWithHelp) {
    CLI::App app{"My prog"};

    std::vector<std::string> input{"-h"};
    try {
        app.parse(input);
    } catch(const CLI::CallForHelp &e) {
        EXPECT_EQ(static_cast<int>(CLI::ExitCodes::Success), e.get_exit_code());
    }
}

TEST(Exit, ErrorWithoutHelp) {
    CLI::App app{"My prog"};

    std::vector<std::string> input{"--none"};
    try {
        app.parse(input);
    } catch(const CLI::ParseError &e) {
        EXPECT_EQ(static_cast<int>(CLI::ExitCodes::Extras), e.get_exit_code());
    }
}

TEST(Exit, ExitCodes) {
    CLI::App app;

    auto i = static_cast<int>(CLI::ExitCodes::Extras);
    EXPECT_EQ(0, app.exit(CLI::Success()));
    EXPECT_EQ(0, app.exit(CLI::CallForHelp()));
    EXPECT_EQ(i, app.exit(CLI::ExtrasError("Thing")));
}
