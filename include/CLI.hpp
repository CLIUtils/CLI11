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
    Error(std::string parent, std::string name, int exit_code=255) : runtime_error(parent + ": " + name), exit_code(exit_code) {}
};

struct CallForHelp : public Error {
    CallForHelp() : Error("CallForHelp","This should be caught in your main function, see examples", 0) {}
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


    inline std::tuple<std::vector<std::string>,std::vector<std::string>> get_names(const std::vector<std::string> &input) {
        std::vector<std::string> short_names;
        std::vector<std::string> long_names;

        for(std::string name : input) {
            if(name.length() == 0)
                continue;
            else if(name.length() == 1)
                if(valid_first_char(name[0]))
                    short_names.push_back(name);
                else
                    throw BadNameString("Invalid one char name: "+name);
            else if(name.length() == 2 && name[0] == '-' && name[1] != '-') {
                if(valid_first_char(name[1]))
                    short_names.push_back(std::string(1,name[1]));
                else
                    throw BadNameString("Invalid one char name: "+name);
            } else {

                if(name.substr(0,2) == "--")
                    name = name.substr(2);
                if(valid_name_string(name))
                    long_names.push_back(name);
                else
                    throw BadNameString("Bad long name"+name);

            }
        }
          
        return std::tuple<std::vector<std::string>,std::vector<std::string>>(short_names, long_names);
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

const detail::Combiner NOTHING    {0, false,false,false, {}};
const detail::Combiner REQUIRED   {1, false,true, false, {}};
const detail::Combiner DEFAULT    {1, false,false,true, {}};
const detail::Combiner POSITIONAL {1, true, false,false, {}};
const detail::Combiner ARGS       {-1, false,false,false, {}};
const detail::Combiner VALIDATORS {1, false, false, false, {}};

// Warning about using these validators:
// The files could be added/deleted after the validation. This is not common,
// but if this is a possibility, check the file you open afterwards
const detail::Combiner ExistingFile {1, false, false, false, {detail::_ExistingFile}};
const detail::Combiner ExistingDirectory {1, false, false, false, {detail::_ExistingDirectory}};
const detail::Combiner NonexistentPath {1, false, false, false, {detail::_NonexistentPath}};

typedef std::vector<std::vector<std::string>> results_t;
typedef std::function<bool(results_t)> callback_t;


class App;

class Option {
    friend App;
protected:
    // Config
    std::vector<std::string> snames;
    std::vector<std::string> lnames;
    detail::Combiner opts;
    std::string discription;
    callback_t callback;

    // These are for help strings
    std::string defaultval;
    std::string typeval;

    // Results
    results_t results {};


public:
    Option(std::string name, std::string discription = "", detail::Combiner opts=NOTHING, std::function<bool(results_t)> callback=[](results_t){return true;}) :
      opts(opts), discription(discription), callback(callback){
        std::tie(snames, lnames) = detail::get_names(detail::split_names(name));
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

    /// If options share any of the same names, they are equal
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

    std::string get_name() const {
        std::vector<std::string> name_list;
        for(const std::string& sname : snames)
            name_list.push_back("-"+sname);
        for(const std::string& lname : lnames)
            name_list.push_back("--"+lname);
        return detail::join(name_list);
    }

    bool check_name(std::string name) const {
        for(int i=0; i<2; i++)
            if(name.length()>2 && name[0] == '-')
                name = name.substr(1);

        return check_sname(name) || check_lname(name);
    }

    bool check_sname(const std::string& name) const {
        return std::find(std::begin(snames), std::end(snames), name) != std::end(snames);
    }

    bool check_lname(const std::string& name) const {
        return std::find(std::begin(lnames), std::end(lnames), name) != std::end(lnames);
    }


    void add_result(int r, std::string s) {
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
            val += detail::join(item);
        }
        val += "]";
        return val;
    }

    std::string help_name() const {
        std::stringstream out;
        out << "  " << get_name();
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

    int help_len() const {
        return help_name().length();
    }

    std::string help(int len = 0) const {
        std::stringstream out;
        if(help_len() > len) {
            out << help_name() << "\n";
            out << std::setw(len) << " ";

        } else {
            out << std::setw(len) << std::left << help_name();
        }
        out << discription;
        return out.str();
    }

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
    /// Note this does not throw on assignment, though
    /// afterwards it seems to work fine. Best to use
    /// explicit * notation.
    T& operator *() const {
        if(*value) {
            return **value;
        }
        else {
            throw EmptyError(name);
        }
    }
};

/// Creates a command line program, with very few defaults.
/** To use, create a new Program() instance with argc, argv, and a help description. The templated
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
            detail::Combiner opts=VALIDATORS    ///< The options (REQUIRED, DEFAULT, POSITIONAL, ARGS())
            ) {
        Option myopt{name, discription, opts, callback};
        if(std::find(std::begin(options), std::end(options), myopt) == std::end(options))
            options.push_back(myopt);
        else
            throw OptionAlreadyAdded(myopt.get_name());
        return &options.back();

    }

    /// Add option for string
    template<typename T, enable_if_t<!is_vector<T>::value, detail::enabler> = detail::dummy>
    Option* add_option(
            std::string name,           ///< The name, long,short
            T &variable,                ///< The variable to set
            std::string discription="", ///< Discription string
            detail::Combiner opts=VALIDATORS    ///< The options (REQUIRED, DEFAULT, POSITIONAL, ARGS())
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
            return detail::lexical_cast(res[0][0], variable);
        };

        Option* retval = add_option(name, fun, discription, opts);
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
            std::string name,           ///< The name, long,short
            std::vector<T> &variable,   ///< The variable to set
            std::string discription="", ///< Discription string
            detail::Combiner opts=ARGS          ///< The options (REQUIRED, DEFAULT, POSITIONAL, ARGS())
            ) {

        if(opts.num==0)
            throw IncorrectConstruction("Must have ARGS or be a vector.");
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

        Option* retval =  add_option(name, fun, discription, opts);
        retval->typeval = detail::type_name<T>();
        if(opts.defaulted) {
            retval->defaultval =  "[" + detail::join(variable) + "]";
        }
        return retval;
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
    template<typename T, enable_if_t<std::is_integral<T>::value, detail::enabler> = detail::dummy>
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
            detail::Combiner opts=VALIDATORS       ///< The options (REQUIRED, DEFAULT, POSITIONAL, ARGS())
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
            bool retval = detail::lexical_cast(res[0][0], member);
            if(!retval)
                return false;
            return std::find(std::begin(options), std::end(options), member) != std::end(options);
        };

        Option* retval = add_option(name, fun, discription, opts);
        retval->typeval = detail::type_name<T>();
        retval->typeval += " in {" + detail::join(options) + "}";
        if(opts.defaulted) {
            std::stringstream out;
            out << member;
            retval->defaultval = out.str();
        }
        return retval;
    }




    //------------ MAKE STYLE ---------//

    /// Prototype for new output style
    template<typename T = std::string,
        enable_if_t<!is_vector<T>::value, detail::enabler> = detail::dummy>
    Value<T> make_option(
            std::string name,              ///< The name, short,long
            std::string discription="",
            detail::Combiner opts=VALIDATORS
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
            return detail::lexical_cast(res[0][0], **ptr);
        };
        Option* retval = add_option(name, fun, discription, opts);
        retval->typeval = detail::type_name<T>();
        return out;
    }

    /// Prototype for new output style with default
    template<typename T,
        enable_if_t<!is_vector<T>::value, detail::enabler> = detail::dummy>
    Value<T> make_option(
            std::string name,              ///< The name, short,long
            const T& default_value,
            std::string discription="",
            detail::Combiner opts=VALIDATORS
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
            return detail::lexical_cast(res[0][0], **ptr);
        };
        Option* retval = add_option(name, fun, discription, opts);
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
            std::string name,              ///< The name, short,long
            std::string discription="",
            detail::Combiner opts=VALIDATORS
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
                    retval &= detail::lexical_cast(b, (*ptr)->back());
                }
            return (*ptr)->size() > 0 && retval;
        };
        Option* retval =  add_option(name, fun, discription, opts);
        retval->typeval = detail::type_name<T>();
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
            detail::Combiner opts=VALIDATORS       ///< The options (REQUIRED, DEFAULT, POSITIONAL, ARGS())
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
            bool retval = detail::lexical_cast(res[0][0], **ptr);
            if(!retval)
                return false;
            return std::find(std::begin(options), std::end(options), **ptr) != std::end(options);
        };

        Option* retval = add_option(name, fun, discription, opts);
        retval->typeval = detail::type_name<T>();
        retval->typeval += " in {" + detail::join(options) + "}";
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
            throw PositionalError("[" + detail::join(positionals) + "]");
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
            std::cerr << help() << std::endl;
        } else {
            std::cout << help() << std::endl;
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

    std::string help() const {
        std::stringstream out;
        if(name != "")
            out << "Subcommand: " << name << " ";
        out << prog_discription << std::endl;
        int len = std::accumulate(std::begin(options), std::end(options), 0,
                [](int val, const Option &opt){
                    return std::max(opt.help_len()+3, val);});
        for(const Option &opt : options) {
            out << opt.help(len) << std::endl;
        }
        if(subcommands.size()> 0) {
            out << "Subcommands:" << std::endl;
            int max = std::accumulate(std::begin(subcommands), std::end(subcommands), 0,
                    [](int i, const std::unique_ptr<App> &j){return std::max(i, (int) j->get_name().length()+3);});
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
