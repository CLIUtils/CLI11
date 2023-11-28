// Copyright (c) 2017-2023, University of Cincinnati, developed by Henry Schreiner
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

static const std::string escapedChars("'\"`])>}\\");
static const std::string bracketChars{"'\"`[(<{"};
static const std::string matchBracketChars("'\"`])>}");

CLI11_INLINE bool has_escapable_character(const std::string &str) {
    return (str.find_first_of(escapedChars) != std::string::npos);
}

CLI11_INLINE std::string add_escaped_characters(const std::string &str) {
    std::string out;
    out.reserve(str.size() + 4);
    for(char s : str) {
        if(escapedChars.find_first_of(s) != std::string::npos) {
            out.push_back('\\');
        }
        out.push_back(s);
    }
    return out;
}

CLI11_INLINE std::string remove_escaped_characters(const std::string &str) {

    std::string out;
    out.reserve(str.size());
    for(auto loc = str.begin(); loc < str.end(); ++loc) {
        if(*loc == '\\') {
            if(escapedChars.find_first_of(*(loc + 1)) != std::string::npos) {
                out.push_back(*(loc + 1));
                ++loc;
            } else {
                out.push_back(*loc);
            }
        } else {
            out.push_back(*loc);
        }
    }
    return out;
}

CLI11_INLINE std::pair<std::size_t, bool> close_sequence(const std::string &str, std::size_t start, char closure_char) {
    std::string closures;
    closures.push_back(closure_char);
    auto loc = start + 1;
    bool inQuote = closure_char == '"' || closure_char == '\'' || closure_char == '`';
    bool hasControlSequence{false};
    while(loc < str.size()) {
        if(str[loc] == closures.back() && ((str[loc - 1] != '\\') || (str[loc - 2] == '\\'))) {
            closures.pop_back();
            if(closures.empty()) {
                return {loc, hasControlSequence};
            }
            inQuote = false;
        }

        if(!inQuote) {
            auto bracket_loc = bracketChars.find(str[loc]);
            if(bracket_loc != std::string::npos) {
                closures.push_back(matchBracketChars[bracket_loc]);
                inQuote = (bracket_loc <= 2);
            }
        } else {
            if(str[loc] == '\\') {
                hasControlSequence = true;
            }
        }
        ++loc;
    }
    return {loc, hasControlSequence};
}

