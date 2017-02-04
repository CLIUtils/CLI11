#pragma once

// Distributed under the LGPL version 3.0 license.  See accompanying
// file LICENSE or https://github.com/henryiii/CLI11 for details.

#include <string>
#include <memory>
#include <deque>
#include <iostream>
#include <functional>
#include <tuple>
#include <exception>
#include <stdexcept>
#include <algorithm>
#include <sstream>
#include <type_traits>
#include <set>
#include <iomanip>
#include <numeric>
#include <vector>
#include <locale>

// C standard library
// Only needed for existence checking
// Could be swapped for filesystem in C++17
#include <sys/types.h>
#include <sys/stat.h>


namespace CLI {


// Error definitions

struct Error : public std::runtime_error {
    int exit_code;
    bool print_help;
    Error(std::string parent, std::string name, int exit_code=255, bool print_help=true) : runtime_error(parent + ": " + name), exit_code(exit_code), print_help(print_help) {}
};

struct Success : public Error {
    Success() : Error("Success", "Successfully completed, should be caught and quit", 0, false) {}
};

struct CallForHelp : public Error {
    CallForHelp() : Error("CallForHelp", "This should be caught in your main function, see examples", 0) {}
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


// Type tools
//
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

template <typename T>
struct is_bool {
  static const bool value = false;
};

template<>
struct is_bool<bool> {
  static bool const value = true;
};

namespace detail {
    // Based generally on https://rmf.io/cxx11/almost-static-if
    /// Simple empty scoped class
    enum class enabler {};

    /// An instance to use in EnableIf
    constexpr enabler dummy = {};

    /// Simple function to join a string
    template <typename T>
    std::string join(const T& v, std::string delim = ",") {
        std::ostringstream s;
        size_t start = 0;
        for (const auto& i : v) {
            if(start++ > 0)
                s << delim;
            s << i;
        }
        return s.str();
    }

    /// Was going to be based on
    ///  http://stackoverflow.com/questions/1055452/c-get-name-of-type-in-template
    /// But this is cleaner and works better in this case
    
    template<typename T,
    enable_if_t<std::is_integral<T>::value && std::is_signed<T>::value, detail::enabler> = detail::dummy>
    constexpr const char* type_name() {
        return "INT";
	}

    template<typename T,
    enable_if_t<std::is_integral<T>::value && std::is_unsigned<T>::value, detail::enabler> = detail::dummy>
    constexpr const char* type_name() {
        return "UINT";
	}
    
        
    template<typename T,
    enable_if_t<std::is_floating_point<T>::value, detail::enabler> = detail::dummy>
    constexpr const char* type_name() {
        return "FLOAT";
	}
    
    
    /// This one should not be used, since vector types print the internal type
    template<typename T,
    enable_if_t<is_vector<T>::value, detail::enabler> = detail::dummy>
    constexpr const char* type_name() {
        return "VECTOR";
	}


	template<typename T,
    enable_if_t<!std::is_floating_point<T>::value && !std::is_integral<T>::value && !is_vector<T>::value
    , detail::enabler> = detail::dummy>
    constexpr const char* type_name() {
        return "STRING";
	}

    void format_help(std::stringstream &out, std::string name, std::string description, size_t wid) {
        name = "  " + name;
        out << std::setw(wid) << std::left << name;
        if(description != "") {
            if(name.length()>=wid)
                out << std::endl << std::setw(wid) << "";
            out << description << std::endl;
        }
    }

    struct Combiner {
        int num;
        bool required;
        bool defaulted;
        std::vector<std::function<bool(std::string)>> validators;

