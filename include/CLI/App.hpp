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

typedef std::unique_ptr<Option> Option_p;
typedef std::unique_ptr<App> App_p;

/// Creates a command line program, with very few defaults.
/** To use, create a new `Program()` instance with `argc`, `argv`, and a help description. The templated
*  add_option methods make it easy to prepare options. Remember to call `.start` before starting your
* program, so that the options can be evaluated and the help option doesn't accidentally run your program. */
class App {
protected:
    
    std::string name;
    std::string prog_description;
    std::vector<Option_p> options;
    
    /// Pair of classifer, string for missing options. (extra detail is removed on returning from parse)
    std::vector<std::pair<detail::Classifer, std::string>> missing;
    bool no_extras {true};

    std::vector<App_p> subcommands;
    bool parsed {false};
    std::vector<App*> selected_subcommands;
    bool required_subcommand = false;
    std::string progname {"program"};
    Option* help_flag {nullptr};

    std::function<void()> app_callback;

    std::string ini_file;
    bool ini_required {false};
    Option* ini_setting {nullptr};

    bool case_insensitive {false};
   

public:
    /// Create a new program. Pass in the same arguments as main(), along with a help string.
    App(std::string prog_description="", bool help=true)
        : prog_description(prog_description) {

        if(help)
            help_flag = add_flag("-h,--help", "Print this help message and exit");

    }


    /// Set a callback for the end of parsing. Due to a bug in c++11,
    /// it is not possible to overload on std::function (fixed in c++14
    /// and backported to c++11 on newer compilers). Use capture by reference
    /// to get a pointer to App if needed.
    void set_callback(std::function<void()> callback) {
        app_callback = callback;
    }

    /// Reset the parsed data
    void reset() {

        parsed = false;
        selected_subcommands.clear();
        missing.clear();

        for(const Option_p &opt : options) {
            opt->clear();
        }
        for(const App_p &app : subcommands) {
            app->reset();
        }
    }

    /// Get a pointer to the help flag.
    Option* get_help_ptr() {
        return help_flag;
    }

    /// Get a pointer to the config option.
    Option* get_config_ptr() {
        return ini_setting;
    }

    /// Produce a string that could be read in as a config of the current values of the App
    std::string config_to_str() const {
        std::stringstream out;
        for(const Option_p &opt : options) {
            if(opt->lnames.size() > 0 && opt->count() > 0 && opt->get_expected() > 0)
                out << opt->lnames[0] << "=" << detail::join(opt->flatten_results()) << std::endl;
        }
        return out.str();
    }
    
    /// Add a subcommand. Like the constructor, you can override the help message addition by setting help=false
    App* add_subcommand(std::string name_, std::string description="", bool help=true) {
        subcommands.emplace_back(new App(description, help));
        subcommands.back()->name = name_;
        subcommands.back()->allow_extras();
        subcommands.back()->case_insensitive = case_insensitive;
        return subcommands.back().get();
    }

    /// Add an option, will automatically understand the type for common types.
    /** To use, create a variable with the expected type, and pass it in after the name.
     * After start is called, you can use count to see if the value was passed, and
     * the value will be initialized properly. 
     *
     * ->required(), ->default, and the validators are options, 
     * The positional options take an optional number of arguments.
     *
     * For example,
     *
     *     std::string filename
     *     program.add_option("filename", filename, "description of filename");
     */
    Option* add_option(
            std::string name_,
            callback_t callback,
            std::string description="", 
            bool defaulted=false
            ) {
        Option myopt{name_, description, callback, defaulted};
        if(std::find_if(std::begin(options), std::end(options),
                    [&myopt](const Option_p &v){return *v == myopt;}) == std::end(options)) {
            options.emplace_back();
            Option_p& option = options.back();
            option.reset(new Option(name_, description, callback, defaulted));
            return option.get();
        } else
            throw OptionAlreadyAdded(myopt.get_name());

    }

    /// Add option for string
    template<typename T, enable_if_t<!is_vector<T>::value, detail::enabler> = detail::dummy>
    Option* add_option(
            std::string name_,
            T &variable,                ///< The variable to set
            std::string description="",
            bool defaulted=false
            ) {

        
        CLI::callback_t fun = [&variable](CLI::results_t res){
            if(res.size()!=1) {
                return false;
            }
            if(res[0].size()!=1) {
                return false;
            }
            return detail::lexical_cast(res[0][0], variable);
        };

        Option* retval = add_option(name_, fun, description, defaulted);
        retval->typeval = detail::type_name<T>();
        if(defaulted) {
            std::stringstream out;
            out << variable;
            retval->defaultval = out.str();
        }
        return retval;
    }

