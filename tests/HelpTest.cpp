#ifdef CLI11_SINGLE_FILE
#include "CLI11.hpp"
#else
#include "CLI/CLI.hpp"
#endif

#include "gmock/gmock.h"
#include "gtest/gtest.h"
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

TEST(THelp, Footer) {
    CLI::App app{"My prog"};
    app.footer("Report bugs to bugs@example.com");

    std::string help = app.help();

    EXPECT_THAT(help, HasSubstr("My prog"));
    EXPECT_THAT(help, HasSubstr("-h,--help"));
    EXPECT_THAT(help, HasSubstr("Options:"));
    EXPECT_THAT(help, HasSubstr("Usage:"));
    EXPECT_THAT(help, HasSubstr("Report bugs to bugs@example.com"));
}

TEST(THelp, OptionalPositional) {
    CLI::App app{"My prog", "program"};

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
    app.add_option("something", x, "My option here")->group("");
    std::string y;
    app.add_option("--another", y)->group("");

    std::string help = app.help();

    EXPECT_THAT(help, HasSubstr("My prog"));
    EXPECT_THAT(help, HasSubstr("-h,--help"));
    EXPECT_THAT(help, HasSubstr("Options:"));
    EXPECT_THAT(help, HasSubstr("[something]"));
    EXPECT_THAT(help, Not(HasSubstr("something ")));
    EXPECT_THAT(help, Not(HasSubstr("another")));
}

TEST(THelp, OptionalPositionalAndOptions) {
    CLI::App app{"My prog", "AnotherProgram"};
    app.add_flag("-q,--quick");

    std::string x;
    app.add_option("something", x, "My option here");

    std::string help = app.help();

    EXPECT_THAT(help, HasSubstr("My prog"));
    EXPECT_THAT(help, HasSubstr("-h,--help"));
    EXPECT_THAT(help, HasSubstr("Options:"));
    EXPECT_THAT(help, HasSubstr("Usage: AnotherProgram [OPTIONS] [something]"));
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
    EXPECT_THAT(help, HasSubstr("Usage: [OPTIONS] something"));
}

TEST(THelp, MultiOpts) {
    CLI::App app{"My prog"};
    std::vector<int> x, y;
    app.add_option("-q,--quick", x, "Disc")->expected(2);
    app.add_option("-v,--vals", y, "Other");

    std::string help = app.help();

    EXPECT_THAT(help, HasSubstr("My prog"));
    EXPECT_THAT(help, Not(HasSubstr("Positionals:")));
    EXPECT_THAT(help, HasSubstr("Usage: [OPTIONS]"));
    EXPECT_THAT(help, HasSubstr("INT x 2"));
    EXPECT_THAT(help, HasSubstr("INT ..."));
}

TEST(THelp, VectorOpts) {
    CLI::App app{"My prog"};
    std::vector<int> x = {1, 2};
    app.add_option("-q,--quick", x)->capture_default_str();

    std::string help = app.help();

    EXPECT_THAT(help, HasSubstr("INT=[1,2] ..."));
}