        /// Can be or-ed together
        Combiner operator | (Combiner b) const {
            Combiner self;
            self.num = std::min(num, b.num) == -1 ? -1 : std::max(num, b.num);
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


    template<typename T>
    bool valid_first_char(T c) {
        return std::isalpha(c) || c=='_';
    }

    template<typename T>
    bool valid_later_char(T c) {
        return std::isalnum(c) || c=='_' || c=='.' || c=='-';
    }

    inline bool valid_name_string(const std::string &str) {
        if(str.size()<1 || !valid_first_char(str[0]))
            return false;
        for(auto c : str.substr(1))
            if(!valid_later_char(c))
                return false;
        return true;
    }

    // Returns false if not a short option. Otherwise, sets opt name and rest and returns true
    inline bool split_short(const std::string &current, std::string &name, std::string &rest) {
        if(current.size()>1 && current[0] == '-' && valid_first_char(current[1])) {
            name = current.substr(1,1);
            rest = current.substr(2);
            return true;
        } else
            return false;
    }

    // Returns false if not a long option. Otherwise, sets opt name and other side of = and returns true
    inline bool split_long(const std::string &current, std::string &name, std::string &value) {
        if(current.size()>2 && current.substr(0,2) == "--" && valid_first_char(current[2])) {
            auto loc = current.find("=");
            if(loc != std::string::npos) {
                name = current.substr(2,loc-2);
                value = current.substr(loc+1);
            } else {
                name = current.substr(2);
                value = "";
            }
            return true;
        } else
            return false;
    }

    // Splits a string into multiple long and short names
    inline std::vector<std::string> split_names(std::string current) {
        std::vector<std::string> output;
        size_t val;
        while((val = current.find(",")) != std::string::npos) {
            output.push_back(current.substr(0,val));
            current = current.substr(val+1);
        }
        output.push_back(current);
        return output;

    }


    inline std::tuple<std::vector<std::string>,std::vector<std::string>, std::string>
      get_names(const std::vector<std::string> &input) {
        
        std::vector<std::string> short_names;
        std::vector<std::string> long_names;
        std::string pos_name;

        for(std::string name : input) {
            if(name.length() == 0)
                continue;
            else if(name.length() > 1 && name[0] == '-' && name[1] != '-') {
                if(name.length()==2 && valid_first_char(name[1]))
                    short_names.push_back(std::string(1,name[1]));
                else
                    throw BadNameString("Invalid one char name: "+name);
            } else if(name.length() > 2 && name.substr(0,2) == "--") {
                name = name.substr(2);
                if(valid_name_string(name))
                    long_names.push_back(name);
                else
                    throw BadNameString("Bad long name: "+name);
            } else if(name == "-" || name == "--") {
                throw BadNameString("Must have a name, not just dashes");
            } else {
                if(pos_name.length() > 0)
                    throw BadNameString("Only one positional name allowed, remove: "+name);
                pos_name = name;

            }
        }
          
        return std::tuple<std::vector<std::string>,std::vector<std::string>, std::string>
            (short_names, long_names, pos_name);
    }

    // Integers
    template<typename T, enable_if_t<std::is_integral<T>::value, detail::enabler> = detail::dummy>
    bool lexical_cast(std::string input, T& output) {
        try{
            output = (T) std::stoll(input);
            return true;
        } catch (std::invalid_argument) {
            return false;
        } catch (std::out_of_range) {
            return false;
        }
    }
        
    // Floats
    template<typename T, enable_if_t<std::is_floating_point<T>::value, detail::enabler> = detail::dummy>
    bool lexical_cast(std::string input, T& output) {
        try{
            output = (T) std::stold(input);
            return true;
        } catch (std::invalid_argument) {
            return false;
        } catch (std::out_of_range) {
            return false;
        }
    }

    // Vector
    template<typename T, 
    enable_if_t<is_vector<T>::value, detail::enabler> = detail::dummy>
    bool lexical_cast(std::string input, T& output) {
        if(output.size() == input.size())
            output.resize(input.size());
        for(size_t i=0; i<input.size(); i++)
            output[i] = input[i];
        return true;
    }

    // String and similar
    template<typename T, 
    enable_if_t<!std::is_floating_point<T>::value && !std::is_integral<T>::value && !is_vector<T>::value
    , detail::enabler> = detail::dummy>
    bool lexical_cast(std::string input, T& output) {
        output = input;
        return true;
    }
}



// Defines for common Combiners (don't use combiners directly)

const detail::Combiner Nothing    {0,  false, false, {}};
const detail::Combiner Required   {1,  true,  false, {}};
const detail::Combiner Default    {1,  false, true,  {}};
const detail::Combiner Args       {-1, false, false, {}};
const detail::Combiner Validators {1,  false, false, {}};

// Warning about using these validators:
// The files could be added/deleted after the validation. This is not common,
// but if this is a possibility, check the file you open afterwards
const detail::Combiner ExistingFile      {1, false, false, {detail::_ExistingFile}};
const detail::Combiner ExistingDirectory {1, false, false, {detail::_ExistingDirectory}};
const detail::Combiner NonexistentPath   {1, false, false, {detail::_NonexistentPath}};

typedef std::vector<std::vector<std::string>> results_t;
typedef std::function<bool(results_t)> callback_t;


class App;

class Option {
    friend App;
protected:
    // Config
    std::vector<std::string> snames;
    std::vector<std::string> lnames;
    std::string pname;

    detail::Combiner opts;
    std::string description;
    callback_t callback;

    // These are for help strings
    std::string defaultval;
    std::string typeval;

