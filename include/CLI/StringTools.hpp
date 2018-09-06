#pragma once

// Distributed under the 3-Clause BSD License.  See accompanying
// file LICENSE or https://github.com/CLIUtils/CLI11 for details.

#include <algorithm>
#include <iomanip>
#include <locale>
#include <sstream>
#include <string>
#include <type_traits>
#include <vector>

namespace CLI {
namespace detail {

// Based on http://stackoverflow.com/questions/236129/split-a-string-in-c
/// Split a string by a delim
inline std::vector<std::string> split(const std::string &s, char delim) {
    std::vector<std::string> elems;
    // Check to see if empty string, give consistent result
    if(s.empty())
        elems.emplace_back("");
    else {
        std::stringstream ss;
        ss.str(s);
        std::string item;
        while(std::getline(ss, item, delim)) {
            elems.push_back(item);
        }
    }
    return elems;
}

/// Simple function to join a string
template <typename T> std::string join(const T &v, std::string delim = ",") {
    std::ostringstream s;
    size_t start = 0;
    for(const auto &i : v) {
        if(start++ > 0)
            s << delim;
        s << i;
    }
    return s.str();
}

/// Join a string in reverse order
template <typename T> std::string rjoin(const T &v, std::string delim = ",") {
    std::ostringstream s;
    for(size_t start = 0; start < v.size(); start++) {
        if(start > 0)
            s << delim;
        s << v[v.size() - start - 1];
    }
    return s.str();
}

// Based roughly on http://stackoverflow.com/questions/25829143/c-trim-whitespace-from-a-string

/// Trim whitespace from left of string
inline std::string &ltrim(std::string &str) {
    auto it = std::find_if(str.begin(), str.end(), [](char ch) { return !std::isspace<char>(ch, std::locale()); });
    str.erase(str.begin(), it);
    return str;
}

/// Trim anything from left of string
inline std::string &ltrim(std::string &str, const std::string &filter) {
    auto it = std::find_if(str.begin(), str.end(), [&filter](char ch) { return filter.find(ch) == std::string::npos; });
    str.erase(str.begin(), it);
    return str;
}

/// Trim whitespace from right of string
inline std::string &rtrim(std::string &str) {
    auto it = std::find_if(str.rbegin(), str.rend(), [](char ch) { return !std::isspace<char>(ch, std::locale()); });
    str.erase(it.base(), str.end());
    return str;
}

/// Trim anything from right of string
inline std::string &rtrim(std::string &str, const std::string &filter) {
    auto it =
        std::find_if(str.rbegin(), str.rend(), [&filter](char ch) { return filter.find(ch) == std::string::npos; });
    str.erase(it.base(), str.end());
    return str;
}

/// Trim whitespace from string
inline std::string &trim(std::string &str) { return ltrim(rtrim(str)); }

/// Trim anything from string
inline std::string &trim(std::string &str, const std::string filter) { return ltrim(rtrim(str, filter), filter); }

/// Make a copy of the string and then trim it
inline std::string trim_copy(const std::string &str) {
    std::string s = str;
    return trim(s);
}

/// Make a copy of the string and then trim it, any filter string can be used (any char in string is filtered)
inline std::string trim_copy(const std::string &str, const std::string &filter) {
    std::string s = str;
    return trim(s, filter);
}
/// Print a two part "help" string
inline std::ostream &format_help(std::ostream &out, std::string name, std::string description, size_t wid) {
    name = "  " + name;
    out << std::setw(static_cast<int>(wid)) << std::left << name;
    if(!description.empty()) {
        if(name.length() >= wid)
            out << "\n" << std::setw(static_cast<int>(wid)) << "";
        out << description;
    }
    out << "\n";
    return out;
}

/// Verify the first character of an option
template <typename T> bool valid_first_char(T c) { return std::isalpha(c, std::locale()) || c == '_'; }

/// Verify following characters of an option
template <typename T> bool valid_later_char(T c) {
    return std::isalnum(c, std::locale()) || c == '_' || c == '.' || c == '-';
}

/// Verify an option name
inline bool valid_name_string(const std::string &str) {
    if(str.empty() || !valid_first_char(str[0]))
        return false;
    for(auto c : str.substr(1))
        if(!valid_later_char(c))
            return false;
    return true;
}

/// Return a lower case version of a string
inline std::string to_lower(std::string str) {
    std::transform(std::begin(str), std::end(str), std::begin(str), [](const std::string::value_type &x) {
        return std::tolower(x, std::locale());
    });
    return str;
}

/// Split a string '"one two" "three"' into 'one two', 'three'
inline std::vector<std::string> split_up(std::string str) {

    std::vector<char> delims = {'\'', '\"'};
    auto find_ws = [](char ch) { return std::isspace<char>(ch, std::locale()); };
    trim(str);

    std::vector<std::string> output;

    while(!str.empty()) {
        if(str[0] == '\'') {
            auto end = str.find('\'', 1);
            if(end != std::string::npos) {
                output.push_back(str.substr(1, end - 1));
                str = str.substr(end + 1);
            } else {
                output.push_back(str.substr(1));
                str = "";
            }
        } else if(str[0] == '\"') {
            auto end = str.find('\"', 1);
            if(end != std::string::npos) {
                output.push_back(str.substr(1, end - 1));
                str = str.substr(end + 1);
            } else {
                output.push_back(str.substr(1));
                str = "";
            }

        } else {
            auto it = std::find_if(std::begin(str), std::end(str), find_ws);
            if(it != std::end(str)) {
                std::string value = std::string(str.begin(), it);
                output.push_back(value);
                str = std::string(it, str.end());
            } else {
                output.push_back(str);
                str = "";
            }
        }
        trim(str);
    }

    return output;
}

/// Add a leader to the beginning of all new lines (nothing is added
/// at the start of the first line). `"; "` would be for ini files
///
/// Can't use Regex, or this would be a subs.
inline std::string fix_newlines(std::string leader, std::string input) {
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

/// Find and replace a subtring with another substring
inline std::string find_and_replace(std::string str, std::string from, std::string to) {

    size_t start_pos = 0;

    while((start_pos = str.find(from, start_pos)) != std::string::npos) {
        str.replace(start_pos, from.length(), to);
        start_pos += to.length();
    }

    return str;
}

} // namespace detail
} // namespace CLI
