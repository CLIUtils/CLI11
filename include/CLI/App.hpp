#pragma once

// Distributed under the 3-Clause BSD License.  See accompanying
// file LICENSE or https://github.com/CLIUtils/CLI11 for details.

#include <algorithm>
#include <deque>
#include <functional>
#include <iostream>
#include <iterator>
#include <memory>
#include <numeric>
#include <set>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

// CLI Library includes
#include "CLI/ConfigFwd.hpp"
#include "CLI/Error.hpp"
#include "CLI/FormatterFwd.hpp"
#include "CLI/Macros.hpp"
#include "CLI/Option.hpp"
#include "CLI/Split.hpp"
#include "CLI/StringTools.hpp"
#include "CLI/TypeTools.hpp"

namespace CLI {

#ifndef CLI11_PARSE
#define CLI11_PARSE(app, argc, argv)                                                                                   \
    try {                                                                                                              \
        (app).parse((argc), (argv));                                                                                   \
    } catch(const CLI::ParseError &e) {                                                                                \
        return (app).exit(e);                                                                                          \
    }
#endif

namespace detail {
enum class Classifer { NONE, POSITIONAL_MARK, SHORT, LONG, SUBCOMMAND };
struct AppFriend;
} // namespace detail

namespace FailureMessage {
std::string simple(const App *app, const Error &e);
std::string help(const App *app, const Error &e);
} // namespace FailureMessage

class App;

using App_p = std::unique_ptr<App>;

/// Creates a command line program, with very few defaults.
/** To use, create a new `Program()` instance with `argc`, `argv`, and a help description. The templated
 *  add_option methods make it easy to prepare options. Remember to call `.start` before starting your
 * program, so that the options can be evaluated and the help option doesn't accidentally run your program. */
class App {
    friend Option;
    friend detail::AppFriend;

  protected:
    // This library follows the Google style guide for member names ending in underscores

    /// @name Basics
    ///@{

    /// Subcommand name or program name (from parser if name is empty)
    std::string name_;

    /// Description of the current program/subcommand
    std::string description_;

    /// If true, allow extra arguments (ie, don't throw an error). INHERITABLE
    bool allow_extras_{false};

    /// If true, allow extra arguments in the ini file (ie, don't throw an error). INHERITABLE
    bool allow_config_extras_{false};

    ///  If true, return immediately on an unrecognised option (implies allow_extras) INHERITABLE
    bool prefix_command_{false};

    /// This is a function that runs when complete. Great for subcommands. Can throw.
    std::function<void()> callback_;

    ///@}
    /// @name Options
    ///@{

    /// The default values for options, customizable and changeable INHERITABLE
    OptionDefaults option_defaults_;

    /// The list of options, stored locally
    std::vector<Option_p> options_;

    ///@}
    /// @name Help
    ///@{

    /// Footer to put after all options in the help output INHERITABLE
    std::string footer_;

    /// A pointer to the help flag if there is one INHERITABLE
    Option *help_ptr_{nullptr};

    /// A pointer to the help all flag if there is one INHERITABLE
    Option *help_all_ptr_{nullptr};

    /// This is the formatter for help printing. Default provided. INHERITABLE (same pointer)
    std::shared_ptr<FormatterBase> formatter_{new Formatter()};

    /// The error message printing function INHERITABLE
    std::function<std::string(const App *, const Error &e)> failure_message_ = FailureMessage::simple;

    ///@}
    /// @name Parsing
    ///@{

    using missing_t = std::vector<std::pair<detail::Classifer, std::string>>;

    /// Pair of classifier, string for missing options. (extra detail is removed on returning from parse)
    ///
    /// This is faster and cleaner than storing just a list of strings and reparsing. This may contain the -- separator.
    missing_t missing_;

    /// This is a list of pointers to options with the original parse order
    std::vector<Option *> parse_order_;

    /// This is a list of the subcommands collected, in order
    std::vector<App *> parsed_subcommands_;

    ///@}
    /// @name Subcommands
    ///@{

    /// Storage for subcommand list
    std::vector<App_p> subcommands_;

    /// If true, the program name is not case sensitive INHERITABLE
    bool ignore_case_{false};

    /// Allow subcommand fallthrough, so that parent commands can collect commands after subcommand.  INHERITABLE
    bool fallthrough_{false};

    /// A pointer to the parent if this is a subcommand
    App *parent_{nullptr};

    /// True if this command/subcommand was parsed
    bool parsed_{false};

    /// Minimum required subcommands (not inheritable!)
    size_t require_subcommand_min_ = 0;

    /// Max number of subcommands allowed (parsing stops after this number). 0 is unlimited INHERITABLE
    size_t require_subcommand_max_ = 0;

    /// The group membership INHERITABLE
    std::string group_{"Subcommands"};

    ///@}
    /// @name Config
    ///@{

    /// The name of the connected config file
    std::string config_name_;

    /// True if ini is required (throws if not present), if false simply keep going.
    bool config_required_{false};

    /// Pointer to the config option
    Option *config_ptr_{nullptr};

    /// This is the formatter for help printing. Default provided. INHERITABLE (same pointer)
    std::shared_ptr<Config> config_formatter_{new ConfigINI()};

    ///@}

    /// Special private constructor for subcommand
    App(std::string description_, std::string name, App *parent)
        : name_(std::move(name)), description_(std::move(description_)), parent_(parent) {
        // Inherit if not from a nullptr
        if(parent_ != nullptr) {
            if(parent_->help_ptr_ != nullptr)
                set_help_flag(parent_->help_ptr_->get_name(false, true), parent_->help_ptr_->get_description());
            if(parent_->help_all_ptr_ != nullptr)
                set_help_all_flag(parent_->help_all_ptr_->get_name(false, true),
                                  parent_->help_all_ptr_->get_description());

            /// OptionDefaults
            option_defaults_ = parent_->option_defaults_;

            // INHERITABLE
            failure_message_ = parent_->failure_message_;
            allow_extras_ = parent_->allow_extras_;
            allow_config_extras_ = parent_->allow_config_extras_;
            prefix_command_ = parent_->prefix_command_;
            ignore_case_ = parent_->ignore_case_;
            fallthrough_ = parent_->fallthrough_;
            group_ = parent_->group_;
            footer_ = parent_->footer_;
            formatter_ = parent_->formatter_;
            config_formatter_ = parent_->config_formatter_;
            require_subcommand_max_ = parent_->require_subcommand_max_;
        }
    }

  public:
    /// @name Basic
    ///@{

    /// Create a new program. Pass in the same arguments as main(), along with a help string.
    explicit App(std::string description_ = "", std::string name = "") : App(description_, name, nullptr) {
        set_help_flag("-h,--help", "Print this help message and exit");
    }

