#pragma once

// Distributed under the LGPL version 3.0 license.  See accompanying
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
#include "CLI/Combiner.hpp"
#include "CLI/Option.hpp"
#include "CLI/Value.hpp"

namespace CLI {

enum class Classifer {NONE, POSITIONAL_MARK, SHORT, LONG, SUBCOMMAND};

/// Creates a command line program, with very few defaults.
/** To use, create a new Program() instance with argc, argv, and a help description. The templated
*  add_option methods make it easy to prepare options. Remember to call `.start` before starting your
* program, so that the options can be evaluated and the help option doesn't accidentally run your program. */
class App {
protected:
    
    std::string name;
    std::string prog_description;
    std::vector<Option> options;
    std::vector<std::string> missing_options;
    std::deque<std::string> positionals;
    std::vector<std::unique_ptr<App>> subcommands;
    bool parsed{false};
    App* subcommand = nullptr;
    std::string progname = "program";

    std::function<void()> app_callback;

public:

    /// Set a callback for the end of parsing. Due to a bug in c++11,
    /// it is not possible to overload on std::function (fixed in c++14
    /// and backported to c++11 on newer compilers). Use capture by reference
    /// to get a pointer to App if needed.
    App* set_callback(std::function<void()> callback) {
        app_callback = callback;
        return this;
    }

    void run_callback() {
        if(app_callback)
            app_callback();
    }

    /// Reset the parsed data
    void reset() {

        parsed = false;
        subcommand = nullptr;

        for(Option& opt : options) {
            opt.clear();
        }
        for(std::unique_ptr<App> &app : subcommands) {
            app->reset();
        }
    }
    
    /// Create a new program. Pass in the same arguments as main(), along with a help string.
    App(std::string prog_description="")
        : prog_description(prog_description) {

            add_flag("-h,--help", "Print this help message and exit");

    }

    App* add_subcommand(std::string name, std::string description="") {
        subcommands.emplace_back(new App(description));
        subcommands.back()->name = name;
        return subcommands.back().get();
    }


    //------------ ADD STYLE ---------//

