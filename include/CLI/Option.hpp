#pragma once

// Distributed under the 3-Clause BSD License.  See accompanying
// file LICENSE or https://github.com/CLIUtils/CLI11 for details.

#include <algorithm>
#include <functional>
#include <memory>
#include <set>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

#include "CLI/Error.hpp"
#include "CLI/Macros.hpp"
#include "CLI/Split.hpp"
#include "CLI/StringTools.hpp"
#include "CLI/Validators.hpp"

namespace CLI {

using results_t = std::vector<std::string>;
using callback_t = std::function<bool(results_t)>;

class Option;
class App;

using Option_p = std::unique_ptr<Option>;

enum class MultiOptionPolicy : char { Throw, TakeLast, TakeFirst, Join };

/// This is the CRTP base class for Option and OptionDefaults. It was designed this way
/// to share parts of the class; an OptionDefaults can copy to an Option.
template <typename CRTP> class OptionBase {
    friend App;

  protected:
    /// The group membership
    std::string group_ = std::string("Options");

    /// True if this is a required option
    bool required_{false};

    /// Ignore the case when matching (option, not value)
    bool ignore_case_{false};

    /// Ignore underscores when matching (option, not value)
    bool ignore_underscore_{false};

    /// Allow this option to be given in a configuration file
    bool configurable_{true};

    /// Disable overriding flag values with '=value'
    bool disable_flag_override_{false};

    /// Specify a delimiter character for vector arguments
    char delimiter_{'\0'};

    /// Automatically capture default value
    bool always_capture_default_{false};

    /// Policy for multiple arguments when `expected_ == 1`  (can be set on bool flags, too)
    MultiOptionPolicy multi_option_policy_{MultiOptionPolicy::Throw};

    /// Copy the contents to another similar class (one based on OptionBase)
    template <typename T> void copy_to(T *other) const {
        other->group(group_);
        other->required(required_);
        other->ignore_case(ignore_case_);
        other->ignore_underscore(ignore_underscore_);
        other->configurable(configurable_);
        other->disable_flag_override(disable_flag_override_);
        other->delimiter(delimiter_);
        other->always_capture_default(always_capture_default_);
        other->multi_option_policy(multi_option_policy_);
    }

  public:
    // setters

    /// Changes the group membership
    CRTP *group(std::string name) {
        group_ = name;
        return static_cast<CRTP *>(this);
    }

    /// Set the option as required
    CRTP *required(bool value = true) {
        required_ = value;
        return static_cast<CRTP *>(this);
    }

    /// Support Plumbum term
    CRTP *mandatory(bool value = true) { return required(value); }

    CRTP *always_capture_default(bool value = true) {
        always_capture_default_ = value;
        return static_cast<CRTP *>(this);
    }

    // Getters

    /// Get the group of this option
    const std::string &get_group() const { return group_; }

    /// True if this is a required option
    bool get_required() const { return required_; }

    /// The status of ignore case
    bool get_ignore_case() const { return ignore_case_; }

    /// The status of ignore_underscore
    bool get_ignore_underscore() const { return ignore_underscore_; }

    /// The status of configurable
    bool get_configurable() const { return configurable_; }

    /// The status of configurable
    bool get_disable_flag_override() const { return disable_flag_override_; }

    /// Get the current delimeter char
    char get_delimiter() const { return delimiter_; }

    /// Return true if this will automatically capture the default value for help printing
    bool get_always_capture_default() const { return always_capture_default_; }

    /// The status of the multi option policy
    MultiOptionPolicy get_multi_option_policy() const { return multi_option_policy_; }

    // Shortcuts for multi option policy

    /// Set the multi option policy to take last
    CRTP *take_last() {
        auto self = static_cast<CRTP *>(this);
        self->multi_option_policy(MultiOptionPolicy::TakeLast);
        return self;
    }

    /// Set the multi option policy to take last
    CRTP *take_first() {
        auto self = static_cast<CRTP *>(this);
        self->multi_option_policy(MultiOptionPolicy::TakeFirst);
        return self;
    }

    /// Set the multi option policy to take last
    CRTP *join() {
        auto self = static_cast<CRTP *>(this);
        self->multi_option_policy(MultiOptionPolicy::Join);
        return self;
    }

    /// Allow in a configuration file
    CRTP *configurable(bool value = true) {
        configurable_ = value;
        return static_cast<CRTP *>(this);
    }

    /// Allow in a configuration file
    CRTP *delimiter(char value = '\0') {
        delimiter_ = value;
        return static_cast<CRTP *>(this);
    }
};

/// This is a version of OptionBase that only supports setting values,
/// for defaults. It is stored as the default option in an App.
class OptionDefaults : public OptionBase<OptionDefaults> {
  public:
    OptionDefaults() = default;

