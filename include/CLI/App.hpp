#pragma once

// Distributed under the LGPL v2.1 license.  See accompanying
// file LICENSE or https://github.com/henryiii/CLI11 for details.

#include <string>
#include <memory>
#include <deque>
#include <iostream>
#include <functional>
#include <algorithm>
#include <sstream>
#include <set>
#include <numeric>
#include <vector>


// CLI Library includes
#include "CLI/Error.hpp"
#include "CLI/TypeTools.hpp"
#include "CLI/StringTools.hpp"
#include "CLI/Split.hpp"
#include "CLI/Option.hpp"
#include "CLI/Ini.hpp"

namespace CLI {

namespace detail {
enum class Classifer {NONE, POSITIONAL_MARK, SHORT, LONG, SUBCOMMAND};
}

class App;

typedef std::unique_ptr<App> App_p;

/// Creates a command line program, with very few defaults.
/** To use, create a new `Program()` instance with `argc`, `argv`, and a help description. The templated
*  add_option methods make it easy to prepare options. Remember to call `.start` before starting your
* program, so that the options can be evaluated and the help option doesn't accidentally run your program. */
class App {
    friend Option;
protected:
    
    // This library follows the Google style guide for member names ending in underscores

    /// @name Basics
    ///@{
    
    /// Subcommand name or program name (from parser)
    std::string name_ {"program"};
    
    /// Description of the current program/subcommand
    std::string description_;

    /// If true, allow extra arguments (ie, don't throw an error).
    bool allow_extras_ {false};

    /// This is a function that runs when complete. Great for subcommands. Can throw.
    std::function<void()> callback_;

    ///@}
    /// @name Options
    ///@{

    /// The list of options, stored locally
    std::vector<Option_p> options_;
    
    /// A pointer to the help flag if there is one
    Option* help_ptr_ {nullptr};

    ///@}
    /// @name Parsing
    ///@{

    typedef std::vector<std::pair<detail::Classifer, std::string>> missing_t;

    /// Pair of classifier, string for missing options. (extra detail is removed on returning from parse)
    /// 
    /// This is faster and cleaner than storing just a list of strings and reparsing. This may contain the -- separator.
    missing_t missing_;

    ///@}
    /// @name Subcommands
    ///@{
    
    /// Storage for subcommand list
    std::vector<App_p> subcommands_;

    /// If true, the program name is not case sensitive
    bool ignore_case_ {false};

    /// Allow subcommand fallthrough, so that parent commands can collect commands after subcommand.
    bool fallthrough_ {false};

    /// A pointer to the parent if this is a subcommand
    App* parent_ {nullptr};

    /// A list of all the subcommand pointers that have been collected on the command line
    std::vector<App*> selected_subcommands_;

    /// -1 for 1 or more, 0 for not required, # for exact number required
    int require_subcommand_ = 0;

    ///@}
    /// @name Config
    ///@{

    /// The name of the connected config file
    std::string config_name_;

    /// True if ini is required (throws if not present), if false simply keep going. 
    bool config_required_ {false};

    /// Pointer to the config option
    Option* config_ptr_ {nullptr};


    ///@}
   
    /// Special private constructor for subcommand
    App(std::string description_, bool help, detail::enabler dummy_param) 
        :  description_(description_) {

        if(help)
            help_ptr_ = add_flag("-h,--help", "Print this help message and exit");

    }

public:
    /// @name Basic
    ///@{

    /// Create a new program. Pass in the same arguments as main(), along with a help string.
    App(std::string description_="", bool help=true)
        : App(description_, help, detail::dummy) {

    }

    /// Set a callback for the end of parsing.
    /// 
    /// Due to a bug in c++11,
    /// it is not possible to overload on std::function (fixed in c++14
    /// and backported to c++11 on newer compilers). Use capture by reference
    /// to get a pointer to App if needed.
    App* set_callback(std::function<void()> callback) {
        callback_ = callback;
        return this;
    }