    /// Add an option, will automatically understand the type for common types.
    /** To use, create a variable with the expected type, and pass it in after the name.
     * After start is called, you can use count to see if the value was passed, and
     * the value will be initialized properly. 
     *
     * Program::Required, Program::Default, and the validators are options, and can be `|`
     * together. The positional options take an optional number of arguments.
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
            detail::Combiner opts=Validators
            ) {
        Option myopt{name, description, opts, callback};
        if(std::find(std::begin(options), std::end(options), myopt) == std::end(options))
            options.push_back(myopt);
        else
            throw OptionAlreadyAdded(myopt.get_name());
        return &options.back();

    }

    /// Add option for string
    template<typename T, enable_if_t<!is_vector<T>::value, detail::enabler> = detail::dummy>
    Option* add_option(
            std::string name,
            T &variable,                ///< The variable to set
            std::string description="",
            detail::Combiner opts=Validators
            ) {

        
        if(opts.num!=1)
            throw IncorrectConstruction("Must have Args(1) or be a vector.");
        CLI::callback_t fun = [&variable](CLI::results_t res){
            if(res.size()!=1) {
                return false;
            }
            if(res[0].size()!=1) {
                return false;
            }
            return detail::lexical_cast(res[0][0], variable);
        };

        Option* retval = add_option(name, fun, description, opts);
        retval->typeval = detail::type_name<T>();
        if(opts.defaulted) {
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
            detail::Combiner opts=Args
            ) {

        if(opts.num==0)
            throw IncorrectConstruction("Must have Args or be a vector.");
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

        Option* retval =  add_option(name, fun, description, opts);
        retval->typeval = detail::type_name<T>();
        if(opts.defaulted) {
            retval->defaultval =  "[" + detail::join(variable) + "]";
        }
        return retval;
    }


    /// Multiple options are supported
    template<typename T, typename... Args>
    Option* add_option(
            std::string name,
            T &variable,                ///< The variable to set
            std::string description,
            detail::Combiner opts,
            detail::Combiner opts2,
            Args... args                 ///< More options
            ) {
        return add_option(name, variable, description, opts|opts2, args...);
    }
    /// Add option for flag
    Option* add_flag(
            std::string name,
            std::string description=""
            ) {
        CLI::callback_t fun = [](CLI::results_t){
            return true;
        };
        
        Option* opt = add_option(name, fun, description, Nothing);
        if(opt->positional())
            throw IncorrectConstruction("Flags cannot be positional");
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
        
        Option* opt = add_option(name, fun, description, Nothing);
        if(opt->positional())
            throw IncorrectConstruction("Flags cannot be positional");
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
        
        Option* opt = add_option(name, fun, description, Nothing);
        if(opt->positional())
            throw IncorrectConstruction("Flags cannot be positional");
        return opt;
    }


    /// Add set of options
    template<typename T>
    Option* add_set(
            std::string name,
            T &member,                     ///< The selected member of the set
            std::set<T> options,           ///< The set of posibilities
            std::string description="",
            detail::Combiner opts=Validators
            ) {

        if(opts.num!=1)
            throw IncorrectConstruction("Must have Args(1).");

        CLI::callback_t fun = [&member, options](CLI::results_t res){
            if(res.size()!=1) {
                return false;
            }
            if(res[0].size()!=1) {
                return false;
            }
            bool retval = detail::lexical_cast(res[0][0], member);
            if(!retval)
                return false;
            return std::find(std::begin(options), std::end(options), member) != std::end(options);
        };

        Option* retval = add_option(name, fun, description, opts);
        retval->typeval = detail::type_name<T>();
        retval->typeval += " in {" + detail::join(options) + "}";
        if(opts.defaulted) {
            std::stringstream out;
            out << member;
            retval->defaultval = out.str();
        }
        return retval;
    }


    template<typename T, typename... Args>
    Option* add_set(
            std::string name,
            T &member,
            std::set<T> options,           ///< The set of posibilities
            std::string description,
            detail::Combiner opts,
            detail::Combiner opts2,
            Args... args
            ) {
        return add_set(name, member, options, description, opts|opts2, args...);
    }


    //------------ MAKE STYLE ---------//

    /// Prototype for new output style
    template<typename T = std::string,
        enable_if_t<!is_vector<T>::value, detail::enabler> = detail::dummy>
    Value<T> make_option(
            std::string name,
            std::string description="",
            detail::Combiner opts=Validators
            ) {

        if(opts.num!=1)
            throw IncorrectConstruction("Must have Args(1).");

        Value<T> out(name);
        std::shared_ptr<std::unique_ptr<T>> ptr = out.value;

        CLI::callback_t fun = [ptr](CLI::results_t res){
            if(res.size()!=1) {
                return false;
            }
            if(res[0].size()!=1) {
                return false;
            }
            ptr->reset(new T()); // resets the internal ptr
            return detail::lexical_cast(res[0][0], **ptr);
        };
        Option* retval = add_option(name, fun, description, opts);
        retval->typeval = detail::type_name<T>();
        return out;
    }
    
    template<typename T = std::string, typename... Args>
    Value<T> make_option(
            std::string name,
            std::string description,
            detail::Combiner opts,
            detail::Combiner opts2,
            Args... args
            ) {
        return make_option(name, description, opts|opts2, args...);
    }

    /// Prototype for new output style with default
    template<typename T,
        enable_if_t<!is_vector<T>::value, detail::enabler> = detail::dummy>
    Value<T> make_option(
            std::string name,
            const T& default_value,
            std::string description="",
            detail::Combiner opts=Validators
            ) {

        if(opts.num!=1)
            throw IncorrectConstruction("Must have Args(1).");

        Value<T> out(name);
        std::shared_ptr<std::unique_ptr<T>> ptr = out.value;
        ptr->reset(new T(default_value)); // resets the internal ptr

        CLI::callback_t fun = [ptr](CLI::results_t res){
            if(res.size()!=1) {
                return false;
            }
            if(res[0].size()!=1) {
                return false;
            }
            ptr->reset(new T()); // resets the internal ptr
            return detail::lexical_cast(res[0][0], **ptr);
        };
        Option* retval = add_option(name, fun, description, opts);
        retval->typeval = detail::type_name<T>();
        std::stringstream ot;
        ot << default_value;
        retval->defaultval = ot.str();
        return out;
    }

    /// Prototype for new output style, vector
    template<typename T,
        enable_if_t<is_vector<T>::value, detail::enabler> = detail::dummy>
    Value<T> make_option(
            std::string name,
            std::string description="",
            detail::Combiner opts=Args
            ) {

        if(opts.num==0)
            throw IncorrectConstruction("Must have Args or be a vector.");

        Value<T> out(name);
        std::shared_ptr<std::unique_ptr<T>> ptr = out.value;

        CLI::callback_t fun = [ptr](CLI::results_t res){
            ptr->reset(new T()); // resets the internal ptr
            bool retval = true;
            for(const auto &a : res)
                for(const auto &b : a) {
                    (*ptr)->emplace_back();
                    retval &= detail::lexical_cast(b, (*ptr)->back());
                }
            return (*ptr)->size() > 0 && retval;
        };
        Option* retval =  add_option(name, fun, description, opts);
        retval->typeval = detail::type_name<T>();
        return out;
    }

    
    template<typename T, typename... Args>
    Value<T> make_option(
            std::string name,
            const T& default_value,
            std::string description,
            detail::Combiner opts,
            detail::Combiner opts2,
            Args... args
            ) {
        return make_option(name, default_value, description, opts|opts2, args...);
    }

    /// Prototype for new output style: flag
    Value<int> make_flag(
            std::string name,
            std::string description=""
            ) {

        Value<int> out(name);
        std::shared_ptr<std::unique_ptr<int>> ptr = out.value;
        ptr->reset(new int()); // resets the internal ptr
        **ptr = 0;

        CLI::callback_t fun = [ptr](CLI::results_t res){
            **ptr = (int) res.size();
            return true;
        };

        Option* opt = add_option(name, fun, description, Nothing);
        if(opt->positional())
            throw IncorrectConstruction("Flags cannot be positional");
        return out;
    }

    /// Add set of options
    template<typename T>
    Value<T> make_set(
            std::string name,
            std::set<T> options,           ///< The set of posibilities
            std::string description="",
            detail::Combiner opts=Validators
            ) {

        Value<T> out(name);
        std::shared_ptr<std::unique_ptr<T>> ptr = out.value;

        if(opts.num!=1)
            throw IncorrectConstruction("Must have Args(1).");

        CLI::callback_t fun = [ptr, options](CLI::results_t res){
            if(res.size()!=1) {
                return false;
            }
            if(res[0].size()!=1) {
                return false;
            }
            ptr->reset(new T());
            bool retval = detail::lexical_cast(res[0][0], **ptr);
            if(!retval)
                return false;
            return std::find(std::begin(options), std::end(options), **ptr) != std::end(options);
        };

        Option* retval = add_option(name, fun, description, opts);
        retval->typeval = detail::type_name<T>();
        retval->typeval += " in {" + detail::join(options) + "}";
        return out;
    }

    
    template<typename T, typename... Args>
    Value<T> make_set(
            std::string name,
            std::set<T> options,
            std::string description,
            detail::Combiner opts,
            detail::Combiner opts2,
            Args... args
            ) {
        return make_set(name, options, description, opts|opts2, args...);
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

    void parse(std::vector<std::string> & args) {
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

        if (count("--help") > 0) {
            throw CallForHelp();
        }



        for(Option& opt : options) {
            while (opt.positional() && opt.count() < opt.expected() && positionals.size() > 0) {
                opt.get_new();
                opt.add_result(0, positionals.front());
                positionals.pop_front();
            }
            if (opt.required() && opt.count() < opt.expected())
                throw RequiredError(opt.get_name());
            if (opt.count() > 0) {
                if(!opt.run_callback())
                    throw ParseError(opt.get_name());
            }

        }
        if(positionals.size()>0)
            throw PositionalError("[" + detail::join(positionals) + "]");

        pre_callback();
        run_callback();
    }

    void _parse_subcommand(std::vector<std::string> &args) {
        for(std::unique_ptr<App> &com : subcommands) {
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

        auto op = std::find_if(std::begin(options), std::end(options), [name](const Option &v){return v.check_sname(name);});

        if(op == std::end(options)) {
            missing_options.push_back("-" + name);
            return;
        }

        int vnum = op->get_new();
        int num = op->expected();
       
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
        for(const std::unique_ptr<App> &com : subcommands) {
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

        auto op = std::find_if(std::begin(options), std::end(options), [name](const Option &v){return v.check_lname(name);});

        if(op == std::end(options)) {
            missing_options.push_back("--" + name);
            return;
        }


        int vnum = op->get_new();
        int num = op->expected();
        

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
        for(const Option &opt : options) {
            if(opt.check_name(name)) {
                return opt.count();
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
        for(const Option &opt : options) {
            if(opt.nonpositional()) {
                npos = true;
                break;
            }
        }

        if(npos)
            out << " [OPTIONS...]";

        // Positionals
        bool pos=false;
        for(const Option &opt : options)
            if(opt.positional()) {
                out << " " << opt.help_positional();
                if(opt.has_description())
                    pos=true;
            }

        out << std::endl << std::endl;

        // Positional descriptions
        if(pos) {
            out << "Positionals:" << std::endl;
            for(const Option &opt : options)
                if(opt.positional() && opt.has_description())
                    detail::format_help(out, opt.get_pname(), opt.get_description(), wid);
            out << std::endl;

        }


        // Options
        if(npos) {
            out << "Options:" << std::endl;
            for(const Option &opt : options) {
                if(opt.nonpositional())
                    detail::format_help(out, opt.help_name(), opt.get_description(), wid);
                
            }
            out << std::endl;
        }

        // Subcommands
        if(subcommands.size()> 0) {
            out << "Subcommands:" << std::endl;
            for(const std::unique_ptr<App> &com : subcommands)
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