    /// virtual destructor
    virtual ~App() = default;

    /// Set a callback for the end of parsing.
    ///
    /// Due to a bug in c++11,
    /// it is not possible to overload on std::function (fixed in c++14
    /// and backported to c++11 on newer compilers). Use capture by reference
    /// to get a pointer to App if needed.
    App *callback(std::function<void()> callback) {
        callback_ = callback;
        return this;
    }

    /// Set a name for the app (empty will use parser to set the name)
    App *name(std::string name = "") {
        name_ = name;
        return this;
    }

    /// Remove the error when extras are left over on the command line.
    App *allow_extras(bool allow = true) {
        allow_extras_ = allow;
        return this;
    }

    /// Remove the error when extras are left over on the command line.
    /// Will also call App::allow_extras().
    App *allow_config_extras(bool allow = true) {
        allow_extras(allow);
        allow_config_extras_ = allow;
        return this;
    }

    /// Do not parse anything after the first unrecognised option and return
    App *prefix_command(bool allow = true) {
        prefix_command_ = allow;
        return this;
    }

    /// Ignore case. Subcommand inherit value.
    App *ignore_case(bool value = true) {
        ignore_case_ = value;
        if(parent_ != nullptr) {
            for(const auto &subc : parent_->subcommands_) {
                if(subc.get() != this && (this->check_name(subc->name_) || subc->check_name(this->name_)))
                    throw OptionAlreadyAdded(subc->name_);
            }
        }
        return this;
    }

    /// Set the help formatter
    App *formatter(std::shared_ptr<FormatterBase> fmt) {
        formatter_ = fmt;
        return this;
    }

    /// Set the help formatter
    App *formatter_fn(std::function<std::string(const App *, std::string, AppFormatMode)> fmt) {
        formatter_ = std::make_shared<FormatterLambda>(fmt);
        return this;
    }

    /// Set the config formatter
    App *config_formatter(std::shared_ptr<Config> fmt) {
        config_formatter_ = fmt;
        return this;
    }

    /// Check to see if this subcommand was parsed, true only if received on command line.
    bool parsed() const { return parsed_; }

    /// Get the OptionDefault object, to set option defaults
    OptionDefaults *option_defaults() { return &option_defaults_; }

    ///@}
    /// @name Adding options
    ///@{

    /// Add an option, will automatically understand the type for common types.
    ///
    /// To use, create a variable with the expected type, and pass it in after the name.
    /// After start is called, you can use count to see if the value was passed, and
    /// the value will be initialized properly. Numbers, vectors, and strings are supported.
    ///
    /// ->required(), ->default, and the validators are options,
    /// The positional options take an optional number of arguments.
    ///
    /// For example,
    ///
    ///     std::string filename;
    ///     program.add_option("filename", filename, "description of filename");
    ///
    Option *add_option(std::string name, callback_t callback, std::string description = "", bool defaulted = false) {
        Option myopt{name, description, callback, defaulted, this};

        if(std::find_if(std::begin(options_), std::end(options_), [&myopt](const Option_p &v) {
               return *v == myopt;
           }) == std::end(options_)) {
            options_.emplace_back();
            Option_p &option = options_.back();
            option.reset(new Option(name, description, callback, defaulted, this));
            option_defaults_.copy_to(option.get());
            return option.get();
        } else
            throw OptionAlreadyAdded(myopt.get_name());
    }

    /// Add option for non-vectors (duplicate copy needed without defaulted to avoid `iostream << value`)
    template <typename T, enable_if_t<!is_vector<T>::value, detail::enabler> = detail::dummy>
    Option *add_option(std::string name,
                       T &variable, ///< The variable to set
                       std::string description = "") {

        CLI::callback_t fun = [&variable](CLI::results_t res) { return detail::lexical_cast(res[0], variable); };

        Option *opt = add_option(name, fun, description, false);
        opt->type_name(detail::type_name<T>());
        return opt;
    }

    /// Add option for non-vectors with a default print
    template <typename T, enable_if_t<!is_vector<T>::value, detail::enabler> = detail::dummy>
    Option *add_option(std::string name,
                       T &variable, ///< The variable to set
                       std::string description,
                       bool defaulted) {

        CLI::callback_t fun = [&variable](CLI::results_t res) { return detail::lexical_cast(res[0], variable); };

        Option *opt = add_option(name, fun, description, defaulted);
        opt->type_name(detail::type_name<T>());
        if(defaulted) {
            std::stringstream out;
            out << variable;
            opt->default_str(out.str());
        }
        return opt;
    }

    /// Add option for vectors (no default)
    template <typename T>
    Option *add_option(std::string name,
                       std::vector<T> &variable, ///< The variable vector to set
                       std::string description = "") {

        CLI::callback_t fun = [&variable](CLI::results_t res) {
            bool retval = true;
            variable.clear();
            for(const auto &a : res) {
                variable.emplace_back();
                retval &= detail::lexical_cast(a, variable.back());
            }
            return (!variable.empty()) && retval;
        };

        Option *opt = add_option(name, fun, description, false);
        opt->type_name(detail::type_name<T>())->type_size(-1);
        return opt;
    }

    /// Add option for vectors
    template <typename T>
    Option *add_option(std::string name,
                       std::vector<T> &variable, ///< The variable vector to set
                       std::string description,
                       bool defaulted) {

        CLI::callback_t fun = [&variable](CLI::results_t res) {
            bool retval = true;
            variable.clear();
            for(const auto &a : res) {
                variable.emplace_back();
                retval &= detail::lexical_cast(a, variable.back());
            }
            return (!variable.empty()) && retval;
        };

        Option *opt = add_option(name, fun, description, defaulted);
        opt->type_name(detail::type_name<T>())->type_size(-1);
        if(defaulted)
            opt->default_str("[" + detail::join(variable) + "]");
        return opt;
    }

    /// Set a help flag, replace the existing one if present
    Option *set_help_flag(std::string name = "", std::string description = "") {
        if(help_ptr_ != nullptr) {
            remove_option(help_ptr_);
            help_ptr_ = nullptr;
        }

        // Empty name will simply remove the help flag
        if(!name.empty()) {
            help_ptr_ = add_flag_function(name, [](size_t) -> void { throw CallForHelp(); }, description);
            help_ptr_->short_circuit(true);
            help_ptr_->configurable(false);
        }

        return help_ptr_;
    }

    /// Set a help all flag, replaced the existing one if present
    Option *set_help_all_flag(std::string name = "", std::string description = "") {
        if(help_all_ptr_ != nullptr) {
            remove_option(help_all_ptr_);
            help_all_ptr_ = nullptr;
        }

        // Empty name will simply remove the help all flag
        if(!name.empty()) {
            help_all_ptr_ = add_flag_function(name, [](size_t) -> void { throw CallForAllHelp(); }, description);
            help_all_ptr_->short_circuit(true);
            help_all_ptr_->configurable(false);
        }

        return help_all_ptr_;
    }

