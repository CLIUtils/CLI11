#pragma once

// Distributed under the 3-Clause BSD License.  See accompanying
// file LICENSE or https://github.com/CLIUtils/CLI11 for details.

#include <algorithm>
#include <fstream>
#include <iostream>
#include <string>

#include "CLI/App.hpp"
#include "CLI/ConfigFwd.hpp"
#include "CLI/StringTools.hpp"

namespace CLI {

inline std::string
ConfigINI::to_config(const App *app, bool default_also, bool write_description, std::string prefix) const {
    std::stringstream out;
    for(const Option *opt : app->get_options({})) {

        // Only process option with a long-name and configurable
        if(!opt->get_lnames().empty() && opt->get_configurable()) {
            std::string name = prefix + opt->get_lnames()[0];
            std::string value;

            // Non-flags
            if(opt->get_type_size() != 0) {

                // If the option was found on command line
                if(opt->count() > 0)
                    value = detail::ini_join(opt->results());

                // If the option has a default and is requested by optional argument
                else if(default_also && !opt->get_defaultval().empty())
                    value = opt->get_defaultval();
                // Flag, one passed
            } else if(opt->count() == 1) {
                value = "true";

                // Flag, multiple passed
            } else if(opt->count() > 1) {
                value = std::to_string(opt->count());

                // Flag, not present
            } else if(opt->count() == 0 && default_also) {
                value = "false";
            }

            if(!value.empty()) {
                if(write_description && opt->has_description()) {
                    if(static_cast<int>(out.tellp()) != 0) {
                        out << std::endl;
                    }
                    out << "; " << detail::fix_newlines("; ", opt->get_description()) << std::endl;
                }

                // Don't try to quote anything that is not size 1
                if(opt->get_items_expected() != 1)
                    out << name << "=" << value << std::endl;
                else
                    out << name << "=" << detail::add_quotes_if_needed(value) << std::endl;
            }
        }
    }

    for(const App *subcom : app->get_subcommands({}))
        out << to_config(subcom, default_also, write_description, prefix + subcom->get_name() + ".");

    return out.str();
}

} // namespace CLI
