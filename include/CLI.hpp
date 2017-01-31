#pragma once

// Distributed under the LGPL version 3.0 license.  See accompanying
// file LICENSE or https://github.com/henryiii/CLI11 for details.

#include <string>
#include <regex>
#include <memory>
#include <deque>
#include <iostream>
#include <functional>
#include <tuple>
#include <exception>
#include <algorithm>
#include <sstream>
#include <type_traits>
#include <set>
#include <iomanip>
#include <numeric>

// C standard library
// Only needed for existence checking
// Could be swapped for filesystem in C++17
#include <sys/types.h>
#include <sys/stat.h>

//#define CLI_LOG 1

namespace CLI {

/// Log a message, can be enabled with CLI_LOG
void logit(std::string) {
#ifdef CLI_LOG
    std::cout << "\033[1;31m" << output << "\033[0m" << std::endl;
#endif
}

/// Simple fucntion to join a string
template <typename T>
std::string join(const T& v, std::string delim = ",") {
    std::ostringstream s;
    for (const auto& i : v) {
        if (&i != &v[0]) {
            s << delim;
        }
        s << i;
    }
    return s.str();
}

// Based generally on https://rmf.io/cxx11/almost-static-if
namespace detail {
    /// Simple empty scoped class
    enum class enabler {};
}

/// An instance to use in EnableIf
constexpr detail::enabler dummy = {};

// Copied from C++14
#if __cplusplus < 201402L
template< bool B, class T = void >
using enable_if_t = typename std::enable_if<B,T>::type;
#else
using std::enable_if_t;
#endif
// If your compiler supports C++14, you can use that definition instead

template <typename T>
struct is_vector {
  static const bool value = false;
};


template<class T, class A>
struct is_vector<std::vector<T, A> > {
  static bool const value = true;
};

struct Combiner {
    int num;
    bool positional;
    bool required;
    bool defaulted;
    std::vector<std::function<bool(std::string)>> validators;

    /// Can be or-ed together
    Combiner operator | (Combiner b) const {
        Combiner self;
        self.num = std::min(num, b.num) == -1 ? -1 : std::max(num, b.num);
        self.positional = positional || b.positional;
        self.required = required || b.required;
        self.defaulted = defaulted || b.defaulted;
        self.validators.reserve(validators.size() + b.validators.size());
        self.validators.insert(self.validators.end(), validators.begin(), validators.end());
        self.validators.insert(self.validators.end(), b.validators.begin(), b.validators.end());
        return self;
    }

