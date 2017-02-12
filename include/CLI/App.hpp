#pragma once

// Distributed under the MIT license.  See accompanying
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

enum class Classifer {NONE, POSITIONAL_MARK, SHORT, LONG, SUBCOMMAND};

class App;

typedef std::unique_ptr<Option> Option_p;
typedef std::unique_ptr<App> App_p;

/// Creates a command line program, with very few defaults.
/** To use, create a new Program() instance with argc, argv, and a help description. The templated
*  add_option methods make it easy to prepare options. Remember to call `.start` before starting your
* program, so that the options can be evaluated and the help option doesn't accidentally run your program. */
class App {
protected:
    
    std::string name;
    std::string prog_description;
    std::vector<Option_p> options;
    std::vector<std::string> missing_options;
    std::deque<std::string> positionals;
    std::vector<App_p> subcommands;
    bool parsed {false};
    App* subcommand {nullptr};
    std::string progname {"program"};
    Option* help_flag {nullptr};

    std::function<void()> app_callback;

    std::string ini_file;
    bool ini_required {false};
    Option* ini_setting {nullptr};

public:

    /// Set a callback for the end of parsing. Due to a bug in c++11,
    /// it is not possible to overload on std::function (fixed in c++14
    /// and backported to c++11 on newer compilers). Use capture by reference
    /// to get a pointer to App if needed.
    void set_callback(std::function<void()> callback) {
        app_callback = callback;
    }

    void run_callback() {
        if(app_callback)
            app_callback();
    }

    /// Reset the parsed data
    void reset() {

        parsed = false;
        subcommand = nullptr;

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

    
    /// Create a new program. Pass in the same arguments as main(), along with a help string.
    App(std::string prog_description="", bool help=true)
        : prog_description(prog_description) {

        if(help)
            help_flag = add_flag("-h,--help", "Print this help message and exit");

    }

    /// Add a subcommand. Like the constructor, you can override the help message addition by setting help=false
    App* add_subcommand(std::string name, std::string description="", bool help=true) {
        subcommands.emplace_back(new App(description, help));
        subcommands.back()->name = name;
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
            std::string name,
            callback_t callback,
            std::string description="", 
            bool defaulted=false
            ) {
        Option myopt{name, description, callback, defaulted};
        if(std::find_if(std::begin(options), std::end(options),
                    [&myopt](const Option_p &v){return *v == myopt;}) == std::end(options)) {
            options.emplace_back();
            Option_p& option = options.back();
            option.reset(new Option(name, description, callback, defaulted));
            return option.get();
        } else
            throw OptionAlreadyAdded(myopt.get_name());

    }

    /// Add option for string
    template<typename T, enable_if_t<!is_vector<T>::value, detail::enabler> = detail::dummy>
    Option* add_option(
            std::string name,
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

        Option* retval = add_option(name, fun, description, defaulted);
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
            std::string name,
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

        Option* retval =  add_option(name, fun, description, defaulted);
        retval->allow_vector = true;
        retval->_expected = -1;
        retval->typeval = detail::type_name<T>();
        if(defaulted)
            retval->defaultval =  "[" + detail::join(variable) + "]";
        return retval;
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
        opt->_expected = 0;
        return opt;
    }

    /// Add option for flag
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
        opt->_expected = 0;
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
        opt->_expected = 0;
        return opt;
    }


    /// Add set of options
    template<typename T>
    Option* add_set(
            std::string name,
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

        Option* retval = add_option(name, fun, description, defaulted);
        retval->typeval = detail::type_name<T>();
        retval->typeval += " in {" + detail::join(_options) + "}";
        if(defaulted) {
            std::stringstream out;
            out << member;
            retval->defaultval = out.str();
        }
        return retval;
    }


