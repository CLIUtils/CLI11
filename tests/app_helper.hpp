// Copyright (c) 2017-2021, University of Cincinnati, developed by Henry Schreiner
// under NSF AWARD 1414736 and by the respective contributors.
// All rights reserved.
//
// SPDX-License-Identifier: BSD-3-Clause

#pragma once

#ifdef CLI11_SINGLE_FILE
#include "CLI11.hpp"
#else
#include "CLI/CLI.hpp"
#endif

#include "catch.hpp"
#include <iostream>
#include <string>
#include <utility>
#include <vector>

using input_t = std::vector<std::string>;

class TApp {
  public:
    CLI::App app{"My Test Program"};
    input_t args{};
    virtual ~TApp() = default;
    void run() {
        // It is okay to re-parse - clear is called automatically before a parse.
        input_t newargs = args;
        std::reverse(std::begin(newargs), std::end(newargs));
        app.parse(newargs);
    }
};

class TempFile {
    std::string _name{};

  public:
    explicit TempFile(std::string name) : _name(std::move(name)) {
        if(!CLI::NonexistentPath(_name).empty())
            throw std::runtime_error(_name);
    }

    ~TempFile() {
        std::remove(_name.c_str());  // Doesn't matter if returns 0 or not
    }

    operator const std::string &() const { return _name; }
    const char *c_str() const { return _name.c_str(); }
};

inline void put_env(std::string name, std::string value) {
#ifdef _WIN32
    _putenv_s(name.c_str(), value.c_str());
#else
    setenv(name.c_str(), value.c_str(), 1);
#endif
}

inline void unset_env(std::string name) {
#ifdef _WIN32
    _putenv_s(name.c_str(), "");
#else
    unsetenv(name.c_str());
#endif
}