    /// Remove the error when extras are left over on the command line.
    App* allow_extras (bool allow=true) {
        allow_extras_ = allow;
        return this;
    }

    /// Ignore case. Subcommand inherit value.
    App* ignore_case(bool value = true) {
        ignore_case_ = value;
        if(parent_ != nullptr) {
            for(const auto &subc : parent_->subcommands_) {
                if(subc.get() != this && (this->check_name(subc->name_) || subc->check_name(this->name_)))
                    throw OptionAlreadyAdded(subc->name_);
            }
        }
        return this;
    }

    /// Require a subcommand to be given (does not affect help call)
    /// Does not return a pointer since it is supposed to be called on the main App.
    App* require_subcommand(int value = -1) {
        require_subcommand_ = value;
        return this;
    }

    /// Stop subcommand fallthrough, so that parent commands cannot collect commands after subcommand.
    /// Default from parent, usually set on parent.
    App* fallthrough(bool value=true) {
        fallthrough_ = value;
        return this;
    }


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
    ///     std::string filename
    ///     program.add_option("filename", filename, "description of filename");
    ///
    Option* add_option(
            std::string name,
            callback_t callback,
            std::string description="", 
            bool defaulted=false
            ) {
        Option myopt{name, description, callback, defaulted, this};

        if(std::find_if(std::begin(options_), std::end(options_),
                    [&myopt](const Option_p &v){return *v == myopt;}) == std::end(options_)) {
            options_.emplace_back();
            Option_p& option = options_.back();
            option.reset(new Option(name, description, callback, defaulted, this));
            return option.get();
        } else
            throw OptionAlreadyAdded(myopt.get_name());

    }

    /// Add option for non-vectors
    template<typename T, enable_if_t<!is_vector<T>::value, detail::enabler> = detail::dummy>
    Option* add_option(
            std::string name,
            T &variable,                ///< The variable to set
            std::string description="",
            bool defaulted=false
            ) {

        
        CLI::callback_t fun = [&variable](CLI::results_t res){
            if(res.size()!=1)
                return false;
            return detail::lexical_cast(res[0], variable);
        };

        Option* opt = add_option(name, fun, description, defaulted);
        opt->typeval_ = detail::type_name<T>();
        if(defaulted) {
            std::stringstream out;
            out << variable;
            opt->defaultval_ = out.str();
        }
        return opt;
    }

    /// Add option for vectors
    template<typename T>
    Option* add_option(
            std::string name,
            std::vector<T> &variable,   ///< The variable vector to set
            std::string description="",
            bool defaulted=false
            ) {

        CLI::callback_t fun = [&variable](CLI::results_t res){
            bool retval = true;
            variable.clear();
            for(const auto &a : res) {
                variable.emplace_back();
                retval &= detail::lexical_cast(a, variable.back());
            }
            return variable.size() > 0 && retval;
        };

        Option* opt =  add_option(name, fun, description, defaulted);
        opt->allow_vector_ = true;
        opt->expected_ = -1;
        opt->typeval_ = detail::type_name<T>();
        if(defaulted)
            opt->defaultval_ =  "[" + detail::join(variable) + "]";
        return opt;
    }

    /// Add option for flag
    Option* add_flag(
            std::string name,
            std::string description=""
            ) {
        CLI::callback_t fun = [](CLI::results_t){
            return true;
        };
        
        Option* opt = add_option(name, fun, description, false);
        if(opt->get_positional())
            throw IncorrectConstruction("Flags cannot be positional");
        opt->expected_ = 0;
        return opt;
    }

    /// Add option for flag integer
    template<typename T,
        enable_if_t<std::is_integral<T>::value && !is_bool<T>::value, detail::enabler> = detail::dummy>
    Option* add_flag(
            std::string name,
            T &count,                   ///< A varaible holding the count
            std::string description=""
            ) {

        count = 0;
        CLI::callback_t fun = [&count](CLI::results_t res){
            count = (T) res.size();
            return true;
        };
        
        Option* opt = add_option(name, fun, description, false);
        if(opt->get_positional())
            throw IncorrectConstruction("Flags cannot be positional");
        opt->expected_ = 0;
        return opt;
    }

