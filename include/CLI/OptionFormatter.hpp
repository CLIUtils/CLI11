#pragma once

// Distributed under the 3-Clause BSD License.  See accompanying
// file LICENSE or https://github.com/CLIUtils/CLI11 for details.

#include <string>

#include "CLI/Option.hpp"
#include "CLI/Formatter.hpp"

namespace CLI {

inline std::string OptionFormatter::make_name(const Option *opt, OptionFormatMode mode) const {
    if(mode == OptionFormatMode::Optional)
        return opt->get_name(false, true);
    else
        return opt->get_name(true, false);
}

inline std::string OptionFormatter::make_opts(const Option *opt) const {
    std::stringstream out;

    if(opt->get_type_size() != 0) {
        if(!opt->get_typeval().empty())
            out << " " << get_label(opt->get_typeval());
        if(!opt->get_defaultval().empty())
            out << "=" << opt->get_defaultval();
        if(opt->get_expected() > 1)
            out << " x " << opt->get_expected();
        if(opt->get_expected() == -1)
            out << " ...";
        if(opt->get_required())
            out << " " << get_label("REQUIRED");
    }
    if(!opt->get_envname().empty())
        out << " (" << get_label("Env") << ":" << opt->get_envname() << ")";
    if(!opt->get_needs().empty()) {
        out << " " << get_label("Needs") << ":";
        for(const Option *op : opt->get_needs())
            out << " " << op->get_name();
    }
    if(!opt->get_excludes().empty()) {
        out << " " << get_label("Excludes") << ":";
        for(const Option *op : opt->get_excludes())
            out << " " << op->get_name();
    }
    return out.str();
}

inline std::string OptionFormatter::make_desc(const Option *opt) const { return opt->get_description(); }

inline std::string OptionFormatter::make_usage(const Option *opt) const {
    // Note that these are positionals usages
    std::stringstream out;
    out << make_name(opt, OptionFormatMode::Usage);

    if(opt->get_expected() > 1)
        out << "(" << std::to_string(opt->get_expected()) << "x)";
    else if(opt->get_expected() < 0)
        out << "...";

    return opt->get_required() ? out.str() : "[" + out.str() + "]";
}

} // namespace CLI