    /// Call to give the number of arguments expected on cli
    Combiner operator() (int n) const {
        Combiner self = *this;
        self.num = n;
        return self;
    }
    /// Call to give a validator
    Combiner operator() (std::function<bool(std::string)> func) const {
        Combiner self = *this;
        self.validators.push_back(func);
        return self;
    }
    Combiner operator, (Combiner b) const {
        return *this | b;
    }
};

bool _ExistingFile(std::string filename) {
//    std::fstream f(name.c_str());
//    return f.good();
//    Fastest way according to http://stackoverflow.com/questions/12774207/fastest-way-to-check-if-a-file-exist-using-standard-c-c11-c
    struct stat buffer;   
    return (stat(filename.c_str(), &buffer) == 0); 
}

bool _ExistingDirectory(std::string filename) {
    struct stat buffer;   
    if(stat(filename.c_str(), &buffer) == 0 && (buffer.st_mode & S_IFDIR) )
        return true;
    return false;
}

bool _NonexistentPath(std::string filename) {
    struct stat buffer;
    return stat(filename.c_str(), &buffer) != 0;
}

struct Error : public std::runtime_error {
    int exit_code;
    Error(std::string parent, std::string name, int exit_code=255) : runtime_error(parent + ": " + name), exit_code(exit_code) {}
};

struct CallForHelp : public Error {
    CallForHelp() : Error("CallForHelp","", 0) {}
};

struct BadNameString : public Error {
    BadNameString(std::string name) : Error("BadNameString", name, 1) {}
};


struct ParseError : public Error {
    ParseError(std::string name) : Error("ParseError", name, 2) {}
};

struct OptionAlreadyAdded : public Error {
    OptionAlreadyAdded(std::string name) : Error("OptionAlreadyAdded", name, 3) {}
};

struct OptionNotFound : public Error {
    OptionNotFound(std::string name) : Error("OptionNotFound", name, 4) {}
};

struct RequiredError : public Error {
    RequiredError(std::string name) : Error("RequiredError", name, 5) {}
};

struct PositionalError : public Error {
    PositionalError(std::string name) : Error("PositionalError", name, 6) {}
};

struct HorribleError : public Error {
    HorribleError(std::string name) : Error("HorribleError", "(You should never see this error) " + name, 7) {}
};
struct IncorrectConstruction : public Error {
    IncorrectConstruction(std::string name) : Error("IncorrectConstruction", name, 8) {}
};
struct EmptyError : public Error {
    EmptyError(std::string name) : Error("EmptyError", name, 9) {}
};

const std::regex reg_split{R"regex((?:([a-zA-Z_]?)(?:,|$)|^)([a-zA-Z0-9_][a-zA-Z0-9_\-]*)?)regex"};
const std::regex reg_short{R"regex(-([a-zA-Z_])(.*))regex"};
const std::regex reg_long{R"regex(--([^-^=][^=]*)=?(.*))regex"};


std::tuple<std::string, std::string> split(std::string fullname) {

    std::smatch match;
    if (std::regex_match(fullname, match, reg_split)) {
        std::string sname = match[1];
        std::string lname = match[2];
        if(sname == "" and lname == "")
            throw BadNameString("EMPTY");
        return std::tuple<std::string, std::string>(sname, lname);
    } else throw BadNameString(fullname);
}

const Combiner NOTHING    {0, false,false,false, {}};
const Combiner REQUIRED   {1, false,true, false, {}};
const Combiner DEFAULT    {1, false,false,true, {}};
const Combiner POSITIONAL {1, true, false,false, {}};
const Combiner ARGS       {-1, false,false,false, {}};
const Combiner VALIDATORS {1, false, false, false, {}};

// Warning about using these validators:
// The files could be added/deleted after the validation. This is not common,
// but if this is a possibility, check the file you open afterwards
const Combiner ExistingFile {1, false, false, false, {_ExistingFile}};
const Combiner ExistingDirectory {1, false, false, false, {_ExistingDirectory}};
const Combiner NonexistentPath {1, false, false, false, {_NonexistentPath}};

typedef std::vector<std::vector<std::string>> results_t;
typedef std::function<bool(results_t)> callback_t;



class Option {
public:
protected:
    // Config
    std::string sname;
    std::string lname;
    Combiner opts;
    std::string discription;
    callback_t callback;

    // Results
    results_t results {};


public:
    Option(std::string name, std::string discription = "", Combiner opts=NOTHING, std::function<bool(results_t)> callback=[](results_t){return true;}) :
      opts(opts), discription(discription), callback(callback){
        std::tie(sname, lname) = split(name);
    }

    void clear() {
        results.clear();
    }

    bool required() const {
        return opts.required;
    }

    int expected() const {
        return opts.num;
    }

    bool positional() const {
        return opts.positional;
    }

    bool defaulted() const {
        return opts.defaulted;
    }

    /// Process the callback
    bool run_callback() const {
        if(opts.validators.size()>0) {
            for(const std::string & result : flatten_results())
                for(const std::function<bool(std::string)> &vali : opts.validators)
                    if(!vali(result))
                        return false;
        }
        return callback(results);
    }

    /// Indistinguishible options are equal
    bool operator== (const Option& other) const {
        if(sname=="" && other.sname=="") 
            return lname==other.lname;
        else if(lname=="" && other.lname=="")
            return sname==other.sname;
        else
            return sname==other.sname || lname==other.lname;
    }

    std::string get_name() const {
        if(sname=="")
            return "--" + lname;
        else if (lname=="")
            return "-" + sname;
        else
            return "-" + sname + ", --" + lname;
    }

    bool check_name(const std::string& name) const {
        return name == sname || name == lname || name == sname + "," + lname;
    }