    /// Add option for vector of results
    template<typename T>
    Option* add_option(
            std::string name_,
            std::vector<T> &variable,   ///< The variable vector to set
            std::string description="",
            bool defaulted=false
            ) {

        CLI::callback_t fun = [&variable](CLI::results_t res){
            bool retval = true;
            variable.clear();
            for(const auto &a : res)
                for(const auto &b : a) {
                    variable.emplace_back();
                    retval &= detail::lexical_cast(b, variable.back());
                }
            return variable.size() > 0 && retval;
        };

        Option* retval =  add_option(name_, fun, description, defaulted);
        retval->allow_vector = true;
        retval->_expected = -1;
        retval->typeval = detail::type_name<T>();
        if(defaulted)
            retval->defaultval =  "[" + detail::join(variable) + "]";
        return retval;
    }


    /// Add option for flag
    Option* add_flag(
            std::string name_,
            std::string description=""
            ) {
        CLI::callback_t fun = [](CLI::results_t){
            return true;
        };
        
        Option* opt = add_option(name_, fun, description, false);
        if(opt->get_positional())
            throw IncorrectConstruction("Flags cannot be positional");
        opt->_expected = 0;
        return opt;
    }

    /// Add option for flag
    template<typename T,
        enable_if_t<std::is_integral<T>::value && !is_bool<T>::value, detail::enabler> = detail::dummy>
    Option* add_flag(
            std::string name_,
            T &count,                   ///< A varaible holding the count
            std::string description=""
            ) {

        count = 0;
        CLI::callback_t fun = [&count](CLI::results_t res){
            count = (T) res.size();
            return true;
        };
        
        Option* opt = add_option(name_, fun, description, false);
        if(opt->get_positional())
            throw IncorrectConstruction("Flags cannot be positional");
        opt->_expected = 0;
        return opt;
    }

    /// Bool version only allows the flag once
    template<typename T,
        enable_if_t<is_bool<T>::value, detail::enabler> = detail::dummy>
    Option* add_flag(
            std::string name_,
            T &count,                   ///< A varaible holding true if passed
            std::string description=""
            ) {

        count = false;
        CLI::callback_t fun = [&count](CLI::results_t res){
            count = true;
            return res.size() == 1;
        };
        
        Option* opt = add_option(name_, fun, description, false);
        if(opt->get_positional())
            throw IncorrectConstruction("Flags cannot be positional");
        opt->_expected = 0;
        return opt;
    }


    /// Add set of options
    template<typename T>
    Option* add_set(
            std::string name_,
            T &member,                     ///< The selected member of the set
            std::set<T> _options,           ///< The set of posibilities
            std::string description="",
            bool defaulted=false
            ) {

        CLI::callback_t fun = [&member, _options](CLI::results_t res){
            if(res.size()!=1) {
                return false;
            }
            if(res[0].size()!=1) {
                return false;
            }
            bool retval = detail::lexical_cast(res[0][0], member);
            if(!retval)
                return false;
            return std::find(std::begin(_options), std::end(_options), member) != std::end(_options);
        };

        Option* retval = add_option(name_, fun, description, defaulted);
        retval->typeval = detail::type_name<T>();
        retval->typeval += " in {" + detail::join(_options) + "}";
        if(defaulted) {
            std::stringstream out;
            out << member;
            retval->defaultval = out.str();
        }
        return retval;
    }

    /// Add set of options, string only, ignore case
    Option* add_set_ignore_case(
            std::string name_,
            std::string &member,                      ///< The selected member of the set
            std::set<std::string> _options,           ///< The set of posibilities
            std::string description="",
            bool defaulted=false
            ) {

        CLI::callback_t fun = [&member, _options](CLI::results_t res){
            if(res.size()!=1) {
                return false;
            }
            if(res[0].size()!=1) {
                return false;
            }
            member = detail::to_lower(res.at(0).at(0));
            auto iter = std::find_if(std::begin(_options), std::end(_options),
                    [&member](std::string val){return detail::to_lower(val) == member;});
            if(iter == std::end(_options))
                return false;
            else {
                member = *iter;
                return true;
            }
        };

        Option* retval = add_option(name_, fun, description, defaulted);
        retval->typeval = detail::type_name<std::string>();
        retval->typeval += " in {" + detail::join(_options) + "}";
        if(defaulted) {
            retval->defaultval = detail::to_lower(member);
        }
        return retval;
    }


