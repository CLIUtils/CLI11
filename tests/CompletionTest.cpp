// Copyright (c) 2017-2026, University of Cincinnati, developed by Henry Schreiner
// under NSF AWARD 1414736 and by the respective contributors.
// All rights reserved.
//
// SPDX-License-Identifier: BSD-3-Clause

#include "app_helper.hpp"

#include <algorithm>
#include <sstream>
#include <string>
#include <vector>

namespace {

const char *const complete_var = "CLI11_COMPLETE";
const char *const index_var = "CLI11_COMPLETE_INDEX";
const char *const proto_var = "CLI11_COMPLETE_PROTO";

void set_request(const std::string &index) {
    put_env(complete_var, "bash");
    put_env(index_var, index);
    put_env(proto_var, "1");
}

void clear_request() {
    CLI::detail::unset_environment_value(complete_var);
    CLI::detail::unset_environment_value(index_var);
    CLI::detail::unset_environment_value(proto_var);
}

/// Drive a completion request through the public parse() entry point exactly as a shell would: the words of the line
/// with the program name excluded, and an index that still counts it.
std::string complete(CLI::App &app, const std::vector<std::string> &words, const std::string &index) {
    set_request(index);
    std::vector<std::string> args = words;
    std::reverse(args.begin(), args.end());
    std::string reply = "<not a completion request>";
    try {
        app.parse(args);
    } catch(const CLI::CallForCompletion &e) {
        reply = e.what();
    }
    clear_request();
    return reply;
}

}  // namespace

TEST_CASE("Completion: un-reverses the argument vector", "[completion]") {
    CLI::App app{"program"};
    app.add_subcommand("start", "");
    app.add_subcommand("stop", "");

    // Both words name something completable, so getting the direction wrong still produces a
    // plausible-looking reply -- the wrong one.
    CHECK(complete(app, {"sta", "stop"}, "1") == "start\n:2\n");
    CHECK(complete(app, {"sta", "stop"}, "2") == "stop\n:2\n");
}

TEST_CASE("Completion: offers every subcommand on an empty word", "[completion]") {
    CLI::App app{"program"};
    app.add_subcommand("start", "");
    app.add_subcommand("stop", "");

    CHECK(complete(app, {""}, "1") == "start\nstop\n:2\n");
}

TEST_CASE("Completion: filters subcommands by the partial word", "[completion]") {
    CLI::App app{"program"};
    app.add_subcommand("start", "");
    app.add_subcommand("stop", "");

    CHECK(complete(app, {"sta"}, "1") == "start\n:2\n");
    CHECK(complete(app, {"zzz"}, "1") == ":2\n");
}

TEST_CASE("Completion: descends into the subcommand named before the cursor", "[completion]") {
    CLI::App app{"program"};
    app.add_subcommand("start", "");
    auto *remote = app.add_subcommand("remote", "");
    remote->add_subcommand("add", "");
    auto *show = remote->add_subcommand("show", "");
    // Two deep, because one level can pass while the walk still only ever looks at the root
    show->add_subcommand("origin", "");

    CHECK(complete(app, {"remote", ""}, "2") == "add\nshow\n:2\n");
    CHECK(complete(app, {"remote", "show", ""}, "3") == "origin\n:2\n");
    // The words after the cursor are not part of the walk
    CHECK(complete(app, {"remote", "", "show"}, "2") == "add\nshow\n:2\n");
    // A word that names nothing leaves the walk where it was
    CHECK(complete(app, {"zzz", ""}, "2") == "start\nremote\n:2\n");
}

TEST_CASE("Completion: an alias both completes and descends", "[completion]") {
    CLI::App app{"program"};
    auto *remote = app.add_subcommand("remote", "");
    remote->alias("rmt");
    remote->add_subcommand("add", "");

    // Either spelling is a token the user may be typing, so both are offered
    CHECK(complete(app, {"r"}, "1") == "remote\nrmt\n:2\n");
    CHECK(complete(app, {"rmt", ""}, "2") == "add\n:2\n");
}

TEST_CASE("Completion: disabled subcommands are not offered", "[completion]") {
    CLI::App app{"program"};
    app.add_subcommand("start", "");
    app.add_subcommand("secret", "")->disabled();

    CHECK(complete(app, {"s"}, "1") == "start\n:2\n");
}

TEST_CASE("Completion: reaches through an unnamed subcommand group", "[completion]") {
    CLI::App app{"program"};
    auto *group = app.add_subcommand("", "");
    group->add_subcommand("start", "");

    // The parser reaches through the group to find `start`, so completion has to offer it
    CHECK(complete(app, {""}, "1") == "start\n:2\n");
}

