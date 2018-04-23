#pragma once

// Distributed under the 3-Clause BSD License.  See accompanying
// file LICENSE or https://github.com/CLIUtils/CLI11 for details.

#include <string>
#include <map>

#include "CLI/StringTools.hpp"

namespace CLI {

class Option;
class App;

/// This enum signifies what situation the option is beig printed in.
///
/// This is passed in as part of the built in formatter in App; it is
/// possible that a custom App formatter could avoid using it, however.
enum class OptionFormatMode {
    Usage,      //< In the program usage line
    Positional, //< In the positionals
    Optional    //< In the normal optionals
};

/// This enum signifies the type of help requested
///
/// This is passed in by App; all user classes must accept this as
/// the second argument.

enum class AppFormatMode {
    Normal, //< The normal, detailed help
    All,    //< A fully expanded help
    Sub,    //<  Used when printed as part of expanded subcommand
};

/// This is an example formatter (and also the default formatter)
/// For option help.
class OptionFormatter {
  protected:
    /// @name Options
    ///@{

    /// @brief The required help printout labels (user changeable)
    /// Values are REQUIRED, NEEDS, EXCLUDES
    std::map<std::string, std::string> labels_{{"REQUIRED", "(REQUIRED)"}};

    /// The width of the first column
    size_t column_width_{30};

    ///@}
    /// @name Basic
    ///@{

  public:
    OptionFormatter() = default;
    OptionFormatter(const OptionFormatter &) = default;
    OptionFormatter(OptionFormatter &&) = default;

    ///@}
    /// @name Setters
    ///@{

    /// Set the "REQUIRED" label
    void label(std::string key, std::string val) { labels_[key] = val; }

    /// Set the column width
    void column_width(size_t val) { column_width_ = val; }

    ///@}
    /// @name Getters
    ///@{

    /// Get the current value of a name (REQUIRED, etc.)
    std::string get_label(std::string key) const {
        if(labels_.find(key) == labels_.end())
            return key;
        else
            return labels_.at(key);
    }

    /// Get the current column width
    size_t get_column_width() const { return column_width_; }

    ///@}
    /// @name Overridables
    ///@{

    /// @brief This is the name part of an option, Default: left column
    virtual std::string make_name(const Option *, OptionFormatMode) const;

    /// @brief This is the options part of the name, Default: combined into left column
    virtual std::string make_opts(const Option *) const;

    /// @brief This is the description. Default: Right column, on new line if left column too large
    virtual std::string make_desc(const Option *) const;

    /// @brief This is used to print the name on the USAGE line (by App formatter)
    virtual std::string make_usage(const Option *opt) const;

    /// @brief This is the standard help combiner that does the "default" thing.
    virtual std::string operator()(const Option *opt, OptionFormatMode mode) const {
        std::stringstream out;
        if(mode == OptionFormatMode::Usage)
            out << make_usage(opt);
        else
            detail::format_help(out, make_name(opt, mode) + make_opts(opt), make_desc(opt), column_width_);
        return out.str();
    }

    ///@}
};

class AppFormatter {
    /// @name Options
    ///@{

    /// The width of the first column
    size_t column_width_{30};

    /// @brief The required help printout labels (user changeable)
    /// Values are Needs, Excludes, etc.
    std::map<std::string, std::string> labels_;

    ///@}
    /// @name Basic
    ///@{

  public:
    AppFormatter() = default;
    AppFormatter(const AppFormatter &) = default;
    AppFormatter(AppFormatter &&) = default;
    ///@}
    /// @name Setters
    ///@{

    /// Set the "REQUIRED" label
    void label(std::string key, std::string val) { labels_[key] = val; }

    /// Set the column width
    void column_width(size_t val) { column_width_ = val; }

    ///@}
    /// @name Getters
    ///@{

    /// Get the current value of a name (REQUIRED, etc.)
    std::string get_label(std::string key) const {
        if(labels_.find(key) == labels_.end())
            return key;
        else
            return labels_.at(key);
    }

    /// Get the current column width
    size_t get_column_width() const { return column_width_; }

    /// @name Overridables
    ///@{

    /// This prints out a group of options
    virtual std::string make_group(std::string group, std::vector<const Option *> opts, OptionFormatMode mode) const;

    /// This prints out all the groups of options
    virtual std::string make_groups(const App *app, AppFormatMode mode) const;

    /// This prints out all the subcommands
    virtual std::string make_subcommands(const App *app, AppFormatMode mode) const;

    /// This prints out a subcommand
    virtual std::string make_subcommand(const App *sub) const;

    /// This prints out a subcommand in help-all
    virtual std::string make_expanded(const App *sub) const;

    /// This prints out all the groups of options
    virtual std::string make_footer(const App *app) const;

    /// This displays the description line
    virtual std::string make_description(const App *app) const;

    /// This displays the usage line
    virtual std::string make_usage(const App *app, std::string name) const;

    /// This puts everything together
    virtual std::string operator()(const App *, std::string, AppFormatMode) const;
    ///@}
};

} // namespace CLI
