// Copyright (c) 2017-2020, University of Cincinnati, developed by Henry Schreiner
// under NSF AWARD 1414736 and by the respective contributors.
// All rights reserved.
//
// SPDX-License-Identifier: BSD-3-Clause

#pragma once

#include <algorithm>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include "Error.hpp"
#include "StringTools.hpp"

namespace CLI {

class App;

/// Holds values to load into Options
struct ConfigItem {
    /// This is the list of parents
    std::vector<std::string> parents{};

    /// This is the name
    std::string name{};

    /// Listing of inputs
    std::vector<std::string> inputs{};

    /// The list of parents and name joined by "."
    std::string fullname() const {
        std::vector<std::string> tmp = parents;
        tmp.emplace_back(name);
        return detail::join(tmp, ".");
    }
};

/// This class provides a converter for configuration files.
class Config {
  protected:
    std::vector<ConfigItem> items{};

  public:
    /// Convert an app into a configuration
    virtual std::string to_config(const App *, bool, bool, std::string) const = 0;

    /// Convert a configuration into an app
    virtual std::vector<ConfigItem> from_config(std::istream &) const = 0;

    /// Get a flag value
    virtual std::string to_flag(const ConfigItem &item) const {
        if(item.inputs.size() == 1) {
            return item.inputs.at(0);
        }
        throw ConversionError::TooManyInputsFlag(item.fullname());
    }

    /// Parse a config file, throw an error (ParseError:ConfigParseError or FileError) on failure
    std::vector<ConfigItem> from_file(const std::string &name) {
        std::ifstream input{name};
        if(!input.good())
            throw FileError::Missing(name);

        return from_config(input);
    }

    /// Virtual destructor
    virtual ~Config() = default;
};

/// This converter works with INI/TOML files; to write proper TOML files use ConfigTOML
class ConfigBase : public Config {
  protected:
    /// the character used for comments
    char commentChar = ';';
    /// the character used to start an array '\0' is a default to not use
    char arrayStart = '\0';
    /// the character used to end an array '\0' is a default to not use
    char arrayEnd = '\0';
    /// the character used to separate elements in an array
    char arraySeparator = ' ';
    /// the character used separate the name from the value
    char valueDelimiter = '=';

  public:
    std::string
    to_config(const App * /*app*/, bool default_also, bool write_description, std::string prefix) const override;

    std::vector<ConfigItem> from_config(std::istream &input) const override;
    /// Specify the configuration for comment characters
    ConfigBase *comment(char cchar) {
        commentChar = cchar;
        return this;
    }
    /// Specify the start and end characters for an array
    ConfigBase *arrayBounds(char aStart, char aEnd) {
        arrayStart = aStart;
        arrayEnd = aEnd;
        return this;
    }
    /// Specify the delimiter character for an array
    ConfigBase *arrayDelimiter(char aSep) {
        arraySeparator = aSep;
        return this;
    }
    /// Specify the delimiter between a name and value
    ConfigBase *valueSeparator(char vSep) {
        valueDelimiter = vSep;
        return this;
    }
};

/// the default Config is the INI file format
using ConfigINI = ConfigBase;

/// ConfigTOML generates a TOML compliant output
class ConfigTOML : public ConfigINI {

  public:
    ConfigTOML() {
        commentChar = '#';
        arrayStart = '[';
        arrayEnd = ']';
        arraySeparator = ',';
        valueDelimiter = '=';
    }
};
}  // namespace CLI
