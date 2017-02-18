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


/// Internal parsing function
std::vector<std::string> parse_ini(std::istream &input) {
    std::string line;
    std::string section = "default";

    std::vector<std::string> output;

    while(getline(input, line)) {
        detail::trim(line);
        size_t len = line.length();
        if(len > 1 && line[0] == '[' && line[len-1] == ']') {
            section = line.substr(1,len-2);
            std::transform(std::begin(section), std::end(section), std::begin(section), ::tolower);
        } else if (len > 0) {
            if(section == "default")
                output.push_back("--" + line);
            else
                output.push_back("--" + section + "." + line);
        }
    }
    return output;
}

/// Parse an INI file, throw an error (ParseError:INIParseError or FileError) on failure
std::vector<std::string> parse_ini(const std::string &name) {

    std::ifstream input{name};
    if(!input.good())
        throw FileError(name);

    return parse_ini(input);
}


}
}
