#ifdef CLI11_SINGLE_FILE
#include "CLI11.hpp"
#else
#include "CLI/CLI.hpp"
#endif

#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include <fstream>

using ::testing::HasSubstr;
using ::testing::Not;

TEST(Formatter, Nothing) {
    CLI::App app{"My prog"};

    app.formatter(
        [](const CLI::App *, std::string, CLI::AppFormatMode) { return std::string("This is really simple"); });

    std::string help = app.help();

    EXPECT_EQ(help, "This is really simple");
}

TEST(Formatter, OptCustomize) {
    CLI::App app{"My prog"};

    CLI::OptionFormatter optfmt;
    optfmt.column_width(25);
    optfmt.label("REQUIRED", "(MUST HAVE)");
    app.option_defaults()->formatter(optfmt);

    int v;
    app.add_option("--opt", v, "Something")->required();

    std::string help = app.help();

    EXPECT_THAT(help, HasSubstr("(MUST HAVE)"));
    EXPECT_EQ(help,
              "My prog\n"
              "Usage: [OPTIONS]\n\n"
              "Options:\n"
              "  -h,--help                   Print this help message and exit\n"
              "  --opt INT (MUST HAVE)  Something\n");
}

TEST(Formatter, AptCustomize) {
    CLI::App app{"My prog"};
    app.add_subcommand("subcom1", "This");

    CLI::AppFormatter appfmt;
    appfmt.column_width(20);
    appfmt.label("Usage", "Run");
    app.formatter(appfmt);

    app.add_subcommand("subcom2", "This");

    std::string help = app.help();
    EXPECT_EQ(help,
              "My prog\n"
              "Run: [OPTIONS] [SUBCOMMAND]\n\n"
              "Options:\n"
              "  -h,--help                   Print this help message and exit\n\n"
              "Subcommands:\n"
              "  subcom1           This\n"
              "  subcom2           This\n");
}

TEST(Formatter, AllSub) {
    CLI::App app{"My prog"};
    CLI::App *sub = app.add_subcommand("subcom", "This");
    sub->add_flag("--insub", "MyFlag");

    std::string help = app.help("", CLI::AppFormatMode::All);
    EXPECT_THAT(help, HasSubstr("--insub"));
    EXPECT_THAT(help, HasSubstr("subcom"));
}