    /// Bool version only allows the flag once
    template<typename T,
        enable_if_t<is_bool<T>::value, detail::enabler> = detail::dummy>
    Option* add_flag(
            std::string name,
            T &count,                   ///< A varaible holding true if passed
            std::string description=""
            ) {

        count = false;
        CLI::callback_t fun = [&count](CLI::results_t res){
            count = true;
            return res.size() == 1;
        };
        
        Option* opt = add_option(name, fun, description, false);
        if(opt->get_positional())
            throw IncorrectConstruction("Flags cannot be positional");
        opt->expected_ = 0;
        return opt;
    }


    /// Add set of options
    template<typename T>
    Option* add_set(
            std::string name,
            T &member,                     ///< The selected member of the set
            std::set<T> options,           ///< The set of posibilities
            std::string description="",
            bool defaulted=false
            ) {

        CLI::callback_t fun = [&member, options](CLI::results_t res){
            if(res.size()!=1) {
                return false;
            }
            bool retval = detail::lexical_cast(res[0], member);
            if(!retval)
                return false;
            return std::find(std::begin(options), std::end(options), member) != std::end(options);
        };

        Option* opt = add_option(name, fun, description, defaulted);
        opt->typeval_ = detail::type_name<T>();
        opt->typeval_ += " in {" + detail::join(options) + "}";
        if(defaulted) {
            std::stringstream out;
            out << member;
            opt->defaultval_ = out.str();
        }
        return opt;
    }

    /// Add set of options, string only, ignore case
    Option* add_set_ignore_case(
            std::string name,
            std::string &member,                      ///< The selected member of the set
            std::set<std::string> options,            ///< The set of posibilities
            std::string description="",
            bool defaulted=false
            ) {

        CLI::callback_t fun = [&member, options](CLI::results_t res){
            if(res.size()!=1) {
                return false;
            }
            member = detail::to_lower(res[0]);
            auto iter = std::find_if(std::begin(options), std::end(options),
                    [&member](std::string val){return detail::to_lower(val) == member;});
            if(iter == std::end(options))
                return false;
            else {
                member = *iter;
                return true;
            }
        };

        Option* opt = add_option(name, fun, description, defaulted);
        opt->typeval_ = detail::type_name<std::string>();
        opt->typeval_ += " in {" + detail::join(options) + "}";
        if(defaulted) {
            opt->defaultval_ = detail::to_lower(member);
        }
        return opt;
    }


    /// Add a configuration ini file option
    Option* add_config(std::string name="--config",
                 std::string default_filename="",
                 std::string help="Read an ini file",
                 bool required=false) {

        // Remove existing config if present
        if(config_ptr_ != nullptr)
            remove_option(config_ptr_);
        config_name_ = default_filename;
        config_required_ = required;
        config_ptr_ = add_option(name, config_name_, help, default_filename!="");
        return config_ptr_;
    }

    /// Removes an option from the App. Takes an option pointer. Returns true if found and removed.
    bool remove_option(Option* opt) {
        auto iterator = std::find_if(std::begin(options_), std::end(options_),
                [opt](const Option_p &v){return v.get() == opt;});
        if (iterator != std::end(options_)) {
            options_.erase(iterator);
            return true;
        }
        return false;
    }
    
    ///@}
    /// @name Subcommmands
    ///@{

    /// Add a subcommand. Like the constructor, you can override the help message addition by setting help=false
    App* add_subcommand(std::string name, std::string description="", bool help=true) {
        subcommands_.emplace_back(new App(description, help, detail::dummy));
        subcommands_.back()->name_ = name;
        subcommands_.back()->allow_extras();
        subcommands_.back()->parent_ = this;
        subcommands_.back()->ignore_case_ = ignore_case_;
        subcommands_.back()->fallthrough_ = fallthrough_;
        for(const auto& subc : subcommands_)
            if(subc.get() != subcommands_.back().get())
                if(subc->check_name(subcommands_.back()->name_) || subcommands_.back()->check_name(subc->name_)) 
                    throw OptionAlreadyAdded(subc->name_);
        return subcommands_.back().get();
    }



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