    // Methods here need a different implementation if they are Option vs. OptionDefault

    /// Take the last argument if given multiple times
    OptionDefaults *multi_option_policy(MultiOptionPolicy value = MultiOptionPolicy::Throw) {
        multi_option_policy_ = value;
        return this;
    }

    /// Ignore the case of the option name
    OptionDefaults *ignore_case(bool value = true) {
        ignore_case_ = value;
        return this;
    }

    /// Ignore underscores in the option name
    OptionDefaults *ignore_underscore(bool value = true) {
        ignore_underscore_ = value;
        return this;
    }

    /// Disable overriding flag values with an '=<value>' segment
    OptionDefaults *disable_flag_override(bool value = true) {
        disable_flag_override_ = value;
        return this;
    }

    /// set a delimiter character to split up single arguments to treat as multiple inputs
    OptionDefaults *delimiter(char value = '\0') {
        delimiter_ = value;
        return this;
    }
};

class Option : public OptionBase<Option> {
    friend App;

  protected:
    /// @name Names
    ///@{

    /// A list of the short names (`-a`) without the leading dashes
    std::vector<std::string> snames_;

    /// A list of the long names (`--a`) without the leading dashes
    std::vector<std::string> lnames_;

    /// A list of the flag names with the appropriate default value, the first part of the pair should be duplicates of
    /// what is in snames or lnames but will trigger a particular response on a flag
    std::vector<std::pair<std::string, std::string>> default_flag_values_;

    /// a list of flag names with specified default values;
    std::vector<std::string> fnames_;

    /// A positional name
    std::string pname_;

    /// If given, check the environment for this option
    std::string envname_;

    ///@}
    /// @name Help
    ///@{

    /// The description for help strings
    std::string description_;

    /// A human readable default value, either manually set, captured, or captured by default
    std::string default_str_;

    /// A human readable type value, set when App creates this
    ///
    /// This is a lambda function so "types" can be dynamic, such as when a set prints its contents.
    std::function<std::string()> type_name_{[]() { return std::string(); }};

    /// Run this function to capture a default (ignore if empty)
    std::function<std::string()> default_function_;

    ///@}
    /// @name Configuration
    ///@{

    /// The number of arguments that make up one option. -1=unlimited (vector-like), 0=flag, 1=normal option,
    /// 2=complex/pair, etc. Set only when the option is created; this is intrinsic to the type. Eventually, -2 may mean
    /// vector of pairs.
    int type_size_{1};

    /// The number of expected values, type_size_ must be < 0. Ignored for flag. N < 0 means at least -N values.
    int expected_{1};

    /// A list of validators to run on each value parsed
    std::vector<Validator> validators_;

    /// A list of options that are required with this option
    std::set<Option *> needs_;

    /// A list of options that are excluded with this option
    std::set<Option *> excludes_;

    ///@}
    /// @name Other
    ///@{

    /// Remember the parent app
    App *parent_;

    /// Options store a callback to do all the work
    callback_t callback_;

    ///@}
    /// @name Parsing results
    ///@{

    /// Results of parsing
    results_t results_;

    /// Whether the callback has run (needed for INI parsing)
    bool callback_run_{false};

    ///@}

    /// Making an option by hand is not defined, it must be made by the App class
    Option(std::string option_name,
           std::string option_description,
           std::function<bool(results_t)> callback,
           App *parent)
        : description_(std::move(option_description)), parent_(parent), callback_(std::move(callback)) {
        std::tie(snames_, lnames_, pname_) = detail::get_names(detail::split_names(option_name));
    }

  public:
    /// @name Basic
    ///@{

    /// Count the total number of times an option was passed
    size_t count() const { return results_.size(); }

    /// True if the option was not passed
    size_t empty() const { return results_.empty(); }

    /// This class is true if option is passed.
    operator bool() const { return !empty(); }