    /// Add option for flag
    Option *add_flag(std::string name, std::string description = "") {
        CLI::callback_t fun = [](CLI::results_t) { return true; };

        Option *opt = add_option(name, fun, description, false);
        if(opt->get_positional())
            throw IncorrectConstruction::PositionalFlag(name);
        opt->type_size(0);
        return opt;
    }

    /// Add option for flag integer
    template <typename T,
              enable_if_t<std::is_integral<T>::value && !is_bool<T>::value, detail::enabler> = detail::dummy>
    Option *add_flag(std::string name,
                     T &count, ///< A variable holding the count
                     std::string description = "") {

        count = 0;
        CLI::callback_t fun = [&count](CLI::results_t res) {
            count = static_cast<T>(res.size());
            return true;
        };

        Option *opt = add_option(name, fun, description, false);
        if(opt->get_positional())
            throw IncorrectConstruction::PositionalFlag(name);
        opt->type_size(0);
        return opt;
    }

    /// Bool version - defaults to allowing multiple passings, but can be forced to one if
    /// `multi_option_policy(CLI::MultiOptionPolicy::Throw)` is used.
    template <typename T, enable_if_t<is_bool<T>::value, detail::enabler> = detail::dummy>
    Option *add_flag(std::string name,
                     T &count, ///< A variable holding true if passed
                     std::string description = "") {

        count = false;
        CLI::callback_t fun = [&count](CLI::results_t res) {
            count = true;
            return res.size() == 1;
        };

        Option *opt = add_option(name, fun, description, false);
        if(opt->get_positional())
            throw IncorrectConstruction::PositionalFlag(name);
        opt->type_size(0);
        opt->multi_option_policy(CLI::MultiOptionPolicy::TakeLast);
        return opt;
    }

    /// Add option for callback
    Option *add_flag_function(std::string name,
                              std::function<void(size_t)> function, ///< A function to call, void(size_t)
                              std::string description = "") {

        CLI::callback_t fun = [function](CLI::results_t res) {
            auto count = static_cast<size_t>(res.size());
            function(count);
            return true;
        };

        Option *opt = add_option(name, fun, description, false);
        if(opt->get_positional())
            throw IncorrectConstruction::PositionalFlag(name);
        opt->type_size(0);
        return opt;
    }

#ifdef CLI11_CPP14
    /// Add option for callback (C++14 or better only)
    Option *add_flag(std::string name,
                     std::function<void(size_t)> function, ///< A function to call, void(size_t)
                     std::string description = "") {
        return add_flag_function(name, function, description);
    }
#endif

    /// Add set of options (No default, temp refernce, such as an inline set)
    template <typename T>
    Option *add_set(std::string name,
                    T &member,                   ///< The selected member of the set
                    const std::set<T> &&options, ///< The set of possibilities
                    std::string description = "") {

        std::string simple_name = CLI::detail::split(name, ',').at(0);
        CLI::callback_t fun = [&member, options, simple_name](CLI::results_t res) {
            bool retval = detail::lexical_cast(res[0], member);
            if(!retval)
                throw ConversionError(res[0], simple_name);
            return std::find(std::begin(options), std::end(options), member) != std::end(options);
        };

        Option *opt = add_option(name, fun, description, false);
        std::string typeval = detail::type_name<T>();
        typeval += " in {" + detail::join(options) + "}";
        opt->type_name(typeval);
        return opt;
    }

    /// Add set of options (No default, non-temp refernce, such as an existing set)
    template <typename T>
    Option *add_set(std::string name,
                    T &member,                  ///< The selected member of the set
                    const std::set<T> &options, ///< The set of possibilities
                    std::string description = "") {

        std::string simple_name = CLI::detail::split(name, ',').at(0);
        CLI::callback_t fun = [&member, &options, simple_name](CLI::results_t res) {
            bool retval = detail::lexical_cast(res[0], member);
            if(!retval)
                throw ConversionError(res[0], simple_name);
            return std::find(std::begin(options), std::end(options), member) != std::end(options);
        };

        Option *opt = add_option(name, fun, description, false);
        opt->type_name_fn(
            [&options]() { return std::string(detail::type_name<T>()) + " in {" + detail::join(options) + "}"; });

        return opt;
    }

    /// Add set of options (with default, R value, such as an inline set)
    template <typename T>
    Option *add_set(std::string name,
                    T &member,                   ///< The selected member of the set
                    const std::set<T> &&options, ///< The set of posibilities
                    std::string description,
                    bool defaulted) {

        std::string simple_name = CLI::detail::split(name, ',').at(0);
        CLI::callback_t fun = [&member, options, simple_name](CLI::results_t res) {
            bool retval = detail::lexical_cast(res[0], member);
            if(!retval)
                throw ConversionError(res[0], simple_name);
            return std::find(std::begin(options), std::end(options), member) != std::end(options);
        };

        Option *opt = add_option(name, fun, description, defaulted);
        std::string typeval = detail::type_name<T>();
        typeval += " in {" + detail::join(options) + "}";
        opt->type_name(typeval);
        if(defaulted) {
            std::stringstream out;
            out << member;
            opt->default_str(out.str());
        }
        return opt;
    }

    /// Add set of options (with default, L value refernce, such as an existing set)
    template <typename T>
    Option *add_set(std::string name,
                    T &member,                  ///< The selected member of the set
                    const std::set<T> &options, ///< The set of posibilities
                    std::string description,
                    bool defaulted) {

        std::string simple_name = CLI::detail::split(name, ',').at(0);
        CLI::callback_t fun = [&member, &options, simple_name](CLI::results_t res) {
            bool retval = detail::lexical_cast(res[0], member);
            if(!retval)
                throw ConversionError(res[0], simple_name);
            return std::find(std::begin(options), std::end(options), member) != std::end(options);
        };

        Option *opt = add_option(name, fun, description, defaulted);
        opt->type_name_fn(
            [&options]() { return std::string(detail::type_name<T>()) + " in {" + detail::join(options) + "}"; });
        if(defaulted) {
            std::stringstream out;
            out << member;
            opt->default_str(out.str());
        }
        return opt;
    }

