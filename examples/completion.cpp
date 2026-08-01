// Copyright (c) 2017-2026, University of Cincinnati, developed by Henry Schreiner
// under NSF AWARD 1414736 and by the respective contributors.
// All rights reserved.
//
// SPDX-License-Identifier: BSD-3-Clause

#include <CLI/CLI.hpp>
#include <iostream>
#include <string>

// Run `source <(completion --completion bash)` and then press TAB after `completion `.

int main(int argc, char **argv) {

    CLI::App app{"A program that knows how to complete itself", "completion"};

    app.set_completion_flag();

    app.add_subcommand("start", "Get going");
    app.add_subcommand("stop", "Do you really want to stop?");

    auto *remote = app.add_subcommand("remote", "Work with remotes");
    remote->add_subcommand("add", "Add a remote");
    remote->add_subcommand("remove", "Drop a remote")->alias("rm");

    app.require_subcommand(0, 1);

    CLI11_PARSE(app, argc, argv);

    for(auto *sub : app.get_subcommands())
        std::cout << "Subcommand: " << sub->get_name() << '\n';

    return 0;
}
