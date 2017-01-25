#pragma once

#include <string>
#include <functional>
#include <unordered_set>
#include <vector>
#include <iostream>

#include "optionparser.h"

// This is unreachable outside this file; you should not use Combiner directly
namespace {

struct Combiner {
    int positional;
    bool required;
    bool defaulted;

    /// Can be or-ed together
    Combiner operator | (Combiner b) const {
        Combiner self;
        self.positional = positional + b.positional;
        self.required = required || b.required;
        self.defaulted = defaulted || b.defaulted;
        return self;
    }

    /// Call to give the number of arguments expected on cli
    Combiner operator() (int n) const {
        return Combiner{n, required, defaulted};
    }
};
}

/// Creates a command line program, with very few defaults.
/** To use, create a new Program() instance with argc, argv, and a help description. The templated
*  add_option methods make it easy to prepare options. Remember to call `.start` before starting your
* program, so that the options can be evaluated and the help option doesn't accidentally run your program. */
class Program {
public:
    static constexpr Combiner REQUIRED{0,true,false};
    static constexpr Combiner DEFAULT{0,false,true};
    static constexpr Combiner POSITIONAL{1,false,false};

protected:
    std::vector<option::Descriptor> usage;
    std::vector<std::function<bool(std::vector<std::string>)>> convert; /// Number is loc+2
    std::unordered_set<int> required;
    std::vector<int> counts;
    std::vector<std::string> random_name_store;

    int argc;
    char **argv;
    
    /// Parses the command line (internal function)
    void parse() {
        usage.push_back(option::Descriptor{0, 0, nullptr, nullptr, nullptr, nullptr});

        option::Stats stats(usage.data(), argc, argv);
        std::vector<option::Option> options(stats.options_max);
        std::vector<option::Option> buffer(stats.buffer_max);
        option::Parser parse(usage.data(), argc, argv, options.data(), buffer.data());

        if(parse.error()) {
            std::cerr << "ERROR. See usage:" << std::endl;
            option::printUsage(std::cerr, usage.data());
            exit(1);
        }


        if(options[1]){
            option::printUsage(std::cerr, usage.data());
            exit(0);
        }

        bool found_unk = false;
        for (option::Option* opt = options[0]; opt; opt = opt->next()) {
            std::cout << "Unknown option: " << opt->name << "\n";
            found_unk = true;
        }
        if(found_unk)
            exit(2);

        for(int i=2; i<convert.size()+2; i++) {
            counts.emplace_back(options[i].count());
            if(options[i]) {
                std::vector<std::string> opt_list;
                for(option::Option* opt = options[i]; opt; opt = opt->next())
                    opt_list.emplace_back(opt->arg ? opt->arg : "");
                convert.at(i-2)(opt_list);
            }
        }
    }
    
public:
    
    /// Create a new program. Pass in the same arguments as main(), along with a help string.
    Program(int argc, char** argv, std::string description)
        : argc(argc), argv(argv) {
        random_name_store.emplace_back(description);
        usage.push_back(option::Descriptor{0, 0, "", "", option::Arg::None, random_name_store.back().c_str()});
        usage.push_back(option::Descriptor{1, 0, "h", "help", option::Arg::None, "Display usage and exit."});
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
     *     program.add_option("filename", filename, "description of filename");
     */
    template<typename T>
    void add_option(
            std::string name,        ///< The name, long,short
            T &value,                ///< The value
            std::string description, ///< Description string
            Combiner options         ///< The options (REQUIRED, DEFAULT, POSITIONAL)
            ) {

        int curr_num = convert.size();

        if(options.required)
            required.emplace(curr_num);

        random_name_store.emplace_back(name);
        random_name_store.emplace_back(description);

        usage.push_back(option::Descriptor{(unsigned int) convert.size()+2, 0, "", random_name_store.at(random_name_store.size()-2).c_str(), option::Arg::Optional, random_name_store.back().c_str()});
        add_option_internal(value);

        if(options.positional!=0)
            std::cout << "positional args not yet supported" << std::endl;


        
    }
    
    /// Adds a flag style option
    void add_flag(std::string name, std::string description, int& flag) {
        counts.emplace_back(0);
        random_name_store.emplace_back(name);
        random_name_store.emplace_back(description);
        usage.push_back(option::Descriptor{(unsigned int) convert.size()+2, 0, "", random_name_store.at(random_name_store.size()-2).c_str(), option::Arg::None, random_name_store.back().c_str()});
        convert.push_back([&flag](std::vector<std::string> v){flag = v.size(); return true;});
    }
    
    void add_option_internal(int &val) {
        convert.push_back([&val](std::vector<std::string> v){val = std::stoi(v.at(0)); return v.size()==1;});
    }
    
    void add_option_internal(std::string &val) {
        convert.push_back([&val](std::vector<std::string> v){val = v.at(0); return v.size()==1;});
    }
    /// This must be called after the options are in but before the rest of the program.
    /** Calls the Boost boost::program_options initialization, causing the program to exit
     * if -h or an invalid option is passed. */
    void start() {
        parse();
    }

    /// Counts the number of times the given option was passed.
    int count(std::string name) const {
        return 0;
    }
    
    
};