    /// Add set of options, string only, ignore case (no default, R value)
    Option *add_set_ignore_case(std::string name,
                                std::string &member,                   ///< The selected member of the set
                                const std::set<std::string> &&options, ///< The set of possibilities
                                std::string description = "") {

        std::string simple_name = CLI::detail::split(name, ',').at(0);
        CLI::callback_t fun = [&member, options, simple_name](CLI::results_t res) {
            member = detail::to_lower(res[0]);
            auto iter = std::find_if(std::begin(options), std::end(options), [&member](std::string val) {
                return detail::to_lower(val) == member;
            });
            if(iter == std::end(options))
                throw ConversionError(member, simple_name);
            else {
                member = *iter;
                return true;
            }
        };

        Option *opt = add_option(name, fun, description, false);
        std::string typeval = detail::type_name<std::string>();
        typeval += " in {" + detail::join(options) + "}";
        opt->type_name(typeval);

        return opt;
    }

    /// Add set of options, string only, ignore case (no default, L value)
    Option *add_set_ignore_case(std::string name,
                                std::string &member,                  ///< The selected member of the set
                                const std::set<std::string> &options, ///< The set of possibilities
                                std::string description = "") {

        std::string simple_name = CLI::detail::split(name, ',').at(0);
        CLI::callback_t fun = [&member, &options, simple_name](CLI::results_t res) {
            member = detail::to_lower(res[0]);
            auto iter = std::find_if(std::begin(options), std::end(options), [&member](std::string val) {
                return detail::to_lower(val) == member;
            });
            if(iter == std::end(options))
                throw ConversionError(member, simple_name);
            else {
                member = *iter;
                return true;
            }
        };

        Option *opt = add_option(name, fun, description, false);
        opt->type_name_fn([&options]() {
            return std::string(detail::type_name<std::string>()) + " in {" + detail::join(options) + "}";
        });

        return opt;
    }

    /// Add set of options, string only, ignore case (default, R value)
    Option *add_set_ignore_case(std::string name,
                                std::string &member,                   ///< The selected member of the set
                                const std::set<std::string> &&options, ///< The set of posibilities
                                std::string description,
                                bool defaulted) {

        std::string simple_name = CLI::detail::split(name, ',').at(0);
        CLI::callback_t fun = [&member, options, simple_name](CLI::results_t res) {
            member = detail::to_lower(res[0]);
            auto iter = std::find_if(std::begin(options), std::end(options), [&member](std::string val) {
                return detail::to_lower(val) == member;
            });
            if(iter == std::end(options))
                throw ConversionError(member, simple_name);
            else {
                member = *iter;
                return true;
            }
        };

        Option *opt = add_option(name, fun, description, defaulted);
        std::string typeval = detail::type_name<std::string>();
        typeval += " in {" + detail::join(options) + "}";
        opt->type_name(typeval);
        if(defaulted) {
            opt->default_str(member);
        }
        return opt;
    }

    /// Add set of options, string only, ignore case (default, L value)
    Option *add_set_ignore_case(std::string name,
                                std::string &member,                  ///< The selected member of the set
                                const std::set<std::string> &options, ///< The set of posibilities
                                std::string description,
                                bool defaulted) {

        std::string simple_name = CLI::detail::split(name, ',').at(0);
        CLI::callback_t fun = [&member, &options, simple_name](CLI::results_t res) {
            member = detail::to_lower(res[0]);
            auto iter = std::find_if(std::begin(options), std::end(options), [&member](std::string val) {
                return detail::to_lower(val) == member;
            });
            if(iter == std::end(options))
                throw ConversionError(member, simple_name);
            else {
                member = *iter;
                return true;
            }
        };

        Option *opt = add_option(name, fun, description, defaulted);
        opt->type_name_fn([&options]() {
            return std::string(detail::type_name<std::string>()) + " in {" + detail::join(options) + "}";
        });
        if(defaulted) {
            opt->default_str(member);
        }
        return opt;
    }

    /// Add a complex number
    template <typename T>
    Option *add_complex(std::string name,
                        T &variable,
                        std::string description = "",
                        bool defaulted = false,
                        std::string label = "COMPLEX") {

        std::string simple_name = CLI::detail::split(name, ',').at(0);
        CLI::callback_t fun = [&variable, simple_name, label](results_t res) {
            if(res[1].back() == 'i')
                res[1].pop_back();
            double x, y;
            bool worked = detail::lexical_cast(res[0], x) && detail::lexical_cast(res[1], y);
            if(worked)
                variable = T(x, y);
            return worked;
        };

        CLI::Option *opt = add_option(name, fun, description, defaulted);
        opt->type_name(label)->type_size(2);
        if(defaulted) {
            std::stringstream out;
            out << variable;
            opt->default_str(out.str());
        }
        return opt;
    }

    /// Set a configuration ini file option, or clear it if no name passed
    Option *set_config(std::string name = "",
                       std::string default_filename = "",
                       std::string help = "Read an ini file",
                       bool required = false) {

        // Remove existing config if present
        if(config_ptr_ != nullptr)
            remove_option(config_ptr_);

        // Only add config if option passed
        if(!name.empty()) {
            config_name_ = default_filename;
            config_required_ = required;
            config_ptr_ = add_option(name, config_name_, help, !default_filename.empty());
            config_ptr_->configurable(false);
        }

        return config_ptr_;
    }

    /// Removes an option from the App. Takes an option pointer. Returns true if found and removed.
    bool remove_option(Option *opt) {
        // Make sure no links exist
        for(Option_p &op : options_) {
            op->remove_needs(opt);
            op->remove_excludes(opt);
        }

        auto iterator =
            std::find_if(std::begin(options_), std::end(options_), [opt](const Option_p &v) { return v.get() == opt; });
        if(iterator != std::end(options_)) {
            options_.erase(iterator);
            return true;
        }
        return false;
    }

    ///@}
    /// @name Subcommmands
    ///@{

    /// Add a subcommand. Inherits INHERITABLE and OptionDefaults, and help flag
    App *add_subcommand(std::string name, std::string description = "") {
        subcommands_.emplace_back(new App(description, name, this));
        for(const auto &subc : subcommands_)
            if(subc.get() != subcommands_.back().get())
                if(subc->check_name(subcommands_.back()->name_) || subcommands_.back()->check_name(subc->name_))
                    throw OptionAlreadyAdded(subc->name_);
        return subcommands_.back().get();
    }

    /// Check to see if a subcommand is part of this command (doesn't have to be in command line)
    App *get_subcommand(App *subcom) const {
        for(const App_p &subcomptr : subcommands_)
            if(subcomptr.get() == subcom)
                return subcom;
        throw OptionNotFound(subcom->get_name());
    }

    /// Check to see if a subcommand is part of this command (text version)
    App *get_subcommand(std::string subcom) const {
        for(const App_p &subcomptr : subcommands_)
            if(subcomptr->check_name(subcom))
                return subcomptr.get();
        throw OptionNotFound(subcom);
    }