    /// Clear the parsed results (mostly for testing)
    void clear() { results_.clear(); }

    ///@}
    /// @name Setting options
    ///@{

    /// Set the number of expected arguments (Flags don't use this)
    Option *expected(int value) {

        // Break if this is a flag
        if(type_size_ == 0)
            throw IncorrectConstruction::SetFlag(get_name(true, true));

        // Setting 0 is not allowed
        else if(value == 0)
            throw IncorrectConstruction::Set0Opt(get_name());

        // No change is okay, quit now
        else if(expected_ == value)
            return this;

        // Type must be a vector
        else if(type_size_ >= 0)
            throw IncorrectConstruction::ChangeNotVector(get_name());

        // TODO: Can support multioption for non-1 values (except for join)
        else if(value != 1 && multi_option_policy_ != MultiOptionPolicy::Throw)
            throw IncorrectConstruction::AfterMultiOpt(get_name());

        expected_ = value;
        return this;
    }

    /// Adds a Validator with a built in type name
    Option *check(Validator validator, std::string validator_name = "") {
        validator.non_modifying();
        validators_.push_back(std::move(validator));
        if(!validator_name.empty())
            validators_.front().name(validator_name);
        return this;
    }

    /// Adds a Validator. Takes a const string& and returns an error message (empty if conversion/check is okay).
    Option *check(std::function<std::string(const std::string &)> validator,
                  std::string validator_description = "",
                  std::string validator_name = "") {
        validators_.emplace_back(validator, std::move(validator_description), std::move(validator_name));
        validators_.back().non_modifying();
        return this;
    }

    /// Adds a transforming validator with a built in type name
    Option *transform(Validator validator, std::string validator_name = "") {
        validators_.insert(validators_.begin(), std::move(validator));
        if(!validator_name.empty())
            validators_.front().name(validator_name);
        return this;
    }

    /// Adds a validator-like function that can change result
    Option *transform(std::function<std::string(std::string)> func,
                      std::string transform_description = "",
                      std::string transform_name = "") {
        validators_.insert(validators_.begin(),
                           Validator(
                               [func](std::string &val) {
                                   val = func(val);
                                   return std::string{};
                               },
                               std::move(transform_description),
                               std::move(transform_name)));

        return this;
    }

    /// Adds a user supplied function to run on each item passed in (communicate though lambda capture)
    Option *each(std::function<void(std::string)> func) {
        validators_.emplace_back(
            [func](std::string &inout) {
                func(inout);
                return std::string{};
            },
            std::string{});
        return this;
    }
    /// Get a named Validator
    Validator *get_validator(const std::string &validator_name = "") {
        for(auto &validator : validators_) {
            if(validator_name == validator.get_name()) {
                return &validator;
            }
        }
        if((validator_name.empty()) && (!validators_.empty())) {
            return &(validators_.front());
        }
        throw OptionNotFound(std::string("Validator ") + validator_name + " Not Found");
    }
    /// Sets required options
    Option *needs(Option *opt) {
        auto tup = needs_.insert(opt);
        if(!tup.second)
            throw OptionAlreadyAdded::Requires(get_name(), opt->get_name());
        return this;
    }

    /// Can find a string if needed
    template <typename T = App> Option *needs(std::string opt_name) {
        for(const Option_p &opt : dynamic_cast<T *>(parent_)->options_)
            if(opt.get() != this && opt->check_name(opt_name))
                return needs(opt.get());
        throw IncorrectConstruction::MissingOption(opt_name);
    }

    /// Any number supported, any mix of string and Opt
    template <typename A, typename B, typename... ARG> Option *needs(A opt, B opt1, ARG... args) {
        needs(opt);
        return needs(opt1, args...);
    }

    /// Remove needs link from an option. Returns true if the option really was in the needs list.
    bool remove_needs(Option *opt) {
        auto iterator = std::find(std::begin(needs_), std::end(needs_), opt);

        if(iterator != std::end(needs_)) {
            needs_.erase(iterator);
            return true;
        } else {
            return false;
        }
    }

    /// Sets excluded options
    Option *excludes(Option *opt) {
        excludes_.insert(opt);

        // Help text should be symmetric - excluding a should exclude b
        opt->excludes_.insert(this);

        // Ignoring the insert return value, excluding twice is now allowed.
        // (Mostly to allow both directions to be excluded by user, even though the library does it for you.)

        return this;
    }