    bool check_sname(const std::string& name) const {
        return name == sname;
    }

    bool check_lname(const std::string& name) const {
        return name == lname;
    }

    std::string get_sname() const {
        return sname;
    }

    std::string get_lname() const {
        return lname;
    }

    void add_result(int r, std::string s) {
        logit("Adding result: " + s);
        results.at(r).push_back(s);
    }
    int get_new() {
        results.emplace_back();
        return results.size() - 1;
    }
    int count() const {
        int out = 0;
        for(const std::vector<std::string>& v : results)
            out += v.size();
        return out;
    }

    std::string string() const {
        std::string val = "Option: " + get_name() + "\n"
             + "  " + discription + "\n"
             + "  [";
        for(const auto& item : results) {
            if(&item!=&results[0])
                val+="],[";
            val += join(item);
        }
        val += "]";
        return val;
    }

    int help_len() const {
        return std::min((int) get_name().length(), 40);
    }

    std::string help(int len = 0) const {
        std::stringstream out;
        out << std::setw(len) << std::left << get_name() << discription;
        return out.str();
    }

    std::vector<std::string> flatten_results() const {
        std::vector<std::string> output;
        for(const std::vector<std::string> result : results)
            output.insert(std::end(output), std::begin(result), std::end(result));
        return output;
    }

};


template<typename T, enable_if_t<std::is_integral<T>::value, detail::enabler> = dummy>
bool lexical_cast(std::string input, T& output) {
    logit("Int lexical cast " + input);
    try{
        output = (T) std::stoll(input);
        return true;
    } catch (std::invalid_argument) {
        return false;
    } catch (std::out_of_range) {
        return false;
    }
}
    
template<typename T, enable_if_t<std::is_floating_point<T>::value, detail::enabler> = dummy>
bool lexical_cast(std::string input, T& output) {
    logit("Floating lexical cast " + input);
    try{
        output = (T) std::stold(input);
        return true;
    } catch (std::invalid_argument) {
        return false;
    } catch (std::out_of_range) {
        return false;
    }
}

// String and similar
template<typename T, 
enable_if_t<is_vector<T>::value, detail::enabler> = dummy>
bool lexical_cast(std::string input, T& output) {
    logit("vector lexical cast: " + input);
    if(output.size() == input.size())
        output.resize(input.size());
    for(size_t i=0; i<input.size(); i++)
        output[i] = input[i];
    return true;
}

// String and similar
template<typename T, 
enable_if_t<!std::is_floating_point<T>::value && !std::is_integral<T>::value && !is_vector<T>::value
, detail::enabler> = dummy>
bool lexical_cast(std::string input, T& output) {
    logit("Direct lexical cast: " + input);
    output = input;
    return true;
}

enum class Classifer {NONE, POSITIONAL_MARK, SHORT, LONG, SUBCOMMAND};


class App;

// Prototype return value test
template <typename T>
class Value {
    friend App;
protected:
    std::shared_ptr<std::unique_ptr<T>> value {new std::unique_ptr<T>()};
    std::string name;
public:
    Value(std::string name) : name(name) {}
    operator bool() const {return (bool) *value;}
    /// Note this does not throw on assignment, though
    /// afterwards it seems to work fine. Best to use
    /// explicit * notation.
    T& operator *() const {
        if(*value) {
            //std::cout << "Succ" << std::endl;
            return **value;
        }
        else {
            //std::cout << "Throwing!!!" << std::endl;
            throw EmptyError(name);
        }
    }
};

/// Creates a command line program, with very few defaults.
/** To use, create a new Program() instance with argc, argv, and a help discription. The templated
*  add_option methods make it easy to prepare options. Remember to call `.start` before starting your
* program, so that the options can be evaluated and the help option doesn't accidentally run your program. */
class App {
protected:
    
    std::string name;
    std::string prog_discription;
    std::vector<Option> options;
    std::vector<std::string> missing_options;
    std::deque<std::string> positionals;
    std::vector<std::unique_ptr<App>> subcommands;
    bool parsed{false};
    App* subcommand = nullptr;

public:


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
    App(std::string prog_discription="")
        : prog_discription(prog_discription) {

            add_flag("h,help", "Print this help message and exit");

    }

