// Copyright (c) 2024 scivision
// Copyright (c) 2025 University of Cincinnati, developed by Henry Schreiner
// under NSF AWARD 1414736 and by the respective contributors.
// All rights reserved.
//
// SPDX-License-Identifier: MIT

// modified from https://github.com/iTrooz/CppModules/blob/cli11 for use in CLI11 tests

module;

#include <CLI/CLI.hpp>

export module cmodule;

export void foo(CLI::App *app) {}

export int add(int a, int b) { return a + b; }

export int subtract(int a, int b) { return a - b; }
