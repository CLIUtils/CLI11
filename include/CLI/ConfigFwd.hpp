#pragma once

// Distributed under the 3-Clause BSD License.  See accompanying
// file LICENSE or https://github.com/CLIUtils/CLI11 for details.

#include <algorithm>
#include <fstream>
#include <iostream>
#include <string>

#include "CLI/StringTools.hpp"

namespace CLI {

class App;

namespace detail {

/// Comma separated join, adds quotes if needed
inline std::string ini_join(std::vector<std::string> args) {
    std::ostringstream s;
    size_t start = 0;
    for(const auto &arg : args) {
        if(start++ > 0)
            s << " ";

        auto it = std::find_if(arg.begin(), arg.end(), [](char ch) { return std::isspace<char>(ch, std::locale()); });
        if(it == arg.end())
            s << arg;
        else if(arg.find_first_of('\"') == std::string::npos)
            s << '\"' << arg << '\"';
        else
            s << '\'' << arg << '\'';
    }

    return s.str();
}

} // namespace detail

/// Holds values to load into Options
struct ConfigItem {
    /// This is the list of parents
    std::vector<std::string> parents;

    /// This is the name
    std::string name;

    /// Listing of inputs
    std::vector<std::string> inputs;

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
    std::vector<ConfigItem> items;

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

/// This converter works with INI files
class ConfigINI : public Config {
  public:
    std::string to_config(const App *, bool default_also, bool write_description, std::string prefix) const override;

    std::vector<ConfigItem> from_config(std::istream &input) const override {
        std::string line;
        std::string section = "default";

        std::vector<ConfigItem> output;

        while(getline(input, line)) {
            std::vector<std::string> items_buffer;

            detail::trim(line);
            size_t len = line.length();
            if(len > 1 && line[0] == '[' && line[len - 1] == ']') {
                section = line.substr(1, len - 2);
            } else if(len > 0 && line[0] != ';') {
                output.emplace_back();
                ConfigItem &out = output.back();

                // Find = in string, split and recombine
                auto pos = line.find('=');
                if(pos != std::string::npos) {
                    out.name = detail::trim_copy(line.substr(0, pos));
                    std::string item = detail::trim_copy(line.substr(pos + 1));
                    items_buffer = detail::split_up(item);
                } else {
                    out.name = detail::trim_copy(line);
                    items_buffer = {"ON"};
                }

                if(detail::to_lower(section) != "default") {
                    out.parents = {section};
                }

                if(out.name.find('.') != std::string::npos) {
                    std::vector<std::string> plist = detail::split(out.name, '.');
                    out.name = plist.back();
                    plist.pop_back();
                    out.parents.insert(out.parents.end(), plist.begin(), plist.end());
                }

                out.inputs.insert(std::end(out.inputs), std::begin(items_buffer), std::end(items_buffer));
            }
        }
        return output;
    }
};

} // namespace CLI