    App* add_subcommand(std::string name, std::string discription="") {
        subcommands.emplace_back(new App(discription));
        subcommands.back()->name = name;
        logit(subcommands.back()->name);
        return subcommands.back().get();
    }


    //------------ ADD STYLE ---------//

    /// Add an option, will automatically understand the type for common types.
    /** To use, create a variable with the expected type, and pass it in after the name.
     * After start is called, you can use count to see if the value was passed, and
     * the value will be initialized properly. 
     *
     * Program::REQUIRED, Program::DEFAULT, and Program::POSITIONAL are options, and can be `|`
     * together. The positional options take an optional number of arguments.
     *
     * For example,
     *
     *     std::string filename
     *     program.add_option("filename", filename, "discription of filename");
     */
    Option* add_option(
            std::string name,           ///< The name, long,short
            callback_t callback,        ///< The callback
            std::string discription="", ///< Discription string
            Combiner opts=VALIDATORS    ///< The options (REQUIRED, DEFAULT, POSITIONAL, ARGS())
            ) {
        Option myopt{name, discription, opts, callback};
        if(std::find(std::begin(options), std::end(options), myopt) == std::end(options))
            options.push_back(myopt);
        else
            throw OptionAlreadyAdded(myopt.get_name());
        return &options.back();

    }

    /// Add option for string
    template<typename T, enable_if_t<!is_vector<T>::value, detail::enabler> = dummy>
    Option* add_option(
            std::string name,           ///< The name, long,short
            T &variable,                ///< The variable to set
            std::string discription="", ///< Discription string
            Combiner opts=VALIDATORS    ///< The options (REQUIRED, DEFAULT, POSITIONAL, ARGS())
            ) {

        
        if(opts.num!=1)
            throw IncorrectConstruction("Must have ARGS(1) or be a vector.");
        CLI::callback_t fun = [&variable](CLI::results_t res){
            if(res.size()!=1) {
                return false;
            }
            if(res[0].size()!=1) {
                return false;
            }
            return lexical_cast(res[0][0], variable);
        };

        return add_option(name, fun, discription, opts);
    }

    /// Add option for vector of results
    template<typename T>
    Option* add_option(
            std::string name,           ///< The name, long,short
            std::vector<T> &variable,   ///< The variable to set
            std::string discription="", ///< Discription string
            Combiner opts=VALIDATORS    ///< The options (REQUIRED, DEFAULT, POSITIONAL, ARGS())
            ) {

        if(opts.num==0)
            throw IncorrectConstruction("Must have ARGS or be a vector.");
        CLI::callback_t fun = [&variable](CLI::results_t res){
            bool retval = true;
            variable.clear();
            for(const auto &a : res)
                for(const auto &b : a) {
                    variable.emplace_back();
                    retval &= lexical_cast(b, variable.back());
                }
            return variable.size() > 0 && retval;
        };

        return add_option(name, fun, discription, opts);
    }


    /// Add option for flag
    Option* add_flag(
            std::string name,           ///< The name, short,long
            std::string discription=""  ///< Discription string
            ) {
        CLI::callback_t fun = [](CLI::results_t){
            return true;
        };
        
        return add_option(name, fun, discription, NOTHING);
    }

    /// Add option for flag
    template<typename T, enable_if_t<std::is_integral<T>::value, detail::enabler> = dummy>
    Option* add_flag(
            std::string name,           ///< The name, short,long
                T  &count,              ///< A varaible holding the count
            std::string discription=""  ///< Discription string
            ) {

        count = 0;
        CLI::callback_t fun = [&count](CLI::results_t res){
            count = (T) res.size();
            return true;
        };
        
        return add_option(name, fun, discription, NOTHING);
    }