    /// Parses the command line - throws errors
    /// This must be called after the options are in but before the rest of the program.
    std::vector<std::string> parse(int argc, char **argv) {
        name_ = argv[0];
        std::vector<std::string> args;
        for(int i=argc-1; i>0; i--)
            args.push_back(argv[i]);
        return parse(args);
        
    }

    /// The real work is done here. Expects a reversed vector.
    /// Changes the vector to the remaining options.
    std::vector<std::string>& parse(std::vector<std::string> &args) {
        _parse(args);
        run_callback();
        return args;
    }

    /// Print a nice error message and return the exit code
    int exit(const Error& e) const {
        if(e.exit_code != 0) {
            std::cerr << "ERROR: ";
            std::cerr << e.what() << std::endl;
            if(e.print_help)
                std::cerr << help();
        } else {
            if(e.print_help)
                std::cout << help();
        }
        return e.exit_code;
    }

    /// Reset the parsed data
    void reset() {

        selected_subcommands_.clear();
        missing_.clear();

        for(const Option_p &opt : options_) {
            opt->clear();
        }
        for(const App_p &app : subcommands_) {
            app->reset();
        }
    }

    ///@}
    /// @name Post parsing
    ///@{

    /// Counts the number of times the given option was passed.
    int count(std::string name) const {
        for(const Option_p &opt : options_) {
            if(opt->check_name(name)) {
                return opt->count();
            }
        }
        throw OptionNotFound(name);
    }

    /// Get a subcommand pointer list to the currently selected subcommands (after parsing)
    std::vector<App*> get_subcommands() {
        return selected_subcommands_;
    }


    /// Check to see if selected subcommand in list
    bool got_subcommand(App* subcom) const {
        return std::find(std::begin(selected_subcommands_),
                         std::end(selected_subcommands_), subcom)
                           != std::end(selected_subcommands_);
    }

    /// Check with name instead of pointer
    bool got_subcommand(std::string name) const {
        for(const auto subcomptr : selected_subcommands_)
            if(subcomptr->check_name(name))
                return true;
        return false;
    }
    
    ///@}
    /// @name Help
    ///@{

    /// Produce a string that could be read in as a config of the current values of the App
    std::string config_to_str() const {
        std::stringstream out;
        for(const Option_p &opt : options_) {
            if(opt->lnames_.size() > 0 && opt->count() > 0 && opt->get_expected() > 0)
                out << opt->lnames_[0] << "=" << detail::join(opt->results()) << std::endl;
        }
        return out.str();
    }