TEST_CASE("Completion: offers option names once the word starts with a dash", "[completion]") {
    CLI::App app{"program"};
    app.set_help_flag("");
    app.add_flag("--verbose,-v", "");
    app.add_option("--file,-f", "");
    app.add_subcommand("start", "");

    // A single dash is a prefix of both spellings, so both are offered
    CHECK(complete(app, {"-"}, "1") == "--verbose\n-v\n--file\n-f\n:2\n");
    CHECK(complete(app, {"--"}, "1") == "--verbose\n--file\n:2\n");
    CHECK(complete(app, {"--v"}, "1") == "--verbose\n:2\n");
    CHECK(complete(app, {"-f"}, "1") == "-f\n:2\n");
    CHECK(complete(app, {"--zzz"}, "1") == ":2\n");

    // Options and subcommands never mix: a dash rules out one, its absence rules out the other
    CHECK(complete(app, {"s"}, "1") == "start\n:2\n");
}

TEST_CASE("Completion: options come from the subcommand the walk ended in", "[completion]") {
    CLI::App app{"program"};
    app.set_help_flag("");
    app.add_flag("--top", "");
    auto *remote = app.add_subcommand("remote", "");
    remote->set_help_flag("");
    remote->add_flag("--inner", "");

    CHECK(complete(app, {"--"}, "1") == "--top\n:2\n");
    CHECK(complete(app, {"remote", "--"}, "2") == "--inner\n:2\n");
}

TEST_CASE("Completion: the help flag is a candidate like any other", "[completion]") {
    CLI::App app{"program"};

    // It is typeable, so leaving it out would be a hole rather than a simplification
    CHECK(complete(app, {"--h"}, "1") == "--help\tPrint this help message and exit\n:2\n");
}

TEST_CASE("Completion: positionals are not offered as option names", "[completion]") {
    CLI::App app{"program"};
    app.set_help_flag("");
    app.add_option("input", "");
    app.add_flag("--verbose", "");

    CHECK(complete(app, {"-"}, "1") == "--verbose\n:2\n");
}

TEST_CASE("Completion: a value written after = is not read as a subcommand", "[completion]") {
    CLI::App app{"program"};
    app.set_help_flag("");
    app.add_option("--file", "");
    app.add_subcommand("value", "");

    // Bash tears `--file=value` apart on COMP_WORDBREAKS before the binary sees it. Values are not completable yet,
    // so the reply is empty -- but with the default directive, leaving the shell free to complete filenames itself.
    CHECK(complete(app, {"--file", "=", ""}, "3") == ":0\n");
    CHECK(complete(app, {"--file", "=", "val"}, "3") == ":0\n");
    CHECK(complete(app, {"--file", "="}, "2") == ":0\n");
}

TEST_CASE("Completion: degenerate cursor positions reply with nothing", "[completion]") {
    CLI::App app{"program"};
    app.add_subcommand("start", "");

    // The cursor is on the program name itself
    CHECK(complete(app, {""}, "0") == ":0\n");
    // Past the last word the shell sent, and a bad index, are both malformed rather than empty
    CHECK(complete(app, {"start", ""}, "9") == ":0\n");
    CHECK(complete(app, {""}, "not-a-number") == ":0\n");
}

TEST_CASE("Completion: the reply format is one candidate per line and a directive", "[completion]") {
    CLI::CompletionReply reply;
    CHECK(CLI::format_completion_reply(reply) == ":0\n");

    reply.results.push_back(CLI::CompletionResult{"one", ""});
    reply.results.push_back(CLI::CompletionResult{"two", ""});
    reply.directive = CLI::CompletionDirective::NoFileComp;
    CHECK(CLI::format_completion_reply(reply) == "one\ntwo\n:2\n");
}

TEST_CASE("Completion: a description rides along on the candidate's line", "[completion]") {
    CLI::CompletionReply reply;
    reply.results.push_back(CLI::CompletionResult{"one", "the first"});
    reply.results.push_back(CLI::CompletionResult{"two", ""});

    // The separator only appears when there is something to separate, so a description-less reply is unchanged
    CHECK(CLI::format_completion_reply(reply) == "one\tthe first\ntwo\n:0\n");
}

TEST_CASE("Completion: subcommands and options send their descriptions", "[completion]") {
    CLI::App app{"program"};
    app.set_help_flag("");
    app.add_flag("--verbose,-v", "Say more");
    auto *remote = app.add_subcommand("remote", "Work with remotes");
    remote->alias("rmt");

    CHECK(complete(app, {"r"}, "1") == "remote\tWork with remotes\nrmt\tWork with remotes\n:2\n");
    // Both spellings name the same option, so both carry its description
    CHECK(complete(app, {"-"}, "1") == "--verbose\tSay more\n-v\tSay more\n:2\n");
}