    /// Add a configuration ini file option
    Option* add_config(std::string name_="--config",
                 std::string default_filename="",
                 std::string help="Read an ini file",
                 bool required=false) {

        // Remove existing config if present
        if(ini_setting != nullptr)
            remove_option(ini_setting);
        ini_file = default_filename;
        ini_required = required;
        ini_setting = add_option(name_, ini_file, help, default_filename!="");
        return ini_setting;
    }

    /// Removes an option from the App. Takes an option pointer. Returns true if found and removed.
    bool remove_option(Option* opt) {
        auto iterator = std::find_if(std::begin(options), std::end(options),
                [opt](const Option_p &v){return v.get() == opt;});
        if (iterator != std::end(options)) {
            options.erase(iterator);
            return true;
        }
        return false;
    }

    /// This allows subclasses to inject code before callbacks but after parse
    /// This does not run if any errors or help is thrown.
    virtual void pre_callback() {}

    /// Parses the command line - throws errors
    /// This must be called after the options are in but before the rest of the program.
    std::vector<std::string> parse(int argc, char **argv) {
        progname = argv[0];
        std::vector<std::string> args;
        for(int i=argc-1; i>0; i--)
            args.push_back(argv[i]);
        return parse(args);
        
    }

    /// The real work is done here. Expects a reversed vector.
    /// Changes the vector to the remaining options.
    std::vector<std::string>& parse(std::vector<std::string> &args) {
        return _parse(args);
    }

