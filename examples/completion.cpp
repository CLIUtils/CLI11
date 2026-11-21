// Copyright (c) 2017-2026, University of Cincinnati, developed by Henry Schreiner
// under NSF AWARD 1414736 and by the respective contributors.
// All rights reserved.
//
// SPDX-License-Identifier: BSD-3-Clause

#define CLI11_ENABLE_EXTRA_VALIDATORS 1
#include <CLI/CLI.hpp>
#include <iostream>
#include <string>
#include <vector>

// Run `eval "$(completion --completion bash)"` and then press TAB after `completion `.

int main(int argc, char **argv) {

    CLI::App app{"A program that knows how to complete itself", "completion"};

    app.set_completion_flag();

    app.add_flag("--verbose,-v", "Say more");

    std::string level;
    app.add_option("--level,-l", level, "How hard to try")->check(CLI::IsMember({"fast", "slow"}));

    // A value can hold a colon. bash splits words on `:` as it does on `=`.
    std::string image;
    app.add_option("--image,-i", image, "Which image to use")
        ->check(CLI::IsMember({"alpine:3.19", "alpine:3.20", "debian:12"}));

    // A group has no name on the command line, so completion offers these two as options of the app.
    auto *paths = app.add_option_group("Paths", "Where to find things");

    // The shell completes a path, because it knows the working directory.
    std::string config;
    paths->add_option("--config,-c", config, "Where the settings live")->check(CLI::ExistingFile);

    std::string workdir;
    paths->add_option("--workdir", workdir, "Where to run")->check(CLI::ExistingDirectory);

    // An option that takes many words offers its values for each word.
    std::vector<std::string> tags;
    app.add_option("--tag", tags, "What to label it")->check(CLI::IsMember({"alpha", "beta"}));

    // `completion -- --verbose` goes here: after the marker, a word that starts with a dash is a positional. The hint
    // is declared, not validated, so completion offers files and the program accepts any word.
    std::vector<std::string> files;
    app.add_option("files", files, "What to work on")->completion_hint(CLI::CompletionHint::File);

    app.add_subcommand("start", "Get going");
    app.add_subcommand("stop", "Do you really want to stop?");

    auto *remote = app.add_subcommand("remote", "Work with remotes");
    remote->add_flag("--force,-f", "Do it anyway");
    remote->add_subcommand("add", "Add a remote");
    remote->add_subcommand("remove", "Drop a remote")->alias("rm");
    // A description can hold any text, so the reply format escapes it and the script unescapes it.
    remote->add_subcommand("sync", "Push\tand pull");

    app.require_subcommand(0, 1);

    CLI11_PARSE(app, argc, argv);

    for(const std::string &tag : tags)
        std::cout << "Tag: " << tag << '\n';

    for(const std::string &file : files)
        std::cout << "File: " << file << '\n';

    for(auto *sub : app.get_subcommands())
        std::cout << "Subcommand: " << sub->get_name() << '\n';

    return 0;
}