TEST_CASE("Completion: a description is cut at its first line break", "[completion]") {
    CLI::CompletionReply reply;
    reply.results.push_back(CLI::CompletionResult{"one", "first line\nsecond line"});
    reply.results.push_back(CLI::CompletionResult{"two", "first line\rsecond line"});

    // The shell has a single line to show a description on, so the rest is dropped rather than escaped into
    // something the listing cannot render
    CHECK(CLI::format_completion_reply(reply) == "one\tfirst line\ntwo\tfirst line\n:0\n");
}

TEST_CASE("Completion: field escaping is reversible", "[completion]") {
    CHECK(CLI::detail::escape_completion_field("plain") == "plain");
    CHECK(CLI::detail::escape_completion_field("a\tb") == "a\\tb");
    CHECK(CLI::detail::escape_completion_field("a\nb\rc") == "a\\nb\\rc");

    // Without escaping the backslash, a value holding a literal backslash-t could not be told apart from a tab
    CHECK(CLI::detail::escape_completion_field("a\\tb") == "a\\\\tb");
    CHECK(CLI::detail::escape_completion_field("a\\b") == "a\\\\b");

    // A leading ':' is the directive line's marker; only the leading one is ambiguous
    CHECK(CLI::detail::escape_completion_field(":target") == "\\:target");
    CHECK(CLI::detail::escape_completion_field("a:b") == "a:b");
    // and the backslash of an already-escaped leading backslash is not itself a leading ':'
    CHECK(CLI::detail::escape_completion_field("\\:x") == "\\\\:x");
}

TEST_CASE("Completion: a tab in a description does not fake a second field", "[completion]") {
    CLI::CompletionReply reply;
    reply.results.push_back(CLI::CompletionResult{"one", "a\tb"});

    // A description is arbitrary user text; unescaped, the shell would read `b` as a field of its own
    CHECK(CLI::format_completion_reply(reply) == "one\ta\\tb\n:0\n");
}

TEST_CASE("Completion: a candidate starting with a colon does not look like the directive", "[completion]") {
    CLI::CompletionReply reply;
    reply.results.push_back(CLI::CompletionResult{":target", ""});
    reply.results.push_back(CLI::CompletionResult{"a:b", ""});

    // Only the leading one is ambiguous, and the unescaper drops the backslash of a sequence it does not know
    CHECK(CLI::format_completion_reply(reply) == "\\:target\na:b\n:0\n");
}

TEST_CASE("Completion: get_completions works in argument coordinates", "[completion]") {
    CLI::App app{"program"};
    app.add_subcommand("start", "");
    app.add_subcommand("stop", "");

    // No program name here, unlike the index the shells report
    CLI::CompletionReply reply = app.get_completions({"sto"}, 0);
    REQUIRE(reply.results.size() == 1);
    CHECK(reply.results[0].value == "stop");
    CHECK(static_cast<int>(reply.directive) == static_cast<int>(CLI::CompletionDirective::NoFileComp));

    CHECK(app.get_completions({"sto"}, 1).results.empty());
    CHECK(app.get_completions({}, 0).results.empty());
}

TEST_CASE("Completion: the request is out of the environment before it is answered", "[completion]") {
    CLI::App app{"program"};
    app.add_subcommand("start", "");

    set_request("1");
    std::vector<std::string> args{"sta"};
    bool thrown = false;
    try {
        app.parse(args);
    } catch(const CLI::CallForCompletion &) {
        thrown = true;
        // A completion callback may run another CLI11 binary, which must not complete itself
        CHECK(CLI::detail::get_environment_value(complete_var).empty());
        CHECK(CLI::detail::get_environment_value(index_var).empty());
        CHECK(CLI::detail::get_environment_value(proto_var).empty());
    }
    clear_request();
    CHECK(thrown);
}

TEST_CASE("Completion: parsing is untouched without the environment variable", "[completion]") {
    clear_request();

    CLI::App app{"program"};
    app.add_subcommand("start", "");
    std::vector<std::string> args{"start"};
    CHECK_NOTHROW(app.parse(args));
    CHECK(app.got_subcommand("start"));
}

TEST_CASE("Completion: the intercept can be turned off", "[completion]") {
    CLI::App app{"program"};
    app.add_subcommand("start", "");
    app.disable_completion();

    set_request("1");
    std::vector<std::string> args{"start"};
    CHECK_NOTHROW(app.parse(args));
    clear_request();
    CHECK(app.got_subcommand("start"));
}