TEST(THelp, MultiPosOpts) {
    CLI::App app{"My prog"};
    app.name("program");
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

TEST(THelp, Needs) {
    CLI::App app{"My prog"};

    CLI::Option *op1 = app.add_flag("--op1");
    app.add_flag("--op2")->needs(op1);

    std::string help = app.help();

    EXPECT_THAT(help, HasSubstr("Needs: --op1"));
}

TEST(THelp, NeedsPositional) {
    CLI::App app{"My prog"};

    int x, y;

    CLI::Option *op1 = app.add_option("op1", x, "one");
    app.add_option("op2", y, "two")->needs(op1);

    std::string help = app.help();

    EXPECT_THAT(help, HasSubstr("Positionals:"));
    EXPECT_THAT(help, HasSubstr("Needs: op1"));
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

TEST(THelp, ExcludesSymmetric) {
    CLI::App app{"My prog"};

    CLI::Option *op1 = app.add_flag("--op1");
    app.add_flag("--op2")->excludes(op1);

    std::string help = app.help();

    EXPECT_THAT(help, HasSubstr("Excludes: --op2"));
}

TEST(THelp, ManualSetters) {

    CLI::App app{"My prog"};

    int x = 1;

    CLI::Option *op1 = app.add_option("--op", x);
    op1->default_str("12");
    op1->type_name("BIGGLES");
    EXPECT_EQ(x, 1);

    std::string help = app.help();

    EXPECT_THAT(help, HasSubstr("=12"));
    EXPECT_THAT(help, HasSubstr("BIGGLES"));

    op1->default_val("14");
    EXPECT_EQ(x, 14);
    help = app.help();
    EXPECT_THAT(help, HasSubstr("=14"));
}

TEST(THelp, ManualSetterOverFunction) {

    CLI::App app{"My prog"};

    int x = 1;

    CLI::Option *op1 = app.add_option("--op1", x)->check(CLI::IsMember({1, 2}));
    CLI::Option *op2 = app.add_option("--op2", x)->transform(CLI::IsMember({1, 2}));
    op1->default_str("12");
    op1->type_name("BIGGLES");
    op2->type_name("QUIGGLES");
    EXPECT_EQ(x, 1);

    std::string help = app.help();
    EXPECT_THAT(help, HasSubstr("=12"));
    EXPECT_THAT(help, HasSubstr("BIGGLES"));
    EXPECT_THAT(help, HasSubstr("QUIGGLES"));
    EXPECT_THAT(help, HasSubstr("{1,2}"));
}

TEST(THelp, Subcom) {
    CLI::App app{"My prog"};

    auto sub1 = app.add_subcommand("sub1");
    app.add_subcommand("sub2");

    std::string help = app.help();
    EXPECT_THAT(help, HasSubstr("Usage: [OPTIONS] [SUBCOMMAND]"));

    app.require_subcommand();

    help = app.help();
    EXPECT_THAT(help, HasSubstr("Usage: [OPTIONS] SUBCOMMAND"));

    help = sub1->help();
    EXPECT_THAT(help, HasSubstr("Usage: sub1"));

    char x[] = "./myprogram";
    char y[] = "sub2";

    std::vector<char *> args = {x, y};
    app.parse((int)args.size(), args.data());

    help = app.help();
    EXPECT_THAT(help, HasSubstr("Usage: ./myprogram sub2"));
}

TEST(THelp, MasterName) {
    CLI::App app{"My prog", "MyRealName"};

    char x[] = "./myprogram";

    std::vector<char *> args = {x};
    app.parse((int)args.size(), args.data());

    EXPECT_THAT(app.help(), HasSubstr("Usage: MyRealName"));
}

TEST(THelp, IntDefaults) {
    CLI::App app{"My prog"};

    int one{1}, two{2};
    app.add_option("--one", one, "Help for one")->capture_default_str();
    app.add_option("--set", two, "Help for set")->capture_default_str()->check(CLI::IsMember({2, 3, 4}));

    std::string help = app.help();

    EXPECT_THAT(help, HasSubstr("--one"));
    EXPECT_THAT(help, HasSubstr("--set"));
    EXPECT_THAT(help, HasSubstr("1"));
    EXPECT_THAT(help, HasSubstr("=2"));
    EXPECT_THAT(help, HasSubstr("2,3,4"));
}

TEST(THelp, SetLower) {
    CLI::App app{"My prog"};
    app.option_defaults()->always_capture_default();

    std::string def{"One"};
    app.add_option("--set", def, "Help for set")->check(CLI::IsMember({"oNe", "twO", "THREE"}));

    std::string help = app.help();

    EXPECT_THAT(help, HasSubstr("--set"));
    EXPECT_THAT(help, HasSubstr("=One"));
    EXPECT_THAT(help, HasSubstr("oNe"));
    EXPECT_THAT(help, HasSubstr("twO"));
    EXPECT_THAT(help, HasSubstr("THREE"));
}

TEST(THelp, OnlyOneHelp) {
    CLI::App app{"My prog"};

    // It is not supported to have more than one help flag, last one wins
    app.set_help_flag("--help", "No short name allowed");
    app.set_help_flag("--yelp", "Alias for help");

    std::vector<std::string> input{"--help"};
    EXPECT_THROW(app.parse(input), CLI::ExtrasError);
}

TEST(THelp, MultiHelp) {
    CLI::App app{"My prog"};

    // It is not supported to have more than one help flag, last one wins
    app.set_help_flag("--help,-h,-?", "No short name allowed");
    app.allow_windows_style_options();

    std::vector<std::string> input{"/?"};
    EXPECT_THROW(app.parse(input), CLI::CallForHelp);
}

TEST(THelp, OnlyOneAllHelp) {
    CLI::App app{"My prog"};

    // It is not supported to have more than one help flag, last one wins
    app.set_help_all_flag("--help-all", "No short name allowed");
    app.set_help_all_flag("--yelp", "Alias for help");

    std::vector<std::string> input{"--help-all"};
    EXPECT_THROW(app.parse(input), CLI::ExtrasError);

    std::vector<std::string> input2{"--yelp"};
    EXPECT_THROW(app.parse(input2), CLI::CallForAllHelp);

    // Remove the flag
    app.set_help_all_flag();
    std::vector<std::string> input3{"--yelp"};
    EXPECT_THROW(app.parse(input3), CLI::ExtrasError);
}

TEST(THelp, RemoveHelp) {
    CLI::App app{"My prog"};
    app.set_help_flag();

    std::string help = app.help();

    EXPECT_THAT(help, HasSubstr("My prog"));
    EXPECT_THAT(help, Not(HasSubstr("-h,--help")));
    EXPECT_THAT(help, Not(HasSubstr("Options:")));
    EXPECT_THAT(help, HasSubstr("Usage:"));

    std::vector<std::string> input{"--help"};
    try {
        app.parse(input);
    } catch(const CLI::ParseError &e) {
        EXPECT_EQ(static_cast<int>(CLI::ExitCodes::ExtrasError), e.get_exit_code());
    }
}

TEST(THelp, RemoveOtherMethodHelp) {
    CLI::App app{"My prog"};

    // Don't do this. Just in case, let's make sure it works.
    app.remove_option(const_cast<CLI::Option *>(app.get_help_ptr()));

    std::string help = app.help();

    EXPECT_THAT(help, HasSubstr("My prog"));
    EXPECT_THAT(help, Not(HasSubstr("-h,--help")));
    EXPECT_THAT(help, Not(HasSubstr("Options:")));
    EXPECT_THAT(help, HasSubstr("Usage:"));

    std::vector<std::string> input{"--help"};
    try {
        app.parse(input);
    } catch(const CLI::ParseError &e) {
        EXPECT_EQ(static_cast<int>(CLI::ExitCodes::ExtrasError), e.get_exit_code());
    }
}

TEST(THelp, RemoveOtherMethodHelpAll) {
    CLI::App app{"My prog"};

    app.set_help_all_flag("--help-all");
    // Don't do this. Just in case, let's make sure it works.
    app.remove_option(const_cast<CLI::Option *>(app.get_help_all_ptr()));

    std::string help = app.help();

    EXPECT_THAT(help, HasSubstr("My prog"));
    EXPECT_THAT(help, Not(HasSubstr("--help-all")));
    EXPECT_THAT(help, HasSubstr("Options:"));
    EXPECT_THAT(help, HasSubstr("Usage:"));

    std::vector<std::string> input{"--help-all"};
    try {
        app.parse(input);
    } catch(const CLI::ParseError &e) {
        EXPECT_EQ(static_cast<int>(CLI::ExitCodes::ExtrasError), e.get_exit_code());
    }
}

TEST(THelp, NoHelp) {
    CLI::App app{"My prog"};
    app.set_help_flag();

    std::string help = app.help();

    EXPECT_THAT(help, HasSubstr("My prog"));
    EXPECT_THAT(help, Not(HasSubstr("-h,--help")));
    EXPECT_THAT(help, Not(HasSubstr("Options:")));
    EXPECT_THAT(help, HasSubstr("Usage:"));

    std::vector<std::string> input{"--help"};
    try {
        app.parse(input);
    } catch(const CLI::ParseError &e) {
        EXPECT_EQ(static_cast<int>(CLI::ExitCodes::ExtrasError), e.get_exit_code());
    }
}

TEST(THelp, CustomHelp) {
    CLI::App app{"My prog"};

    CLI::Option *help_option = app.set_help_flag("--yelp", "display help and exit");
    EXPECT_EQ(app.get_help_ptr(), help_option);

    std::string help = app.help();

    EXPECT_THAT(help, HasSubstr("My prog"));
    EXPECT_THAT(help, Not(HasSubstr("-h,--help")));
    EXPECT_THAT(help, HasSubstr("--yelp"));
    EXPECT_THAT(help, HasSubstr("Options:"));
    EXPECT_THAT(help, HasSubstr("Usage:"));

    std::vector<std::string> input{"--yelp"};
    try {
        app.parse(input);
    } catch(const CLI::CallForHelp &e) {
        EXPECT_EQ(static_cast<int>(CLI::ExitCodes::Success), e.get_exit_code());
    }
}

TEST(THelp, NextLineShouldBeAlignmentInMultilineDescription) {
    CLI::App app;
    int i;
    const std::string first{"first line"};
    const std::string second{"second line"};
    app.add_option("-i,--int", i, first + "\n" + second);

    const std::string help = app.help();
    const auto width = app.get_formatter()->get_column_width();
    EXPECT_THAT(help, HasSubstr(first + "\n" + std::string(width, ' ') + second));
}

TEST(THelp, NiceName) {
    CLI::App app;

    int x;
    auto long_name = app.add_option("-s,--long,-q,--other,that", x);
    auto short_name = app.add_option("more,-x,-y", x);
    auto positional = app.add_option("posit", x);

    EXPECT_EQ(long_name->get_name(), "--long");
    EXPECT_EQ(short_name->get_name(), "-x");
    EXPECT_EQ(positional->get_name(), "posit");
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

TEST(Exit, ErrorWithAllHelp) {
    CLI::App app{"My prog"};
    app.set_help_all_flag("--help-all", "All help");

    std::vector<std::string> input{"--help-all"};
    try {
        app.parse(input);
    } catch(const CLI::CallForAllHelp &e) {
        EXPECT_EQ(static_cast<int>(CLI::ExitCodes::Success), e.get_exit_code());
    }
}

TEST(Exit, ErrorWithoutHelp) {
    CLI::App app{"My prog"};

    std::vector<std::string> input{"--none"};
    try {
        app.parse(input);
    } catch(const CLI::ParseError &e) {
        EXPECT_EQ(static_cast<int>(CLI::ExitCodes::ExtrasError), e.get_exit_code());
    }
}

TEST(Exit, ExitCodes) {
    CLI::App app;

    auto i = static_cast<int>(CLI::ExitCodes::ExtrasError);
    EXPECT_EQ(0, app.exit(CLI::Success()));
    EXPECT_EQ(0, app.exit(CLI::CallForHelp()));
    EXPECT_EQ(i, app.exit(CLI::ExtrasError({"Thing"})));
    EXPECT_EQ(42, app.exit(CLI::RuntimeError(42)));
    EXPECT_EQ(1, app.exit(CLI::RuntimeError())); // Not sure if a default here is a good thing
}

struct CapturedHelp : public ::testing::Test {
    CLI::App app{"My Test Program"};
    std::stringstream out;
    std::stringstream err;

    int run(const CLI::Error &e) { return app.exit(e, out, err); }

    void reset() {
        out.clear();
        err.clear();
    }
};

TEST_F(CapturedHelp, Sucessful) {
    EXPECT_EQ(run(CLI::Success()), 0);
    EXPECT_EQ(out.str(), "");
    EXPECT_EQ(err.str(), "");
}

TEST_F(CapturedHelp, JustAnError) {
    EXPECT_EQ(run(CLI::RuntimeError(42)), 42);
    EXPECT_EQ(out.str(), "");
    EXPECT_EQ(err.str(), "");
}

TEST_F(CapturedHelp, CallForHelp) {
    EXPECT_EQ(run(CLI::CallForHelp()), 0);
    EXPECT_EQ(out.str(), app.help());
    EXPECT_EQ(err.str(), "");
}
TEST_F(CapturedHelp, CallForAllHelp) {
    EXPECT_EQ(run(CLI::CallForAllHelp()), 0);
    EXPECT_EQ(out.str(), app.help("", CLI::AppFormatMode::All));
    EXPECT_EQ(err.str(), "");
}
TEST_F(CapturedHelp, CallForAllHelpOutput) {
    app.set_help_all_flag("--help-all", "Help all");
    app.add_subcommand("one", "One description");
    CLI::App *sub = app.add_subcommand("two");
    sub->add_flag("--three");

    EXPECT_EQ(run(CLI::CallForAllHelp()), 0);
    EXPECT_EQ(out.str(), app.help("", CLI::AppFormatMode::All));
    EXPECT_EQ(err.str(), "");
    EXPECT_THAT(out.str(), HasSubstr("one"));
    EXPECT_THAT(out.str(), HasSubstr("two"));
    EXPECT_THAT(out.str(), HasSubstr("--three"));

    EXPECT_EQ(out.str(),
              "My Test Program\n"
              "Usage: [OPTIONS] [SUBCOMMAND]\n"
              "\n"
              "Options:\n"
              "  -h,--help                   Print this help message and exit\n"
              "  --help-all                  Help all\n"
              "\n"
              "Subcommands:\n"
              "one\n"
              "  One description\n\n"
              "two\n"
              "  Options:\n"
              "    --three                     \n\n");
}
TEST_F(CapturedHelp, NewFormattedHelp) {
    app.formatter_fn([](const CLI::App *, std::string, CLI::AppFormatMode) { return "New Help"; });
    EXPECT_EQ(run(CLI::CallForHelp()), 0);
    EXPECT_EQ(out.str(), "New Help");
    EXPECT_EQ(err.str(), "");
}

TEST_F(CapturedHelp, NormalError) {
    EXPECT_EQ(run(CLI::ExtrasError({"Thing"})), static_cast<int>(CLI::ExitCodes::ExtrasError));
    EXPECT_EQ(out.str(), "");
    EXPECT_THAT(err.str(), HasSubstr("for more information"));
    EXPECT_THAT(err.str(), Not(HasSubstr("ExtrasError")));
    EXPECT_THAT(err.str(), HasSubstr("Thing"));
    EXPECT_THAT(err.str(), Not(HasSubstr(" or ")));
    EXPECT_THAT(err.str(), Not(HasSubstr("Usage")));
}

TEST_F(CapturedHelp, DoubleError) {
    app.set_help_all_flag("--help-all");
    EXPECT_EQ(run(CLI::ExtrasError({"Thing"})), static_cast<int>(CLI::ExitCodes::ExtrasError));
    EXPECT_EQ(out.str(), "");
    EXPECT_THAT(err.str(), HasSubstr("for more information"));
    EXPECT_THAT(err.str(), HasSubstr(" --help "));
    EXPECT_THAT(err.str(), HasSubstr(" --help-all "));
    EXPECT_THAT(err.str(), HasSubstr(" or "));
    EXPECT_THAT(err.str(), Not(HasSubstr("ExtrasError")));
    EXPECT_THAT(err.str(), HasSubstr("Thing"));
    EXPECT_THAT(err.str(), Not(HasSubstr("Usage")));
}

TEST_F(CapturedHelp, AllOnlyError) {
    app.set_help_all_flag("--help-all");
    app.set_help_flag();
    EXPECT_EQ(run(CLI::ExtrasError({"Thing"})), static_cast<int>(CLI::ExitCodes::ExtrasError));
    EXPECT_EQ(out.str(), "");
    EXPECT_THAT(err.str(), HasSubstr("for more information"));
    EXPECT_THAT(err.str(), Not(HasSubstr(" --help ")));
    EXPECT_THAT(err.str(), HasSubstr(" --help-all "));
    EXPECT_THAT(err.str(), Not(HasSubstr(" or ")));
    EXPECT_THAT(err.str(), Not(HasSubstr("ExtrasError")));
    EXPECT_THAT(err.str(), HasSubstr("Thing"));
    EXPECT_THAT(err.str(), Not(HasSubstr("Usage")));
}

TEST_F(CapturedHelp, RepacedError) {
    app.failure_message(CLI::FailureMessage::help);

    EXPECT_EQ(run(CLI::ExtrasError({"Thing"})), static_cast<int>(CLI::ExitCodes::ExtrasError));
    EXPECT_EQ(out.str(), "");
    EXPECT_THAT(err.str(), Not(HasSubstr("for more information")));
    EXPECT_THAT(err.str(), HasSubstr("ERROR: ExtrasError"));
    EXPECT_THAT(err.str(), HasSubstr("Thing"));
    EXPECT_THAT(err.str(), HasSubstr("Usage"));
}

// #87
TEST(THelp, CustomDoubleOption) {

    std::pair<int, double> custom_opt;

    CLI::App app;

    auto opt = app.add_option("posit", [&custom_opt](CLI::results_t vals) {
        custom_opt = {stol(vals.at(0)), stod(vals.at(1))};
        return true;
    });
    opt->type_name("INT FLOAT")->type_size(2);

    EXPECT_THAT(app.help(), Not(HasSubstr("x 2")));
}

TEST(THelp, CheckEmptyTypeName) {
    CLI::App app;

    auto opt = app.add_flag("-f,--flag");
    std::string name = opt->get_type_name();
    EXPECT_TRUE(name.empty());
}

TEST(THelp, AccessDescription) {
    CLI::App app{"My description goes here"};

    EXPECT_EQ(app.get_description(), "My description goes here");
}

TEST(THelp, SetDescriptionAfterCreation) {
    CLI::App app{""};

    app.description("My description goes here");

    EXPECT_EQ(app.get_description(), "My description goes here");
    EXPECT_THAT(app.help(), HasSubstr("My description goes here"));
}

TEST(THelp, AccessOptionDescription) {
    CLI::App app{};

    int x;
    auto opt = app.add_option("-a,--alpha", x, "My description goes here");

    EXPECT_EQ(opt->get_description(), "My description goes here");
}

TEST(THelp, SetOptionDescriptionAfterCreation) {
    CLI::App app{};

    int x;
    auto opt = app.add_option("-a,--alpha", x);
    opt->description("My description goes here");

    EXPECT_EQ(opt->get_description(), "My description goes here");
    EXPECT_THAT(app.help(), HasSubstr("My description goes here"));
}

TEST(THelp, CleanNeeds) {
    CLI::App app;

    int x;
    auto a_name = app.add_option("-a,--alpha", x);
    app.add_option("-b,--boo", x)->needs(a_name);

    EXPECT_THAT(app.help(), Not(HasSubstr("Requires")));
    EXPECT_THAT(app.help(), Not(HasSubstr("Needs: -a,--alpha")));
    EXPECT_THAT(app.help(), HasSubstr("Needs: --alpha"));
}

TEST(THelp, RequiredPrintout) {
    CLI::App app;

    int x;
    app.add_option("-a,--alpha", x)->required();

    EXPECT_THAT(app.help(), HasSubstr(" REQUIRED"));
}

TEST(THelp, GroupOrder) {
    CLI::App app;

    app.add_flag("--one")->group("zee");
    app.add_flag("--two")->group("aee");

    std::string help = app.help();

    auto zee_loc = help.find("zee");
    auto aee_loc = help.find("aee");

    EXPECT_NE(zee_loc, std::string::npos);
    EXPECT_NE(aee_loc, std::string::npos);
    EXPECT_LT(zee_loc, aee_loc);
}

TEST(THelp, ValidatorsText) {
    CLI::App app;

    std::string filename;
    int x;
    unsigned int y;
    app.add_option("--f1", filename)->check(CLI::ExistingFile);
    app.add_option("--f3", x)->check(CLI::Range(1, 4));
    app.add_option("--f4", y)->check(CLI::Range(12));

    std::string help = app.help();
    EXPECT_THAT(help, HasSubstr("TEXT:FILE"));
    EXPECT_THAT(help, HasSubstr("INT in [1 - 4]"));
    EXPECT_THAT(help, HasSubstr("UINT:INT in [0 - 12]")); // Loses UINT
}

TEST(THelp, ValidatorsNonPathText) {
    CLI::App app;

    std::string filename;
    app.add_option("--f2", filename)->check(CLI::NonexistentPath);

    std::string help = app.help();
    EXPECT_THAT(help, HasSubstr("TEXT:PATH"));
}

TEST(THelp, ValidatorsDirText) {
    CLI::App app;

    std::string filename;
    app.add_option("--f2", filename)->check(CLI::ExistingDirectory);

    std::string help = app.help();
    EXPECT_THAT(help, HasSubstr("TEXT:DIR"));
}

TEST(THelp, ValidatorsPathText) {
    CLI::App app;

    std::string filename;
    app.add_option("--f2", filename)->check(CLI::ExistingPath);

    std::string help = app.help();
    EXPECT_THAT(help, HasSubstr("TEXT:PATH"));
}

TEST(THelp, CombinedValidatorsText) {
    CLI::App app;

    std::string filename;
    app.add_option("--f1", filename)->check(CLI::ExistingFile | CLI::ExistingDirectory);

    // This would be nice if it put something other than string, but would it be path or file?
    // Can't programatically tell!
    // (Users can use ExistingPath, by the way)
    std::string help = app.help();
    EXPECT_THAT(help, HasSubstr("TEXT:(FILE) OR (DIR)"));
    EXPECT_THAT(help, Not(HasSubstr("PATH")));
}

// Don't do this in real life, please
TEST(THelp, CombinedValidatorsPathyText) {
    CLI::App app;

    std::string filename;
    app.add_option("--f1", filename)->check(CLI::ExistingPath | CLI::NonexistentPath);

    // Combining validators with the same type string is OK
    std::string help = app.help();
    EXPECT_THAT(help, HasSubstr("TEXT:"));
    EXPECT_THAT(help, HasSubstr("PATH"));
}

// Don't do this in real life, please (and transform does nothing here)
TEST(THelp, CombinedValidatorsPathyTextAsTransform) {
    CLI::App app;

    std::string filename;
    app.add_option("--f1", filename)->transform(CLI::ExistingPath | CLI::NonexistentPath);

    // Combining validators with the same type string is OK
    std::string help = app.help();
    EXPECT_THAT(help, HasSubstr("TEXT:(PATH(existing)) OR (PATH"));
}

// #113 Part 2
TEST(THelp, ChangingSet) {
    CLI::App app;

    std::set<int> vals{1, 2, 3};
    int val;
    app.add_option("--val", val)->check(CLI::IsMember(&vals));

    std::string help = app.help();

    EXPECT_THAT(help, HasSubstr("1"));
    EXPECT_THAT(help, Not(HasSubstr("4")));

    vals.insert(4);
    vals.erase(1);

    help = app.help();

    EXPECT_THAT(help, Not(HasSubstr("1")));
    EXPECT_THAT(help, HasSubstr("4"));
}

TEST(THelp, ChangingSetDefaulted) {
    CLI::App app;

    std::set<int> vals{1, 2, 3};
    int val = 2;
    app.add_option("--val", val, "")->check(CLI::IsMember(&vals))->capture_default_str();

    std::string help = app.help();

    EXPECT_THAT(help, HasSubstr("1"));
    EXPECT_THAT(help, Not(HasSubstr("4")));

    vals.insert(4);
    vals.erase(1);

    help = app.help();

    EXPECT_THAT(help, Not(HasSubstr("1")));
    EXPECT_THAT(help, HasSubstr("4"));
}

TEST(THelp, ChangingCaselessSet) {
    CLI::App app;

    std::set<std::string> vals{"1", "2", "3"};
    std::string val;
    app.add_option("--val", val)->check(CLI::IsMember(&vals, CLI::ignore_case));

    std::string help = app.help();

    EXPECT_THAT(help, HasSubstr("1"));
    EXPECT_THAT(help, Not(HasSubstr("4")));

    vals.insert("4");
    vals.erase("1");

    help = app.help();

    EXPECT_THAT(help, Not(HasSubstr("1")));
    EXPECT_THAT(help, HasSubstr("4"));
}

TEST(THelp, ChangingCaselessSetDefaulted) {
    CLI::App app;
    app.option_defaults()->always_capture_default();

    std::set<std::string> vals{"1", "2", "3"};
    std::string val = "2";
    app.add_option("--val", val)->check(CLI::IsMember(&vals, CLI::ignore_case));

    std::string help = app.help();

    EXPECT_THAT(help, HasSubstr("1"));
    EXPECT_THAT(help, Not(HasSubstr("4")));

    vals.insert("4");
    vals.erase("1");

    help = app.help();

    EXPECT_THAT(help, Not(HasSubstr("1")));
    EXPECT_THAT(help, HasSubstr("4"));
}

// New defaults tests (1.8)

TEST(THelp, ChangingDefaults) {

    CLI::App app;

    std::vector<int> x = {1, 2};
    CLI::Option *opt = app.add_option("-q,--quick", x);
    x = {3, 4};

    opt->capture_default_str();

    x = {5, 6};
    std::string help = app.help();

    EXPECT_THAT(help, HasSubstr("INT=[3,4] ..."));
}

TEST(THelp, ChangingDefaultsWithAutoCapture) {

    CLI::App app;
    app.option_defaults()->always_capture_default();

    std::vector<int> x = {1, 2};
    app.add_option("-q,--quick", x);
    x = {3, 4};

    std::string help = app.help();

    EXPECT_THAT(help, HasSubstr("INT=[1,2] ..."));
}

TEST(THelp, FunctionDefaultString) {

    CLI::App app;

    std::vector<int> x = {1, 2};
    CLI::Option *opt = app.add_option("-q,--quick", x);

    opt->default_function([]() { return std::string("Powerful"); });
    opt->capture_default_str();

    std::string help = app.help();

    EXPECT_THAT(help, HasSubstr("INT=Powerful"));
}