    /// Changes the group membership
    App *group(std::string name) {
        group_ = name;
        return this;
    }

    /// The argumentless form of require subcommand requires 1 or more subcommands
    App *require_subcommand() {
        require_subcommand_min_ = 1;
        require_subcommand_max_ = 0;
        return this;
    }

    /// Require a subcommand to be given (does not affect help call)
    /// The number required can be given. Negative values indicate maximum
    /// number allowed (0 for any number). Max number inheritable.
    App *require_subcommand(int value) {
        if(value < 0) {
            require_subcommand_min_ = 0;
            require_subcommand_max_ = static_cast<size_t>(-value);
        } else {
            require_subcommand_min_ = static_cast<size_t>(value);
            require_subcommand_max_ = static_cast<size_t>(value);
        }
        return this;
    }

    /// Explicitly control the number of subcommands required. Setting 0
    /// for the max means unlimited number allowed. Max number inheritable.
    App *require_subcommand(size_t min, size_t max) {
        require_subcommand_min_ = min;
        require_subcommand_max_ = max;
        return this;
    }

    /// Stop subcommand fallthrough, so that parent commands cannot collect commands after subcommand.
    /// Default from parent, usually set on parent.
    App *fallthrough(bool value = true) {
        fallthrough_ = value;
        return this;
    }

    /// Check to see if this subcommand was parsed, true only if received on command line.
    /// This allows the subcommand to be directly checked.
    operator bool() const { return parsed_; }

    ///@}
    /// @name Extras for subclassing
    ///@{

    /// This allows subclasses to inject code before callbacks but after parse.
    ///
    /// This does not run if any errors or help is thrown.
    virtual void pre_callback() {}

    ///@}
    /// @name Parsing
    ///@{
    //
    /// Reset the parsed data
    void clear() {

        parsed_ = false;
        missing_.clear();
        parsed_subcommands_.clear();

        for(const Option_p &opt : options_) {
            opt->clear();
        }
        for(const App_p &app : subcommands_) {
            app->clear();
        }
    }

    /// Parses the command line - throws errors
    /// This must be called after the options are in but before the rest of the program.
    void parse(int argc, const char *const *argv) {
        // If the name is not set, read from command line
        if(name_.empty())
            name_ = argv[0];

        std::vector<std::string> args;
        for(int i = argc - 1; i > 0; i--)
            args.emplace_back(argv[i]);
        parse(args);
    }

    /// The real work is done here. Expects a reversed vector.
    /// Changes the vector to the remaining options.
    void parse(std::vector<std::string> &args) {
        // Clear if parsed
        if(parsed_)
            clear();

        // Redundant (set by _parse on commands/subcommands)
        // but placed here to make sure this is cleared when
        // running parse after an error is thrown, even by _validate.
        parsed_ = true;

        _validate();
        _parse(args);
        run_callback();
    }

    /// Provide a function to print a help message. The function gets access to the App pointer and error.
    void failure_message(std::function<std::string(const App *, const Error &e)> function) {
        failure_message_ = function;
    }

    /// Print a nice error message and return the exit code
    int exit(const Error &e, std::ostream &out = std::cout, std::ostream &err = std::cerr) const {

        /// Avoid printing anything if this is a CLI::RuntimeError
        if(dynamic_cast<const CLI::RuntimeError *>(&e) != nullptr)
            return e.get_exit_code();

        if(dynamic_cast<const CLI::CallForHelp *>(&e) != nullptr) {
            out << help();
            return e.get_exit_code();
        }

        if(dynamic_cast<const CLI::CallForAllHelp *>(&e) != nullptr) {
            out << help("", AppFormatMode::All);
            return e.get_exit_code();
        }

        if(e.get_exit_code() != static_cast<int>(ExitCodes::Success)) {
            if(failure_message_)
                err << failure_message_(this, e) << std::flush;
        }

        return e.get_exit_code();
    }

    ///@}
    /// @name Post parsing
    ///@{

    /// Counts the number of times the given option was passed.
    size_t count(std::string name) const {
        for(const Option_p &opt : options_) {
            if(opt->check_name(name)) {
                return opt->count();
            }
        }
        throw OptionNotFound(name);
    }

    /// Get a subcommand pointer list to the currently selected subcommands (after parsing by by default, in command
    /// line order; use parsed = false to get the original definition list.)
    std::vector<App *> get_subcommands() const { return parsed_subcommands_; }

    /// Get a filtered subcommand pointer list from the original definition list. An empty function will provide all
    /// subcommands (const)
    std::vector<const App *> get_subcommands(const std::function<bool(const App *)> &filter) const {
        std::vector<const App *> subcomms(subcommands_.size());
        std::transform(std::begin(subcommands_), std::end(subcommands_), std::begin(subcomms), [](const App_p &v) {
            return v.get();
        });

        if(filter) {
            subcomms.erase(std::remove_if(std::begin(subcomms),
                                          std::end(subcomms),
                                          [&filter](const App *app) { return !filter(app); }),
                           std::end(subcomms));
        }

        return subcomms;
    }

    /// Get a filtered subcommand pointer list from the original definition list. An empty function will provide all
    /// subcommands
    std::vector<App *> get_subcommands(const std::function<bool(App *)> &filter) {
        std::vector<App *> subcomms(subcommands_.size());
        std::transform(std::begin(subcommands_), std::end(subcommands_), std::begin(subcomms), [](const App_p &v) {
            return v.get();
        });

        if(filter) {
            subcomms.erase(
                std::remove_if(std::begin(subcomms), std::end(subcomms), [&filter](App *app) { return !filter(app); }),
                std::end(subcomms));
        }

        return subcomms;
    }

    /// Check to see if given subcommand was selected
    bool got_subcommand(App *subcom) const {
        // get subcom needed to verify that this was a real subcommand
        return get_subcommand(subcom)->parsed_;
    }

    /// Check with name instead of pointer to see if subcommand was selected
    bool got_subcommand(std::string name) const { return get_subcommand(name)->parsed_; }

    ///@}
    /// @name Help
    ///@{

    /// Set footer.
    App *footer(std::string footer) {
        footer_ = footer;
        return this;
    }

    /// Produce a string that could be read in as a config of the current values of the App. Set default_also to include
    /// default arguments. Prefix will add a string to the beginning of each option.
    std::string config_to_str(bool default_also = false, bool write_description = false) const {
        return config_formatter_->to_config(this, default_also, write_description, "");
    }

    /// Makes a help message, using the currently configured formatter
    /// Will only do one subcommand at a time
    std::string help(std::string prev = "", AppFormatMode mode = AppFormatMode::Normal) const {
        if(prev.empty())
            prev = get_name();
        else
            prev += " " + get_name();

        // Delegate to subcommand if needed
        auto selected_subcommands = get_subcommands();
        if(!selected_subcommands.empty())
            return selected_subcommands.at(0)->help(prev, mode);
        else
            return formatter_->make_help(this, prev, mode);
    }