    /// Can find a string if needed
    template <typename T = App> Option *excludes(std::string opt_name) {
        for(const Option_p &opt : dynamic_cast<T *>(parent_)->options_)
            if(opt.get() != this && opt->check_name(opt_name))
                return excludes(opt.get());
        throw IncorrectConstruction::MissingOption(opt_name);
    }

    /// Any number supported, any mix of string and Opt
    template <typename A, typename B, typename... ARG> Option *excludes(A opt, B opt1, ARG... args) {
        excludes(opt);
        return excludes(opt1, args...);
    }

    /// Remove needs link from an option. Returns true if the option really was in the needs list.
    bool remove_excludes(Option *opt) {
        auto iterator = std::find(std::begin(excludes_), std::end(excludes_), opt);

        if(iterator != std::end(excludes_)) {
            excludes_.erase(iterator);
            return true;
        } else {
            return false;
        }
    }

    /// Sets environment variable to read if no option given
    Option *envname(std::string name) {
        envname_ = name;
        return this;
    }

    /// Ignore case
    ///
    /// The template hides the fact that we don't have the definition of App yet.
    /// You are never expected to add an argument to the template here.
    template <typename T = App> Option *ignore_case(bool value = true) {
        ignore_case_ = value;
        auto *parent = dynamic_cast<T *>(parent_);

        for(const Option_p &opt : parent->options_)
            if(opt.get() != this && *opt == *this)
                throw OptionAlreadyAdded(opt->get_name(true, true));

        return this;
    }

    /// Ignore underscores in the option names
    ///
    /// The template hides the fact that we don't have the definition of App yet.
    /// You are never expected to add an argument to the template here.
    template <typename T = App> Option *ignore_underscore(bool value = true) {
        ignore_underscore_ = value;
        auto *parent = dynamic_cast<T *>(parent_);
        for(const Option_p &opt : parent->options_)
            if(opt.get() != this && *opt == *this)
                throw OptionAlreadyAdded(opt->get_name(true, true));

        return this;
    }

    /// Take the last argument if given multiple times (or another policy)
    Option *multi_option_policy(MultiOptionPolicy value = MultiOptionPolicy::Throw) {

        if(get_items_expected() < 0)
            throw IncorrectConstruction::MultiOptionPolicy(get_name());
        multi_option_policy_ = value;
        return this;
    }

    /// Disable flag overrides values, e.g. --flag=<value> is not allowed
    Option *disable_flag_override(bool value = true) {
        disable_flag_override_ = value;
        return this;
    }
    ///@}
    /// @name Accessors
    ///@{

    /// The number of arguments the option expects
    int get_type_size() const { return type_size_; }

    /// The environment variable associated to this value
    std::string get_envname() const { return envname_; }

    /// The set of options needed
    std::set<Option *> get_needs() const { return needs_; }

    /// The set of options excluded
    std::set<Option *> get_excludes() const { return excludes_; }

    /// The default value (for help printing) DEPRECATED Use get_default_str() instead
    CLI11_DEPRECATED("Use get_default_str() instead")
    std::string get_defaultval() const { return default_str_; }

    /// The default value (for help printing)
    std::string get_default_str() const { return default_str_; }

    /// Get the callback function
    callback_t get_callback() const { return callback_; }

    /// Get the long names
    const std::vector<std::string> get_lnames() const { return lnames_; }

    /// Get the short names
    const std::vector<std::string> get_snames() const { return snames_; }

    /// Get the flag names with specified default values
    const std::vector<std::string> get_fnames() const { return fnames_; }

    /// The number of times the option expects to be included
    int get_expected() const { return expected_; }

    /// \brief The total number of expected values (including the type)
    /// This is positive if exactly this number is expected, and negative for at least N values
    ///
    /// v = fabs(size_type*expected)
    /// !MultiOptionPolicy::Throw
    ///           | Expected < 0  | Expected == 0 | Expected > 0
    /// Size < 0  |      -v       |       0       |     -v
    /// Size == 0 |       0       |       0       |      0
    /// Size > 0  |      -v       |       0       |     -v       // Expected must be 1
    ///
    /// MultiOptionPolicy::Throw
    ///           | Expected < 0  | Expected == 0 | Expected > 0
    /// Size < 0  |      -v       |       0       |      v
    /// Size == 0 |       0       |       0       |      0
    /// Size > 0  |       v       |       0       |      v      // Expected must be 1
    ///
    int get_items_expected() const {
        return std::abs(type_size_ * expected_) *
               ((multi_option_policy_ != MultiOptionPolicy::Throw || (expected_ < 0 && type_size_ < 0) ? -1 : 1));
    }