    /// Makes a help message, with a column wid for column 1
    std::string help(size_t wid=30, std::string prev="") const {
        // Delegate to subcommand if needed
        if(prev == "")
            prev = name_;
        else
            prev += " " + name_;

        if(selected_subcommands_.size() > 0)
            return selected_subcommands_.at(0)->help(wid, prev);

        std::stringstream out;
        out << description_ << std::endl;
        out << "Usage: " << prev;
        
        // Check for options_
        bool npos = false;
        std::set<std::string> groups;
        for(const Option_p &opt : options_) {
            if(opt->nonpositional()) {
                npos = true;
                groups.insert(opt->get_group());
            }
        }

        if(npos)
            out << " [OPTIONS]";

        // Positionals
        bool pos=false;
        for(const Option_p &opt : options_)
            if(opt->get_positional()) {
                // A hidden positional should still show up in the usage statement
                //if(detail::to_lower(opt->get_group()) == "hidden")
                //    continue;
                out << " " << opt->help_positional();
                if(opt->has_description())
                    pos=true;
            }

        if(subcommands_.size() > 0) {
            if(require_subcommand_ != 0)
                out << " SUBCOMMAND";
            else
                out << " [SUBCOMMAND]";
        }

        out << std::endl << std::endl;

        // Positional descriptions
        if(pos) {
            out << "Positionals:" << std::endl;
            for(const Option_p &opt : options_) {
                if(detail::to_lower(opt->get_group()) == "hidden")
                    continue;
                if(opt->get_positional() && opt->has_description())
                    detail::format_help(out, opt->help_pname(), opt->get_description(), wid);
            }
            out << std::endl;

        }


        // Options
        if(npos) {
            for (const std::string& group : groups) {
                if(detail::to_lower(group) == "hidden")
                    continue;
                out << group << ":" << std::endl;
                for(const Option_p &opt : options_) {
                    if(opt->nonpositional() && opt->get_group() == group)
                        detail::format_help(out, opt->help_name(), opt->get_description(), wid);
                    
                }
                out << std::endl;
            }
        }

        // Subcommands
        if(subcommands_.size()> 0) {
            out << "Subcommands:" << std::endl;
            for(const App_p &com : subcommands_)
                detail::format_help(out, com->get_name(), com->description_, wid);
        }
        return out.str();
    }

    ///@}
    /// @name Getters
    ///@{

    /// Get a pointer to the help flag.
    Option* get_help_ptr() {
        return help_ptr_;
    }

    /// Get a pointer to the config option.
    Option* get_config_ptr() {
        return config_ptr_;
    }

    /// Get the name of the current app
    std::string get_name() const {
        return name_;
    }

    /// Check the name, case insensitive if set
    bool check_name(std::string name_to_check) const {
        std::string local_name = name_;
        if(ignore_case_) {
            local_name = detail::to_lower(name_);
            name_to_check = detail::to_lower(name_to_check);
        }

        return local_name == name_to_check;
    }

    ///@}


protected:


    /// Return missing from the master
    missing_t* missing() {
        if(parent_ != nullptr)
            return parent_->missing();
        return &missing_;
    }

    /// Internal function to run (App) callback, top down
    void run_callback() {
        pre_callback();
        if(callback_)
            callback_();
        for(App* subc : selected_subcommands_) {
            subc->run_callback();
        }
    }

    bool _valid_subcommand(const std::string &current) const {
        for(const App_p &com : subcommands_)
            if(com->check_name(current))
                return true;
        if(parent_ != nullptr)
            return parent_->_valid_subcommand(current);
        else
            return false;
    }


    /// Selects a Classifer enum based on the type of the current argument
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
        bool positional_only = false;
        
        while(args.size()>0) {
            _parse_single(args, positional_only);
        }

        if (help_ptr_ != nullptr && help_ptr_->count() > 0) {
            throw CallForHelp();
        }


        // Process an INI file
        if (config_ptr_ != nullptr && config_name_ != "") {
            try {
                std::vector<detail::ini_ret_t> values = detail::parse_ini(config_name_);
                while(values.size() > 0) {
                    if(!_parse_env(values)) {
                        throw ExtrasError(values.back().name());
                    }
                }
            } catch (const FileError &) {
                if(config_required_)
                    throw;
            }
        }

        
        // Get envname options if not yet passed
        for(const Option_p& opt : options_) {
            if (opt->count() == 0 && opt->envname_ != "") {
                char *ename = std::getenv(opt->envname_.c_str());
                if(ename != nullptr) {
                    opt->add_result(std::string(ename));
                }
            }
        }

        // Process callbacks
        for(const Option_p& opt : options_) {
            if (opt->count() > 0) {
                opt->run_callback();
            }
        }

        // Verify required options 
        for(const Option_p& opt : options_) {
            // Required
            if (opt->get_required()
                    && (opt->count() < opt->get_expected() || opt->count() == 0))
                throw RequiredError(opt->get_name());
            // Requires
            for (const Option* opt_req : opt->requires_)
                if (opt->count() > 0 && opt_req->count() == 0)
                    throw RequiresError(opt->get_name(), opt_req->get_name());
            // Excludes
            for (const Option* opt_ex : opt->excludes_)
                if (opt->count() > 0 && opt_ex->count() != 0)
                    throw ExcludesError(opt->get_name(), opt_ex->get_name());
        }

