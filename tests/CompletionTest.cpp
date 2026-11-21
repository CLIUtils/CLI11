// Copyright (c) 2017-2026, University of Cincinnati, developed by Henry Schreiner
// under NSF AWARD 1414736 and by the respective contributors.
// All rights reserved.
//
// SPDX-License-Identifier: BSD-3-Clause

#include "app_helper.hpp"

#include <algorithm>
#include <map>
#include <memory>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

namespace {

// The built-in hints (ExistingFile and friends) are all non-modifying; this one is modifying, with a hint
struct ModifyingHintValidator : CLI::Validator {
    explicit ModifyingHintValidator(CLI::CompletionHint completion_hint) : CLI::Validator("MODHINT") {
        completion_meta_ = CLI::detail::completion_hint_meta(completion_hint);
        func_ = [](std::string &) { return std::string{}; };
    }
};

const char *const complete_var = "CLI11_COMPLETE";
const char *const index_var = "CLI11_COMPLETE_INDEX";
const char *const proto_var = "CLI11_COMPLETE_PROTO";

const char *const words_var = "CLI11_COMPLETE_WORDS";

/// Put the words of a line in the environment the way the script does, under the names that follow `prefix`
void set_words(const std::string &prefix, const std::vector<std::string> &words) {
    put_env(prefix + "_WORDS", std::to_string(words.size()));
    for(std::size_t ii = 0; ii < words.size(); ++ii)
        put_env(prefix + "_WORD_" + std::to_string(ii + 1), words[ii]);
}

void set_request(const std::string &index, const std::vector<std::string> &words) {
    put_env(complete_var, "bash");
    put_env(index_var, index);
    put_env(proto_var, "1");
    set_words(complete_var, words);
}

void clear_request() {
    CLI::detail::unset_environment_value(complete_var);
    CLI::detail::unset_environment_value(index_var);
    CLI::detail::unset_environment_value(proto_var);
    CLI::detail::unset_environment_value(words_var);
}

/// The one argument the script runs the program with. It is not part of the request.
const char *const guard_argument = "--cli11-complete";

/// Parse with whatever request is in the environment, and return the reply if there was one
std::string parse_reply(CLI::App &app) {
    try {
        app.parse(std::vector<std::string>{guard_argument});
    } catch(const CLI::CallForCompletion &e) {
        return e.what();
    }
    return "<not a completion request>";
}

/// Drive a completion request through the public parse() entry point exactly as a shell would: the words of the line
/// with the program name excluded, and an index that still counts it.
std::string complete(CLI::App &app, const std::vector<std::string> &words, const std::string &index) {
    set_request(index, words);
    std::string reply = parse_reply(app);
    clear_request();
    return reply;
}

}  // namespace

TEST_CASE("Completion: the words of the request are read in order", "[completion]") {
    CLI::App app{"program"};
    app.add_subcommand("start", "");
    app.add_subcommand("stop", "");

    CHECK(complete(app, {"sta", "stop"}, "1") == "start\n:2\n");
    CHECK(complete(app, {"sta", "stop"}, "2") == "stop\n:2\n");
}

