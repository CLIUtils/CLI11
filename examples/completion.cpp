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

// Run `source <(completion --completion bash)` and then press TAB after `completion `.

int main(int argc, char **argv) {

    CLI::App app{"A program that knows how to complete itself", "completion"};

    app.set_completion_flag();

    app.add_flag("--verbose,-v", "Say more");

    std::string level;
    app.add_option("--level,-l", level, "How hard to try")->check(CLI::IsMember({"fast", "slow"}));

    // A value may hold a colon, which bash breaks words on just as it does an `=`
    std::string image;
    app.add_option("--image,-i", image, "Which image to use")
        ->check(CLI::IsMember({"alpine:3.19", "alpine:3.20", "debian:12"}));

    // A path is left to the shell, which is the one that knows the directory the line is being typed in
    std::string config;
    app.add_option("--config,-c", config, "Where the settings live")->check(CLI::ExistingFile);

    std::string workdir;
    app.add_option("--workdir", workdir, "Where to run")->check(CLI::ExistingDirectory);

    // Somewhere for `completion -- --verbose` to land: past the marker even a dash-shaped word is a positional. The
    // hint is declared rather than validated, so completion offers files while the program still takes any word.
    std::vector<std::string> files;
    app.add_option("files", files, "What to work on")->completion_hint(CLI::CompletionHint::File);

    app.add_subcommand("start", "Get going");
    app.add_subcommand("stop", "Do you really want to stop?");

    auto *remote = app.add_subcommand("remote", "Work with remotes");
    remote->add_flag("--force,-f", "Do it anyway");
    remote->add_subcommand("add", "Add a remote");
    remote->add_subcommand("remove", "Drop a remote")->alias("rm");
    // A description is arbitrary text, so the reply format has to escape it and the script has to put it back
    remote->add_subcommand("sync", "Push\tand pull");

    app.require_subcommand(0, 1);

    CLI11_PARSE(app, argc, argv);

    for(const std::string &file : files)
        std::cout << "File: " << file << '\n';

    for(auto *sub : app.get_subcommands())
        std::cout << "Subcommand: " << sub->get_name() << '\n';

    return 0;
}