    /// Add a configuration ini file option
    Option* add_config(std::string name="--config",
                 std::string default_filename="",
                 std::string help="Read an ini file",
                 bool required=false) {

        // Remove existing config if present
        if(ini_setting != nullptr)
            remove_option(ini_setting);
        ini_file = default_filename;
        ini_required = required;
        ini_setting = add_option(name, ini_file, help, default_filename!="");
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
    virtual void pre_callback() {}

    /// Parses the command line - throws errors
    void parse(int argc, char **argv) {
        progname = argv[0];
        std::vector<std::string> args;
        for(int i=argc-1; i>0; i--)
            args.push_back(argv[i]);
        parse(args);
    }

    /// The real work is done here. Expects a reversed vector
    void parse(std::vector<std::string> & args, bool first_parse=true) {
        parsed = true;

        bool positional_only = false;
        
        while(args.size()>0) {


            Classifer classifer = positional_only ? Classifer::NONE : _recognize(args.back());
            switch(classifer) {
            case Classifer::POSITIONAL_MARK:
                args.pop_back();
                positional_only = true;
                break;
            case Classifer::SUBCOMMAND:
                _parse_subcommand(args);
                break;
            case Classifer::LONG:
                _parse_long(args);
                break;
            case Classifer::SHORT:
                _parse_short(args);
                break;
            case Classifer::NONE:
                positionals.push_back(args.back());
                args.pop_back();
            }
        }

        if (help_flag != nullptr && help_flag->count() > 0) {
            throw CallForHelp();
        }


        for(const Option_p& opt : options) {
            while (opt->get_positional() && opt->count() < opt->get_expected() && positionals.size() > 0) {
                opt->get_new();
                opt->add_result(0, positionals.front());
                positionals.pop_front();
            }
            if (opt->count() > 0) {
                if(!opt->run_callback())
                    throw ConversionError(opt->get_name() + "=" + detail::join(opt->flatten_results()));
            }
        }

        if (first_parse && ini_setting != nullptr && ini_file != "") {
            try {
                std::vector<std::string> values = detail::parse_ini(ini_file);
                std::reverse(std::begin(values), std::end(values));
                
                values.insert(std::begin(values), std::begin(positionals), std::end(positionals));
                return parse(values, false);
            } catch (const FileError &e) {
                if(ini_required)
                    throw;
            }
        }

        for(const Option_p& opt : options) {
            if (opt->get_required() && opt->count() < opt->get_expected())
                throw RequiredError(opt->get_name());
        }

        if(positionals.size()>0)
            throw PositionalError("[" + detail::join(positionals) + "]");

        pre_callback();
        run_callback();
    }

    void _parse_subcommand(std::vector<std::string> &args) {
        for(const App_p &com : subcommands) {
            if(com->name == args.back()){ 
                args.pop_back();
                subcommand = com.get();
                com->parse(args);
                return;
            }
        }
        throw HorribleError("Subcommand");
    }
 
    void _parse_short(std::vector<std::string> &args) {
        std::string current = args.back();

        std::string name;
        std::string rest;
        if(!detail::split_short(current, name, rest))
            throw HorribleError("Short");
        args.pop_back();

        auto op_ptr = std::find_if(std::begin(options), std::end(options), [name](const Option_p &opt){return opt->check_sname(name);});

        if(op_ptr == std::end(options)) {
            missing_options.push_back("-" + name);
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
            while(args.size()>0 && _recognize(args.back()) == Classifer::NONE) {
                op->add_result(vnum, args.back());
                args.pop_back();
            }
        } else while(num>0 && args.size() > 0) {
            num--;
            std::string current = args.back();
            args.pop_back();
            op->add_result(vnum,current);
        }

        if(rest != "") {
            rest = "-" + rest;
            args.push_back(rest);
        }
    }

    Classifer _recognize(std::string current) const {
        std::string dummy1, dummy2;

        if(current == "--")
            return Classifer::POSITIONAL_MARK;
        for(const App_p &com : subcommands) {
            if(com->name == current)
                return Classifer::SUBCOMMAND;
        }
        if(detail::split_long(current, dummy1, dummy2))
            return Classifer::LONG;
        if(detail::split_short(current, dummy1, dummy2))
            return Classifer::SHORT;
        return Classifer::NONE;
    }

    void _parse_long(std::vector<std::string> &args) {
        std::string current = args.back();

        std::string name;
        std::string value;
        if(!detail::split_long(current, name, value))
            throw HorribleError("Long");
        args.pop_back();

        auto op_ptr = std::find_if(std::begin(options), std::end(options), [name](const Option_p &v){return v->check_lname(name);});

        if(op_ptr == std::end(options)) {
            missing_options.push_back("--" + name);
            return;
        }

        // Get a reference to the pointer to make syntax bearable
        Option_p& op = *op_ptr;

        int vnum = op->get_new();
        int num = op->get_expected();
        

        if(value != "") {
            if(num!=-1) num--;
            op->add_result(vnum, value);
        } else if (num == 0) {
            op->add_result(vnum, "");
        }

        if(num == -1) {
            while(args.size() > 0 && _recognize(args.back()) == Classifer::NONE) {
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

    /// This must be called after the options are in but before the rest of the program.
    /** Instead of throwing erros, this gives an error code
     * if -h or an invalid option is passed. Continue with your program if returns -1 */
    void run(int argc, char** argv) {
        parse(argc, argv);
    }

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
    int count(std::string name) const {
        for(const Option_p &opt : options) {
            if(opt->check_name(name)) {
                return opt->count();
            }
        }
        throw OptionNotFound(name);
    }

    std::string help(size_t wid=30, std::string prev="") const {
        // Delegate to subcommand if needed
        if(prev == "")
            prev = progname;
        else
            prev += " " + name;

        if(subcommand != nullptr)
            return subcommand->help(wid, prev);

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
    
    App* get_subcommand() {
        return subcommand;
    }
    
    std::string get_name() const {
        return name;
    }
};


}