        if(require_subcommand_ < 0 && selected_subcommands_.size() == 0)
            throw RequiredError("Subcommand required");
        else if(require_subcommand_ > 0 && (int) selected_subcommands_.size() != require_subcommand_)
            throw RequiredError(std::to_string(require_subcommand_) + " subcommand(s) required");

        // Convert missing (pairs) to extras (string only)
        if(parent_ == nullptr) {
            args.resize(missing()->size());
            std::transform(std::begin(*missing()), std::end(*missing()), std::begin(args),
                    [](const std::pair<detail::Classifer, std::string>& val){return val.second;});
            std::reverse(std::begin(args), std::end(args));

            size_t num_left_over = std::count_if(std::begin(*missing()), std::end(*missing()),
                    [](std::pair<detail::Classifer, std::string>& val){return val.first != detail::Classifer::POSITIONAL_MARK;});

            if(num_left_over>0 && !allow_extras_)
                throw ExtrasError("[" + detail::rjoin(args, " ") + "]");
        }
    }

    /// Parse one env param, ignore if not applicable, remove if it is
    ///
    /// If this has more than one dot.separated.name, go into the subcommand matching it
    /// Returns true if it managed to find the option, if false you'll need to remove the arg manully.
    bool _parse_env(std::vector<detail::ini_ret_t> &args) {
        detail::ini_ret_t& current = args.back();
        std::string parent = current.parent();
        std::string name = current.name();
        if(parent != "") {
            current.level++;
            for(const App_p &com : subcommands_) 
                if(com->check_name(parent))
                    return com->_parse_env(args);
            return false;
        }

        auto op_ptr = std::find_if(std::begin(options_), std::end(options_),
                [name](const Option_p &v){return v->check_lname(name);});
        
        if(op_ptr == std::end(options_)) 
            return false;
            
        // Let's not go crazy with pointer syntax
        Option_p& op = *op_ptr;

        
        if(op->results_.empty()) {
            // Flag parsing
            if(op->get_expected() == 0) {
                if(current.inputs.size() == 1) {
                    std::string val = current.inputs.at(0);
                    val = detail::to_lower(val);
                    if(val == "true" || val == "on" || val == "yes")
                        op->results_ = {""};
                    else if(val == "false" || val == "off" || val == "no")
                        ;
                    else
                        try {
                            size_t ui = std::stoul(val);
                            for (size_t i=0; i<ui; i++)
                                op->results_.push_back("");
                        } catch (const std::invalid_argument &) {
                            throw ConversionError(current.fullname + ": Should be true/false or a number");
                        }
                } else
                    throw ConversionError(current.fullname + ": too many inputs for a flag");
            } else
                op->results_ = current.inputs;
        }

        args.pop_back();
        return true;
    }

    /// Parse "one" argument (some may eat more than one), delegate to parent if fails, add to missing if missing from master
    void _parse_single(std::vector<std::string> &args, bool &positional_only) {

        detail::Classifer classifer = positional_only ? detail::Classifer::NONE : _recognize(args.back());
        switch(classifer) {
        case detail::Classifer::POSITIONAL_MARK:
            missing()->emplace_back(classifer, args.back());
            args.pop_back();
            positional_only = true;
            break;
        case detail::Classifer::SUBCOMMAND:
            _parse_subcommand(args);
            break;
        case detail::Classifer::LONG:
            // If already parsed a subcommand, don't accept options_
            _parse_long(args);
            break;
        case detail::Classifer::SHORT:
            // If already parsed a subcommand, don't accept options_
            _parse_short(args);
            break;
        case detail::Classifer::NONE:
            // Probably a positional or something for a parent (sub)command
            _parse_positional(args);
        }

        
    }