    /// Provided for backwards compatibility \deprecated
    CLI11_DEPRECATED("Please use footer instead")
    App *set_footer(std::string msg) { return footer(msg); }

    /// Provided for backwards compatibility \deprecated
    CLI11_DEPRECATED("Please use name instead")
    App *set_name(std::string msg) { return name(msg); }

    /// Provided for backwards compatibility \deprecated
    CLI11_DEPRECATED("Please use callback instead")
    App *set_callback(std::function<void()> fn) { return callback(fn); }

    ///@}
    /// @name Getters
    ///@{

    /// Access the formatter
    std::shared_ptr<FormatterBase> get_formatter() const { return formatter_; }

    /// Access the config formatter
    std::shared_ptr<Config> get_config_formatter() const { return config_formatter_; }

    /// Get the app or subcommand description
    std::string get_description() const { return description_; }

    /// Get the list of options (user facing function, so returns raw pointers), has optional filter function
    std::vector<const Option *> get_options(const std::function<bool(const Option *)> filter = {}) const {
        std::vector<const Option *> options(options_.size());
        std::transform(std::begin(options_), std::end(options_), std::begin(options), [](const Option_p &val) {
            return val.get();
        });

        if(filter) {
            options.erase(std::remove_if(std::begin(options),
                                         std::end(options),
                                         [&filter](const Option *opt) { return !filter(opt); }),
                          std::end(options));
        }

        return options;
    }

    /// Get an option by name
    const Option *get_option(std::string name) const {
        for(const Option_p &opt : options_) {
            if(opt->check_name(name)) {
                return opt.get();
            }
        }
        throw OptionNotFound(name);
    }

    /// Get an option by name (non-const version)
    Option *get_option(std::string name) {
        for(Option_p &opt : options_) {
            if(opt->check_name(name)) {
                return opt.get();
            }
        }
        throw OptionNotFound(name);
    }

    /// Check the status of ignore_case
    bool get_ignore_case() const { return ignore_case_; }

    /// Check the status of fallthrough
    bool get_fallthrough() const { return fallthrough_; }

    /// Get the group of this subcommand
    const std::string &get_group() const { return group_; }

    /// Get footer.
    std::string get_footer() const { return footer_; }

    /// Get the required min subcommand value
    size_t get_require_subcommand_min() const { return require_subcommand_min_; }

    /// Get the required max subcommand value
    size_t get_require_subcommand_max() const { return require_subcommand_max_; }

    /// Get the prefix command status
    bool get_prefix_command() const { return prefix_command_; }

    /// Get the status of allow extras
    bool get_allow_extras() const { return allow_extras_; }

    /// Get the status of allow extras
    bool get_allow_config_extras() const { return allow_config_extras_; }

    /// Get a pointer to the help flag.
    Option *get_help_ptr() { return help_ptr_; }

    /// Get a pointer to the help flag. (const)
    const Option *get_help_ptr() const { return help_ptr_; }

    /// Get a pointer to the help all flag. (const)
    const Option *get_help_all_ptr() const { return help_all_ptr_; }

    /// Get a pointer to the config option.
    Option *get_config_ptr() { return config_ptr_; }

    /// Get a pointer to the config option. (const)
    const Option *get_config_ptr() const { return config_ptr_; }

    /// Get the parent of this subcommand (or nullptr if master app)
    App *get_parent() { return parent_; }

    /// Get the parent of this subcommand (or nullptr if master app) (const version)
    const App *get_parent() const { return parent_; }

    /// Get the name of the current app
    std::string get_name() const { return name_; }

    /// Check the name, case insensitive if set
    bool check_name(std::string name_to_check) const {
        std::string local_name = name_;
        if(ignore_case_) {
            local_name = detail::to_lower(name_);
            name_to_check = detail::to_lower(name_to_check);
        }

        return local_name == name_to_check;
    }

    /// Get the groups available directly from this option (in order)
    std::vector<std::string> get_groups() const {
        std::vector<std::string> groups;

        for(const Option_p &opt : options_) {
            // Add group if it is not already in there
            if(std::find(groups.begin(), groups.end(), opt->get_group()) == groups.end()) {
                groups.push_back(opt->get_group());
            }
        }

        return groups;
    }

    /// This gets a vector of pointers with the original parse order
    const std::vector<Option *> &parse_order() const { return parse_order_; }

    /// This returns the missing options from the current subcommand
    std::vector<std::string> remaining(bool recurse = false) const {
        std::vector<std::string> miss_list;
        for(const std::pair<detail::Classifer, std::string> &miss : missing_) {
            miss_list.push_back(std::get<1>(miss));
        }

        // Recurse into subcommands
        if(recurse) {
            for(const App *sub : parsed_subcommands_) {
                std::vector<std::string> output = sub->remaining(recurse);
                std::copy(std::begin(output), std::end(output), std::back_inserter(miss_list));
            }
        }
        return miss_list;
    }

    /// This returns the number of remaining options, minus the -- seperator
    size_t remaining_size(bool recurse = false) const {
        auto count = static_cast<size_t>(std::count_if(
            std::begin(missing_), std::end(missing_), [](const std::pair<detail::Classifer, std::string> &val) {
                return val.first != detail::Classifer::POSITIONAL_MARK;
            }));
        if(recurse) {
            for(const App_p &sub : subcommands_) {
                count += sub->remaining_size(recurse);
            }
        }
        return count;
    }

    ///@}

  protected:
    /// Check the options to make sure there are no conflicts.
    ///
    /// Currently checks to see if multiple positionals exist with -1 args
    void _validate() const {
        auto count = std::count_if(std::begin(options_), std::end(options_), [](const Option_p &opt) {
            return opt->get_items_expected() < 0 && opt->get_positional();
        });
        if(count > 1)
            throw InvalidError(name_);
        for(const App_p &app : subcommands_)
            app->_validate();
    }

    /// Internal function to run (App) callback, top down
    void run_callback() {
        pre_callback();
        if(callback_)
            callback_();
        for(App *subc : get_subcommands()) {
            subc->run_callback();
        }
    }

    /// Check to see if a subcommand is valid. Give up immediately if subcommand max has been reached.
    bool _valid_subcommand(const std::string &current) const {
        // Don't match if max has been reached - but still check parents
        if(require_subcommand_max_ != 0 && parsed_subcommands_.size() >= require_subcommand_max_) {
            return parent_ != nullptr && parent_->_valid_subcommand(current);
        }

        for(const App_p &com : subcommands_)
            if(com->check_name(current) && !*com)
                return true;

        // Check parent if exists, else return false
        return parent_ != nullptr && parent_->_valid_subcommand(current);
    }