    // Results
    results_t results {};


public:
    Option(std::string name, std::string description = "", detail::Combiner opts=Nothing, std::function<bool(results_t)> callback=[](results_t){return true;}) :
      opts(opts), description(description), callback(callback){
        std::tie(snames, lnames, pname) = detail::get_names(detail::split_names(name));
    }

    /// Clear the parsed results (mostly for testing)
    void clear() {
        results.clear();
    }

    /// True if option is required
    bool required() const {
        return opts.required;
    }

    /// The number of arguments the option expects
    int expected() const {
        return opts.num;
    }

    /// True if the argument can be given directly
    bool positional() const {
        return pname.length() > 0;
    }

    /// True if option has at least one non-positional name
    bool nonpositional() const {
        return (snames.size() + lnames.size()) > 0;
    }

    /// True if this should print the default string
    bool defaulted() const {
        return opts.defaulted;
    }

    /// True if option has description
    bool has_description() const {
        return description.length() > 0;
    }

    /// Get the description
    const std::string& get_description() const {
        return description;
    }

    /// The name and any extras needed for positionals
    std::string help_positional() const {
        std::string out = pname;
        if(expected()<1)
            out = out + "x" + std::to_string(expected());
        else if(expected()==-1)
            out = out + "...";
        out = required() ? out : "["+out+"]";
        return out;
    }

    // Just the pname
    std::string get_pname() const {
        return pname;
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

    /// If options share any of the same names, they are equal (not counting positional)
    bool operator== (const Option& other) const {
        for(const std::string &sname : snames)
            for(const std::string &othersname : other.snames)
                if(sname == othersname)
                    return true;
        for(const std::string &lname : lnames)
            for(const std::string &otherlname : other.lnames)
                if(lname == otherlname)
                    return true;
        return false;
    }

    /// Gets a , sep list of names. Does not include the positional name.
    std::string get_name() const {
        std::vector<std::string> name_list;
        for(const std::string& sname : snames)
            name_list.push_back("-"+sname);
        for(const std::string& lname : lnames)
            name_list.push_back("--"+lname);
        return detail::join(name_list);
    }

    /// Check a name. Requires "-" or "--" for short / long, supports positional name
    bool check_name(std::string name) const {

        if(name.length()>2 && name.substr(0,2) == "--")
            return check_lname(name.substr(2));
        else if (name.length()>1 && name.substr(0,1) == "-")
            return check_sname(name.substr(1));
        else
            return name == pname;
    }

    /// Requires "-" to be removed from string
    bool check_sname(const std::string& name) const {
        return std::find(std::begin(snames), std::end(snames), name) != std::end(snames);
    }

    /// Requires "--" to be removed from string
    bool check_lname(const std::string& name) const {
        return std::find(std::begin(lnames), std::end(lnames), name) != std::end(lnames);
    }


    /// Puts a result at position r
    void add_result(int r, std::string s) {
        results.at(r).push_back(s);
    }

    /// Starts a new results vector (used for r in add_result)
    int get_new() {
        results.emplace_back();
        return results.size() - 1;
    }

    /// Count the total number of times an option was passed
    int count() const {
        int out = 0;
        for(const std::vector<std::string>& v : results)
            out += v.size();
        return out;
    }

    /// Diagnostic representation
    std::string string() const {
        std::string val = "Option: " + get_name() + "\n"
             + "  " + description + "\n"
             + "  [";
        for(const auto& item : results) {
            if(&item!=&results[0])
                val+="],[";
            val += detail::join(item);
        }
        val += "]";
        return val;
    }

    /// The first half of the help print, name plus default, etc
    std::string help_name() const {
        std::stringstream out;
        out << get_name();
        if(expected() != 0) {
            if(typeval != "")
                out << " " << typeval;
            if(defaultval != "")
                out << "=" << defaultval; 
            if(expected() > 1)
                out << " x " << expected();
            if(expected() == -1)
                out << " ...";
        }
        return out.str();
    }

    /// Produce a flattened vector of results, vs. a vector of vectors.
    std::vector<std::string> flatten_results() const {
        std::vector<std::string> output;
        for(const std::vector<std::string> result : results)
            output.insert(std::end(output), std::begin(result), std::end(result));
        return output;
    }

};



enum class Classifer {NONE, POSITIONAL_MARK, SHORT, LONG, SUBCOMMAND};


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

    T& get() const {
        if(*value)
            return **value;
        else
            throw EmptyError(name);
    }
    /// Note this does not throw on assignment, though
    /// afterwards it seems to work fine. Best to use
    /// explicit * notation.
    T& operator *() const {
        return get();
    }
};

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