TEST_CASE("Completion: the activation variable is configurable", "[completion]") {
    CLI::App app{"program"};
    app.add_subcommand("start", "");

    // The default is part of the protocol: a packaged script is generated against it
    CHECK(app.get_completion_env_var() == "CLI11_COMPLETE");
    app.set_completion_env_var("MYPROG_COMPLETE");
    CHECK(app.get_completion_env_var() == "MYPROG_COMPLETE");

    // The default variable no longer activates anything, so this is an ordinary parse
    set_request("1");
    std::vector<std::string> args{"start"};
    CHECK_NOTHROW(app.parse(args));
    clear_request();

    put_env("MYPROG_COMPLETE", "bash");
    put_env("MYPROG_COMPLETE_INDEX", "1");
    std::vector<std::string> again{"sta"};
    CHECK_THROWS_AS(app.parse(again), CLI::CallForCompletion);
    CLI::detail::unset_environment_value("MYPROG_COMPLETE");
    CLI::detail::unset_environment_value("MYPROG_COMPLETE_INDEX");
}

TEST_CASE("Completion: the script flag is off unless it is asked for", "[completion]") {
    CLI::App app{"program", "myprog"};

    // Adding it by default would change the help output of every program that upgrades
    CHECK(app.get_completion_ptr() == nullptr);
    CHECK(app.help().find("--completion") == std::string::npos);

    app.set_completion_flag();
    REQUIRE(app.get_completion_ptr() != nullptr);
    CHECK(app.help().find("--completion SHELL") != std::string::npos);

    // and an empty name takes it back off, like the help and version flags
    app.set_completion_flag("");
    CHECK(app.get_completion_ptr() == nullptr);
    CHECK(app.help().find("--completion") == std::string::npos);
}

TEST_CASE("Completion: the script flag throws so exit() prints it", "[completion]") {
    clear_request();

    CLI::App app{"program", "myprog"};
    app.set_completion_flag();

    std::vector<std::string> args{"bash", "--completion"};  // reversed, as parse expects
    try {
        app.parse(args);
        FAIL("expected CallForCompletion");
    } catch(const CLI::CallForCompletion &e) {
        std::ostringstream out;
        CHECK(app.exit(e, out) == static_cast<int>(CLI::ExitCodes::Success));
        CHECK(out.str() == app.get_completion_script("bash"));
    }
}

TEST_CASE("Completion: the script flag name can be changed", "[completion]") {
    clear_request();

    CLI::App app{"program", "myprog"};
    app.set_completion_flag("--dump-completion", "Dump it");
    CHECK(app.help().find("--dump-completion SHELL") != std::string::npos);
    CHECK(app.help().find("Dump it") != std::string::npos);

    std::vector<std::string> args{"bash", "--dump-completion"};
    CHECK_THROWS_AS(app.parse(args), CLI::CallForCompletion);
}

TEST_CASE("Completion: the script flag rejects a shell it cannot generate", "[completion]") {
    clear_request();

    CLI::App app{"program", "myprog"};
    app.set_completion_flag();

    std::vector<std::string> args{"tcsh", "--completion"};
    CHECK_THROWS_AS(app.parse(args), CLI::ValidationError);
}

TEST_CASE("Completion: exit() writes the reply verbatim", "[completion]") {
    CLI::App app{"program"};
    std::ostringstream out;
    std::ostringstream err;
    const int code = app.exit(CLI::CallForCompletion("start\n:2\n"), out, err);
    CHECK(code == static_cast<int>(CLI::ExitCodes::Success));
    CHECK(out.str() == "start\n:2\n");
    CHECK(err.str().empty());
}

TEST_CASE("Completion: the bash script is filled in for the program", "[completion]") {
    CLI::App app{"program", "my-prog.exe"};
    const std::string script = app.get_completion_script("bash");

    // The function is named after the program, sanitized into something a shell will accept
    CHECK(script.find("\n_cli11_complete_my_prog_exe() {\n") != std::string::npos);
    // and the same name has to be the one registered, against the unsanitized program name
    CHECK(script.find("\ncomplete -o default -F _cli11_complete_my_prog_exe my-prog.exe\n") != std::string::npos);

    CHECK(script.find("@FUNCTION@") == std::string::npos);
    CHECK(script.find("@PROGRAM@") == std::string::npos);
    CHECK(script.find("@ENV@") == std::string::npos);

    CHECK_THROWS_AS(app.get_completion_script("zsh"), CLI::ValidationError);
}

TEST_CASE("Completion: the script asks for the configured environment variable", "[completion]") {
    CLI::App app{"program", "myprog"};
    app.set_completion_env_var("MYPROG_COMPLETE");
    const std::string script = app.get_completion_script("bash");

    CHECK(script.find("MYPROG_COMPLETE=bash MYPROG_COMPLETE_INDEX=") != std::string::npos);
    CHECK(script.find("CLI11_COMPLETE") == std::string::npos);
}

TEST_CASE("Completion: the script function name survives an awkward program name", "[completion]") {
    CHECK(CLI::detail::completion_script_identifier("my-prog.exe") == "my_prog_exe");
}