    /// Selects a Classifier enum based on the type of the current argument
    detail::Classifer _recognize(const std::string &current) const {
        std::string dummy1, dummy2;

        if(current == "--")
            return detail::Classifer::POSITIONAL_MARK;
        if(_valid_subcommand(current))
            return detail::Classifer::SUBCOMMAND;
        if(detail::split_long(current, dummy1, dummy2))
            return detail::Classifer::LONG;
        if(detail::split_short(current, dummy1, dummy2))
            return detail::Classifer::SHORT;
        return detail::Classifer::NONE;
    }

    /// Internal parse function
    void _parse(std::vector<std::string> &args) {
        parsed_ = true;
        bool positional_only = false;

        while(!args.empty()) {
            _parse_single(args, positional_only);
        }

        for(const Option_p &opt : options_)
            if(opt->get_short_circuit() && opt->count() > 0)
                opt->run_callback();

        // Process an INI file
        if(config_ptr_ != nullptr) {
            if(*config_ptr_) {
                config_ptr_->run_callback();
                config_required_ = true;
            }
            if(!config_name_.empty()) {
                try {
                    std::vector<ConfigItem> values = config_formatter_->from_file(config_name_);
                    _parse_config(values);
                } catch(const FileError &) {
                    if(config_required_)
                        throw;
                }
            }
        }

        // Get envname options if not yet passed
        for(const Option_p &opt : options_) {
            if(opt->count() == 0 && !opt->envname_.empty()) {
                char *buffer = nullptr;
                std::string ename_string;

#ifdef _MSC_VER
                // Windows version
                size_t sz = 0;
                if(_dupenv_s(&buffer, &sz, opt->envname_.c_str()) == 0 && buffer != nullptr) {
                    ename_string = std::string(buffer);
                    free(buffer);
                }
#else
                // This also works on Windows, but gives a warning
                buffer = std::getenv(opt->envname_.c_str());
                if(buffer != nullptr)
                    ename_string = std::string(buffer);
#endif

                if(!ename_string.empty()) {
                    opt->add_result(ename_string);
                }
            }
        }

        // Process callbacks
        for(const Option_p &opt : options_) {
            if(opt->count() > 0 && !opt->get_callback_run()) {
                opt->run_callback();
            }
        }

        // Verify required options
        for(const Option_p &opt : options_) {
            // Exit if a help flag was passed (requirements not required in that case)
            if(_any_help_flag())
                break;

            // Required or partially filled
            if(opt->get_required() || opt->count() != 0) {
                // Make sure enough -N arguments parsed (+N is already handled in parsing function)
                if(opt->get_items_expected() < 0 && opt->count() < static_cast<size_t>(-opt->get_items_expected()))
                    throw ArgumentMismatch::AtLeast(opt->get_name(), -opt->get_items_expected());

                // Required but empty
                if(opt->get_required() && opt->count() == 0)
                    throw RequiredError(opt->get_name());
            }
            // Requires
            for(const Option *opt_req : opt->needs_)
                if(opt->count() > 0 && opt_req->count() == 0)
                    throw RequiresError(opt->get_name(), opt_req->get_name());
            // Excludes
            for(const Option *opt_ex : opt->excludes_)
                if(opt->count() > 0 && opt_ex->count() != 0)
                    throw ExcludesError(opt->get_name(), opt_ex->get_name());
        }

        auto selected_subcommands = get_subcommands();
        if(require_subcommand_min_ > selected_subcommands.size())
            throw RequiredError::Subcommand(require_subcommand_min_);

        // Convert missing (pairs) to extras (string only)
        if(!(allow_extras_ || prefix_command_)) {
            size_t num_left_over = remaining_size();
            if(num_left_over > 0) {
                args = remaining(false);
                throw ExtrasError(args);
            }
        }

        if(parent_ == nullptr) {
            args = remaining(false);
        }
    }

    /// Return True if a help flag detected (checks all parents) (only run if help called before subcommand)
    bool _any_help_flag() const {
        bool result = false;
        const Option *help_ptr = get_help_ptr();
        const Option *help_all_ptr = get_help_all_ptr();
        if(help_ptr != nullptr && help_ptr->count() > 0)
            result = true;
        if(help_all_ptr != nullptr && help_all_ptr->count() > 0)
            result = true;
        if(parent_ != nullptr)
            return result || parent_->_any_help_flag();
        else
            return result;
    }

    /// Parse one config param, return false if not found in any subcommand, remove if it is
    ///
    /// If this has more than one dot.separated.name, go into the subcommand matching it
    /// Returns true if it managed to find the option, if false you'll need to remove the arg manually.
    void _parse_config(std::vector<ConfigItem> &args) {
        for(ConfigItem item : args) {
            if(!_parse_single_config(item) && !allow_config_extras_)
                throw ConfigError::Extras(item.fullname());
        }
    }

    /// Fill in a single config option
    bool _parse_single_config(const ConfigItem &item, size_t level = 0) {
        if(level < item.parents.size()) {
            App *subcom;
            try {
                subcom = get_subcommand(item.parents.at(level));
            } catch(const OptionNotFound &) {
                return false;
            }
            return subcom->_parse_single_config(item, level + 1);
        }

        Option *op;
        try {
            op = get_option("--" + item.name);
        } catch(const OptionNotFound &) {
            // If the option was not present
            if(get_allow_config_extras())
                // Should we worry about classifying the extras properly?
                missing_.emplace_back(detail::Classifer::NONE, item.fullname());
            return false;
        }

        if(!op->get_configurable())
            throw ConfigError::NotConfigurable(item.fullname());

        if(op->empty()) {
            // Flag parsing
            if(op->get_type_size() == 0) {
                op->set_results(config_formatter_->to_flag(item));
            } else {
                op->set_results(item.inputs);
                op->run_callback();
            }
        }

        return true;
    }

    /// Parse "one" argument (some may eat more than one), delegate to parent if fails, add to missing if missing from
    /// master
    void _parse_single(std::vector<std::string> &args, bool &positional_only) {

        detail::Classifer classifer = positional_only ? detail::Classifer::NONE : _recognize(args.back());
        switch(classifer) {
        case detail::Classifer::POSITIONAL_MARK:
            missing_.emplace_back(classifer, args.back());
            args.pop_back();
            positional_only = true;
            break;
        case detail::Classifer::SUBCOMMAND:
            _parse_subcommand(args);
            break;
        case detail::Classifer::LONG:
            // If already parsed a subcommand, don't accept options_
            _parse_arg(args, true);
            break;
        case detail::Classifer::SHORT:
            // If already parsed a subcommand, don't accept options_
            _parse_arg(args, false);
            break;
        case detail::Classifer::NONE:
            // Probably a positional or something for a parent (sub)command
            _parse_positional(args);
        }
    }

