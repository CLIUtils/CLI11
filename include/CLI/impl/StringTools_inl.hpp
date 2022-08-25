// Copyright (c) 2017-2022, University of Cincinnati, developed by Henry Schreiner
// under NSF AWARD 1414736 and by the respective contributors.
// All rights reserved.
//
// SPDX-License-Identifier: BSD-3-Clause

#pragma once

// This include is only needed for IDEs to discover symbols
#include <CLI/StringTools.hpp>

// [CLI11:public_includes:set]
#include <string>
#include <vector>
// [CLI11:public_includes:end]

namespace CLI {
// [CLI11:string_tools_inl_hpp:verbatim]

namespace detail {
CLI11_INLINE std::vector<std::string> split(const std::string &s, char delim) {
    std::vector<std::string> elems;
    // Check to see if empty string, give consistent result
    if(s.empty()) {
        elems.emplace_back();
    } else {
        std::stringstream ss;
        ss.str(s);
        std::string item;
        while(std::getline(ss, item, delim)) {
            elems.push_back(item);
        }
    }
    return elems;
}

CLI11_INLINE std::string &ltrim(std::string &str) {
    auto it = std::find_if(str.begin(), str.end(), [](char ch) { return !std::isspace<char>(ch, std::locale()); });
    str.erase(str.begin(), it);
    return str;
}

CLI11_INLINE std::string &ltrim(std::string &str, const std::string &filter) {
    auto it = std::find_if(str.begin(), str.end(), [&filter](char ch) { return filter.find(ch) == std::string::npos; });
    str.erase(str.begin(), it);
    return str;
}

CLI11_INLINE std::string &rtrim(std::string &str) {
    auto it = std::find_if(str.rbegin(), str.rend(), [](char ch) { return !std::isspace<char>(ch, std::locale()); });
    str.erase(it.base(), str.end());
    return str;
}

CLI11_INLINE std::string &rtrim(std::string &str, const std::string &filter) {
    auto it =
        std::find_if(str.rbegin(), str.rend(), [&filter](char ch) { return filter.find(ch) == std::string::npos; });
    str.erase(it.base(), str.end());
    return str;
}

CLI11_INLINE std::string &remove_quotes(std::string &str) {
    if(str.length() > 1 && (str.front() == '"' || str.front() == '\'')) {
        if(str.front() == str.back()) {
            str.pop_back();
            str.erase(str.begin(), str.begin() + 1);
        }
    }
    return str;
}

CLI11_INLINE std::string fix_newlines(const std::string &leader, std::string input) {
    std::string::size_type n = 0;
    while(n != std::string::npos && n < input.size()) {
        n = input.find('\n', n);
        if(n != std::string::npos) {
            input = input.substr(0, n + 1) + leader + input.substr(n + 1);
            n += leader.size();
        }
    }
    return input;
}

CLI11_INLINE std::ostream &
format_help(std::ostream &out, std::string name, const std::string &description, std::size_t wid) {
    name = "  " + name;
    out << std::setw(static_cast<int>(wid)) << std::left << name;
    if(!description.empty()) {
        if(name.length() >= wid)
            out << "\n" << std::setw(static_cast<int>(wid)) << "";
        for(const char c : description) {
            out.put(c);
            if(c == '\n') {
                out << std::setw(static_cast<int>(wid)) << "";
            }
        }
    }
    out << "\n";
    return out;
}

CLI11_INLINE std::ostream &format_aliases(std::ostream &out, const std::vector<std::string> &aliases, std::size_t wid) {
    if(!aliases.empty()) {
        out << std::setw(static_cast<int>(wid)) << "     aliases: ";
        bool front = true;
        for(const auto &alias : aliases) {
            if(!front) {
                out << ", ";
            } else {
                front = false;
            }
            out << detail::fix_newlines("              ", alias);
        }
        out << "\n";
    }
    return out;
}

CLI11_INLINE bool valid_name_string(const std::string &str) {
    if(str.empty() || !valid_first_char(str[0])) {
        return false;
    }
    auto e = str.end();
    for(auto c = str.begin() + 1; c != e; ++c)
        if(!valid_later_char(*c))
            return false;
    return true;
}

CLI11_INLINE std::string find_and_replace(std::string str, std::string from, std::string to) {

    std::size_t start_pos = 0;

    while((start_pos = str.find(from, start_pos)) != std::string::npos) {
        str.replace(start_pos, from.length(), to);
        start_pos += to.length();
    }

    return str;
}

CLI11_INLINE void remove_default_flag_values(std::string &flags) {
    auto loc = flags.find_first_of('{', 2);
    while(loc != std::string::npos) {
        auto finish = flags.find_first_of("},", loc + 1);
        if((finish != std::string::npos) && (flags[finish] == '}')) {
            flags.erase(flags.begin() + static_cast<std::ptrdiff_t>(loc),
                        flags.begin() + static_cast<std::ptrdiff_t>(finish) + 1);
        }
        loc = flags.find_first_of('{', loc + 1);
    }
    flags.erase(std::remove(flags.begin(), flags.end(), '!'), flags.end());
}

CLI11_INLINE std::ptrdiff_t
find_member(std::string name, const std::vector<std::string> names, bool ignore_case, bool ignore_underscore) {
    auto it = std::end(names);
    if(ignore_case) {
        if(ignore_underscore) {
            name = detail::to_lower(detail::remove_underscore(name));
            it = std::find_if(std::begin(names), std::end(names), [&name](std::string local_name) {
                return detail::to_lower(detail::remove_underscore(local_name)) == name;
            });
        } else {
            name = detail::to_lower(name);
            it = std::find_if(std::begin(names), std::end(names), [&name](std::string local_name) {
                return detail::to_lower(local_name) == name;
            });
        }

    } else if(ignore_underscore) {
        name = detail::remove_underscore(name);
        it = std::find_if(std::begin(names), std::end(names), [&name](std::string local_name) {
            return detail::remove_underscore(local_name) == name;
        });
    } else {
        it = std::find(std::begin(names), std::end(names), name);
    }

    return (it != std::end(names)) ? (it - std::begin(names)) : (-1);
}

CLI11_INLINE std::vector<std::string> split_up(std::string str, char delimiter) {

    const std::string delims("\'\"`");
    auto find_ws = [delimiter](char ch) {
        return (delimiter == '\0') ? std::isspace<char>(ch, std::locale()) : (ch == delimiter);
    };
    trim(str);

    std::vector<std::string> output;
    bool embeddedQuote = false;
    char keyChar = ' ';
    while(!str.empty()) {
        if(delims.find_first_of(str[0]) != std::string::npos) {
            keyChar = str[0];
            auto end = str.find_first_of(keyChar, 1);
            while((end != std::string::npos) && (str[end - 1] == '\\')) {  // deal with escaped quotes
                end = str.find_first_of(keyChar, end + 1);
                embeddedQuote = true;
            }
            if(end != std::string::npos) {
                output.push_back(str.substr(1, end - 1));
                if(end + 2 < str.size()) {
                    str = str.substr(end + 2);
                } else {
                    str.clear();
                }

            } else {
                output.push_back(str.substr(1));
                str = "";
            }
        } else {
            auto it = std::find_if(std::begin(str), std::end(str), find_ws);
            if(it != std::end(str)) {
                std::string value = std::string(str.begin(), it);
                output.push_back(value);
                str = std::string(it + 1, str.end());
            } else {
                output.push_back(str);
                str = "";
            }
        }
        // transform any embedded quotes into the regular character
        if(embeddedQuote) {
            output.back() = find_and_replace(output.back(), std::string("\\") + keyChar, std::string(1, keyChar));
            embeddedQuote = false;
        }
        trim(str);
    }
    return output;
}

CLI11_INLINE std::size_t escape_detect(std::string &str, std::size_t offset) {
    auto next = str[offset + 1];
    if((next == '\"') || (next == '\'') || (next == '`')) {
        auto astart = str.find_last_of("-/ \"\'`", offset - 1);
        if(astart != std::string::npos) {
            if(str[astart] == ((str[offset] == '=') ? '-' : '/'))
                str[offset] = ' ';  // interpret this as a space so the split_up works properly
        }
    }
    return offset + 1;
}

CLI11_INLINE std::string &add_quotes_if_needed(std::string &str) {
    if((str.front() != '"' && str.front() != '\'') || str.front() != str.back()) {
        char quote = str.find('"') < str.find('\'') ? '\'' : '"';
        if(str.find(' ') != std::string::npos) {
            str.insert(0, 1, quote);
            str.append(1, quote);
        }
    }
    return str;
}

}  // namespace detail
// [CLI11:string_tools_inl_hpp:end]
}  // namespace CLI
