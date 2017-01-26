#pragma once

#include <string>
#include <regex>
#include <iostream>
#include <functional>
#include <tuple>
#include <exception>
#include <algorithm>
#include <sstream>
#include <type_traits>
#include <unordered_set>
#include <iomanip>
#include <numeric>

// This is unreachable outside this file; you should not use Combiner directly
namespace {

void logit(std::string output) {
    std::cout << "\033[1;31m" << output << "\033[0m" << std::endl;
}

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


struct Combiner {
    int num;
    bool positional;
    bool required;
    bool defaulted;

    /// Can be or-ed together
    Combiner operator | (Combiner b) const {
        Combiner self;
        self.num = std::min(num, b.num) == -1 ? -1 : std::max(num, b.num);
        self.positional = positional || b.positional;
        self.required = required || b.required;
        self.defaulted = defaulted || b.defaulted;
        return self;
    }

    /// Call to give the number of arguments expected on cli
    Combiner operator() (int n) const {
        return Combiner{n, positional, required, defaulted};
    }
    Combiner operator, (Combiner b) const {
        return *this | b;
    }
};
}


namespace CLI {

class Error : public std::runtime_error {
public:
    Error(std::string parent, std::string name) : runtime_error(parent + ": " + name) {}
};

class BadNameString : public Error {
public:
    BadNameString(std::string name) : Error("BadNameString", name) {}
};

class CallForHelp : public Error {
public:
    CallForHelp() : Error("CallForHelp","") {}
};

class ParseError : public Error {
public:
    ParseError(std::string name) : Error("ParseError", name) {}
};

class OptionAlreadyAdded : public Error {
public:
    OptionAlreadyAdded(std::string name) : Error("OptionAlreadyAdded", name) {}
};

class OptionNotFound : public Error {
public:
    OptionNotFound(std::string name) : Error("OptionNotFound", name) {}
};

class RequiredError : public Error {
public:
    RequiredError(std::string name) : Error("RequiredError", name) {}
};

class ExtraPositionalsError : public Error {
public:
    ExtraPositionalsError(std::string name) : Error("ExtraPositionalsError", name) {}
};

class HorribleError : public Error {
public:
    HorribleError(std::string name) : Error("HorribleError", "(You should never see this error) " + name) {}
};
class IncorrectConstruction : public Error {
public:
    IncorrectConstruction(std::string name) : Error("IncorrectConstruction", name) {}
};

const std::regex reg_split{R"regex((?:([a-zA-Z0-9]?)(?:,|$)|^)([a-zA-Z0-9][a-zA-Z0-9_\-]*)?)regex"};
const std::regex reg_short{R"regex(-([^-])(.*))regex"};
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

const Combiner NOTHING   {0, false,false,false};
const Combiner REQUIRED  {1, false,true, false};
const Combiner DEFAULT   {1, false,false,true};
const Combiner POSITIONAL{1, true, false,false};
const Combiner ARGS      {1, false,false,false};
const Combiner UNLIMITED {-1,false,false,false};

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

};


