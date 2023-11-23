// Copyright (c) 2017-2023, University of Cincinnati, developed by Henry Schreiner
// under NSF AWARD 1414736 and by the respective contributors.
// All rights reserved.
//
// SPDX-License-Identifier: BSD-3-Clause

#pragma once

// This include is only needed for IDEs to discover symbols
#include <CLI/Config.hpp>

// [CLI11:public_includes:set]
#include <algorithm>
#include <string>
#include <utility>
#include <vector>
// [CLI11:public_includes:end]

namespace CLI {
// [CLI11:config_inl_hpp:verbatim]

static constexpr auto tquote = R"(""")";

namespace detail {

CLI11_INLINE std::string escape_string(const std::string &string_to_escape) {
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

CLI11_INLINE bool is_printable(const std::string &test_string) {
    return std::all_of(test_string.begin(), test_string.end(), [](char x) {
        return (isprint(static_cast<unsigned char>(x)) != 0 || x == '\n');
    });
}

CLI11_INLINE std::string convert_arg_for_ini(const std::string &arg, char stringQuote, char characterQuote,bool disable_multi_line) {
    if(arg.empty()) {
        return std::string(2, stringQuote);
    }
    // some specifically supported strings
    if(arg == "true" || arg == "false" || arg == "nan" || arg == "inf") {
        return arg;
    }
    // floating point conversion can convert some hex codes, but don't try that here
    if(arg.compare(0, 2, "0x") != 0 && arg.compare(0, 2, "0X") != 0) {
        using CLI::detail::lexical_cast;
        double val = 0.0;
        if(lexical_cast(arg, val)) {
            return arg;
        }
    }
    // just quote a single non numeric character
    if(arg.size() == 1) {
        if(isprint(static_cast<unsigned char>(arg.front())) == 0) {
            return escape_string(arg);
        }
        return std::string(1, characterQuote) + arg + characterQuote;
    }
    // handle hex, binary or octal arguments
    if(arg.front() == '0') {
        if(arg[1] == 'x') {
            if(std::all_of(arg.begin() + 2, arg.end(), [](char x) {
                   return (x >= '0' && x <= '9') || (x >= 'A' && x <= 'F') || (x >= 'a' && x <= 'f');
               })) {
                return arg;
            }
        } else if(arg[1] == 'o') {
            if(std::all_of(arg.begin() + 2, arg.end(), [](char x) { return (x >= '0' && x <= '7'); })) {
                return arg;
            }
        } else if(arg[1] == 'b') {
            if(std::all_of(arg.begin() + 2, arg.end(), [](char x) { return (x == '0' || x == '1'); })) {
                return arg;
            }
        }
    }
    if(!is_printable(arg)) {
        return escape_string(arg);
    }
    if(arg.find_first_of("\n") != std::string::npos) {
        if (disable_multi_line)
        {
            return escape_string(arg);
        }
        else
        {
            return std::string(tquote) + arg + tquote;
        }
        
    }
    if(arg.find_first_of(stringQuote) == std::string::npos) {
        return std::string(1, stringQuote) + arg + stringQuote;
    }
    return characterQuote + arg + characterQuote;
}

CLI11_INLINE std::string ini_join(const std::vector<std::string> &args,
                                  char sepChar,
                                  char arrayStart,
                                  char arrayEnd,
                                  char stringQuote,
                                  char characterQuote) {
    bool disable_multi_line{false};
    std::string joined;
    if(args.size() > 1 && arrayStart != '\0') {
        joined.push_back(arrayStart);
        disable_multi_line=true;
    }
    std::size_t start = 0;
    for(const auto &arg : args) {
        if(start++ > 0) {
            joined.push_back(sepChar);
            if(!std::isspace<char>(sepChar, std::locale())) {
                joined.push_back(' ');
            }
        }
        joined.append(convert_arg_for_ini(arg, stringQuote, characterQuote,disable_multi_line));
    }
    if(args.size() > 1 && arrayEnd != '\0') {
        joined.push_back(arrayEnd);
    }
    return joined;
}

CLI11_INLINE std::vector<std::string>
generate_parents(const std::string &section, std::string &name, char parentSeparator) {
    std::vector<std::string> parents;
    if(detail::to_lower(section) != "default") {
        if(section.find(parentSeparator) != std::string::npos) {
            parents = detail::split(section, parentSeparator);
        } else {
            parents = {section};
        }
    }
    if(name.find(parentSeparator) != std::string::npos) {
        std::vector<std::string> plist = detail::split(name, parentSeparator);
        name = plist.back();
        detail::remove_quotes(name);
        plist.pop_back();
        parents.insert(parents.end(), plist.begin(), plist.end());
    }

    // clean up quotes on the parents
    for(auto &parent : parents) {
        detail::remove_quotes(parent);
    }
    return parents;
}

CLI11_INLINE void
checkParentSegments(std::vector<ConfigItem> &output, const std::string &currentSection, char parentSeparator) {

    std::string estring;
    auto parents = detail::generate_parents(currentSection, estring, parentSeparator);
    if(!output.empty() && output.back().name == "--") {
        std::size_t msize = (parents.size() > 1U) ? parents.size() : 2;
        while(output.back().parents.size() >= msize) {
            output.push_back(output.back());
            output.back().parents.pop_back();
        }

        if(parents.size() > 1) {
            std::size_t common = 0;
            std::size_t mpair = (std::min)(output.back().parents.size(), parents.size() - 1);
            for(std::size_t ii = 0; ii < mpair; ++ii) {
                if(output.back().parents[ii] != parents[ii]) {
                    break;
                }
                ++common;
            }
            if(common == mpair) {
                output.pop_back();
            } else {
                while(output.back().parents.size() > common + 1) {
                    output.push_back(output.back());
                    output.back().parents.pop_back();
                }
            }
            for(std::size_t ii = common; ii < parents.size() - 1; ++ii) {
                output.emplace_back();
                output.back().parents.assign(parents.begin(), parents.begin() + static_cast<std::ptrdiff_t>(ii) + 1);
                output.back().name = "++";
            }
        }
    } else if(parents.size() > 1) {
        for(std::size_t ii = 0; ii < parents.size() - 1; ++ii) {
            output.emplace_back();
            output.back().parents.assign(parents.begin(), parents.begin() + static_cast<std::ptrdiff_t>(ii) + 1);
            output.back().name = "++";
        }
    }

    // insert a section end which is just an empty items_buffer
    output.emplace_back();
    output.back().parents = std::move(parents);
    output.back().name = "++";
}

CLI11_INLINE bool hasMLString(std::string const &fullString, char check) {
    if(fullString.length() < 3) {
        return false;
    }
    auto it = fullString.rbegin();
    return (*it == check) && (*(it + 1) == check) && (*(it + 2) == check);
}
}  // namespace detail

inline std::vector<ConfigItem> ConfigBase::from_config(std::istream &input) const {
    std::string line;
    std::string buffer;
    std::string currentSection = "default";
    std::string previousSection = "default";
    std::vector<ConfigItem> output;
    bool isDefaultArray = (arrayStart == '[' && arrayEnd == ']' && arraySeparator == ',');
    bool isINIArray = (arrayStart == '\0' || arrayStart == ' ') && arrayStart == arrayEnd;
    bool inSection{false};
    bool inMLineComment{false};
    bool inMLineValue{false};

    char aStart = (isINIArray) ? '[' : arrayStart;
    char aEnd = (isINIArray) ? ']' : arrayEnd;
    char aSep = (isINIArray && arraySeparator == ' ') ? ',' : arraySeparator;
    int currentSectionIndex{0};
    std::unique_ptr<ConfigItem> alt_config{nullptr};

    while(getline(input, buffer)) {
        std::vector<std::string> items_buffer;
        std::string name;
        bool literalName{false};
        line = detail::trim_copy(buffer);
        std::size_t len = line.length();
        // lines have to be at least 3 characters to have any meaning to CLI just skip the rest
        if(len < 3) {
            continue;
        }
        if(line.compare(0, 3, tquote) == 0 || line.compare(0, 3, "'''") == 0) {
            inMLineComment = true;
            auto cchar = line.front();
            while(inMLineComment) {
                if(getline(input, line)) {
                    detail::trim(line);
                } else {
                    break;
                }
                if(detail::hasMLString(line, cchar)) {
                    inMLineComment = false;
                }
            }
            continue;
        }
        if(line.front() == '[' && line.back() == ']') {
            if(currentSection != "default") {
                // insert a section end which is just an empty items_buffer
                output.emplace_back();
                output.back().parents = detail::generate_parents(currentSection, name, parentSeparatorChar);
                output.back().name = "--";
            }
            currentSection = line.substr(1, len - 2);
            // deal with double brackets for TOML
            if(currentSection.size() > 1 && currentSection.front() == '[' && currentSection.back() == ']') {
                currentSection = currentSection.substr(1, currentSection.size() - 2);
            }
            if(detail::to_lower(currentSection) == "default") {
                currentSection = "default";
            } else {
                detail::checkParentSegments(output, currentSection, parentSeparatorChar);
            }
            inSection = false;
            if(currentSection == previousSection) {
                ++currentSectionIndex;
            } else {
                currentSectionIndex = 0;
                previousSection = currentSection;
            }
            continue;
        }

        // comment lines
        if(line.front() == ';' || line.front() == '#' || line.front() == commentChar) {
            if(line.compare(2, 13, "cli11:literal") == 0) {
                literalName = true;
                getline(input, buffer);
                line = detail::trim_copy(buffer);
                len = line.length();
                if(len < 3) {
                    continue;
                }
            } else {
                continue;
            }
        }

        // Find = in string, split and recombine
        auto delimiter_pos = line.find_first_of(valueDelimiter, 1);
        auto comment_pos = (literalName) ? std::string::npos : line.find_first_of(commentChar);
        std::string orig_name;
        if(comment_pos < delimiter_pos) {
            if(delimiter_pos != std::string::npos) {
                alt_config = std::unique_ptr<ConfigItem>(new ConfigItem);
                alt_config->name = detail::trim_copy(line.substr(0, delimiter_pos));
            }
            delimiter_pos = std::string::npos;
        }
        if(delimiter_pos != std::string::npos) {

            name = detail::trim_copy(line.substr(0, delimiter_pos));
            std::string item = detail::trim_copy(line.substr(delimiter_pos + 1, comment_pos - delimiter_pos - 1));
            if(item.compare(0, 3, "'''") == 0 || item.compare(0, 3, tquote) == 0) {
                // mutliline string
                auto keyChar = item.front();
                item = buffer.substr(delimiter_pos + 1, std::string::npos);
                detail::ltrim(item);
                item.erase(0, 3);
                inMLineValue = true;
                bool lineExtension{false};
                bool firstLine = true;
                if(!item.empty() && item.back() == '\\') {
                    item.pop_back();
                    lineExtension = true;
                }
                while(inMLineValue) {
                    std::string l2;
                    if(!std::getline(input, l2)) {
                        break;
                    }
                    line = l2;
                    detail::rtrim(line);
                    if(detail::hasMLString(line, keyChar)) {
                        line.pop_back();
                        line.pop_back();
                        line.pop_back();
                        if(lineExtension) {
                            detail::ltrim(line);
                        } else if(!(firstLine && item.empty())) {
                            item.push_back('\n');
                        }
                        firstLine = false;
                        item += line;
                        inMLineValue = false;
                        if(!item.empty() && item.back() == '\n') {
                            item.pop_back();
                        }
                    } else {
                        if(lineExtension) {
                            detail::trim(l2);
                        } else if(!(firstLine && item.empty())) {
                            item.push_back('\n');
                        }
                        lineExtension = false;
                        firstLine = false;
                        if(!l2.empty() && l2.back() == '\\') {
                            lineExtension = true;
                            l2.pop_back();
                        }
                        item += l2;
                    }
                }
                items_buffer = {item};
            } else if(item.size() > 1 && item.front() == aStart) {
                for(std::string multiline; item.back() != aEnd && std::getline(input, multiline);) {
                    detail::trim(multiline);
                    item += multiline;
                }
                items_buffer = detail::split_up(item.substr(1, item.length() - 2), aSep);
            } else if((isDefaultArray || isINIArray) && item.find_first_of(aSep) != std::string::npos) {
                items_buffer = detail::split_up(item, aSep);
            } else if((isDefaultArray || isINIArray) && item.find_first_of(' ') != std::string::npos) {
                items_buffer = detail::split_up(item);
            } else {
                if(literalName) {
                    items_buffer = {item};
                } else {
                    auto citems = detail::split_up(line.substr(delimiter_pos + 1, std::string::npos), commentChar);
                    item = detail::trim_copy(citems.front());
                    items_buffer = {item};
                }
            }
        } else {
            name = detail::trim_copy(line.substr(0, comment_pos));
            items_buffer = {"true"};
        }
        if(name.find(parentSeparatorChar) == std::string::npos) {
            detail::remove_quotes(name);
        }
        // clean up quotes on the items
        for(auto &it : items_buffer) {
            detail::remove_quotes(it);
        }
        std::vector<std::string> parents;
        if(literalName) {
            std::string noname{};
            parents = detail::generate_parents(currentSection, noname, parentSeparatorChar);
        } else {
            parents = detail::generate_parents(currentSection, name, parentSeparatorChar);
        }
        if(parents.size() > maximumLayers) {
            continue;
        }
        if(!configSection.empty() && !inSection) {
            if(parents.empty() || parents.front() != configSection) {
                continue;
            }
            if(configIndex >= 0 && currentSectionIndex != configIndex) {
                continue;
            }
            parents.erase(parents.begin());
            inSection = true;
        }
        if(!output.empty() && name == output.back().name && parents == output.back().parents) {
            output.back().inputs.insert(output.back().inputs.end(), items_buffer.begin(), items_buffer.end());
        } else {
            output.emplace_back();
            output.back().parents = std::move(parents);
            output.back().name = std::move(name);
            output.back().inputs = std::move(items_buffer);
        }
    }
    if(currentSection != "default") {
        // insert a section end which is just an empty items_buffer
        std::string ename;
        output.emplace_back();
        output.back().parents = detail::generate_parents(currentSection, ename, parentSeparatorChar);
        output.back().name = "--";
        while(output.back().parents.size() > 1) {
            output.push_back(output.back());
            output.back().parents.pop_back();
        }
    }
    return output;
}

CLI11_INLINE std::string
ConfigBase::to_config(const App *app, bool default_also, bool write_description, std::string prefix) const {
    std::stringstream out;
    std::string commentLead;
    commentLead.push_back(commentChar);
    commentLead.push_back(' ');

    std::string commentTest = "#;";
    commentTest.push_back(commentChar);
    commentTest.push_back(parentSeparatorChar);

    std::vector<std::string> groups = app->get_groups();
    bool defaultUsed = false;
    groups.insert(groups.begin(), std::string("Options"));
    if(write_description && (app->get_configurable() || app->get_parent() == nullptr || app->get_name().empty())) {
        out << commentLead << detail::fix_newlines(commentLead, app->get_description()) << '\n';
    }
    for(auto &group : groups) {
        if(group == "Options" || group.empty()) {
            if(defaultUsed) {
                continue;
            }
            defaultUsed = true;
        }
        if(write_description && group != "Options" && !group.empty()) {
            out << '\n' << commentLead << group << " Options\n";
        }
        for(const Option *opt : app->get_options({})) {

            // Only process options that are configurable
            if(opt->get_configurable()) {
                if(opt->get_group() != group) {
                    if(!(group == "Options" && opt->get_group().empty())) {
                        continue;
                    }
                }
                std::string name = prefix + opt->get_single_name();
                std::string value = detail::ini_join(
                    opt->reduced_results(), arraySeparator, arrayStart, arrayEnd, stringQuote, characterQuote);

                if(value.empty() && default_also) {
                    if(!opt->get_default_str().empty()) {
                        value = detail::convert_arg_for_ini(opt->get_default_str(), stringQuote, characterQuote,false);
                    } else if(opt->get_expected_min() == 0) {
                        value = "false";
                    } else if(opt->get_run_callback_for_default()) {
                        value = "\"\"";  // empty string default value
                    }
                }

                if(!value.empty()) {
                    if(!opt->get_fnames().empty()) {
                        try {
                            value = opt->get_flag_value(name, value);
                        } catch(const CLI::ArgumentMismatch &) {
                            bool valid{false};
                            for(const auto &test_name : opt->get_fnames()) {
                                try {
                                    value = opt->get_flag_value(test_name, value);
                                    name = test_name;
                                    valid = true;
                                } catch(const CLI::ArgumentMismatch &) {
                                    continue;
                                }
                            }
                            if(!valid) {
                                value = detail::ini_join(
                                    opt->results(), arraySeparator, arrayStart, arrayEnd, stringQuote, characterQuote);
                            }
                        }
                    }
                    if(write_description && opt->has_description()) {
                        out << '\n';
                        out << commentLead << detail::fix_newlines(commentLead, opt->get_description()) << '\n';
                    }
                    if(name.find_first_of(commentTest) != std::string::npos || name.compare(0, 3, tquote) == 0 ||
                       name.compare(0, 3, "'''") == 0 || (name.front() == '[' && name.back() == ']')) {
                        out << commentChar << " cli11:literal\n";
                    }
                    out << name << valueDelimiter << value << '\n';
                }
            }
        }
    }
    auto subcommands = app->get_subcommands({});
    for(const App *subcom : subcommands) {
        if(subcom->get_name().empty()) {
            if(write_description && !subcom->get_group().empty()) {
                out << '\n' << commentLead << subcom->get_group() << " Options\n";
            }
            out << to_config(subcom, default_also, write_description, prefix);
        }
    }

    for(const App *subcom : subcommands) {
        if(!subcom->get_name().empty()) {
            if(subcom->get_configurable() && app->got_subcommand(subcom)) {
                if(!prefix.empty() || app->get_parent() == nullptr) {
                    out << '[' << prefix << subcom->get_name() << "]\n";
                } else {
                    std::string subname = app->get_name() + parentSeparatorChar + subcom->get_name();
                    const auto *p = app->get_parent();
                    while(p->get_parent() != nullptr) {
                        subname = p->get_name() + parentSeparatorChar + subname;
                        p = p->get_parent();
                    }
                    out << '[' << subname << "]\n";
                }
                out << to_config(subcom, default_also, write_description, "");
            } else {
                out << to_config(
                    subcom, default_also, write_description, prefix + subcom->get_name() + parentSeparatorChar);
            }
        }
    }

    return out.str();
}
// [CLI11:config_inl_hpp:end]
}  // namespace CLI
