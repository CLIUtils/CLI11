#ifdef CLI_SINGLE_FILE
#include "CLI11.hpp"
#else
#include "CLI/CLI.hpp"
#endif

#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include <fstream>

using ::testing::HasSubstr;

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
    EXPECT_THAT(help, HasSubstr("something STRING"));
    EXPECT_THAT(help, HasSubstr("My option here"));
    EXPECT_THAT(help, HasSubstr("Usage: program [OPTIONS] [something]"));

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
    app.add_option("something", x, "My option here")
        ->required();

    std::string help = app.help();

    EXPECT_THAT(help, HasSubstr("My prog"));
    EXPECT_THAT(help, HasSubstr("-h,--help"));
    EXPECT_THAT(help, HasSubstr("Options:"));
    EXPECT_THAT(help, HasSubstr("Positionals:"));
    EXPECT_THAT(help, HasSubstr("Usage: program [OPTIONS] something"));
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

    CLI::Option* op1 = app.add_flag("--op1");
    app.add_flag("--op2")->requires(op1);

    std::string help = app.help();

    EXPECT_THAT(help, HasSubstr("Requires: --op1"));
}

TEST(THelp, Excludes) {
    CLI::App app{"My prog"};

    CLI::Option* op1 = app.add_flag("--op1");
    app.add_flag("--op2")->excludes(op1);

    std::string help = app.help();

    EXPECT_THAT(help, HasSubstr("Excludes: --op1"));
}

