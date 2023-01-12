// Copyright (c) 2017-2023, University of Cincinnati, developed by Henry Schreiner
// under NSF AWARD 1414736 and by the respective contributors.
// All rights reserved.
//
// SPDX-License-Identifier: BSD-3-Clause

#include <CLI/CLI.hpp>
#include <cstring>

int main(int argc, char **argv) {
    if(argc != CLI::argc()) {
        return -1;
    }

    for(int i = 0; i < argc; i++) {
        if(std::strcmp(argv[i], CLI::argv()[i]) != 0) {
            return i + 1;
        }
    }

    return 0;
}
