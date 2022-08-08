// Copyright (c) 2017-2022, University of Cincinnati, developed by Henry Schreiner
// under NSF AWARD 1414736 and by the respective contributors.
// All rights reserved.
//
// SPDX-License-Identifier: BSD-3-Clause

#pragma once

// [CLI11:public_includes:set]
#include <algorithm>
#include <cctype>
#include <fstream>
#include <iostream>
#include <string>
#include <utility>
#include <vector>
// [CLI11:public_includes:set]

#include "App.hpp"
#include "ConfigFwd.hpp"
#include "StringTools.hpp"

namespace CLI {
// [CLI11:config_hpp:verbatim]
namespace detail {

std::string convert_arg_for_ini(const std::string &arg, char stringQuote = '"', char characterQuote = '\'');

/// Comma separated join, adds quotes if needed
std::string ini_join(const std::vector<std::string> &args,
                     char sepChar = ',',
                     char arrayStart = '[',
                     char arrayEnd = ']',
                     char stringQuote = '"',
                     char characterQuote = '\'');

std::vector<std::string> generate_parents(const std::string &section, std::string &name, char parentSeparator);

/// assuming non default segments do a check on the close and open of the segments in a configItem structure
void checkParentSegments(std::vector<ConfigItem> &output, const std::string &currentSection, char parentSeparator);
}  // namespace detail

// [CLI11:config_hpp:end]
}  // namespace CLI

#ifndef CLI11_COMPILE
#include "impl/Config_inl.hpp"
#endif