    /// Remove the error when extras are left over on the command line.
    void allow_extras (bool allow=true) {
        no_extras = !allow;
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

    /// Counts the number of times the given option was passed.
    int count(std::string name_) const {
        for(const Option_p &opt : options) {
            if(opt->check_name(name_)) {
                return opt->count();
            }
        }
        throw OptionNotFound(name_);
    }

    /// Makes a help message, with a column `wid` for column 1
    std::string help(size_t wid=30, std::string prev="") const {
        // Delegate to subcommand if needed
        if(prev == "")
            prev = progname;
        else
            prev += " " + name;

        if(selected_subcommands.size() > 0)
            return selected_subcommands.at(0)->help(wid, prev);

        std::stringstream out;
        out << prog_description << std::endl;
        out << "Usage: " << prev;
        
        // Check for options
        bool npos = false;
        std::set<std::string> groups;
        for(const Option_p &opt : options) {
            if(opt->nonpositional()) {
                npos = true;
                groups.insert(opt->get_group());
            }
        }

        if(npos)
            out << " [OPTIONS]";

        // Positionals
        bool pos=false;
        for(const Option_p &opt : options)
            if(opt->get_positional()) {
                out << " " << opt->help_positional();
                if(opt->has_description())
                    pos=true;
            }

        if(subcommands.size() > 0) {
            if(required_subcommand)
                out << " SUBCOMMAND";
            else
                out << " [SUBCOMMAND]";
        }

        out << std::endl << std::endl;

        // Positional descriptions
        if(pos) {
            out << "Positionals:" << std::endl;
            for(const Option_p &opt : options)
                if(opt->get_positional() && opt->has_description())
                    detail::format_help(out, opt->help_pname(), opt->get_description(), wid);
            out << std::endl;

        }


        // Options
        if(npos) {
            for (const std::string& group : groups) {
                out << group << ":" << std::endl;
                for(const Option_p &opt : options) {
                    if(opt->nonpositional() && opt->get_group() == group)
                        detail::format_help(out, opt->help_name(), opt->get_description(), wid);
                    
                }
                out << std::endl;
            }
        }

        // Subcommands
        if(subcommands.size()> 0) {
            out << "Subcommands:" << std::endl;
            for(const App_p &com : subcommands)
                detail::format_help(out, com->get_name(), com->prog_description, wid);
        }
        return out.str();
    }
    
    /// Get a subcommand pointer list to the currently selected subcommands (after parsing)
    std::vector<App*> get_subcommands() {
        return selected_subcommands;
    }


    /// Check to see if selected subcommand in list
    bool got_subcommand(App* subcom) const {
        return std::find(std::begin(selected_subcommands), std::end(selected_subcommands), subcom) != std::end(selected_subcommands);
    }

    /// Check with name instead of pointer
    bool got_subcommand(std::string name_) const {
        for(const auto subcomptr : selected_subcommands)
            if(subcomptr->check_name(name_))
                return true;
        return false;
    }
    
    /// Get the name of the current app
    std::string get_name() const {
        return name;
    }

    /// Check the name, case insensitive if set
    bool check_name(std::string name_to_check) const {
        std::string local_name = name;
        if(case_insensitive) {
            local_name = detail::to_lower(name);
            name_to_check = detail::to_lower(name_to_check);
        }

        return local_name == name_to_check;
    }

    /// Ignore case
    App* ignore_case(bool value = true) {
        case_insensitive = value;
        return this;
    }

    /// Require a subcommand to be given (does not affect help call)
    /// Does not return a pointer since it is supposed to be called on the main App.
    void require_subcommand(bool value = true) {
        required_subcommand = value;
    }

protected:

    /// Internal function to run (App) callback
    void run_callback() {
        if(app_callback)
            app_callback();
    }

    /// Selects a Classifer enum based on the type of the current argument
    detail::Classifer _recognize(std::string current) const {
        std::string dummy1, dummy2;

        if(current == "--")
            return detail::Classifer::POSITIONAL_MARK;
        for(const App_p &com : subcommands) {
            if(com->check_name(current))
                return detail::Classifer::SUBCOMMAND;
        }
        if(detail::split_long(current, dummy1, dummy2))
            return detail::Classifer::LONG;
        if(detail::split_short(current, dummy1, dummy2))
            return detail::Classifer::SHORT;
        return detail::Classifer::NONE;
    }


    /// Internal parse function
    std::vector<std::string>& _parse(std::vector<std::string> &args) {
        parsed = true;

        bool positional_only = false;
        
        while(args.size()>0) {


            detail::Classifer classifer = positional_only ? detail::Classifer::NONE : _recognize(args.back());
            switch(classifer) {
            case detail::Classifer::POSITIONAL_MARK:
                missing.emplace_back(classifer, args.back());
                args.pop_back();
                positional_only = true;
                break;
            case detail::Classifer::SUBCOMMAND:
                _parse_subcommand(args);
                break;
            case detail::Classifer::LONG:
                // If already parsed a subcommand, don't accept options
                if(selected_subcommands.size() > 0) {
                    missing.emplace_back(classifer, args.back());
                    args.pop_back();
                } else
                    _parse_long(args);
                break;
            case detail::Classifer::SHORT:
                // If already parsed a subcommand, don't accept options
                if(selected_subcommands.size() > 0) {
                    missing.emplace_back(classifer, args.back());
                    args.pop_back();
                } else
                    _parse_short(args);
                break;
            case detail::Classifer::NONE:
                // Probably a positional or something for a parent (sub)command
                missing.emplace_back(classifer, args.back());
                args.pop_back();
            }
        }

        if (help_flag != nullptr && help_flag->count() > 0) {
            throw CallForHelp();
        }


        // Collect positionals
        
        // Loop over all positionals
        for(size_t i=0; i<missing.size(); i++) {

            // Skip non-positionals (speedup)
            if(missing.at(i).first != detail::Classifer::NONE)
                continue; 

            // Loop over all options
            for(const Option_p& opt : options) {

                // Eat options, one by one, until done
                while (    opt->get_positional()
                        && opt->count() < opt->get_expected()
                        && i < missing.size()
                        ) {

                    // Skip options, only eat positionals
                    if(missing.at(i).first != detail::Classifer::NONE) {
                        i++;
                        continue;
                    }

                    opt->get_new();
                    opt->add_result(0, missing.at(i).second);
                    missing.erase(missing.begin() + i); // Remove option that was eaten
                    // Don't need to remove 1 from i since this while loop keeps reading i
                }
            }
        }

        // Process an INI file
        if (ini_setting != nullptr && ini_file != "") {
            try {
                std::vector<std::string> values = detail::parse_ini(ini_file);
                while(values.size() > 0) {
                    _parse_long(values, false);
                }
                
            } catch (const FileError &) {
                if(ini_required)
                    throw;
            }
        }

        
        // Get envname options if not yet passed
        for(const Option_p& opt : options) {
            if (opt->count() == 0 && opt->_envname != "") {
                char *ename = std::getenv(opt->_envname.c_str());
                if(ename != nullptr) {
                    opt->get_new();
                    opt->add_result(0, std::string(ename));
                }
            }
        }

        // Process callbacks
        for(const Option_p& opt : options) {
            if (opt->count() > 0) {
                opt->run_callback();
            }
        }

        // Verify required options 
        for(const Option_p& opt : options) {
            // Required
            if (opt->get_required()
                    && (opt->count() < opt->get_expected() || opt->count() == 0))
                throw RequiredError(opt->get_name());
            // Requires
            for (const Option* opt_req : opt->_requires)
                if (opt->count() > 0 && opt_req->count() == 0)
                    throw RequiresError(opt->get_name(), opt_req->get_name());
            // Excludes
            for (const Option* opt_ex : opt->_excludes)
                if (opt->count() > 0 && opt_ex->count() != 0)
                    throw ExcludesError(opt->get_name(), opt_ex->get_name());
        }

        if(required_subcommand && selected_subcommands.size() == 0)
            throw RequiredError("Subcommand required");

        // Convert missing (pairs) to extras (string only)
        args.resize(missing.size());
        std::transform(std::begin(missing), std::end(missing), std::begin(args),
                [](const std::pair<detail::Classifer, std::string>& val){return val.second;});
        std::reverse(std::begin(args), std::end(args));

        size_t num_left_over = std::count_if(std::begin(missing), std::end(missing),
                [](std::pair<detail::Classifer, std::string>& val){return val.first != detail::Classifer::POSITIONAL_MARK;});

        if(num_left_over>0 && no_extras)
            throw ExtrasError("[" + detail::join(args, " ") + "]");

        pre_callback();
        run_callback();

        return args;
    }


    /// Parse a subcommand, modify args and continue
    void _parse_subcommand(std::vector<std::string> &args) {
        for(const App_p &com : subcommands) {
            if(com->check_name(args.back())){ 
                args.pop_back();
                selected_subcommands.push_back(com.get());
                com->parse(args);
                return;
            }
        }
        throw HorribleError("Subcommand");
    }
 
    /// Parse a short argument, must be at the top of the list
    void _parse_short(std::vector<std::string> &args) {
        std::string current = args.back();

        std::string name_;
        std::string rest;
        if(!detail::split_short(current, name_, rest))
            throw HorribleError("Short");
        args.pop_back();

        auto op_ptr = std::find_if(std::begin(options), std::end(options), [name_](const Option_p &opt){return opt->check_sname(name_);});

        if(op_ptr == std::end(options)) {
            missing.emplace_back(detail::Classifer::SHORT, "-" + name_);
            return;
        }

        // Get a reference to the pointer to make syntax bearable
        Option_p& op = *op_ptr;

        int vnum = op->get_new();
        int num = op->get_expected();
       
        if(num == 0)
            op->add_result(vnum, "");
        else if(rest!="") {
            if (num > 0)
                num--;
            op->add_result(vnum, rest);
            rest = "";
        }


        if(num == -1) {
            while(args.size()>0 && _recognize(args.back()) == detail::Classifer::NONE) {
                op->add_result(vnum, args.back());
                args.pop_back();
            }
        } else while(num>0 && args.size() > 0) {
            num--;
            std::string current_ = args.back();
            args.pop_back();
            op->add_result(vnum, current_);
        }

        if(rest != "") {
            rest = "-" + rest;
            args.push_back(rest);
        }
    }

    /// Parse a long argument, must be at the top of the list
    void _parse_long(std::vector<std::string> &args, bool overwrite=true) {
        std::string current = args.back();

        std::string name_;
        std::string value;
        if(!detail::split_long(current, name_, value))
            throw HorribleError("Long");
        args.pop_back();

        auto op_ptr = std::find_if(std::begin(options), std::end(options), [name_](const Option_p &v){return v->check_lname(name_);});

        if(op_ptr == std::end(options)) {
            missing.emplace_back(detail::Classifer::LONG, "--" + name_);
            return;
        }

        // Get a reference to the pointer to make syntax bearable
        Option_p& op = *op_ptr;


        // Stop if not overwriting options (for ini parse)
        if(!overwrite && op->count() > 0)
            return;

        int vnum = op->get_new();
        int num = op->get_expected();
        

        if(value != "") {
            if(num!=-1) num--;
            op->add_result(vnum, value);
        } else if (num == 0) {
            op->add_result(vnum, "");
        }

        if(num == -1) {
            while(args.size() > 0 && _recognize(args.back()) == detail::Classifer::NONE) {
                op->add_result(vnum, args.back());
                args.pop_back();
            }
        } else while(num>0 && args.size()>0) {
            num--;
            op->add_result(vnum,args.back());
            args.pop_back();
        }
        return;
    }

};


}