    /// True if the argument can be given directly
    bool get_positional() const { return pname_.length() > 0; }

    /// True if option has at least one non-positional name
    bool nonpositional() const { return (snames_.size() + lnames_.size()) > 0; }

    /// True if option has description
    bool has_description() const { return description_.length() > 0; }

    /// Get the description
    const std::string &get_description() const { return description_; }

    /// Set the description
    Option *description(std::string option_description) {
        description_ = std::move(option_description);
        return this;
    }

    ///@}
    /// @name Help tools
    ///@{

    /// \brief Gets a comma separated list of names.
    /// Will include / prefer the positional name if positional is true.
    /// If all_options is false, pick just the most descriptive name to show.
    /// Use `get_name(true)` to get the positional name (replaces `get_pname`)
    std::string get_name(bool positional = false, //<[input] Show the positional name
                         bool all_options = false //<[input] Show every option
                         ) const {

        if(all_options) {

            std::vector<std::string> name_list;

            /// The all list will never include a positional unless asked or that's the only name.
            if((positional && pname_.length()) || (snames_.empty() && lnames_.empty()))
                name_list.push_back(pname_);
            if((get_items_expected() == 0) && (!fnames_.empty())) {
                for(const std::string &sname : snames_) {
                    name_list.push_back("-" + sname);
                    if(check_fname(sname)) {
                        name_list.back() += "{" + get_flag_value(sname, "") + "}";
                    }
                }

                for(const std::string &lname : lnames_) {
                    name_list.push_back("--" + lname);
                    if(check_fname(lname)) {
                        name_list.back() += "{" + get_flag_value(lname, "") + "}";
                    }
                }
            } else {
                for(const std::string &sname : snames_)
                    name_list.push_back("-" + sname);

                for(const std::string &lname : lnames_)
                    name_list.push_back("--" + lname);
            }

            return detail::join(name_list);

        } else {

            // This returns the positional name no matter what
            if(positional)
                return pname_;

            // Prefer long name
            else if(!lnames_.empty())
                return std::string("--") + lnames_[0];

            // Or short name if no long name
            else if(!snames_.empty())
                return std::string("-") + snames_[0];

            // If positional is the only name, it's okay to use that
            else
                return pname_;
        }
    }

    ///@}
    /// @name Parser tools
    ///@{

    /// Process the callback
    void run_callback() {

        callback_run_ = true;

        // Run the validators (can change the string)
        if(!validators_.empty()) {
            for(std::string &result : results_) {
                auto err_msg = _validate(result);
                if(!err_msg.empty())
                    throw ValidationError(get_name(), err_msg);
            }
        }
        if(!(callback_)) {
            return;
        }
        bool local_result;

        // Num items expected or length of vector, always at least 1
        // Only valid for a trimming policy
        int trim_size =
            std::min<int>(std::max<int>(std::abs(get_items_expected()), 1), static_cast<int>(results_.size()));

        // Operation depends on the policy setting
        if(multi_option_policy_ == MultiOptionPolicy::TakeLast) {
            // Allow multi-option sizes (including 0)
            results_t partial_result{results_.end() - trim_size, results_.end()};
            local_result = !callback_(partial_result);

        } else if(multi_option_policy_ == MultiOptionPolicy::TakeFirst) {
            results_t partial_result{results_.begin(), results_.begin() + trim_size};
            local_result = !callback_(partial_result);

        } else if(multi_option_policy_ == MultiOptionPolicy::Join) {
            results_t partial_result = {detail::join(results_, "\n")};
            local_result = !callback_(partial_result);

        } else {
            // Exact number required
            if(get_items_expected() > 0) {
                if(results_.size() != static_cast<size_t>(get_items_expected()))
                    throw ArgumentMismatch(get_name(), get_items_expected(), results_.size());
                // Variable length list
            } else if(get_items_expected() < 0) {
                // Require that this be a multiple of expected size and at least as many as expected
                if(results_.size() < static_cast<size_t>(-get_items_expected()) ||
                   results_.size() % static_cast<size_t>(std::abs(get_type_size())) != 0u)
                    throw ArgumentMismatch(get_name(), get_items_expected(), results_.size());
            }
            local_result = !callback_(results_);
        }

        if(local_result)
            throw ConversionError(get_name(), results_);
    }

