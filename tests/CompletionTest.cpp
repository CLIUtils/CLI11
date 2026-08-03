// Copyright (c) 2017-2026, University of Cincinnati, developed by Henry Schreiner
// under NSF AWARD 1414736 and by the respective contributors.
// All rights reserved.
//
// SPDX-License-Identifier: BSD-3-Clause

#include "app_helper.hpp"

#include <algorithm>
#include <map>
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
    // plausible-looking reply, the wrong one.
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
    // Either kind of path is what the shell offers unprompted, so saying so is the same as saying nothing
    CHECK(complete(app, {"--any", ""}, "2") == ":0\n");
    // A path that must not exist yet is still typed against the ones that do, and gets the same treatment
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
    CHECK(app.add_option("--either", path)->check(CLI::ExistingDirectory | CLI::ExistingFile)->get_completion_hint() ==
          CLI::CompletionHint::Dir);

    // A validator that is not applied does not describe what the option accepts
    CHECK(app.add_option("--off", path)->check(CLI::ExistingFile.active(false))->get_completion_hint() ==
          CLI::CompletionHint::None);
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

    // An empty word after a finished one is a token of its own, whatever the one before it ended with
    CHECK(complete(app, {"--file=", ""}, "2") == "value\n:2\n");
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

    // A flag has no value to complete, and neither has an unknown option. Both fall back to no candidates and the
    // default directive rather than to the subcommand that shares the word.
    CHECK(complete(app, {"--verbose=val"}, "1") == ":0\n");
    CHECK(complete(app, {"--nope=val"}, "1") == ":0\n");
}

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
    CHECK(complete(app, {"start", "--", "++", "--outer"}, "4") == ":2\n");
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

TEST_CASE("Completion: a request from another protocol version gets no candidates", "[completion]") {
    CLI::App app{"program"};
    app.add_subcommand("start", "");

    // A script generated against a different format would misread whatever came back, so the Error directive goes
    // out alone: no completions beats wrong completions
    for(const std::string &proto : {std::string("2"), std::string("bash"), std::string()}) {
        put_env(complete_var, "bash");
        put_env(index_var, "1");
        if(proto.empty())
            CLI::detail::unset_environment_value(proto_var);
        else
            put_env(proto_var, proto);

        std::vector<std::string> args{"sta"};
        std::string reply = "<not a completion request>";
        try {
            app.parse(args);
        } catch(const CLI::CallForCompletion &e) {
            reply = e.what();
        }
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

    // The version travels under the configured name too, or the request would be refused as unversioned
    put_env("MYPROG_COMPLETE", "bash");
    put_env("MYPROG_COMPLETE_INDEX", "1");
    put_env("MYPROG_COMPLETE_PROTO", "1");
    std::vector<std::string> again{"sta"};
    std::string reply = "<not a completion request>";
    try {
        app.parse(again);
    } catch(const CLI::CallForCompletion &e) {
        reply = e.what();
    }
    CHECK(reply == "start\n:2\n");
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
