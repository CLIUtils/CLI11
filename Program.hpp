#pragma once

#include <string>

#include <boost/program_options.hpp>


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
    Combiner operator, (Combiner b) const {
        return *this | b;
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
    boost::program_options::options_description desc;
    boost::program_options::positional_options_description p;
    boost::program_options::variables_map vm;
    
    int argc;
    char **argv;
    
    /// Parses the command line (internal function)
    void parse() {
        try {
            boost::program_options::store(boost::program_options::command_line_parser(argc, argv)
                    .options(desc).positional(p).run(), vm);

            if(vm.count("help")){
                std::cout << desc;
                exit(0);
            }

            boost::program_options::notify(vm);
        } catch(const boost::program_options::error& e) {
            std::cerr << "ERROR: " << e.what() << std::endl << std::endl; 
            std::cerr << desc << std::endl; 
            exit(1);
        }
    }

    
public:
    
    /// Create a new program. Pass in the same arguments as main(), along with a help string.
    Program(int argc, char** argv, std::string discription)
        : argc(argc), argv(argv), desc(discription) {
        desc.add_options()
        ("help,h", "Display this help message");
    }
    
    /// Allows you to manually add options in the boost style.
    /** Usually the specialized methods are easier, but this remains for people used to Boost and for
     * unusual situations. */
    boost::program_options::options_description_easy_init add_options() {
        return desc.add_options();
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
            std::string description, ///< Discription string
            Combiner options         ///< The options (REQUIRED, DEFAULT, POSITIONAL)
            ) {
        auto po_value = boost::program_options::value<T>(&value);
        if(options.defaulted)
            po_value = po_value->default_value(value);
        if(options.required)
            po_value = po_value->required();
        desc.add_options()(name.c_str(),po_value,description.c_str());
        if(options.positional!=0)
            p.add(name.c_str(), options.positional);
    }
    
    /// Adds a flag style option
    void add_option(std::string name, std::string description) {
        desc.add_options()(name.c_str(),description.c_str());
    }
    
    
    /// This must be called after the options are in but before the rest of the program.
    /** Calls the Boost boost::program_options initialization, causing the program to exit
     * if -h or an invalid option is passed. */
    void start() {
        parse();
    }

    /// Counts the number of times the given option was passed.
    int count(std::string name) const {
        return vm.count(name.c_str());
    }
    
    
};