    /// If options share any of the same names, they are equal (not counting positional)
    bool operator==(const Option &other) const {
        for(const std::string &sname : snames_)
            if(other.check_sname(sname))
                return true;
        for(const std::string &lname : lnames_)
            if(other.check_lname(lname))
                return true;

        if(ignore_case_ ||
           ignore_underscore_) { // We need to do the inverse, in case we are ignore_case or ignore underscore
            for(const std::string &sname : other.snames_)
                if(check_sname(sname))
                    return true;
            for(const std::string &lname : other.lnames_)
                if(check_lname(lname))
                    return true;
        }
        return false;
    }

    /// Check a name. Requires "-" or "--" for short / long, supports positional name
    bool check_name(std::string name) const {

        if(name.length() > 2 && name[0] == '-' && name[1] == '-')
            return check_lname(name.substr(2));
        else if(name.length() > 1 && name.front() == '-')
            return check_sname(name.substr(1));
        else {
            std::string local_pname = pname_;
            if(ignore_underscore_) {
                local_pname = detail::remove_underscore(local_pname);
                name = detail::remove_underscore(name);
            }
            if(ignore_case_) {
                local_pname = detail::to_lower(local_pname);
                name = detail::to_lower(name);
            }
            return name == local_pname;
        }
    }

    /// Requires "-" to be removed from string
    bool check_sname(std::string name) const { return (detail::find_member(name, snames_, ignore_case_) >= 0); }

    /// Requires "--" to be removed from string
    bool check_lname(std::string name) const {
        return (detail::find_member(name, lnames_, ignore_case_, ignore_underscore_) >= 0);
    }

    /// Requires "--" to be removed from string
    bool check_fname(std::string name) const {
        if(fnames_.empty()) {
            return false;
        }
        return (detail::find_member(name, fnames_, ignore_case_, ignore_underscore_) >= 0);
    }

    /// Get the value that goes for a flag, nominally gets the default value but allows for overrides if not disabled
    std::string get_flag_value(std::string name, std::string input_value) const {
        static const std::string trueString{"true"};
        static const std::string falseString{"false"};
        static const std::string emptyString{"{}"};
        // check for disable flag override_
        if(disable_flag_override_) {
            if(!((input_value.empty()) || (input_value == emptyString))) {
                auto default_ind = detail::find_member(name, fnames_, ignore_case_, ignore_underscore_);
                if(default_ind >= 0) {
                    // We can static cast this to size_t because it is more than 0 in this block
                    if(default_flag_values_[static_cast<size_t>(default_ind)].second != input_value) {
                        throw(ArgumentMismatch::FlagOverride(name));
                    }
                } else {
                    if(input_value != trueString) {
                        throw(ArgumentMismatch::FlagOverride(name));
                    }
                }
            }
        }
        auto ind = detail::find_member(name, fnames_, ignore_case_, ignore_underscore_);
        if((input_value.empty()) || (input_value == emptyString)) {
            return (ind < 0) ? trueString : default_flag_values_[static_cast<size_t>(ind)].second;
        }
        if(ind < 0) {
            return input_value;
        }
        if(default_flag_values_[static_cast<size_t>(ind)].second == falseString) {
            try {
                auto val = detail::to_flag_value(input_value);
                return (val == 1) ? falseString : (val == (-1) ? trueString : std::to_string(-val));
            } catch(const std::invalid_argument &) {
                return input_value;
            }
        } else {
            return input_value;
        }
    }

    /// Puts a result at the end
    Option *add_result(std::string s) {
        _add_result(std::move(s));
        callback_run_ = false;
        return this;
    }

    /// Puts a result at the end and get a count of the number of arguments actually added
    Option *add_result(std::string s, int &results_added) {
        results_added = _add_result(std::move(s));
        callback_run_ = false;
        return this;
    }

    /// Puts a result at the end
    Option *add_result(std::vector<std::string> s) {
        for(auto &str : s) {
            _add_result(std::move(str));
        }
        callback_run_ = false;
        return this;
    }