    /// Parse a positional, go up the tree to check
    void _parse_positional(std::vector<std::string> &args) {

        std::string positional = args.back();
        for(const Option_p& opt : options_) {
            // Eat options, one by one, until done
            if (    opt->get_positional()
                    && opt->count() < opt->get_expected()
                    ) {

                opt->add_result(positional);
                args.pop_back();
                return;
            }
        }

        if(parent_ != nullptr && fallthrough_)
            return parent_->_parse_positional(args);
        else {
            args.pop_back();
            missing()->emplace_back(detail::Classifer::NONE, positional);
        }
    }

    /// Parse a subcommand, modify args and continue
    ///
    /// Unlike the others, this one will always allow fallthrough
    void _parse_subcommand(std::vector<std::string> &args) {
        for(const App_p &com : subcommands_) {
            if(com->check_name(args.back())){ 
                args.pop_back();
                selected_subcommands_.push_back(com.get());
                com->_parse(args);
                return;
            }
        }
        if(parent_ != nullptr)
            return parent_->_parse_subcommand(args);
        else
            throw HorribleError("Subcommand " + args.back() + " missing");
    }
 
    /// Parse a short argument, must be at the top of the list
    void _parse_short(std::vector<std::string> &args) {
        std::string current = args.back();

        std::string name;
        std::string rest;
        if(!detail::split_short(current, name, rest))
            throw HorribleError("Short");

        auto op_ptr = std::find_if(std::begin(options_), std::end(options_), [name](const Option_p &opt){return opt->check_sname(name);});


        // Option not found
        if(op_ptr == std::end(options_)) {
            // If a subcommand, try the master command
            if(parent_ != nullptr && fallthrough_)
                return parent_->_parse_short(args);
            // Otherwise, add to missing
            else {
                args.pop_back();
                missing()->emplace_back(detail::Classifer::SHORT, current);
                return;
            }
        }

        args.pop_back();


        // Get a reference to the pointer to make syntax bearable
        Option_p& op = *op_ptr;

        int num = op->get_expected();
       
        if(num == 0)
            op->add_result("");
        else if(rest!="") {
            if (num > 0)
                num--;
            op->add_result(rest);
            rest = "";
        }


        if(num == -1) {
            while(args.size()>0 && _recognize(args.back()) == detail::Classifer::NONE) {
                op->add_result(args.back());
                args.pop_back();
            }
        } else while(num>0 && args.size() > 0) {
            num--;
            std::string current_ = args.back();
            args.pop_back();
            op->add_result(current_);
        }

        if(rest != "") {
            rest = "-" + rest;
            args.push_back(rest);
        }
    }

    /// Parse a long argument, must be at the top of the list
    void _parse_long(std::vector<std::string> &args) {
        std::string current = args.back();

        std::string name;
        std::string value;
        if(!detail::split_long(current, name, value))
            throw HorribleError("Long:" + args.back());

        auto op_ptr = std::find_if(std::begin(options_), std::end(options_), [name](const Option_p &v){return v->check_lname(name);});

        // Option not found
        if(op_ptr == std::end(options_)) {
            // If a subcommand, try the master command
            if(parent_ != nullptr && fallthrough_)
                return parent_->_parse_long(args);
            // Otherwise, add to missing
            else {
                args.pop_back();
                missing()->emplace_back(detail::Classifer::LONG, current);
                return;
            }
        }

        args.pop_back();

        // Get a reference to the pointer to make syntax bearable
        Option_p& op = *op_ptr;

        int num = op->get_expected();
        
        if(value != "") {
            if(num!=-1) num--;
            op->add_result(value);
        } else if (num == 0) {
            op->add_result("");
        }

        if(num == -1) {
            while(args.size() > 0 && _recognize(args.back()) == detail::Classifer::NONE) {
                op->add_result(args.back());
                args.pop_back();
            }
        } else while(num>0 && args.size()>0) {
            num--;
            op->add_result(args.back());
            args.pop_back();
        }
        return;
    }

};


}