template<typename T>
typename std::enable_if<std::is_integral<T>::value, bool>::type
lexical_cast(std::string input, T& output) {
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
    
template<typename T>
typename std::enable_if<std::is_floating_point<T>::value, bool>::type
lexical_cast(std::string input, T& output) {
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
template<typename T>
bool lexical_cast(std::string input, T& output) {
    logit("Direct lexical cast: " + input);
    output = input;
    return true;
}

enum class Classifer {NONE, POSITIONAL_MARK, SHORT, LONG, SUBCOMMAND};

class App;

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
    std::vector<std::string> positionals;
    std::vector<App*> subcommands;
    bool parsed{false};
    App* subcommand = nullptr;

public:

    
    /// Create a new program. Pass in the same arguments as main(), along with a help string.
    App(std::string prog_discription="")
        : prog_discription(prog_discription) {

            add_flag("h,help", "Print this help message and exit");

    }

    ~App() {
        for(App* app : subcommands)
            delete app;
    }

    App* add_subcommand(std::string name, std::string discription="") {
        subcommands.push_back(new App(discription));
        subcommands.back()->name = name;
        logit(subcommands.back()->name);
        return subcommands.back();
    }
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
    void add_option(
            std::string name,           ///< The name, long,short
            callback_t callback,        ///< The callback
            std::string discription="", ///< Discription string
            Combiner opts=ARGS          ///< The options (REQUIRED, DEFAULT, POSITIONAL, ARGS())
            ) {
        Option myopt{name, discription, opts, callback};
        if(std::find(std::begin(options), std::end(options), myopt) == std::end(options))
            options.push_back(myopt);
        else
            throw OptionAlreadyAdded(myopt.get_name());

    }

    /// Add option for string
    template<typename T>
    typename std::enable_if<!std::is_array<T>::value, void>::type
    add_option(
            std::string name,           ///< The name, long,short
            T &variable,                ///< The variable to set
            std::string discription="", ///< Discription string
            Combiner opts=ARGS          ///< The options (REQUIRED, DEFAULT, POSITIONAL, ARGS())
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

        add_option(name, fun, discription, opts);
    }

    /// Add option for vector of results
    template<typename T>
    void add_option(
            std::string name,           ///< The name, long,short
            std::vector<T> &variable,   ///< The variable to set
            std::string discription="", ///< Discription string
            Combiner opts=ARGS          ///< The options (REQUIRED, DEFAULT, POSITIONAL, ARGS())
            ) {

        if(opts.num==0)
            throw IncorrectConstruction("Must have ARGS(1) or be a vector.");
        CLI::callback_t fun = [&variable](CLI::results_t res){
            bool retval = true;
            int count = 0;
            variable.clear();
            for(const auto &a : res)
                for(const auto &b : a) {
                    variable.emplace_back();
                    retval &= lexical_cast(b, variable.back());
                }
            return count != 0 && retval;
        };

        add_option(name, fun, discription, opts);
    }


    /// Add option for flag
    void add_flag(
            std::string name,           ///< The name, short,long
            std::string discription=""  ///< Discription string
            ) {
        CLI::callback_t fun = [](CLI::results_t res){
            return true;
        };
        
        add_option(name, fun, discription, NOTHING);
    }

    /// Add option for flag
    template<typename T>
    typename std::enable_if<std::is_integral<T>::value, void>::type
    add_flag(
            std::string name,           ///< The name, short,long
                    T  &count,              ///< A varaible holding the count
            std::string discription=""  ///< Discription string
            ) {

        count = 0;
        CLI::callback_t fun = [&count](CLI::results_t res){
            count = (T) res.size();
            return true;
        };
        
        add_option(name, fun, discription, NOTHING);
    }

    /// Add set of options
    template<typename T>
    void add_set(
            std::string name,              ///< The name, short,long
            T &member,                     ///< The selected member of the set
            std::unordered_set<T> options, ///< The set of posibilities
            std::string discription="",    ///< Discription string
            Combiner opts=ARGS             ///< The options (REQUIRED, DEFAULT, POSITIONAL, ARGS())
            ) {

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
            return std::find(std::begin(options), std::end(options), retval) != std::end(options);
        };

        add_option(name, fun, discription, opts);
    }

    

    /// Parses the command line - throws errors
    void parse(int argc, char **argv) {
        std::vector<std::string> args;
        for(int i=argc-1; i>0; i--)
            args.push_back(argv[i]);
        parse(args);
    }

    void parse(std::vector<std::string> args) {
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
                opt.add_result(0, positionals.back());
                positionals.pop_back();
            }
            if (opt.required() && opt.count() < opt.expected())
                throw RequiredError(opt.get_name());
            if (opt.count() > 0) {
                if(!opt.run_callback())
                    throw ParseError(opt.get_name());
            }

        }
        if(positionals.size()>0)
            throw ExtraPositionalsError("[" + join(positionals) + "]");
    }

    void _parse_subcommand(std::vector<std::string> &args) {
        for(App *com : subcommands) {
            if(com->name == args.back()){ 
                args.pop_back();
                com->parse(args);
                subcommand = com;
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
            std::string current = args.back();
            while(_recognize(current) == Classifer::NONE) {
                std::string current = args.back();
                args.pop_back();
                op->add_result(vnum,current);
                if(args.size()==0)
                    return;

            }
        } else while(num>0) {
            num--;
            std::string current = args.back();
            logit("Adding: "+current);
            args.pop_back();
            op->add_result(vnum,current);
            if(args.size()==0)
                return;
        }

        if(rest != "") {
            rest = "-" + rest;
            args.push_back(rest);
        }
    }

    Classifer _recognize(std::string current) const {
        if(current == "--")
            return Classifer::POSITIONAL_MARK;
        for(const App* com : subcommands) {
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
            std::string current = args.back();
            while(_recognize(current) == Classifer::NONE) {
                std::string current = args.back();
                args.pop_back();
                op->add_result(vnum,current);
                if(args.size()==0)
                    return;

            }
        } else while(num>0) {
            num--;
            std::string current = args.back();
            args.pop_back();
            op->add_result(vnum,current);
            if(args.size()==0)
                return;
        }
        return;
    }

    /// This must be called after the options are in but before the rest of the program.
    /** Instead of throwing erros, causes the program to exit
     * if -h or an invalid option is passed. */
    void start(int argc, char** argv) {
        try {
            parse(argc, argv);
        } catch(const CallForHelp &e) {
            std::cout << help() << std::endl;
            exit(0);
        } catch(const Error &e) {
            std::cerr << "ERROR: ";
            std::cerr << e.what() << std::endl;
            std::cerr << help() << std::endl;
            exit(1);
        }

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
                    [](int i, const App* j){return std::max(i, (int) j->get_name().length()+1);});
            for(const App* com : subcommands) {
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