    /// Get a copy of the results
    std::vector<std::string> results() const { return results_; }

    /// Get the results as a specified type
    template <typename T,
              enable_if_t<!is_vector<T>::value && !std::is_const<T>::value, detail::enabler> = detail::dummy>
    void results(T &output) const {
        bool retval;
        if(results_.empty()) {
            retval = detail::lexical_cast(default_str_, output);
        } else if(results_.size() == 1) {
            retval = detail::lexical_cast(results_[0], output);
        } else {
            switch(multi_option_policy_) {
            case MultiOptionPolicy::TakeFirst:
                retval = detail::lexical_cast(results_.front(), output);
                break;
            case MultiOptionPolicy::TakeLast:
            default:
                retval = detail::lexical_cast(results_.back(), output);
                break;
            case MultiOptionPolicy::Throw:
                throw ConversionError(get_name(), results_);
            case MultiOptionPolicy::Join:
                retval = detail::lexical_cast(detail::join(results_), output);
                break;
            }
        }
        if(!retval) {
            throw ConversionError(get_name(), results_);
        }
    }
    /// Get the results as a vector of the specified type
    template <typename T> void results(std::vector<T> &output) const {
        output.clear();
        bool retval = true;

        for(const auto &elem : results_) {
            output.emplace_back();
            retval &= detail::lexical_cast(elem, output.back());
        }

        if(!retval) {
            throw ConversionError(get_name(), results_);
        }
    }

    /// Return the results as the specified type
    template <typename T> T as() const {
        T output;
        results(output);
        return output;
    }

    /// See if the callback has been run already
    bool get_callback_run() const { return callback_run_; }

    ///@}
    /// @name Custom options
    ///@{

    /// Set the type function to run when displayed on this option
    Option *type_name_fn(std::function<std::string()> typefun) {
        type_name_ = typefun;
        return this;
    }

    /// Set a custom option typestring
    Option *type_name(std::string typeval) {
        type_name_fn([typeval]() { return typeval; });
        return this;
    }

    /// Set a custom option size
    Option *type_size(int option_type_size) {
        type_size_ = option_type_size;
        if(type_size_ == 0)
            required_ = false;
        if(option_type_size < 0)
            expected_ = -1;
        return this;
    }

    /// Set a capture function for the default. Mostly used by App.
    Option *default_function(const std::function<std::string()> &func) {
        default_function_ = func;
        return this;
    }

    /// Capture the default value from the original value (if it can be captured)
    Option *capture_default_str() {
        if(default_function_) {
            default_str_ = default_function_();
        }
        return this;
    }

    /// Set the default value string representation (does not change the contained value)
    Option *default_str(std::string val) {
        default_str_ = val;
        return this;
    }

    /// Set the default value string representation and evaluate into the bound value
    Option *default_val(std::string val) {
        default_str(val);
        auto old_results = results_;
        results_ = {val};
        run_callback();
        results_ = std::move(old_results);
        return this;
    }

    /// Get the full typename for this option
    std::string get_type_name() const {
        std::string full_type_name = type_name_();
        if(!validators_.empty()) {
            for(auto &validator : validators_) {
                std::string vtype = validator.get_description();
                if(!vtype.empty()) {
                    full_type_name += ":" + vtype;
                }
            }
        }
        return full_type_name;
    }

  private:
    // Run a result through the validators
    std::string _validate(std::string &result) {
        std::string err_msg;
        for(const auto &vali : validators_) {
            try {
                err_msg = vali(result);
            } catch(const ValidationError &err) {
                err_msg = err.what();
            }
            if(!err_msg.empty())
                break;
        }
        return err_msg;
    }

    /// Add a single result to the result set, taking into account delimiters
    int _add_result(std::string &&result) {
        int result_count = 0;
        if(delimiter_ == '\0') {
            results_.push_back(std::move(result));
            ++result_count;
        } else {
            if((result.find_first_of(delimiter_) != std::string::npos)) {
                for(const auto &var : CLI::detail::split(result, delimiter_)) {
                    if(!var.empty()) {
                        results_.push_back(var);
                        ++result_count;
                    }
                }
            } else {
                results_.push_back(std::move(result));
                ++result_count;
            }
        }
        return result_count;
    }
};

} // namespace CLI