    /// Count the required remaining positional arguments
    size_t _count_remaining_positionals(bool required = false) const {
        size_t retval = 0;
        for(const Option_p &opt : options_)
            if(opt->get_positional() && (!required || opt->get_required()) && opt->get_items_expected() > 0 &&
               static_cast<int>(opt->count()) < opt->get_items_expected())
                retval = static_cast<size_t>(opt->get_items_expected()) - opt->count();

        return retval;
    }

    /// Parse a positional, go up the tree to check
    void _parse_positional(std::vector<std::string> &args) {

        std::string positional = args.back();
        for(const Option_p &opt : options_) {
            // Eat options, one by one, until done
            if(opt->get_positional() &&
               (static_cast<int>(opt->count()) < opt->get_items_expected() || opt->get_items_expected() < 0)) {

                opt->add_result(positional);
                parse_order_.push_back(opt.get());
                args.pop_back();
                return;
            }
        }

        if(parent_ != nullptr && fallthrough_)
            return parent_->_parse_positional(args);
        else {
            args.pop_back();
            missing_.emplace_back(detail::Classifer::NONE, positional);

            if(prefix_command_) {
                while(!args.empty()) {
                    missing_.emplace_back(detail::Classifer::NONE, args.back());
                    args.pop_back();
                }
            }
        }
    }

    /// Parse a subcommand, modify args and continue
    ///
    /// Unlike the others, this one will always allow fallthrough
    void _parse_subcommand(std::vector<std::string> &args) {
        if(_count_remaining_positionals(/* required */ true) > 0)
            return _parse_positional(args);
        for(const App_p &com : subcommands_) {
            if(com->check_name(args.back())) {
                args.pop_back();
                if(std::find(std::begin(parsed_subcommands_), std::end(parsed_subcommands_), com.get()) ==
                   std::end(parsed_subcommands_))
                    parsed_subcommands_.push_back(com.get());
                com->_parse(args);
                return;
            }
        }
        if(parent_ != nullptr)
            return parent_->_parse_subcommand(args);
        else
            throw HorribleError("Subcommand " + args.back() + " missing");
    }

    /// Parse a short (false) or long (true) argument, must be at the top of the list
    void _parse_arg(std::vector<std::string> &args, bool second_dash) {

        detail::Classifer current_type = second_dash ? detail::Classifer::LONG : detail::Classifer::SHORT;

        std::string current = args.back();

        std::string name;
        std::string value;
        std::string rest;

        if(second_dash) {
            if(!detail::split_long(current, name, value))
                throw HorribleError("Long parsed but missing (you should not see this):" + args.back());
        } else {
            if(!detail::split_short(current, name, rest))
                throw HorribleError("Short parsed but missing! You should not see this");
        }

        auto op_ptr = std::find_if(std::begin(options_), std::end(options_), [name, second_dash](const Option_p &opt) {
            return second_dash ? opt->check_lname(name) : opt->check_sname(name);
        });

        // Option not found
        if(op_ptr == std::end(options_)) {
            // If a subcommand, try the master command
            if(parent_ != nullptr && fallthrough_)
                return parent_->_parse_arg(args, second_dash);
            // Otherwise, add to missing
            else {
                args.pop_back();
                missing_.emplace_back(current_type, current);
                return;
            }
        }

        args.pop_back();

        // Get a reference to the pointer to make syntax bearable
        Option_p &op = *op_ptr;

        int num = op->get_items_expected();

        // Make sure we always eat the minimum for unlimited vectors
        int collected = 0;

        // --this=value
        if(!value.empty()) {
            // If exact number expected
            if(num > 0)
                num--;
            op->add_result(value);
            parse_order_.push_back(op.get());
            collected += 1;
        } else if(num == 0) {
            op->add_result("");
            parse_order_.push_back(op.get());
            // -Trest
        } else if(!rest.empty()) {
            if(num > 0)
                num--;
            op->add_result(rest);
            parse_order_.push_back(op.get());
            rest = "";
            collected += 1;
        }

        // Unlimited vector parser
        if(num < 0) {
            while(!args.empty() && _recognize(args.back()) == detail::Classifer::NONE) {
                if(collected >= -num) {
                    // We could break here for allow extras, but we don't

                    // If any positionals remain, don't keep eating
                    if(_count_remaining_positionals() > 0)
                        break;
                }
                op->add_result(args.back());
                parse_order_.push_back(op.get());
                args.pop_back();
                collected++;
            }

            // Allow -- to end an unlimited list and "eat" it
            if(!args.empty() && _recognize(args.back()) == detail::Classifer::POSITIONAL_MARK)
                args.pop_back();

        } else {
            while(num > 0 && !args.empty()) {
                num--;
                std::string current_ = args.back();
                args.pop_back();
                op->add_result(current_);
                parse_order_.push_back(op.get());
            }

            if(num > 0) {
                throw ArgumentMismatch::TypedAtLeast(op->get_name(), num, op->get_type_name());
            }
        }

        if(!rest.empty()) {
            rest = "-" + rest;
            args.push_back(rest);
        }
    }
};

namespace FailureMessage {

/// Printout a clean, simple message on error (the default in CLI11 1.5+)
inline std::string simple(const App *app, const Error &e) {
    std::string header = std::string(e.what()) + "\n";
    if(app->get_help_ptr() != nullptr)
        header += "Run with " + app->get_help_ptr()->get_name() + " for more information.\n";
    return header;
}

/// Printout the full help string on error (if this fn is set, the old default for CLI11)
inline std::string help(const App *app, const Error &e) {
    std::string header = std::string("ERROR: ") + e.get_name() + ": " + e.what() + "\n";
    header += app->help();
    return header;
}

} // namespace FailureMessage

namespace detail {
/// This class is simply to allow tests access to App's protected functions
struct AppFriend {

    /// Wrap _parse_short, perfectly forward arguments and return
    template <typename... Args>
    static auto parse_arg(App *app, Args &&... args) ->
        typename std::result_of<decltype (&App::_parse_arg)(App, Args...)>::type {
        return app->_parse_arg(std::forward<Args>(args)...);
    }

    /// Wrap _parse_subcommand, perfectly forward arguments and return
    template <typename... Args>
    static auto parse_subcommand(App *app, Args &&... args) ->
        typename std::result_of<decltype (&App::_parse_subcommand)(App, Args...)>::type {
        return app->_parse_subcommand(std::forward<Args>(args)...);
    }
};
} // namespace detail

} // namespace CLI