    /// Add set of options
    template<typename T>
    Option* add_set(
            std::string name,              ///< The name, short,long
            T &member,                     ///< The selected member of the set
            std::set<T> options, ///< The set of posibilities
            std::string discription="",    ///< Discription string
            Combiner opts=VALIDATORS       ///< The options (REQUIRED, DEFAULT, POSITIONAL, ARGS())
            ) {

        if(opts.num!=1)
            throw IncorrectConstruction("Must have ARGS(1).");

        CLI::callback_t fun = [&member, options](CLI::results_t res){
            if(res.size()!=1) {
                return false;
            }
            if(res[0].size()!=1) {
                return false;
            }
            bool retval = lexical_cast(res[0][0], member);
            if(!retval)
                return false;
            return std::find(std::begin(options), std::end(options), member) != std::end(options);
        };

        return add_option(name, fun, discription, opts);
    }




    //------------ MAKE STYLE ---------//

    /// Prototype for new output style
    template<typename T = std::string,
        enable_if_t<!is_vector<T>::value, detail::enabler> = dummy>
    Value<T> make_option(
            std::string name,              ///< The name, short,long
            std::string discription="",
            Combiner opts=VALIDATORS
            ) {

        if(opts.num!=1)
            throw IncorrectConstruction("Must have ARGS(1).");

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
            return lexical_cast(res[0][0], **ptr);
        };
        add_option(name, fun, discription, opts);
        return out;
    }

