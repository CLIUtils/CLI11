#pragma once

// Distributed under the 3-Clause BSD License.  See accompanying
// file LICENSE or https://github.com/CLIUtils/CLI11 for details.

#include <string>
#include <tuple>
#include <vector>

#include "CLI/Error.hpp"
#include "CLI/StringTools.hpp"

namespace CLI {
namespace detail {

// Returns false if not a short option. Otherwise, sets opt name and rest and returns true
inline bool split_short(const std::string &current, std::string &name, std::string &rest) {
    if(current.size() > 1 && current[0] == '-' && valid_first_char(current[1])) {
        name = current.substr(1, 1);
        rest = current.substr(2);
        return true;
    } else
        return false;
}

// Returns false if not a long option. Otherwise, sets opt name and other side of = and returns true
inline bool split_long(const std::string &current, std::string &name, std::string &value) {
    if(current.size() > 2 && current.substr(0, 2) == "--" && valid_first_char(current[2])) {
        auto loc = current.find("=");
        if(loc != std::string::npos) {
            name = current.substr(2, loc - 2);
            value = current.substr(loc + 1);
        } else {
            name = current.substr(2);
            value = "";
        }
        return true;
    } else
        return false;
}

// Splits a string into multiple long and short names
inline std::vector<std::string> split_names(std::string current) {
    std::vector<std::string> output;
    size_t val;
    while((val = current.find(",")) != std::string::npos) {
        output.push_back(trim_copy(current.substr(0, val)));
        current = current.substr(val + 1);
    }
    output.push_back(trim_copy(current));
    return output;
}

/// Get a vector of short names, one of long names, and a single name
inline std::tuple<std::vector<std::string>, std::vector<std::string>, std::string>
get_names(const std::vector<std::string> &input) {

    std::vector<std::string> short_names;
    std::vector<std::string> long_names;
    std::string pos_name;

    for(std::string name : input) {
        if(name.length() == 0)
            continue;
        else if(name.length() > 1 && name[0] == '-' && name[1] != '-') {
            if(name.length() == 2 && valid_first_char(name[1]))
                short_names.emplace_back(1, name[1]);
            else
                throw BadNameString::OneCharName(name);
        } else if(name.length() > 2 && name.substr(0, 2) == "--") {
            name = name.substr(2);
            if(valid_name_string(name))
                long_names.push_back(name);
            else
                throw BadNameString::BadLongName(name);
        } else if(name == "-" || name == "--") {
            throw BadNameString::DashesOnly(name);
        } else {
            if(pos_name.length() > 0)
                throw BadNameString::MultiPositionalNames(name);
            pos_name = name;
        }
    }

    return std::tuple<std::vector<std::string>, std::vector<std::string>, std::string>(
        short_names, long_names, pos_name);
}

} // namespace detail
} // namespace CLI