CLI11_INLINE std::vector<std::string> split_up(std::string str, char delimiter, bool removeQuotes) {

    auto find_ws = [delimiter](char ch) {
        return (delimiter == '\0') ? std::isspace<char>(ch, std::locale()) : (ch == delimiter);
    };
    trim(str);

    std::vector<std::string> output;
    bool embeddedQuote = false;
    int adjust = removeQuotes ? 1 : 0;
    while(!str.empty()) {
        if(bracketChars.find_first_of(str[0]) != std::string::npos) {
            auto bracketLoc = bracketChars.find_first_of(str[0]);
            auto closure = close_sequence(str, 1, matchBracketChars[bracketLoc]);
            auto end = closure.first;
            if(end != std::string::npos) {
                output.push_back(str.substr(adjust, end + 1 - 2 * adjust));
                if(end + 2 < str.size()) {
                    str = str.substr(end + 2);
                } else {
                    str.clear();
                }
            } else {
                output.push_back(str.substr(adjust));
                str.clear();
            }
            embeddedQuote = embeddedQuote || closure.second;
        } else {
            auto it = std::find_if(std::begin(str), std::end(str), find_ws);
            if(it != std::end(str)) {
                std::string value = std::string(str.begin(), it);
                output.push_back(value);
                str = std::string(it + 1, str.end());
            } else {
                output.push_back(str);
                str.clear();
            }
        }
        // transform any embedded quotes into the regular character if the quotes are removed
        if(embeddedQuote && removeQuotes) {
            output.back() = remove_escaped_characters(output.back());
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

CLI11_INLINE std::string binary_escape_string(const std::string &string_to_escape) {
    // s is our escaped output string
    std::string escaped_string{};
    // loop through all characters
    for(char c : string_to_escape) {
        // check if a given character is printable
        // the cast is necessary to avoid undefined behaviour
        if(isprint((unsigned char)c) == 0) {
            std::stringstream stream;
            // if the character is not printable
            // we'll convert it to a hex string using a stringstream
            // note that since char is signed we have to cast it to unsigned first
            stream << std::hex << (unsigned int)(unsigned char)(c);
            std::string code = stream.str();
            escaped_string += std::string("\\x") + (code.size() < 2 ? "0" : "") + code;

        } else {
            escaped_string.push_back(c);
        }
    }
    if(escaped_string != string_to_escape) {
        auto sqLoc = escaped_string.find('\'');
        while(sqLoc != std::string::npos) {
            escaped_string.replace(sqLoc, sqLoc + 1, "\\x27");
            sqLoc = escaped_string.find('\'');
        }
        escaped_string.insert(0, "'B(\"");
        escaped_string.push_back(')');
        escaped_string.push_back('"');
        escaped_string.push_back('\'');
    }
    return escaped_string;
}

CLI11_INLINE bool is_binary_escaped_string(const std::string &escaped_string) {
    size_t ssize = escaped_string.size();
    if(escaped_string.compare(0, 3, "B(\"") == 0 && escaped_string.compare(ssize - 2, 2, ")\"") == 0) {
        return true;
    }
    return (escaped_string.compare(0, 4, "'B(\"") == 0 && escaped_string.compare(ssize - 3, 3, ")\"'") == 0);
}

CLI11_INLINE std::string extract_binary_string(const std::string &escaped_string) {
    std::size_t start{0};
    std::size_t tail{0};
    size_t ssize = escaped_string.size();
    if(escaped_string.compare(0, 3, "B(\"") == 0 && escaped_string.compare(ssize - 2, 2, ")\"") == 0) {
        start = 3;
        tail = 2;
    } else if(escaped_string.compare(0, 4, "'B(\"") == 0 && escaped_string.compare(ssize - 3, 3, ")\"'") == 0) {
        start = 4;
        tail = 3;
    }

    if(start == 0) {
        return escaped_string;
    }
    std::string outstring;

    outstring.reserve(ssize - start - tail);
    std::size_t loc = start;
    while(loc < ssize - tail) {
        // ssize-2 to skip )" at the end
        if(escaped_string[loc] == '\\' && escaped_string[loc + 1] == 'x') {
            auto c1 = escaped_string[loc + 2];
            auto c2 = escaped_string[loc + 3];
            unsigned int res{0};
            bool invalid{false};
            if(c1 >= 48 && c1 <= 57) {
                res = (c1 - 48) * 16;
            } else if(c1 >= 65 && c1 <= 70) {
                res = (c1 - 55) * 16;
            } else if(c1 >= 97 && c1 <= 102) {
                res = (c1 - 87) * 16;
            } else {
                invalid = true;
            }

            if(c2 >= 48 && c2 <= 57) {
                res += (c2 - 48);
            } else if(c2 >= 65 && c2 <= 70) {
                res += (c2 - 55);
            } else if(c2 >= 97 && c2 <= 102) {
                res += (c2 - 87);
            } else {
                invalid = true;
            }
            if(!invalid) {
                loc += 4;
                outstring.push_back(static_cast<char>(res));
                continue;
            }
        }
        outstring.push_back(escaped_string[loc]);
        ++loc;
    }
    return outstring;
}

std::string get_environment_value(const std::string &env_name) {
    char *buffer = nullptr;
    std::string ename_string;

#ifdef _MSC_VER
    // Windows version
    std::size_t sz = 0;
    if(_dupenv_s(&buffer, &sz, env_name.c_str()) == 0 && buffer != nullptr) {
        ename_string = std::string(buffer);
        free(buffer);
    }
#else
    // This also works on Windows, but gives a warning
    buffer = std::getenv(env_name.c_str());
    if(buffer != nullptr) {
        ename_string = std::string(buffer);
    }
#endif
    return ename_string;
}

}  // namespace detail
// [CLI11:string_tools_inl_hpp:end]
}  // namespace CLI