    /// Prototype for new output style with default
    template<typename T,
        enable_if_t<!is_vector<T>::value, detail::enabler> = dummy>
    Value<T> make_option(
            std::string name,              ///< The name, short,long
            const T& default_value,
            std::string discription="",
            Combiner opts=VALIDATORS
            ) {

        if(opts.num!=1)
            throw IncorrectConstruction("Must have ARGS(1).");

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
            return lexical_cast(res[0][0], **ptr);
        };
        add_option(name, fun, discription, opts);
        return out;
    }
    
    /// Prototype for new output style, vector
    template<typename T,
        enable_if_t<is_vector<T>::value, detail::enabler> = dummy>
    Value<T> make_option(
            std::string name,              ///< The name, short,long
            std::string discription="",
            Combiner opts=VALIDATORS
            ) {

        if(opts.num==0)
            throw IncorrectConstruction("Must have ARGS or be a vector.");

        Value<T> out(name);
        std::shared_ptr<std::unique_ptr<T>> ptr = out.value;

        CLI::callback_t fun = [ptr](CLI::results_t res){
            ptr->reset(new T()); // resets the internal ptr
            bool retval = true;
            for(const auto &a : res)
                for(const auto &b : a) {
                    (*ptr)->emplace_back();
                    retval &= lexical_cast(b, (*ptr)->back());
                }
            return (*ptr)->size() > 0 && retval;
        };
        add_option(name, fun, discription, opts);
        return out;
    }


    /// Prototype for new output style: flag
    Value<int> make_flag(
            std::string name,              ///< The name, short,long
            std::string discription=""
            ) {

        Value<int> out(name);
        std::shared_ptr<std::unique_ptr<int>> ptr = out.value;
        ptr->reset(new int()); // resets the internal ptr
        **ptr = 0;

        CLI::callback_t fun = [ptr](CLI::results_t res){
            **ptr = (int) res.size();
            return true;
        };
        add_option(name, fun, discription, NOTHING);
        return out;
    }

    /// Add set of options
    template<typename T>
    Value<T> make_set(
            std::string name,              ///< The name, short,long
            std::set<T> options,           ///< The set of posibilities
            std::string discription="",    ///< Discription string
            Combiner opts=VALIDATORS       ///< The options (REQUIRED, DEFAULT, POSITIONAL, ARGS())
            ) {

        Value<T> out(name);
        std::shared_ptr<std::unique_ptr<T>> ptr = out.value;

        if(opts.num!=1)
            throw IncorrectConstruction("Must have ARGS(1).");

        CLI::callback_t fun = [ptr, options](CLI::results_t res){
            if(res.size()!=1) {
                return false;
            }
            if(res[0].size()!=1) {
                return false;
            }
            ptr->reset(new T());
            bool retval = lexical_cast(res[0][0], **ptr);
            if(!retval)
                return false;
            return std::find(std::begin(options), std::end(options), **ptr) != std::end(options);
        };

        add_option(name, fun, discription, opts);
        return out;
    }



    /// Parses the command line - throws errors
    void parse(int argc, char **argv) {
        std::vector<std::string> args;
        for(int i=argc-1; i>0; i--)
            args.push_back(argv[i]);
        parse(args);
    }

    void parse(std::vector<std::string> & args) {
        parsed = true;

        bool positional_only = false;
        
        while(args.size()>0) {

            logit("Parse: ["+join(args)+"]");

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
                logit("Positional: "+args.back());
                positionals.push_back(args.back());
                args.pop_back();
            }
        }

        if (count("help") > 0) {
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
            throw PositionalError("[" + join(positionals) + "]");
    }

    void _parse_subcommand(std::vector<std::string> &args) {
        for(std::unique_ptr<App> &com : subcommands) {
            if(com->name == args.back()){ 
                args.pop_back();
                com->parse(args);
                subcommand = com.get();
                return;
            }
        }
        throw HorribleError("Subcommand");
    }
 
    void _parse_short(std::vector<std::string> &args) {
        std::string current = args.back();
        std::smatch match;

        if(!std::regex_match(current, match, reg_short))
            throw HorribleError("Short");
        
        args.pop_back();
        std::string name = match[1];
        std::string rest = match[2];

        logit("Working on short: " + name + " (" + rest + ")");

        auto op = std::find_if(std::begin(options), std::end(options), [name](const Option &v){return v.check_sname(name);});

        if(op == std::end(options)) {
            missing_options.push_back("-" + op->get_sname());
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
            logit("Adding: "+current);
            args.pop_back();
            op->add_result(vnum,current);
        }

        if(rest != "") {
            rest = "-" + rest;
            args.push_back(rest);
        }
    }

    Classifer _recognize(std::string current) const {
        if(current == "--")
            return Classifer::POSITIONAL_MARK;
        for(const std::unique_ptr<App> &com : subcommands) {
            if(com->name == current)
                return Classifer::SUBCOMMAND;
        }
        if(std::regex_match(current, reg_long))
            return Classifer::LONG;
        if(std::regex_match(current, reg_short))
            return Classifer::SHORT;
        return Classifer::NONE;
            

    }

    void _parse_long(std::vector<std::string> &args) {
        std::string current = args.back();
        std::smatch match;

        if(!std::regex_match(current, match, reg_long))
            throw HorribleError("Long");

        args.pop_back();
        std::string name = match[1];
        std::string value = match[2];


        logit("Working on long: " + name + " (" + value + ")");

        auto op = std::find_if(std::begin(options), std::end(options), [name](const Option &v){return v.check_lname(name);});

        if(op == std::end(options)) {
            missing_options.push_back("--" + op->get_lname());
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
            std::cerr << help() << std::endl;
        } else {
            std::cout << help() << std::endl;
        }
        return e.exit_code;
    }

    /// Counts the number of times the given option was passed.
    int count(std::string name) const {
        for(const Option &opt : options) {
            logit("Computing: " + opt.get_name());
            if(opt.check_name(name)) {
                logit("Counting: " + opt.get_name() + std::to_string(opt.count()));
                return opt.count();
            }
        }
        throw OptionNotFound(name);
    }

    std::string help() const {
        std::stringstream out;
        if(name != "")
            out << "Subcommand: " << name << " ";
        out << prog_discription << std::endl;
        int len = std::accumulate(std::begin(options), std::end(options), 0,
                [](int val, const Option &opt){
                    return std::max(opt.help_len()+1, val);});
        for(const Option &opt : options) {
            out << opt.help(len) << std::endl;
        }
        if(subcommands.size()> 0) {
            out << "Subcommands:" << std::endl;
            int max = std::accumulate(std::begin(subcommands), std::end(subcommands), 0,
                    [](int i, const std::unique_ptr<App> &j){return std::max(i, (int) j->get_name().length()+1);});
            for(const std::unique_ptr<App> &com : subcommands) {
                out << std::setw(max) << std::left << com->get_name() << " " << com->prog_discription << std::endl;
            }
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
