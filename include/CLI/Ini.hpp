#pragma once

// Distributed under the LGPL v2.1 license.  See accompanying
// file LICENSE or https://github.com/henryiii/CLI11 for details.

#include <fstream>
#include <string>
#include <iostream>
#include <algorithm>

#include "CLI/StringTools.hpp"


namespace CLI {
namespace detail {


struct ini_ret_t {
    /// This is the full name with dots
    std::string fullname;

    /// Listing of inputs
    std::vector<std::string> inputs;

    /// Current parent level
    size_t level = 0;

    /// Return parent or empty string, based on level
    ///
    /// Level 0, a.b.c would return a
    /// Level 1, a.b.c could return b
    std::string parent () const {
        std::vector<std::string> plist = detail::split(fullname, '.');
        if(plist.size() > (level+1))
            return plist[level];
        else
            return "";
    }

    /// Return name
    std::string name () const {
        std::vector<std::string> plist = detail::split(fullname, '.');
        return plist.at(plist.size()-1);
    }
};

/// Internal parsing function
std::vector<ini_ret_t> parse_ini(std::istream &input) {
    std::string name, line;
    std::string section = "default";

    std::vector<ini_ret_t> output;

    while(getline(input, line)) {
        std::vector<std::string> items;

        detail::trim(line);
        size_t len = line.length();
        if(len > 1 && line[0] == '[' && line[len-1] == ']') {
            section = line.substr(1,len-2);
        } else if (len > 0 && line[0] != ';') {
            output.emplace_back();
            ini_ret_t& out = output.back();

            // Find = in string, split and recombine
            auto pos = line.find("=");
            if(pos != std::string::npos) {
                name = detail::trim_copy(line.substr(0,pos));
                std::string item = detail::trim_copy(line.substr(pos+1));
                items = detail::split_up(item);
            } else {
                name = detail::trim_copy(line);
                items = {"ON"};
            }

            if(detail::to_lower(section) == "default")
                out.fullname = name;
            else
                out.fullname = section + "." + name;

            out.inputs.insert(std::end(out.inputs), std::begin(items), std::end(items));
        }
    }
    return output;
}

/// Parse an INI file, throw an error (ParseError:INIParseError or FileError) on failure
std::vector<ini_ret_t> parse_ini(const std::string &name) {

    std::ifstream input{name};
    if(!input.good())
        throw FileError(name);

    return parse_ini(input);
}


}
}
