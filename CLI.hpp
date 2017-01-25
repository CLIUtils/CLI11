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

// This is unreachable outside this file; you should not use Combiner directly
namespace {

void logit(std::string output) {
    std::cout << output << std::endl;
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
        self.num = num + b.num;
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

class BadNameString : public std::runtime_error {
public:
    BadNameString(std::string name) : runtime_error("Failed to parse: " + name) {};
};

class CallForHelp : public std::runtime_error {
public:
    CallForHelp() : runtime_error("Help option passed") {};
};

class ParseError : public std::runtime_error {
public:
    ParseError(std::string info="") : runtime_error(info) {};
};

class OptionAlreadyAdded : public std::runtime_error {
public:
    OptionAlreadyAdded(std::string name) : runtime_error("Already added:" + name) {};
};



const std::regex reg_split{R"regex((?:([a-zA-Z0-9]?)(?:,|$)|^)([a-zA-Z0-9][a-zA-Z0-9_\-]*)?)regex"};
const std::regex reg_short{R"regex(-([^-])(.*))regex"};
const std::regex reg_long{R"regex(--([^-^=][^=]*)=?(.*))regex"};


std::tuple<std::string, std::string> split(std::string fullname) throw(BadNameString) {

    std::smatch match;
    if (std::regex_match(fullname, match, reg_split)) {
        std::string sname = match[1];
        std::string lname = match[2];
        if(sname == "" and lname == "")
            throw BadNameString("EMPTY");
        return std::tuple<std::string, std::string>(sname, lname);
    } else throw BadNameString(fullname);
}

const Combiner NOTHING   {0,false,false,false};
const Combiner REQUIRED  {0,false,true, false};
const Combiner DEFAULT   {0,false,false,true};
const Combiner POSITIONAL{0,true, false,false};
const Combiner ARGS      {1,false,false,false};

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
    Option(std::string name, std::string discription = "", Combiner opts=NOTHING, std::function<bool(results_t)> callback=[](results_t){return true;}) throw (BadNameString) :
      opts(opts), discription(discription), callback(callback){
        std::tie(sname, lname) = split(name);
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

    std::string getName() const {
        if(sname=="")
            return "--" + lname;
        else if (lname=="")
            return "-" + sname;
        else
            return "-" + sname + ", --" + lname;
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


    int get_num() const {
        return opts.num;
    }

    void add_result(int r, std::string s) {
        results.at(r).push_back(s);
    }
    int get_new() {
        results.emplace_back();
        return results.size() - 1;
    }
    int count() {
        return results.size();
    }

    std::string string() const {
        std::string val = "Option: " + getName() + "\n"
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

};

/// Creates a command line program, with very few defaults.
/** To use, create a new Program() instance with argc, argv, and a help discription. The templated
*  add_option methods make it easy to prepare options. Remember to call `.start` before starting your
* program, so that the options can be evaluated and the help option doesn't accidentally run your program. */
class App {
public:

protected:
    
    std::string desc;
    std::vector<Option> options;
    std::vector<std::string> missing_options;
    std::vector<std::string> positionals;

public:

    
    /// Create a new program. Pass in the same arguments as main(), along with a help string.
    App(std::string discription)
        : desc(discription) {
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
            throw OptionAlreadyAdded(myopt.getName());

    }

    /// Add option for string
    void add_option(
            std::string name,           ///< The name, long,short
            std::string &variable,      ///< The variable to set
            std::string discription="", ///< Discription string
            Combiner opts=ARGS          ///< The options (REQUIRED, DEFAULT, POSITIONAL, ARGS())
            ) {

        CLI::callback_t fun = [&variable](CLI::results_t res){
            if(res.size()!=1) {
                return false;
            }
            if(res[0].size()!=1) {
                return false;
            }
            variable = res[0][0];
            return true;
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
            count = res.size();
            return true;
        };
        
        add_option(name, fun, discription, NOTHING);
    }
    

    /// Parses the command line - throws errors
    void parse(int argc, char **argv) throw(CallForHelp, ParseError) {
        std::vector<std::string> args;
        for(int i=1; i<argc; i++)
            args.emplace_back(argv[i]);
        parse(args);
    }

    void parse(std::vector<std::string> args) throw(CallForHelp, ParseError) {
        std::reverse(args.begin(), args.end());

        bool positional_only = false;
        
        while(args.size()>0) {

            if(args.back() == "--") {
                args.pop_back();
                positional_only = true;
            } else if(positional_only || (!_parse_long(args) && !_parse_short(args))) {

                logit("Positional: "+args.back());
                positionals.push_back(args.back());
                args.pop_back();
            }
        }

        for(Option& opt : options)
            if (opt.count() > 0) {
                logit(opt.string());
                if(!opt.run_callback())
                    logit("Failed");
            }

        //TODO: Check for false callbacks
    }
 
    bool _parse_short(std::vector<std::string> &args) {
        std::string current = args.back();
        std::smatch match;

        if(!std::regex_match(current, match, reg_short))
            return false;
        
        args.pop_back();
        std::string name = match[1];
        std::string rest = match[2];

        logit("Working on short:");
        logit(name);
        logit(rest);

        auto op = std::find_if(std::begin(options), std::end(options), [name](const Option &v){return v.check_sname(name);});

        if(op == std::end(options)) {
            missing_options.push_back("-" + op->get_sname());
            return true;
        }

        int vnum = op->get_new();
        int num = op->get_num();
        

        if(rest != "" && num > 0) {
            num--;
            op->add_result(vnum, rest);
            rest = "";
        }

        while(num>0) {
            num--;
            std::string current = args.back();
            logit("Adding: "+current);
            args.pop_back();
            op->add_result(vnum,current);
            if(args.size()==0)
                return true;
        }

        if(rest != "") {
            rest = "-" + rest;
            args.push_back(rest);
        }
        return true;
    }

    bool _parse_long(std::vector<std::string> &args) {
        std::string current = args.back();
        std::smatch match;

        if(!std::regex_match(current, match, reg_long))
            return false;
        
        args.pop_back();
        std::string name = match[1];
        std::string value = match[2];


        logit("Working on long:");
        logit(name);
        logit(value);

        auto op = std::find_if(std::begin(options), std::end(options), [name](const Option &v){return v.check_lname(name);});

        if(op == std::end(options)) {
            missing_options.push_back("--" + op->get_lname());
            return true;
        }


        int vnum = op->get_new();
        int num = op->get_num();
        

        if(value != "") {
            num--;
            op->add_result(vnum, value);
        }

        while(num>0) {
            num--;
            std::string current = args.back();
            args.pop_back();
            op->add_result(vnum,current);
            if(args.size()==0)
                return true;
        }
        return true;
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
        } catch(const ParseError &e) {
            std::cerr << "ERROR:" << std::endl;
            std::cerr << e.what() << std::endl;
            std::cerr << help() << std::endl;
            exit(1);
        }

    }

    /// Counts the number of times the given option was passed.
    int count(std::string name) const {
        return 0;
    }

    std::string help() const {return "";}
    
    
};
}