TEST_CASE("Completion: the arguments of the run are not the request", "[completion]") {
    CLI::App app{"program"};
    app.add_subcommand("start", "");
    app.add_subcommand("stop", "");

    // A script that outlives its binary must not have run the line being typed, so the line is not in the arguments
    set_request("1", {"sta"});
    std::string reply;
    try {
        app.parse(std::vector<std::string>{"stop"});
    } catch(const CLI::CallForCompletion &e) {
        reply = e.what();
    }
    clear_request();
    CHECK(reply == "start\n:2\n");
    CHECK_FALSE(app.got_subcommand("stop"));

    // A count that cannot be read is not a usable request
    set_request("1", {"sta"});
    put_env(words_var, "many");
    CHECK(parse_reply(app) == ":1\n");
    clear_request();
    CLI::detail::unset_environment_value("CLI11_COMPLETE_WORD_1");
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

TEST_CASE("Completion: an empty word before the cursor does not stop the walk", "[completion]") {
    CLI::App app{"program"};
    app.set_help_flag("");
    CLI::App *start = app.add_subcommand("start", "");
    start->set_help_flag("");
    start->add_flag("--inner", "");

    // An empty argument is an extra here, and the words after it are read as before
    CHECK(complete(app, {"", "start", "--"}, "3") == "--inner\n:2\n");
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

TEST_CASE("Completion: an option name is matched the way a parse would match it", "[completion]") {
    CLI::App app{"program"};
    app.set_help_flag("");
    app.add_option("--my_opt", "")->ignore_underscore();
    app.add_option("--other,-o", "")->ignore_case();

    // The setting normalises the typed word as well as the name, so either spelling reaches the option
    CHECK(complete(app, {"--my_o"}, "1") == "--my_opt\n:2\n");
    CHECK(complete(app, {"--myo"}, "1") == "--my_opt\n:2\n");
    CHECK(complete(app, {"--OT"}, "1") == "--other\n:2\n");
    CHECK(complete(app, {"-O"}, "1") == "-o\n:2\n");

    // Each setting is on for one option only, so the word that needs the other one reaches neither
    CHECK(complete(app, {"--MY"}, "1") == ":2\n");
    CHECK(complete(app, {"--oth_"}, "1") == ":2\n");
}

TEST_CASE("Completion: a subcommand name is matched the way a parse would match it", "[completion]") {
    CLI::App app{"program"};
    app.set_help_flag("");
    app.add_subcommand("My_Sub", "")->ignore_case()->ignore_underscore()->alias("Other_Name");
    app.add_subcommand("plain", "");

    // The candidate is the declared spelling either way: it is the whole token the shell has to insert
    CHECK(complete(app, {"my"}, "1") == "My_Sub\n:2\n");
    CHECK(complete(app, {"othername"}, "1") == "Other_Name\n:2\n");
    CHECK(complete(app, {"PLA"}, "1") == ":2\n");
}

TEST_CASE("Completion: a partial word completes whether or not a parse would accept the abbreviation", "[completion]") {
    CLI::App app{"program"};
    app.set_help_flag("");
    app.add_subcommand("start", "");

    // allow_subcommand_prefix_matching decides whether `sta` can be run; completing it to `start` is what makes
    // that question moot, so completion does not consult the setting
    CHECK_FALSE(app.get_allow_subcommand_prefix_matching());
    CHECK(complete(app, {"sta"}, "1") == "start\n:2\n");
    app.allow_subcommand_prefix_matching();
    CHECK(complete(app, {"sta"}, "1") == "start\n:2\n");
}

TEST_CASE("Completion: the word after a flag is a fresh token", "[completion]") {
    CLI::App app{"program"};
    app.set_help_flag("");
    app.add_flag("--verbose", "");
    app.add_subcommand("start", "");

    // A flag consumes nothing, so what follows it is whatever it would have been at the start of the line
    CHECK(complete(app, {"--verbose", ""}, "2") == "start\n:2\n");
    CHECK(complete(app, {"--verbose", "--"}, "2") == "--verbose\n:2\n");
}

TEST_CASE("Completion: an option with nothing to offer leaves the shell to it", "[completion]") {
    CLI::App app{"program"};
    app.set_help_flag("");
    std::string file;
    app.add_option("--file", file);
    app.add_subcommand("start", "");

    // Nothing is known about the value, and a filename is a better guess than a subcommand that cannot go there
    CHECK(complete(app, {"--file", ""}, "2") == ":0\n");
    // An unknown option name says nothing about the next word either
    CHECK(complete(app, {"--zzz", ""}, "2") == "start\n:2\n");
}

TEST_CASE("Completion: a path is handed back to the shell", "[completion]") {
    CLI::App app{"program"};
    app.set_help_flag("");
    std::string path;
    app.add_option("--file", path)->check(CLI::ExistingFile);
    app.add_option("--dir", path)->check(CLI::ExistingDirectory);
    app.add_option("--any", path)->check(CLI::ExistingPath);
    app.add_option("--new", path)->check(CLI::NonexistentPath);

    // The directive is the whole reply: only the shell knows the directory the line is being typed in, so it produces
    // the candidates and none are sent to it
    CHECK(complete(app, {"--file", ""}, "2") == ":8\n");
    CHECK(complete(app, {"--dir", ""}, "2") == ":16\n");
    // Either kind of path is both of them
    CHECK(complete(app, {"--any", ""}, "2") == ":24\n");
    // A path that must not exist yet says nothing about itself, which leaves it to the shell as well
    CHECK(complete(app, {"--new", ""}, "2") == ":0\n");
    // A partial value changes nothing, because the side doing the filtering is the side doing the completing
    CHECK(complete(app, {"--file", "/et"}, "2") == ":8\n");
}

TEST_CASE("Completion: an option can declare the kind of path it wants", "[completion]") {
    CLI::App app{"program"};
    app.set_help_flag("");
    std::string path;
    // Nothing validates this, so the hint is the only way the shell can be told
    app.add_option("--out", path)->completion_hint(CLI::CompletionHint::Dir);

    CHECK(complete(app, {"--out", ""}, "2") == ":16\n");
}

TEST_CASE("Completion: a hint survives being copied into the Validator base", "[completion]") {
    CLI::App app{"program"};
    std::string path;
    // Option::check slices ExistingFileValidator away, so the hint has to be reachable from the base class
    CHECK(app.add_option("--file", path)->check(CLI::ExistingFile)->get_completion_hint() == CLI::CompletionHint::File);

    const CLI::Validator plain = CLI::Validator([](std::string &) { return std::string{}; }, "anything");
    // A side that describes nothing leaves the other side's answer standing, whichever way round it is written
    CHECK(app.add_option("--and", path)->check(CLI::ExistingFile & plain)->get_completion_hint() ==
          CLI::CompletionHint::File);
    CHECK(app.add_option("--and-rev", path)->check(plain & CLI::ExistingFile)->get_completion_hint() ==
          CLI::CompletionHint::File);
    // and there is no intersecting "a file" with "a directory", so the left side wins
    CHECK(app.add_option("--both", path)->check(CLI::ExistingFile & CLI::ExistingDirectory)->get_completion_hint() ==
          CLI::CompletionHint::File);
    // but either one is a path
    CHECK(app.add_option("--either", path)->check(CLI::ExistingDirectory | CLI::ExistingFile)->get_completion_hint() ==
          CLI::CompletionHint::Path);

    // A validator that is not applied does not describe what the option accepts
    CHECK(app.add_option("--off", path)->check(CLI::ExistingFile.active(false))->get_completion_hint() ==
          CLI::CompletionHint::None);
}

TEST_CASE("Completion: a modifying validator's hint waits for the whole loop", "[completion]") {
    CLI::App app{"program"};
    std::string path;

    // A transform's hint waits in case a later check outranks it
    CHECK(app.add_option("--out", path)
              ->transform(ModifyingHintValidator(CLI::CompletionHint::Dir))
              ->get_completion_hint() == CLI::CompletionHint::Dir);
}

#if (defined(CLI11_ENABLE_EXTRA_VALIDATORS) && CLI11_ENABLE_EXTRA_VALIDATORS == 1) ||                                  \
    (!defined(CLI11_DISABLE_EXTRA_VALIDATORS) || CLI11_DISABLE_EXTRA_VALIDATORS == 0)

TEST_CASE("Completion: an option's values come from a set it is checked against", "[completion]") {
    CLI::App app{"program"};
    app.set_help_flag("");
    std::string level;
    app.add_option("--level,-l", level)->check(CLI::IsMember({"fast", "slow"}));
    app.add_subcommand("fast-subcommand", "");

    // The declared order is meaningful, so KeepOrder rides along with NoFileComp
    CHECK(complete(app, {"--level", ""}, "2") == "fast\nslow\n:6\n");
    CHECK(complete(app, {"--level", "f"}, "2") == "fast\n:6\n");
    CHECK(complete(app, {"--level", "zzz"}, "2") == ":6\n");
    // Either spelling of the option introduces its values
    CHECK(complete(app, {"-l", ""}, "2") == "fast\nslow\n:6\n");

    // A real parse would give the word to the option, so the subcommand that shares its prefix is not a candidate
    CHECK(complete(app, {"--level", "fast-sub"}, "2") == ":6\n");
}

TEST_CASE("Completion: values survive being copied into the Validator base", "[completion]") {
    CLI::App app{"program"};
    std::string level;
    // Option::check copies its argument into a Validator, slicing IsMember away, so the values have to be reachable
    // from the base class
    CLI::Option *opt = app.add_option("--level", level)->check(CLI::IsMember({"fast", "slow"}));
    CHECK(opt->get_completion_choices() == std::vector<std::string>{"fast", "slow"});

    // A value holding the characters generate_set writes around a set comes back whole
    std::string odd;
    CLI::Option *strange = app.add_option("--odd", odd)->check(CLI::IsMember({"a,b", "{c}"}));
    CHECK(strange->get_completion_choices() == std::vector<std::string>{"a,b", "{c}"});
}

TEST_CASE("Completion: combining validators combines what they accept", "[completion]") {
    CLI::App app{"program"};
    std::string level;

    // A value has to satisfy both sides of an `&`, so offering all of the left one's set would offer values the right
    // one rejects
    CHECK(app.add_option("--and", level)
              ->check(CLI::IsMember({"fast", "slow"}) & CLI::IsMember({"slow", "other"}))
              ->get_completion_choices() == std::vector<std::string>{"slow"});
    // and either side of an `|` is enough, so both sets are worth offering, but a value both accept is one value
    CHECK(app.add_option("--or", level)
              ->check(CLI::IsMember({"fast"}) | CLI::IsMember({"slow"}))
              ->get_completion_choices() == std::vector<std::string>{"fast", "slow"});
    CHECK(app.add_option("--overlap", level)
              ->check(CLI::IsMember({"fast"}) | CLI::IsMember({"fast", "slow"}))
              ->get_completion_choices() == std::vector<std::string>{"fast", "slow"});
}

TEST_CASE("Completion: separate check calls narrow the values like a single & does", "[completion]") {
    CLI::App app{"program"};
    std::string level;

    // Two check() calls are the same demand as one `&`: the value has to get past both, so the two spellings cannot
    // answer differently
    CHECK(app.add_option("--split", level)
              ->check(CLI::IsMember({"fast", "slow"}))
              ->check(CLI::IsMember({"slow", "other"}))
              ->get_completion_choices() == std::vector<std::string>{"slow"});
    CHECK(app.add_option("--joined", level)
              ->check(CLI::IsMember({"fast", "slow"}) & CLI::IsMember({"slow", "other"}))
              ->get_completion_choices() == std::vector<std::string>{"slow"});

    // and a check with nothing to enumerate narrows nothing
    CHECK(app.add_option("--partial", level)
              ->check(CLI::IsMember({"fast", "slow"}))
              ->check([](const std::string &) { return std::string{}; })
              ->get_completion_choices() == std::vector<std::string>{"fast", "slow"});
}

TEST_CASE("Completion: a check outranks the transform in front of it", "[completion]") {
    CLI::App app{"program"};
    std::string level;

    // Whether `quick` is acceptable depends on the check accepting what the transform turns it into, which is only
    // knowable by running the transform. So the check answers, and a key it would reject is never offered
    CHECK(app.add_option("--level", level)
              ->transform(CLI::Transformer(std::map<std::string, std::string>{{"quick", "fast"}}))
              ->check(CLI::IsMember({"fast", "slow"}))
              ->get_completion_choices() == std::vector<std::string>{"fast", "slow"});

    // and with nothing to check against, the keys are all there is to go on
    std::string plain;
    CHECK(app.add_option("--plain", plain)
              ->transform(CLI::Transformer(std::map<std::string, std::string>{{"quick", "fast"}}))
              ->get_completion_choices() == std::vector<std::string>{"quick"});
}

TEST_CASE("Completion: a validator that enumerates nothing does not erase the one that does", "[completion]") {
    CLI::App app{"program"};
    std::string level;
    const CLI::Validator plain = CLI::Validator([](std::string &) { return std::string{}; }, "anything");

    // The unenumerated side still constrains the value, but applying that constraint would mean running it, and
    // completion does not run user code, so `ExistingFile & size_check` keeps whichever side has something to say
    CHECK(app.add_option("--and", level)->check(CLI::IsMember({"fast"}) & plain)->get_completion_choices() ==
          std::vector<std::string>{"fast"});
    CHECK(app.add_option("--and-rev", level)->check(plain & CLI::IsMember({"fast"}))->get_completion_choices() ==
          std::vector<std::string>{"fast"});
    // For `|` the acceptable set is unbounded once one side does not enumerate, so this is a subset of it
    CHECK(app.add_option("--or", level)->check(CLI::IsMember({"fast"}) | plain)->get_completion_choices() ==
          std::vector<std::string>{"fast"});
    // and neither side saying anything leaves nothing to offer
    CHECK(app.add_option("--neither", level)->check(plain & plain)->get_completion_choices().empty());
}

TEST_CASE("Completion: a transformer offers the names it translates", "[completion]") {
    CLI::App app{"program"};
    int level{0};
    // The keys are what there is to type; the values are what the program ends up with
    CHECK(app.add_option("--t", level)
              ->transform(CLI::Transformer(std::map<std::string, int>{{"one", 1}, {"two", 2}}))
              ->get_completion_choices()
              .size() == 2);
    CHECK(app.add_option("--c", level)
              ->transform(CLI::CheckedTransformer(std::map<std::string, int>{{"one", 1}}))
              ->get_completion_choices() == std::vector<std::string>{"one"});
}

TEST_CASE("Completion: an inactive validator is not asked for values", "[completion]") {
    CLI::App app{"program"};
    std::string level;
    CLI::Option *opt = app.add_option("--level", level)->check(CLI::IsMember({"fast"}).active(false));

    // A validator that is not applied does not describe what the option accepts
    CHECK(opt->get_completion_choices().empty());
}

TEST_CASE("Completion: values worth listing outrank the hint beside them", "[completion]") {
    CLI::App app{"program"};
    app.set_help_flag("");
    std::string path;
    // Two named files are a better answer than every file in the directory, so the enumerated side is the one offered
    app.add_option("--file", path)->check(CLI::ExistingFile & CLI::IsMember({"a.txt", "b.txt"}));
    CHECK(complete(app, {"--file", ""}, "2") == "a.txt\nb.txt\n:6\n");

    // and a declared hint outranks both, since it is the program speaking rather than a guess about what it meant
    app.add_option("--dir", path)->check(CLI::IsMember({"a.txt"}))->completion_hint(CLI::CompletionHint::Dir);
    CHECK(complete(app, {"--dir", ""}, "2") == ":16\n");
}

#endif

TEST_CASE("Completion: a value written after = is not read as a subcommand", "[completion]") {
    CLI::App app{"program"};
    app.set_help_flag("");
    app.add_option("--file", "");
    app.add_subcommand("value", "");

    // One word per argument, whatever a shell had to do to produce that: `value` here is the option's value, not the
    // subcommand of the same name
    CHECK(complete(app, {"--file=value"}, "1") == ":prefix=--file=\n:0\n");
    CHECK(complete(app, {"--file="}, "1") == ":prefix=--file=\n:0\n");

    // An `=` with nothing after it hands the option no value at all, so it still takes the next word — and takes it
    // without classifying it, so the subcommand of that name is not a candidate. Verified against the parser.
    CHECK(complete(app, {"--file=", ""}, "2") == ":0\n");
}

#if (defined(CLI11_ENABLE_EXTRA_VALIDATORS) && CLI11_ENABLE_EXTRA_VALIDATORS == 1) ||                                  \
    (!defined(CLI11_DISABLE_EXTRA_VALIDATORS) || CLI11_DISABLE_EXTRA_VALIDATORS == 0)

TEST_CASE("Completion: a value written after = completes to a whole token", "[completion]") {
    CLI::App app{"program"};
    app.set_help_flag("");
    std::string level;
    app.add_option("--level", level)->check(CLI::IsMember({"fast", "slow"}));

    // The candidate is the whole word that has to end up on the command line; the prefix says how much of it the shell
    // already has
    CHECK(complete(app, {"--level="}, "1") == "--level=fast\n--level=slow\n:prefix=--level=\n:6\n");
    CHECK(complete(app, {"--level=s"}, "1") == "--level=slow\n:prefix=--level=\n:6\n");
    CHECK(complete(app, {"--level=zzz"}, "1") == ":prefix=--level=\n:6\n");
}

#endif

#if (defined(CLI11_ENABLE_EXTRA_VALIDATORS) && CLI11_ENABLE_EXTRA_VALIDATORS == 1) ||                                  \
    (!defined(CLI11_DISABLE_EXTRA_VALIDATORS) || CLI11_DISABLE_EXTRA_VALIDATORS == 0)

TEST_CASE("Completion: a value written onto a short name completes to a whole token", "[completion]") {
    CLI::App app{"program"};
    app.set_help_flag("");
    std::string level;
    app.add_option("--level,-l", level)->check(CLI::IsMember({"fast", "slow"}));
    app.add_flag("--verbose,-v", "");

    // A short name takes its value with no separator at all, so the option and the value share one word
    CHECK(complete(app, {"-ls"}, "1") == "-lslow\n:prefix=-l\n:6\n");
    // An `=` is no separator for a short name, a parse taking it as the first character of the value, so it
    // matches no value here either
    CHECK(complete(app, {"-l="}, "1") == ":prefix=-l\n:6\n");
    // The flags in front of it are part of the word the shell will replace, so they are part of every candidate
    CHECK(complete(app, {"-vlf"}, "1") == "-vlfast\n:prefix=-vl\n:6\n");

    // A word that ends at the name has no value in it yet: the value is the next word, which the shell reaches by
    // adding a space rather than by completing this one
    CHECK(complete(app, {"-l"}, "1") == "-l\n:2\n");
    CHECK(complete(app, {"-vl"}, "1") == "-vl\n:2\n");
}

TEST_CASE("Completion: the value after a bundle belongs to the last name in it", "[completion]") {
    CLI::App app{"program"};
    app.set_help_flag("");
    std::string level;
    app.add_option("--level,-l", level)->check(CLI::IsMember({"fast", "slow"}));
    app.add_flag("--verbose,-v", "");
    app.add_subcommand("start", "");

    // A parse hands `-vl` to `-v` and then to `-l`, so the word after it is `-l`'s value and not a token of its own
    CHECK(complete(app, {"-vl", ""}, "2") == "fast\nslow\n:6\n");
    CHECK(complete(app, {"-vl", "f"}, "2") == "fast\n:6\n");

    // A bundle that already holds its value has nothing left for the next word
    CHECK(complete(app, {"-vlfast", ""}, "2") == "start\n:2\n");
    // and neither has one that ends in a flag, or one holding a name this app does not have
    CHECK(complete(app, {"-vv", ""}, "2") == "start\n:2\n");
    CHECK(complete(app, {"-vz", ""}, "2") == "start\n:2\n");
}

#endif

TEST_CASE("Completion: a finished bundle of flags is offered back unchanged", "[completion]") {
    CLI::App app{"program"};
    app.set_help_flag("");
    app.add_flag("--verbose,-v", "");
    app.add_flag("--quiet,-q", "");
    app.add_option("input", "");

    // Every name in it is one this app has, so the word stands as typed, which is how the shell is told to move on
    CHECK(complete(app, {"-vq"}, "1") == "-vq\n:2\n");
    // A short name is one character long, so trailing text no name matches can only be a mistake. A filename is not a
    // candidate for the middle of an option word either.
    CHECK(complete(app, {"-vz"}, "1") == ":2\n");
    CHECK(complete(app, {"-zv"}, "1") == ":2\n");
    // A positional is not part of a bundle, whatever its name
    CHECK(complete(app, {"-vinput"}, "1") == ":2\n");
}

TEST_CASE("Completion: a path written onto a short name is still the shell's to complete", "[completion]") {
    CLI::App app{"program"};
    app.set_help_flag("");
    std::string path;
    app.add_option("--config,-c", path)->check(CLI::ExistingFile);

    // The shell comes back from `/et` with `/etc/`, and only the binary could have said that `-c` goes in front of it
    CHECK(complete(app, {"-c/et"}, "1") == ":prefix=-c\n:8\n");
}

TEST_CASE("Completion: a path written after = is still the shell's to complete", "[completion]") {
    CLI::App app{"program"};
    app.set_help_flag("");
    std::string path;
    app.add_option("--config", path)->check(CLI::ExistingFile);

    // No candidates, but still a prefix: the shell comes back from `/et` with `/etc/`, and only the binary could have
    // said that `--config=` goes in front of it
    CHECK(complete(app, {"--config=/et"}, "1") == ":prefix=--config=\n:8\n");
    // A value holding a colon is a value, not a name to look up; bash tears one apart the same way it does an `=`
    CHECK(complete(app, {"--config", "a:b"}, "2") == ":8\n");
}

TEST_CASE("Completion: an = after something that takes no value offers nothing", "[completion]") {
    CLI::App app{"program"};
    app.set_help_flag("");
    app.add_flag("--verbose", "");
    app.add_subcommand("value", "");

    // A flag has no value to complete, and neither has an unknown option. The subcommand that shares the word is
    // not a candidate, and a file name is not one either.
    CHECK(complete(app, {"--verbose=val"}, "1") == ":2\n");
    CHECK(complete(app, {"--nope=val"}, "1") == ":2\n");
}

TEST_CASE("Completion: a Windows-style name is offered once the word starts with a slash", "[completion]") {
    CLI::App app{"program"};
    app.set_help_flag("");
    app.allow_windows_style_options(true);
    app.add_option("--level,-l", "");
    app.add_subcommand("levy", "");

    CHECK(complete(app, {"/le"}, "1") == "/level\n:2\n");
    CHECK(complete(app, {"/l"}, "1") == "/level\n/l\n:2\n");
    CHECK(complete(app, {"--le"}, "1") == "--level\n:2\n");
    // `/etc/passwd` reaches this program as an unrecognised option, verified, so it is not a path to complete
    CHECK(complete(app, {"/et"}, "1") == ":2\n");
}

TEST_CASE("Completion: a lone slash starts an option name, as a lone dash does", "[completion]") {
    CLI::App app{"program"};
    app.set_help_flag("");
    app.allow_windows_style_options(true);
    app.add_option("--level,-l", "")->completion_hint(CLI::CompletionHint::File);
    app.add_option("files", "")->completion_hint(CLI::CompletionHint::File);

    // Every absolute path the shell would offer from `/` is an unknown option to this program
    CHECK(complete(app, {"/"}, "1") == "/level\n/l\n:2\n");
    CHECK(complete(app, {"-"}, "1") == "--level\n-l\n:2\n");
    // A relative path is still the positional's
    CHECK(complete(app, {"sub"}, "1") == ":8\n");
}

TEST_CASE("Completion: a slash is an ordinary word where Windows-style options are off", "[completion]") {
    CLI::App app{"program"};
    app.set_help_flag("");
    app.add_option("--level,-l", "");
    app.add_option("file", "");

    // The walk reads the setting, not the platform it was built for
    app.allow_windows_style_options(false);
    CHECK(complete(app, {"/et"}, "1") == ":0\n");
    app.allow_windows_style_options(true);
    CHECK(complete(app, {"/et"}, "1") == ":2\n");
}

TEST_CASE("Completion: an option and its Windows-style value do not take a positional's turn", "[completion]") {
    CLI::App app{"program"};
    app.set_help_flag("");
    app.allow_windows_style_options(true);
    app.add_option("--level,-l", "");
    CLI::App *start = app.add_subcommand("start", "");
    start->add_subcommand("inner", "");

    // `start` is the option's value, not the subcommand, so the walk stays at the root
    CHECK(complete(app, {"/level", "start", ""}, "3") == "start\n:2\n");
    CHECK(complete(app, {"/level:fast", ""}, "2") == "start\n:2\n");
}

TEST_CASE("Completion: an unknown Windows-style option has nothing to say", "[completion]") {
    CLI::App app{"program"};
    app.set_help_flag("");
    app.allow_windows_style_options(true);
    app.add_flag("--verbose,-v", "");
    app.add_subcommand("val", "");

    // Neither has a value to complete, so the subcommand sharing the word is still not a candidate
    CHECK(complete(app, {"/verbose:val"}, "1") == ":2\n");
    CHECK(complete(app, {"/nope:val"}, "1") == ":2\n");
}

TEST_CASE("Completion: a slash into an invalid name character is not the Windows-style shape", "[completion]") {
    CLI::App app{"program"};
    app.set_help_flag("");
    app.allow_windows_style_options(true);
    app.add_option("--level,-l", "");

    // A dash can't start a name, so this never becomes /name:value, even with a colon later in the word
    CHECK(complete(app, {"/-x:y"}, "1") == ":0\n");
}

TEST_CASE("Completion: an option group's options are the app's own", "[completion]") {
    CLI::App app{"program"};
    app.set_help_flag("");
    app.add_flag("--verbose", "");
    CLI::App *outer = app.add_option_group("Outer");
    outer->add_option("--host", "");
    // Nested, because a walk that reaches one level deep can pass while getting the recursion wrong
    CLI::App *inner = outer->add_option_group("Inner");
    inner->add_option("--port", "");

    CHECK(complete(app, {"--"}, "1") == "--verbose\n--host\n--port\n:2\n");
    CHECK(complete(app, {"--ho"}, "1") == "--host\n:2\n");
    // A group is reached through, not entered: its own name is empty and there is no word that names it
    CHECK(complete(app, {""}, "1") == ":2\n");
}

TEST_CASE("Completion: an option group's option is offered once", "[completion]") {
    CLI::App app{"program"};
    app.set_help_flag("");
    CLI::App *group = app.add_option_group("Group");
    group->add_option("--host,-h", "");

    // Each Option belongs to exactly one App, so reaching through the groups cannot reach the same one twice
    CHECK(complete(app, {"-"}, "1") == "--host\n-h\n:2\n");
}

TEST_CASE("Completion: a disabled option group is not reached through", "[completion]") {
    CLI::App app{"program"};
    app.set_help_flag("");
    app.add_flag("--verbose", "");
    CLI::App *group = app.add_option_group("Group");
    group->add_option("--host", "");

    CHECK(complete(app, {"--"}, "1") == "--verbose\n--host\n:2\n");
    // A parse skips a disabled group, so its options are not this app's to offer
    group->disabled();
    CHECK(complete(app, {"--"}, "1") == "--verbose\n:2\n");
}

#if (defined(CLI11_ENABLE_EXTRA_VALIDATORS) && CLI11_ENABLE_EXTRA_VALIDATORS == 1) ||                                  \
    (!defined(CLI11_DISABLE_EXTRA_VALIDATORS) || CLI11_DISABLE_EXTRA_VALIDATORS == 0)

TEST_CASE("Completion: an option group's option completes its values", "[completion]") {
    CLI::App app{"program"};
    app.set_help_flag("");
    CLI::App *group = app.add_option_group("Group");
    group->add_option("--level,-l", "")->check(CLI::IsMember({"fast", "slow"}));

    CHECK(complete(app, {"--level", ""}, "2") == "fast\nslow\n:6\n");
    CHECK(complete(app, {"--level=f"}, "1") == "--level=fast\n:prefix=--level=\n:6\n");
    CHECK(complete(app, {"-lf"}, "1") == "-lfast\n:prefix=-l\n:6\n");
}

TEST_CASE("Completion: a Windows-style name reaches into an option group", "[completion]") {
    CLI::App app{"program"};
    app.set_help_flag("");
    app.allow_windows_style_options(true);
    CLI::App *group = app.add_option_group("Group");
    group->add_option("--level,-l", "")->check(CLI::IsMember({"fast", "slow"}));

    // This classifier has one lookup for both name kinds, so it cannot borrow get_option_no_throw's reach
    CHECK(complete(app, {"/le"}, "1") == "/level\n:2\n");
    CHECK(complete(app, {"/level:f"}, "1") == "/level:fast\n:prefix=/level:\n:6\n");
    CHECK(complete(app, {"/l:"}, "1") == "/l:fast\n/l:slow\n:prefix=/l:\n:6\n");
}

TEST_CASE("Completion: an option group's positionals take their turn after the app's own", "[completion]") {
    CLI::App app{"program"};
    app.set_help_flag("");
    app.add_option("first", "")->check(CLI::IsMember({"a1", "a2"}));
    CLI::App *outer = app.add_option_group("Outer");
    CLI::App *inner = outer->add_option_group("Inner");
    inner->add_option("second", "")->check(CLI::IsMember({"b1", "b2"}));

    // _parse_positional fills this app's own positionals before it tries the nameless groups, so that is the order
    CHECK(complete(app, {""}, "1") == "a1\na2\n:6\n");
    CHECK(complete(app, {"a1", ""}, "2") == "b1\nb2\n:6\n");
    // Both are full, and a third word is an error rather than a value, verified against a parse
    CHECK(complete(app, {"a1", "b1", ""}, "3") == ":2\n");
}

TEST_CASE("Completion: an option group's positional is not reserved for", "[completion]") {
    CLI::App app{"program"};
    app.set_help_flag("");
    app.add_option("--files", "")->expected(-1);
    CLI::App *group = app.add_option_group("Group");
    group->add_option("target", "")->required()->check(CLI::IsMember({"t1", "t2"}));

    // A required positional of this app's own would reserve this word and make it the positional's turn. A group's
    // reserves nothing, so `--files a b` really does leave target empty and fail to parse, verified.
    CHECK(complete(app, {"--files", "a", ""}, "3") == ":0\n");
}

TEST_CASE("Completion: a value written after a colon completes to a whole token", "[completion]") {
    CLI::App app{"program"};
    app.set_help_flag("");
    app.allow_windows_style_options(true);
    std::string level;
    app.add_option("--level,-l", level)->check(CLI::IsMember({"fast", "slow"}));

    CHECK(complete(app, {"/level:"}, "1") == "/level:fast\n/level:slow\n:prefix=/level:\n:6\n");
    CHECK(complete(app, {"/level:s"}, "1") == "/level:slow\n:prefix=/level:\n:6\n");
    // One spelling reaches the option by either of its names
    CHECK(complete(app, {"/l:s"}, "1") == "/l:slow\n:prefix=/l:\n:6\n");
    // A colon with nothing after it hands over no value, so the option still takes the following word
    CHECK(complete(app, {"/level:", ""}, "2") == "fast\nslow\n:6\n");
    CHECK(complete(app, {"/level", ""}, "2") == "fast\nslow\n:6\n");
}

#endif

TEST_CASE("Completion: nothing is offered after the end-of-options marker", "[completion]") {
    CLI::App app{"program"};
    app.set_help_flag("");
    app.add_flag("--verbose", "");
    app.add_subcommand("start", "");

    // Before the marker both sources still apply
    CHECK(complete(app, {"--"}, "1") == "--verbose\n:2\n");
    CHECK(complete(app, {"sta"}, "1") == "start\n:2\n");
    // After it every word is a positional, so neither an option name nor a subcommand name can be one. This app
    // declares no positional either, so a word there would be a parse error and even a filename is a wrong answer.
    CHECK(complete(app, {"--", "--verb"}, "2") == ":2\n");
    CHECK(complete(app, {"--", "sta"}, "2") == ":2\n");
    // The marker holds for the rest of the line, not just for the word after it
    CHECK(complete(app, {"--", "one", "sta"}, "3") == ":2\n");
    // And a word that would otherwise move the walk does not, so the marker cannot be escaped through a subcommand
    CHECK(complete(app, {"--", "start", ""}, "3") == ":2\n");
}

TEST_CASE("Completion: a positional nothing is known about is left to the shell", "[completion]") {
    CLI::App app{"program"};
    app.set_help_flag("");
    std::string anything;
    app.add_option("input", anything);

    // Nothing constrains the value, so a path is the best guess left, which the shell offers unprompted and is
    // reached by saying nothing rather than by ringing the bell with NoFileComp
    CHECK(complete(app, {""}, "1") == ":0\n");
    // Once the positionals are full there is nothing a word could become, and a filename could not be right either
    CHECK(complete(app, {"x", ""}, "2") == ":2\n");
}

TEST_CASE("Completion: a path-shaped positional waits until no name matches", "[completion]") {
    CLI::App app{"program"};
    app.set_help_flag("");
    std::string path;
    app.add_option("file", path)->check(CLI::ExistingFile);
    app.add_subcommand("start", "");

    // A whole directory in the same menu as a name buries it, so the name answers while one still matches
    CHECK(complete(app, {""}, "1") == "start\n:2\n");
    CHECK(complete(app, {"s"}, "1") == "start\n:2\n");
    // and the word is the shell's as soon as none does
    CHECK(complete(app, {"R"}, "1") == ":8\n");
}

TEST_CASE("Completion: a positional that takes many values keeps its turn", "[completion]") {
    CLI::App app{"program"};
    app.set_help_flag("");
    std::vector<std::string> files;
    app.add_option("files", files)->check(CLI::ExistingFile);

    // A vector positional never fills up, so every word of the line is still one of its values
    CHECK(complete(app, {""}, "1") == ":8\n");
    CHECK(complete(app, {"a", "b", ""}, "3") == ":8\n");
}

#if (defined(CLI11_ENABLE_EXTRA_VALIDATORS) && CLI11_ENABLE_EXTRA_VALIDATORS == 1) ||                                  \
    (!defined(CLI11_DISABLE_EXTRA_VALIDATORS) || CLI11_DISABLE_EXTRA_VALIDATORS == 0)

TEST_CASE("Completion: a positional offers its values beside the subcommand names", "[completion]") {
    CLI::App app{"program"};
    app.set_help_flag("");
    std::string mode;
    app.add_option("mode", mode)->check(CLI::IsMember({"fast", "slow"}));
    app.add_subcommand("start", "");

    // A word with no dash on it can become either, and there is nothing in the word to say which, so both sources
    // apply. The names are produced first, which is the order a script that keeps it will show.
    CHECK(complete(app, {""}, "1") == "start\nfast\nslow\n:6\n");
    CHECK(complete(app, {"f"}, "1") == "fast\n:6\n");
    CHECK(complete(app, {"s"}, "1") == "start\nslow\n:6\n");
}

TEST_CASE("Completion: each positional takes its turn", "[completion]") {
    CLI::App app{"program"};
    app.set_help_flag("");
    std::string first;
    std::string second;
    app.add_option("first", first)->check(CLI::IsMember({"one"}));
    app.add_option("second", second)->check(CLI::IsMember({"two"}));

    CHECK(complete(app, {""}, "1") == "one\n:6\n");
    CHECK(complete(app, {"one", ""}, "2") == "two\n:6\n");
    // A word that no value of the positional matches still took its turn: the walk classifies, it does not validate
    CHECK(complete(app, {"zzz", ""}, "2") == "two\n:6\n");
    CHECK(complete(app, {"one", "two", ""}, "3") == ":2\n");
}

TEST_CASE("Completion: an option and its value do not take a positional's turn", "[completion]") {
    CLI::App app{"program"};
    app.set_help_flag("");
    std::string level;
    std::string first;
    app.add_option("--level,-l", level)->check(CLI::IsMember({"fast"}));
    app.add_flag("--verbose,-v", "");
    app.add_option("first", first)->check(CLI::IsMember({"one"}));

    // The word after the option name is its value, so the positional is still waiting for its first
    CHECK(complete(app, {"--level", "fast", ""}, "3") == "one\n:6\n");
    // and so are the shapes that carry the value inside the option's own word
    CHECK(complete(app, {"--level=fast", ""}, "2") == "one\n:6\n");
    CHECK(complete(app, {"-lfast", ""}, "2") == "one\n:6\n");
    // A flag takes no word at all, and neither does a name this app does not have
    CHECK(complete(app, {"--verbose", ""}, "2") == "one\n:6\n");
    CHECK(complete(app, {"--nope", ""}, "2") == "one\n:6\n");
}

TEST_CASE("Completion: an option takes as many words as its multiplicities say", "[completion]") {
    CLI::App app{"program"};
    app.set_help_flag("");
    std::pair<std::string, std::string> pair;
    app.add_option("--pair", pair)->check(CLI::IsMember({"one", "two"}));
    std::string first;
    app.add_option("first", first)->check(CLI::IsMember({"pos"}));

    // Two words go into one value, so the second of them is the option's turn and not the positional's
    CHECK(complete(app, {"--pair", ""}, "2") == "one\ntwo\n:6\n");
    CHECK(complete(app, {"--pair", "one", ""}, "3") == "one\ntwo\n:6\n");
    CHECK(complete(app, {"--pair", "one", "two", ""}, "4") == "pos\n:6\n");
    // A word the option still needs is taken as its value without being classified at all, so no other source applies:
    // `--pair one --nope` really does put the `--nope` in the pair. The directive is what says which source answered.
    CHECK(complete(app, {"--pair", "one", "-"}, "3") == ":6\n");
}

TEST_CASE("Completion: an option that takes any number of words keeps taking them", "[completion]") {
    CLI::App app{"program"};
    app.set_help_flag("");
    std::vector<std::string> files;
    app.add_option("--files", files)->check(CLI::IsMember({"a", "b"}));
    std::string first;
    app.add_option("first", first)->check(CLI::IsMember({"pos"}));
    app.add_subcommand("start", "");

    // Its first value is one it must have, so nothing else can be there
    CHECK(complete(app, {"--files", ""}, "2") == "a\nb\n:6\n");
    // Past that it goes on eating words a parse would not recognise, so the next one is both another of its values and
    // the name that would stop it
    CHECK(complete(app, {"--files", "a", ""}, "3") == "start\na\nb\n:6\n");
    CHECK(complete(app, {"--files", "a", "b", ""}, "4") == "start\na\nb\n:6\n");
    // A value inside the option's own word does not fill it up either
    CHECK(complete(app, {"--files=a", ""}, "2") == "start\na\nb\n:6\n");
    // A dash-shaped word is one it would stop at, so that word is an option name again
    CHECK(complete(app, {"--files", "a", "-"}, "3") == "--files\n:2\n");
    // and a word that names a subcommand moves the walk there, as the parse would
    CHECK(complete(app, {"--files", "a", "start", ""}, "4") == ":2\n");
}

TEST_CASE("Completion: a required positional keeps a word back from a greedy option", "[completion]") {
    CLI::App app{"program"};
    app.set_help_flag("");
    std::vector<std::string> files;
    app.add_option("--files", files)->check(CLI::IsMember({"a", "b"}));
    std::string first;
    app.add_option("first", first)->check(CLI::IsMember({"pos"}))->required();

    // A parse stops eating while the words left are the ones a required positional has to have, so the last word of the
    // line is the positional's turn rather than another value
    CHECK(complete(app, {"--files", "a", ""}, "3") == "pos\n:6\n");
    // Once that positional has been given one, the option is free to eat to the end of the line again
    CHECK(complete(app, {"pos", "--files", "a", ""}, "4") == "a\nb\n:6\n");
}

TEST_CASE("Completion: the marker that ends a list of values is eaten with it", "[completion]") {
    CLI::App app{"program"};
    app.set_help_flag("");
    std::vector<std::string> files;
    app.add_option("--files", files)->check(CLI::IsMember({"a", "b"}));
    std::string first;
    app.add_option("first", first)->check(CLI::IsMember({"pos"}));
    app.add_subcommand("start", "");

    // A parse ends an unbounded list at the marker and consumes it, so positional-only mode is never entered and the
    // word after it is an ordinary one: `--files a -- start` really does run the subcommand. Verified.
    CHECK(complete(app, {"--files", "a", "--", ""}, "4") == "start\npos\n:6\n");
    // The minimum is not classified at all, so a marker inside it is a value rather than a marker, and the list it
    // would have ended goes on: `--files -- b` really does put both words in the list. Verified.
    CHECK(complete(app, {"--files", "--", ""}, "3") == "start\na\nb\n:6\n");
}

TEST_CASE("Completion: a subcommand fills its own positionals", "[completion]") {
    CLI::App app{"program"};
    app.set_help_flag("");
    std::string outer;
    std::string inner;
    app.add_option("outer", outer)->check(CLI::IsMember({"one"}));
    CLI::App *sub = app.add_subcommand("start", "");
    sub->add_option("inner", inner)->check(CLI::IsMember({"two"}));

    // Descending starts the count again, since each app fills its positionals from the words that reach it
    CHECK(complete(app, {"start", ""}, "2") == "two\n:6\n");
    // and the name that moved the walk is not one of the parent's positional values either
    CHECK(complete(app, {"one", "start", ""}, "3") == "two\n:6\n");
}

TEST_CASE("Completion: the positional after the end-of-options marker is still offered", "[completion]") {
    CLI::App app{"program"};
    app.set_help_flag("");
    std::string first;
    std::string second;
    app.add_option("first", first)->check(CLI::IsMember({"one"}));
    app.add_option("second", second)->check(CLI::IsMember({"two"}));
    app.add_subcommand("start", "");

    // The marker rules out the other two sources, not the positional; a positional value is the only thing left a
    // word after it can be
    CHECK(complete(app, {"--", ""}, "2") == "one\n:6\n");
    // Every word past the marker takes a turn, including one that would otherwise have moved the walk
    CHECK(complete(app, {"--", "one", ""}, "3") == "two\n:6\n");
    CHECK(complete(app, {"--", "start", ""}, "3") == "two\n:6\n");
    // and once they are full, nothing valid can go there, not even a path
    CHECK(complete(app, {"--", "one", "two", ""}, "4") == ":2\n");
}

TEST_CASE("Completion: a positional with nothing left to consume claims no slot", "[completion]") {
    CLI::App app{"program"};
    app.set_help_flag("");
    // expected(0) gives this positional no capacity, so it claims no slot
    app.add_option("zero", "")->expected(0);
    app.add_option("first", "")->check(CLI::IsMember({"one"}));

    CHECK(complete(app, {"--", ""}, "2") == "one\n:6\n");
}

#endif

TEST_CASE("Completion: the subcommand terminator hands the walk back to the parent", "[completion]") {
    CLI::App app{"program"};
    app.set_help_flag("");
    app.add_flag("--outer", "");
    CLI::App *sub = app.add_subcommand("start", "");
    sub->add_flag("--inner", "");
    CLI::App *nested = sub->add_subcommand("now", "");
    nested->add_flag("--deep", "");

    // Inside the subcommand its own options are the candidates
    CHECK(complete(app, {"start", "--"}, "2") == "--inner\n:2\n");
    // `++` closes it, so the root's options are back and the subcommand's are gone
    CHECK(complete(app, {"start", "++", "--"}, "3") == "--outer\n:2\n");
    // One terminator closes one level
    CHECK(complete(app, {"start", "now", "++", "--"}, "4") == "--inner\n:2\n");
    CHECK(complete(app, {"start", "now", "++", "++", "--"}, "5") == "--outer\n:2\n");
    // Past the root there is nothing left to close, so the extra one is an ordinary word
    CHECK(complete(app, {"start", "++", "++", "--"}, "4") == "--outer\n:2\n");
    // _recognize only reads it as a terminator inside a subcommand, and so does the walk
    CHECK(complete(app, {"++", "--"}, "2") == "--outer\n:2\n");
    // The marker outranks it: after `--` there are no more terminators either
    std::string value;
    sub->add_option("value", value);
    CHECK(complete(app, {"start", "--", "++", "--outer"}, "4") == ":2\n");
}

TEST_CASE("Completion: degenerate cursor positions reply with nothing", "[completion]") {
    CLI::App app{"program"};
    app.add_subcommand("start", "");

    // The cursor is on the program name itself
    CHECK(complete(app, {""}, "0") == ":2\n");
    // Past the last word the shell sent, and a bad index, are both malformed rather than empty
    CHECK(complete(app, {"start", ""}, "9") == ":2\n");
    CHECK(complete(app, {""}, "not-a-number") == ":2\n");
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

TEST_CASE("Completion: the prefix line comes before the directive line", "[completion]") {
    CLI::CompletionReply reply;
    reply.results.push_back(CLI::CompletionResult{"--file=a", ""});
    reply.prefix = "--file=";

    // The directive line stays last, since that is what tells a script the reply is whole rather than truncated
    CHECK(CLI::format_completion_reply(reply) == "--file=a\n:prefix=--file=\n:0\n");

    // and the prefix is escaped like any other field: an option name is user text too
    reply.results.clear();
    reply.prefix = "--od\td=";
    CHECK(CLI::format_completion_reply(reply) == ":prefix=--od\\td=\n:0\n");
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

    set_request("1", {"sta"});
    bool thrown = false;
    try {
        app.parse(std::vector<std::string>{guard_argument});
    } catch(const CLI::CallForCompletion &) {
        thrown = true;
        // A completion callback may run another CLI11 binary, which must not complete itself
        CHECK(CLI::detail::get_environment_value(complete_var).empty());
        CHECK(CLI::detail::get_environment_value(index_var).empty());
        CHECK(CLI::detail::get_environment_value(proto_var).empty());
        CHECK(CLI::detail::get_environment_value(words_var).empty());
        CHECK(CLI::detail::get_environment_value("CLI11_COMPLETE_WORD_1").empty());
    }
    clear_request();
    CHECK(thrown);
}

TEST_CASE("Completion: a request from another protocol version gets no candidates", "[completion]") {
    CLI::App app{"program"};
    app.add_subcommand("start", "");

    // A script generated against a different format would misread whatever came back, so the Error directive goes
    // out alone: no completions beats wrong completions
    for(const std::string &proto : {std::string("2"), std::string("bash"), std::string()}) {
        set_request("1", {"sta"});
        if(proto.empty())
            CLI::detail::unset_environment_value(proto_var);
        else
            put_env(proto_var, proto);

        const std::string reply = parse_reply(app);
        clear_request();
        CHECK(reply == ":1\n");
    }

    // and the version the scripts export is the one that is accepted
    CHECK(complete(app, {"sta"}, "1") == "start\n:2\n");
}

TEST_CASE("Completion: the script asks for the version it was generated against", "[completion]") {
    CLI::App app{"program", "myprog"};
    const std::string script = app.get_completion_script("bash");

    CHECK(script.find("CLI11_COMPLETE_PROTO=" + std::to_string(CLI::CLI11_COMPLETE_PROTO_VERSION)) !=
          std::string::npos);
    CHECK(script.find("@PROTO@") == std::string::npos);
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

    set_request("1", {"sta"});
    std::vector<std::string> args{"start"};
    CHECK_NOTHROW(app.parse(args));
    clear_request();
    CLI::detail::unset_environment_value("CLI11_COMPLETE_WORD_1");
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
    set_request("1", {"sta"});
    std::vector<std::string> args{"start"};
    CHECK_NOTHROW(app.parse(args));
    clear_request();
    CLI::detail::unset_environment_value("CLI11_COMPLETE_WORD_1");

    // The version travels under the configured name too, or the request would be refused as unversioned
    put_env("MYPROG_COMPLETE", "bash");
    put_env("MYPROG_COMPLETE_INDEX", "1");
    put_env("MYPROG_COMPLETE_PROTO", "1");
    set_words("MYPROG_COMPLETE", {"sta"});
    CHECK(parse_reply(app) == "start\n:2\n");
    CLI::detail::unset_environment_value("MYPROG_COMPLETE");
    CLI::detail::unset_environment_value("MYPROG_COMPLETE_INDEX");
    CLI::detail::unset_environment_value("MYPROG_COMPLETE_PROTO");
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

    try {
        std::vector<std::string> args{"bash", "--completion"};  // reversed, as parse expects
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
    CHECK(script.find("\ncomplete -F _cli11_complete_my_prog_exe 'my-prog.exe'\n") != std::string::npos);

    CHECK(script.find("@FUNCTION@") == std::string::npos);
    CHECK(script.find("@PROGRAM@") == std::string::npos);
    CHECK(script.find("@ENV@") == std::string::npos);

    CHECK_THROWS_AS(app.get_completion_script("zsh"), CLI::ValidationError);
}

TEST_CASE("Completion: the script asks for the configured environment variable", "[completion]") {
    CLI::App app{"program", "myprog"};
    app.set_completion_env_var("MYPROG_COMPLETE");
    const std::string script = app.get_completion_script("bash");

    CHECK(script.find("export MYPROG_COMPLETE=bash MYPROG_COMPLETE_INDEX=") != std::string::npos);
    CHECK(script.find("export \"MYPROG_COMPLETE_WORD_${ii}=") != std::string::npos);
    CHECK(script.find("CLI11_COMPLETE") == std::string::npos);
}

TEST_CASE("Completion: the script function name survives an awkward program name", "[completion]") {
    CHECK(CLI::detail::completion_script_identifier("my-prog.exe") == "my_prog_exe");
}

#if (defined(CLI11_ENABLE_EXTRA_VALIDATORS) && CLI11_ENABLE_EXTRA_VALIDATORS == 1) ||                                  \
    (!defined(CLI11_DISABLE_EXTRA_VALIDATORS) || CLI11_DISABLE_EXTRA_VALIDATORS == 0)
TEST_CASE("Completion: a marker after an option that is full starts positional-only mode", "[completion]") {
    CLI::App app{"program"};
    app.set_help_flag("");
    app.add_flag("--verbose");
    std::string level;
    app.add_option("--level", level);
    std::string first;
    app.add_option("first", first)->check(CLI::IsMember({"--vvv", "other"}));

    // `--level` takes one value, so a parse does not consume the marker with it
    CHECK(complete(app, {"--level", "fast", "--", "--v"}, "4") == "--vvv\n:6\n");
    CHECK(complete(app, {"--level=fast", "--", "--v"}, "3") == "--vvv\n:6\n");
}

#endif

TEST_CASE("Completion: the script is registered for the basename of the program", "[completion]") {
    CLI::App app{"program", "/usr/local/bin/my-prog"};
    const std::string script = app.get_completion_script("bash");
    CHECK(script.find("\ncomplete -F _cli11_complete_my_prog 'my-prog'\n") != std::string::npos);

    CLI::App windows{"program", "C:\\tools\\my-prog.exe"};
    CHECK(windows.get_completion_script("bash").find(" 'my-prog.exe'\n") != std::string::npos);

    CLI::App quoted{"program", "it's"};
    CHECK(quoted.get_completion_script("bash").find(" 'it'\\''s'\n") != std::string::npos);

    CLI::App nameless{"program"};
    CHECK_THROWS_AS(nameless.get_completion_script("bash"), CLI::ValidationError);
    CLI::App directory{"program", "bin/"};
    CHECK_THROWS_AS(directory.get_completion_script("bash"), CLI::ValidationError);
}

#if (defined(CLI11_ENABLE_EXTRA_VALIDATORS) && CLI11_ENABLE_EXTRA_VALIDATORS == 1) ||                                  \
    (!defined(CLI11_DISABLE_EXTRA_VALIDATORS) || CLI11_DISABLE_EXTRA_VALIDATORS == 0)
TEST_CASE("Completion: what is hidden from help is not offered", "[completion]") {
    CLI::App app{"program"};
    app.set_help_flag("");
    app.add_flag("--shown");
    std::string secret;
    app.add_option("--secret", secret)->group("")->check(CLI::IsMember({"a", "b"}));
    app.add_subcommand("visible", "");
    auto *internal = app.add_subcommand("internal", "");
    // Before the group is set, which a later subcommand would inherit
    internal->add_subcommand("inner", "");
    internal->group("");

    CHECK(complete(app, {"--"}, "1") == "--shown\n:2\n");
    CHECK(complete(app, {""}, "1") == "visible\n:2\n");
    // A hidden name still parses, so what follows it is still completed
    CHECK(complete(app, {"--secret", ""}, "2") == "a\nb\n:6\n");
    CHECK(complete(app, {"internal", ""}, "2") == "inner\n:2\n");
}

TEST_CASE("Completion: a negative number is a positional value", "[completion]") {
    CLI::App app{"program"};
    app.set_help_flag("");
    int first{0};
    app.add_option("first", first);
    std::string second;
    app.add_option("second", second)->check(CLI::IsMember({"one", "two"}));

    CHECK(complete(app, {"5", ""}, "2") == "one\ntwo\n:6\n");
    CHECK(complete(app, {"-5", ""}, "2") == "one\ntwo\n:6\n");
    CHECK(complete(app, {"-.5", ""}, "2") == "one\ntwo\n:6\n");

    // Where the digit is an option name, the word is that option
    app.add_flag("-5");
    CHECK(complete(app, {"-5", "x", ""}, "3") == "one\ntwo\n:6\n");
}

TEST_CASE("Completion: a value is matched through the filters of its validator", "[completion]") {
    CLI::App app{"program"};
    app.set_help_flag("");
    std::string level;
    app.add_option("--level", level)->check(CLI::IsMember({"Fast", "Slow_Down"}, CLI::ignore_case));
    std::string mode;
    app.add_option("--mode", mode)
        ->check(CLI::IsMember({"Fast", "Slow_Down"}, CLI::ignore_case, CLI::ignore_underscore));
    std::string exact;
    app.add_option("--exact", exact)->check(CLI::IsMember({"Fast", "Slow"}));
    int number{0};
    app.add_option("--number", number)
        ->transform(CLI::CheckedTransformer(std::map<std::string, int>{{"One", 1}, {"Two", 2}}, CLI::ignore_case));
    std::string both;
    app.add_option("--both", both)
        ->check(CLI::IsMember({"Fast", "Slow"}, CLI::ignore_case) & CLI::IsMember({"Fast", "Other"}));

    CHECK(complete(app, {"--level", "f"}, "2") == "Fast\n:6\n");
    CHECK(complete(app, {"--level", "slowd"}, "2") == ":6\n");
    CHECK(complete(app, {"--mode", "slowd"}, "2") == "Slow_Down\n:6\n");
    CHECK(complete(app, {"--level=s"}, "1") == "--level=Slow_Down\n:prefix=--level=\n:6\n");
    CHECK(complete(app, {"--exact", "f"}, "2") == ":6\n");
    CHECK(complete(app, {"--number", "t"}, "2") == "Two\n:6\n");
    CHECK(complete(app, {"--both", "f"}, "2") == "Fast\n:6\n");
}

TEST_CASE("Completion: text that is not an item of the set skips its filter", "[completion]") {
    CLI::App app{"program"};
    app.set_help_flag("");
    int number{0};
    app.add_option("--number", number)->check(CLI::IsMember({1, 20, 21}, [](int value) { return value; }));

    CHECK(complete(app, {"--number", "2"}, "2") == "20\n21\n:6\n");
    // An empty or non-numeric prefix cannot be converted, so it is compared unfiltered
    CHECK(complete(app, {"--number", ""}, "2") == "1\n20\n21\n:6\n");
    CHECK(complete(app, {"--number", "x"}, "2") == ":6\n");
}

TEST_CASE("Completion: a fallthrough subcommand offers the options of its parent", "[completion]") {
    CLI::App app{"program"};
    app.set_help_flag("");
    app.allow_windows_style_options();
    std::string level;
    app.add_option("--level", level)->check(CLI::IsMember({"fast", "slow"}));
    app.add_flag("--shared");
    auto *sub = app.add_subcommand("sub", "")->fallthrough();
    sub->allow_windows_style_options();
    sub->add_flag("--own");
    // The subcommand's own option hides the parent's one of the same name
    sub->add_flag("--shared");
    auto *plain = app.add_subcommand("plain", "");
    plain->add_flag("--own");

    CHECK(complete(app, {"sub", "--"}, "2") == "--own\n--shared\n--level\n:2\n");
    CHECK(complete(app, {"sub", "--level", ""}, "3") == "fast\nslow\n:6\n");
    CHECK(complete(app, {"sub", "/level", ""}, "3") == "fast\nslow\n:6\n");
    CHECK(complete(app, {"plain", "--"}, "2") == "--own\n:2\n");
}

namespace {

/// The reply for the last of these words, which are the arguments of the line without the program name
std::string ask(const CLI::App &app, const std::vector<std::string> &words) {
    return CLI::format_completion_reply(app.get_completions(words, words.size() - 1));
}

}  // namespace

TEST_CASE("Completion: removing the script flag forgets it", "[completion]") {
    CLI::App app{"program", "myprog"};
    app.remove_option(app.set_completion_flag());
    CHECK(app.get_completion_ptr() == nullptr);

    // An option group owns what is moved into it, and the app would be left with a pointer it cannot track
    auto *group = app.add_option_group("Group");
    CHECK_THROWS_AS(group->add_option(app.set_completion_flag()), CLI::OptionAlreadyAdded);
    CHECK(app.get_completion_ptr() != nullptr);
}

TEST_CASE("Completion: the activation variable has to be a shell identifier", "[completion]") {
    CLI::App app{"program", "myprog"};

    // The name is written into a shell assignment, where anything else is a command or worse
    CHECK_THROWS_AS(app.set_completion_env_var("MY-PROG_COMPLETE"), CLI::IncorrectConstruction);
    CHECK_THROWS_AS(app.set_completion_env_var("X=1; touch /tmp/x; Y"), CLI::IncorrectConstruction);
    CHECK_THROWS_AS(app.set_completion_env_var("1COMPLETE"), CLI::IncorrectConstruction);
    CHECK_THROWS_AS(app.set_completion_env_var(""), CLI::IncorrectConstruction);
    CHECK(app.get_completion_env_var() == "CLI11_COMPLETE");
    CHECK_NOTHROW(app.set_completion_env_var("_my_prog2_COMPLETE"));
}

TEST_CASE("Completion: a required positional takes a word that spells a subcommand", "[completion]") {
    CLI::App app{"program"};
    app.set_help_flag("");
    app.add_flag("--outer", "");
    std::string target;
    app.add_option("target", target)->required();
    CLI::App *start = app.add_subcommand("start", "");
    start->set_help_flag("");
    start->add_flag("--inner", "");

    // _parse_subcommand hands the word to a required positional that is still empty
    CHECK(ask(app, {"start", "--"}) == "--outer\n:2\n");
    CHECK(ask(app, {"x", "start", "--"}) == "--inner\n:2\n");
    // and while it is empty, the name is not something the word can become
    CHECK(ask(app, {"st"}) == ":0\n");
    CHECK(ask(app, {"x", "st"}) == "start\n:2\n");
}

TEST_CASE("Completion: a marker in a subcommand with no positionals closes it", "[completion]") {
    CLI::App app{"program"};
    app.set_help_flag("");
    app.add_flag("--root-flag", "");
    CLI::App *child = app.add_subcommand("child", "");
    child->set_help_flag("");

    // _parse_single returns to the parent, whose own words are classified as before
    CHECK(ask(app, {"child", "--", "--root-"}) == "--root-flag\n:2\n");

    // With a positional left to fill, the marker keeps its meaning and the subcommand stays open
    std::string value;
    child->add_option("value", value);
    CHECK(ask(app, {"child", "--", "--root-"}) == ":0\n");
}

TEST_CASE("Completion: a subcommand found through a parent closes the one it was typed in", "[completion]") {
    CLI::App app{"program"};
    app.set_help_flag("");
    CLI::App *start = app.add_subcommand("start", "");
    start->set_help_flag("");
    start->add_flag("--inner", "");
    CLI::App *stop = app.add_subcommand("stop", "");
    stop->set_help_flag("");
    stop->add_flag("--halt", "");

    // subcommand_fallthrough is on by default, so `stop` here is the root's
    CHECK(ask(app, {"start", "stop", "--"}) == "--halt\n:2\n");
    start->subcommand_fallthrough(false);
    CHECK(ask(app, {"start", "stop", "--"}) == "--inner\n:2\n");
}

TEST_CASE("Completion: an empty argument before the cursor is a value", "[completion]") {
    CLI::App app{"program"};
    app.set_help_flag("");
    std::string first;
    std::string second;
    app.add_option("first", first)->check(CLI::IsMember({"a1", "a2"}));
    app.add_option("second", second)->check(CLI::IsMember({"b1", "b2"}));

    CHECK(ask(app, {""}) == "a1\na2\n:6\n");
    CHECK(ask(app, {"", ""}) == "b1\nb2\n:6\n");
}

TEST_CASE("Completion: an option with a positional name is completed in both places", "[completion]") {
    CLI::App app{"program"};
    app.set_help_flag("");
    std::string input;
    app.add_option("input,--input", input)->check(CLI::IsMember({"abc"}));
    std::string other;
    app.add_option("other", other)->check(CLI::IsMember({"xyz"}));

    CHECK(ask(app, {""}) == "abc\n:6\n");
    CHECK(ask(app, {"--input", ""}) == "abc\n:6\n");
    CHECK(ask(app, {"--in"}) == "--input\n:2\n");
    // A value given through either spelling fills it
    CHECK(ask(app, {"abc", ""}) == "xyz\n:6\n");
    CHECK(ask(app, {"--input", "abc", ""}) == "xyz\n:6\n");
    CHECK(ask(app, {"--input=abc", ""}) == "xyz\n:6\n");
}

TEST_CASE("Completion: the terminator restores the command that was left", "[completion]") {
    CLI::App app{"program"};
    app.set_help_flag("");
    app.add_flag("--outer", "");
    std::string first;
    std::string second;
    app.add_option("first", first)->check(CLI::IsMember({"a1"}));
    app.add_option("second", second)->check(CLI::IsMember({"b2"}));
    CLI::App *group = app.add_option_group("Group");
    CLI::App *sub = group->add_subcommand("sub", "");
    sub->set_help_flag("");
    sub->add_flag("--inner", "");

    // `first` is still filled when the subcommand closes
    CHECK(ask(app, {"a1", "sub", "++", ""}) == "sub\nb2\n:6\n");
    // and the walk is back in the command, not in the group the subcommand belongs to
    CHECK(ask(app, {"a1", "sub", "++", "--"}) == "--outer\n:2\n");
}

TEST_CASE("Completion: a delimited value counts for the items it holds", "[completion]") {
    CLI::App app{"program"};
    app.set_help_flag("");
    app.add_flag("--other", "");
    std::pair<int, int> point;
    app.add_option("--point", point)->delimiter(',');

    CHECK(ask(app, {"--point", "1,2", "--"}) == "--other\n--point\n:2\n");
    CHECK(ask(app, {"--point=1,2", "--"}) == "--other\n--point\n:2\n");
    // One item is half of it, so the next word is the other half whatever it looks like
    CHECK(ask(app, {"--point", "1", "--"}) == ":0\n");
}

TEST_CASE("Completion: a validator applies to the value index it was given", "[completion]") {
    CLI::App app{"program"};
    app.set_help_flag("");
    std::pair<std::string, std::string> route;
    app.add_option("--route", route)
        ->check(CLI::IsMember({"north", "south"}).application_index(0))
        ->check(CLI::IsMember({"fast", "slow"}).application_index(1));

    CHECK(ask(app, {"--route", ""}) == "north\nsouth\n:6\n");
    CHECK(ask(app, {"--route", "north", ""}) == "fast\nslow\n:6\n");
    CHECK(ask(app, {"--route=n"}) == "--route=north\n:prefix=--route=\n:6\n");

    std::vector<std::string> choices{"fast", "slow"};
    CHECK(app.get_option("--route")->get_completion_choices(1) == choices);
}

TEST_CASE("Completion: a set of numbers is offered in the spelling the parser takes", "[completion]") {
    CLI::App app{"program"};
    app.set_help_flag("");
    std::uint8_t byte{0};
    app.add_option("--byte", byte)->check(CLI::IsMember(std::vector<std::uint8_t>{65, 66}));

    CHECK(ask(app, {"--byte", ""}) == "65\n66\n:6\n");
    CHECK_NOTHROW(app.parse(std::vector<std::string>{"65", "--byte"}));
    CHECK(byte == 65);
}

TEST_CASE("Completion: a set with nothing in it is not an unknown value", "[completion]") {
    CLI::App app{"program"};
    app.set_help_flag("");
    std::string value;
    // The set is live, and the program has not filled it in
    auto live = std::make_shared<std::vector<std::string>>();
    app.add_option("--live", value)->check(CLI::IsMember(live));
    app.add_option("--disjoint", value)->check(CLI::IsMember({"a"}))->check(CLI::IsMember({"b"}));
    app.add_option("--file", value)->check(CLI::ExistingFile)->check(CLI::IsMember({"a"}) & CLI::IsMember({"b"}));

    // Nothing is accepted, so a file name is not either
    CHECK(ask(app, {"--live", ""}) == ":6\n");
    CHECK(ask(app, {"--disjoint", ""}) == ":6\n");
    CHECK(ask(app, {"--file", ""}) == ":6\n");
    live->push_back("now");
    CHECK(ask(app, {"--live", ""}) == "now\n:6\n");
}

TEST_CASE("Completion: sets are intersected through their filters", "[completion]") {
    CLI::App app{"program"};
    app.set_help_flag("");
    std::string value;
    app.add_option("--speed", value)
        ->check(CLI::IsMember({"FAST", "Slow"}, CLI::ignore_case) & CLI::IsMember({"fast", "other"}, CLI::ignore_case));
    app.add_option("--exact", value)->check(CLI::IsMember({"FAST"}) & CLI::IsMember({"fast"}));
    app.add_option("--nested", value)
        ->check((CLI::IsMember({"FAST", "Slow"}, CLI::ignore_case) | CLI::IsMember({"mid"})) &
                CLI::IsMember({"fast", "MID"}, CLI::ignore_case));

    // A combination on the right decides the way its own sides do
    app.add_option("--either", value)
        ->check(CLI::IsMember({"fast", "MID", "other"}, CLI::ignore_case) &
                (CLI::IsMember({"FAST", "Slow"}, CLI::ignore_case) | CLI::IsMember({"MID"})));
    app.add_option("--all", value)
        ->check(CLI::IsMember({"fast", "MID"}, CLI::ignore_case) &
                (CLI::IsMember({"FAST", "mid"}, CLI::ignore_case) & CLI::IsMember({"Fast"}, CLI::ignore_case)));

    CHECK(ask(app, {"--speed", ""}) == "FAST\n:6\n");
    CHECK(ask(app, {"--exact", ""}) == ":6\n");
    CHECK(ask(app, {"--nested", ""}) == "FAST\nmid\n:6\n");
    CHECK(ask(app, {"--either", ""}) == "fast\nMID\n:6\n");
    CHECK(ask(app, {"--all", ""}) == "fast\n:6\n");
    // A Validator that lists nothing is taken to accept anything
    CHECK(CLI::detail::completion_meta_accepts(*CLI::detail::completion_hint_meta(CLI::CompletionHint::File), "x"));
    CHECK_NOTHROW(app.parse(std::vector<std::string>{"FAST", "--speed"}));
}

TEST_CASE("Completion: a subcommand disabled by default is not offered", "[completion]") {
    CLI::App app{"program"};
    app.set_help_flag("");
    app.add_flag("--outer", "");
    CLI::App *off = app.add_subcommand("off", "")->disabled_by_default();
    off->set_help_flag("");
    off->add_flag("--inner", "");
    // disabled now, and enabled when a parse starts
    CLI::App *on = app.add_subcommand("on", "")->disabled()->enabled_by_default();
    on->set_help_flag("");
    on->add_flag("--inner", "");
    CLI::App *group = app.add_option_group("Group")->disabled_by_default();
    group->add_flag("--grouped", "");

    CHECK(ask(app, {"o"}) == "on\n:2\n");
    CHECK(ask(app, {"off", "--"}) == "--outer\n:2\n");
    CHECK(ask(app, {"on", "--"}) == "--inner\n:2\n");
    // The app is not changed by being asked
    CHECK_FALSE(off->get_disabled());
    CHECK(on->get_disabled());
}

TEST_CASE("Completion: an option in a disabled group has no values", "[completion]") {
    CLI::App app{"program"};
    app.set_help_flag("");
    app.add_flag("-v", "");
    CLI::App *group = app.add_option_group("Group");
    std::string mode;
    group->add_option("--mode,-m", mode)->check(CLI::IsMember({"fast", "slow"}));
    std::string positional;
    app.add_option("positional", positional)->check(CLI::IsMember({"pos"}));

    CHECK(ask(app, {"--mode", ""}) == "fast\nslow\n:6\n");
    CHECK(ask(app, {"-vm", ""}) == "fast\nslow\n:6\n");
    group->disabled();
    // A parse does not find the option, so the next word is the positional's
    CHECK(ask(app, {"--mode", ""}) == "pos\n:6\n");
    CHECK(ask(app, {"-vm", ""}) == "pos\n:6\n");
    CHECK(ask(app, {"--mode=f"}) == ":2\n");
    CHECK(ask(app, {"-mf"}) == ":2\n");
}

TEST_CASE("Completion: a word past the marker can still close a subcommand", "[completion]") {
    CLI::App app{"program"};
    app.set_help_flag("");
    CLI::App *stop = app.add_subcommand("stop", "");
    stop->set_help_flag("");
    stop->add_flag("--halt", "");
    CLI::App *plain = app.add_subcommand("plain", "");
    plain->set_help_flag("");
    std::string value;
    plain->add_option("value", value);
    CLI::App *through = app.add_subcommand("through", "")->fallthrough();
    through->set_help_flag("");
    through->add_option("value", value);

    // Once the positional is full, _parse_positional leaves a name the parent has to the parent
    CHECK(ask(app, {"plain", "--", "x", "stop", "--"}) == "--halt\n:2\n");
    // A fallthrough subcommand has the parent start it, and with a parse_complete_callback it closes first
    CHECK(ask(app, {"through", "--", "x", "stop", "--"}) == "--halt\n:2\n");
    through->parse_complete_callback([]() {});
    CHECK(ask(app, {"through", "--", "x", "stop", "--"}) == "--halt\n:2\n");
}

TEST_CASE("Completion: a fallthrough subcommand fills the positionals of its parent", "[completion]") {
    CLI::App app{"program"};
    app.set_help_flag("");
    std::string first;
    std::string second;
    app.add_option("first", first)->check(CLI::IsMember({"a1"}));
    app.add_option("second", second)->check(CLI::IsMember({"b2"}));
    CLI::App *sub = app.add_subcommand("sub", "")->fallthrough();
    sub->set_help_flag("");

    CHECK(ask(app, {"sub", ""}) == "a1\n:6\n");
    CHECK(ask(app, {"sub", "a1", ""}) == "b2\n:6\n");
}

TEST_CASE("Completion: an abbreviation two subcommands share names neither", "[completion]") {
    CLI::App app{"program"};
    app.set_help_flag("");
    app.allow_subcommand_prefix_matching();
    app.add_flag("--outer", "");
    app.add_subcommand("start", "")->add_flag("--begin", "");
    app.add_subcommand("stop", "")->add_flag("--end", "");
    app.add_subcommand("run", "")->add_flag("--go", "");

    CHECK(ask(app, {"st", "--"}) == "--outer\n:2\n");
    CHECK(ask(app, {"ru", "--"}) == "--go\n:2\n");
}

TEST_CASE("Completion: a negative number at the cursor is a value", "[completion]") {
    CLI::App app{"program"};
    app.set_help_flag("");
    int number{0};
    app.add_option("number", number)->check(CLI::IsMember({-10, -11, 5}));

    CHECK(ask(app, {"-1"}) == "-10\n-11\n:6\n");
    // A lone dash is still the start of an option name
    CHECK(ask(app, {"-"}) == ":2\n");
    // and where the digit names an option, the word is that option
    app.add_flag("-1", "");
    CHECK(ask(app, {"-1"}) == "-1\n:2\n");
}

TEST_CASE("Completion: positionals_at_end stops options after a positional", "[completion]") {
    CLI::App app{"program"};
    app.set_help_flag("");
    app.positionals_at_end();
    std::string file;
    std::string level;
    app.add_option("file", file);
    app.add_option("--level", level);

    CHECK(ask(app, {"--"}) == "--level\n:2\n");
    // A parse reads each word after the first positional as a positional, and `file` is full
    CHECK(ask(app, {"file", "--"}) == ":2\n");
}

TEST_CASE("Completion: a prefix command offers nothing for the words it passes through", "[completion]") {
    CLI::App app{"program"};
    app.set_help_flag("");
    app.prefix_command();
    std::string file;
    app.add_option("--file", file);

    CHECK(ask(app, {"--"}) == "--file\n:2\n");
    CHECK(ask(app, {"foo", "--"}) == ":2\n");
    CHECK(ask(app, {"foo", ""}) == ":2\n");
    CHECK(ask(app, {"--unknown", "--"}) == ":2\n");
}

TEST_CASE("Completion: validate_positionals offers nothing after a positional", "[completion]") {
    CLI::App app{"program"};
    app.set_help_flag("");
    app.validate_positionals();
    std::string first;
    std::string second;
    app.add_option("first", first)->check(CLI::IsMember({"a1"}));
    app.add_option("second", second)->check(CLI::IsMember({"b2"}));

    CHECK(ask(app, {""}) == "a1\n:6\n");
    // A parse gives `b2` to `second`, which completion cannot know without a validator run
    CHECK(ask(app, {"b2", ""}) == ":2\n");
}

TEST_CASE("Completion: validate_optional_arguments offers nothing after an optional value", "[completion]") {
    CLI::App app{"program"};
    app.set_help_flag("");
    app.validate_optional_arguments();
    std::vector<std::string> tags;
    app.add_option("--tag", tags)->check(CLI::IsMember({"x"}));
    std::string rest;
    app.add_option("rest", rest)->check(CLI::IsMember({"pos"}));

    CHECK(ask(app, {"--tag", ""}) == "x\n:6\n");
    // A parse ends the list at `other`, which completion cannot know without a validator run
    CHECK(ask(app, {"--tag", "x", "other", ""}) == ":2\n");
}

TEST_CASE("Completion: a subcommand limit that is reached stops subcommand names", "[completion]") {
    CLI::App app{"program"};
    app.set_help_flag("");
    CLI::App *start = app.add_subcommand("start", "");
    start->set_help_flag("");
    start->add_flag("--inner", "");
    CLI::App *stop = app.add_subcommand("stop", "");
    stop->set_help_flag("");
    stop->add_flag("--halt", "");
    app.require_subcommand(0, 1);

    CHECK(ask(app, {""}) == "start\nstop\n:2\n");
    // A parse reads `stop` as an extra word of `start`
    CHECK(ask(app, {"start", "stop", "--"}) == "--inner\n:2\n");

    // The names of a command that is full are not candidates
    start->add_subcommand("deep", "");
    start->require_subcommand(0, 1);
    CHECK(ask(app, {"start", ""}) == "deep\n:2\n");
    CHECK(ask(app, {"start", "deep", "++", ""}) == ":2\n");
}

TEST_CASE("Completion: a validator index counts each use of the option", "[completion]") {
    CLI::App app{"program"};
    app.set_help_flag("");
    std::vector<std::string> tags;
    app.add_option("--tag,-t", tags)->check(CLI::IsMember({"x", "y"}).application_index(1));

    CHECK(ask(app, {"--tag", "a", ""}) == "x\ny\n:6\n");
    CHECK(ask(app, {"--tag", "a", "--tag", ""}) == "x\ny\n:6\n");
    CHECK(ask(app, {"--tag", "a", "--tag=x"}) == "--tag=x\n:prefix=--tag=\n:6\n");
    CHECK(ask(app, {"--tag", "a", "-tx"}) == "-tx\n:prefix=-t\n:6\n");
    CHECK(ask(app, {"--tag=a", "--tag", "b", "--tag", ""}) == ":0\n");
}

TEST_CASE("Completion: a set or a path offers the set and the path", "[completion]") {
    CLI::App app{"program"};
    app.set_help_flag("");
    std::string value;
    app.add_option("--in", value)->check(CLI::IsMember({"stdin"}) | CLI::ExistingFile);
    app.add_option("--out", value)->check(CLI::ExistingDirectory | CLI::IsMember({"stdout"}));
    app.add_option("--path", value)->check(CLI::ExistingDirectory | CLI::ExistingFile);
    app.add_option("--both", value)->check(CLI::ExistingFile & CLI::IsMember({"listed"}));

    // KeepOrder with FilterFileExt: the shell adds file names to the set
    CHECK(ask(app, {"--in", ""}) == "stdin\n:12\n");
    CHECK(ask(app, {"--in", "/et"}) == ":12\n");
    CHECK(ask(app, {"--out", ""}) == "stdout\n:20\n");
    CHECK(ask(app, {"--path", ""}) == ":24\n");
    // Each side of an `&` must accept the value, so the set is the full answer
    CHECK(ask(app, {"--both", ""}) == "listed\n:6\n");
}

TEST_CASE("Completion: a long name behind one dash is read as a parse reads it", "[completion]") {
    CLI::App app{"program"};
    app.set_help_flag("");
    app.allow_non_standard_option_names();
    std::string level;
    app.add_option("-level", level)->check(CLI::IsMember({"fast", "slow"}));
    app.add_flag("-v", "");
    app.add_subcommand("start", "");

    CHECK(ask(app, {"-le"}) == "-level\n:2\n");
    CHECK(ask(app, {"-level", ""}) == "fast\nslow\n:6\n");
    CHECK(ask(app, {"-level", "fast", ""}) == "start\n:2\n");
    CHECK(ask(app, {"-level=f"}) == "-level=fast\n:prefix=-level=\n:6\n");
    CHECK(ask(app, {"-vx"}) == ":2\n");
}

TEST_CASE("Completion: a word count that is too large is not a usable request", "[completion]") {
    CLI::App app{"program"};
    app.add_subcommand("start", "");

    set_request("1", {"sta"});
    put_env(words_var, "99999999999");
    const std::string reply = parse_reply(app);
    clear_request();
    CLI::detail::unset_environment_value("CLI11_COMPLETE_WORD_1");
    CHECK(reply == ":1\n");
}

#if CLI11_HAS_FILESYSTEM && !defined(_WIN32)
TEST_CASE("Completion: a set of paths still builds", "[completion]") {
    CLI::App app{"program"};
    app.set_help_flag("");
    std::filesystem::path path;
    app.add_option("--path", path)->check(CLI::IsMember(std::vector<std::filesystem::path>{"a", "b"}));

    CHECK(ask(app, {"--path", ""}) == "a\nb\n:6\n");
}
#endif
#endif
