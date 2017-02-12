#pragma once

// Distributed under the MIT license.  See accompanying
// file LICENSE or https://github.com/henryiii/CLI11 for details.

#include <string>
#include <functional>
#include <vector>
#include <tuple>
#include <algorithm>
#include <set>

#include "CLI/Error.hpp"
#include "CLI/StringTools.hpp"
#include "CLI/Split.hpp"

namespace CLI {

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

    std::string description;
    callback_t callback;

    // These are for help strings
    std::string defaultval;
    std::string typeval;
    std::string _group {"Options"};

    bool _default {false};
    bool _required {false};
    int _expected {1};
    bool allow_vector {false};
    std::vector<std::function<bool(std::string)>> _validators;

    std::set<Option*> _requires;
    std::set<Option*> _excludes;
    std::string _envname;

    // Results
    results_t results;


public:
    Option(std::string name, std::string description = "", std::function<bool(results_t)> callback=[](results_t){return true;}, bool _default=true) :
      description(description), callback(callback), _default(_default) {
        std::tie(snames, lnames, pname) = detail::get_names(detail::split_names(name));
    }


    // This class is "true" if optio passed.
    operator bool() const {
        return results.size() > 0;
    }

    /// Clear the parsed results (mostly for testing)
    void clear() {
        results.clear();
    }

    /// Set the option as required
    Option* required(bool value = true) {
        _required = value;
        return this;
    }

    /// Support Plubmum term
    Option* mandatory(bool value = true) {
        return required(value);
    }

    /// True if this is a required option
    bool get_required() const {
        return _required;
    }

    /// Set the number of expected arguments (Flags bypass this)
    Option* expected(int value) {
        if(value == 0)
            throw IncorrectConstruction("Cannot set 0 expected, use a flag instead");
        if(!allow_vector && value != 1)
            throw IncorrectConstruction("You can only change the Expected arguments for vectors");
        _expected = value;
        return this;
    }

    /// The number of arguments the option expects
    int get_expected() const {
        return _expected;
    }

    /// True if this has a default value
    int get_default() const {
        return _default;
    }

    /// True if the argument can be given directly
    bool get_positional() const {
        return pname.length() > 0;
    }

    /// True if option has at least one non-positional name
    bool nonpositional() const {
        return (snames.size() + lnames.size()) > 0;
    }

    /// True if option has description
    bool has_description() const {
        return description.length() > 0;
    }

    /// Adds a validator
    Option* check(std::function<bool(std::string)> validator) {

        _validators.push_back(validator);
        return this;
    }

    /// Changes the group membership
    Option* group(std::string name) {
        _group = name;
        return this;
    }

    /// Get the group of this option
    const std::string& get_group() const {
        return _group;
    }

    /// Get the description
    const std::string& get_description() const {
        return description;
    }

    /// Sets required options
    Option* requires(Option* opt) {
        auto tup = _requires.insert(opt);
        if(!tup.second)
            throw OptionAlreadyAdded(get_name() + " requires " + opt->get_name());
        return this;
    }

    /// Any number supported
    template<typename... ARG>
    Option* requires(Option* opt, Option* opt1, ARG... args) {
        requires(opt);
        return requires(opt1, args...);
    }

    /// Sets excluded options
    Option* excludes(Option* opt) {
        auto tup = _excludes.insert(opt);
        if(!tup.second)
            throw OptionAlreadyAdded(get_name() + " excludes " + opt->get_name());
        return this;
    }

    /// Any number supported
    template<typename... ARG>
    Option* excludes(Option* opt, Option* opt1, ARG... args) {
        excludes(opt);
        return excludes(opt1, args...);
    }

    /// Sets environment variable to read if no option given
    Option* envname(std::string name) {
        _envname = name;
        return this;
    }

    /// The name and any extras needed for positionals
    std::string help_positional() const {
        std::string out = pname;
        if(get_expected()<1)
            out = out + "x" + std::to_string(get_expected());
        else if(get_expected()==-1)
            out = out + "...";
        out = get_required() ? out : "["+out+"]";
        return out;
    }

    // Just the pname
    std::string get_pname() const {
        return pname;
    }


    /// Process the callback
    bool run_callback() const {
        if(_validators.size()>0) {
            for(const std::string & result : flatten_results())
                for(const std::function<bool(std::string)> &vali : _validators)
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

    /// Gets a , sep list of names. Does not include the positional name if opt_only=true.
    std::string get_name(bool opt_only=false) const {
        std::vector<std::string> name_list;
        if(!opt_only && pname.length() > 0)
            name_list.push_back(pname);
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
        for(const std::vector<std::string>& vec : results)
            out += vec.size();
        return out;
    }

    /// The first half of the help print, name plus default, etc
    std::string help_name() const {
        std::stringstream out;
        out << get_name(true);
        if(get_expected() != 0) {
            if(typeval != "")
                out << " " << typeval;
            if(defaultval != "")
                out << "=" << defaultval; 
            if(get_expected() > 1)
                out << " x " << get_expected();
            if(get_expected() == -1)
                out << " ...";
        }
        return out.str();
    }
    
    /// pname with type info
    std::string help_pname() const {
        std::stringstream out;
        out << get_pname();
        if(typeval != "")
            out << " " << typeval;
        if(defaultval != "")
            out << "=" << defaultval; 
        if(get_expected() > 1)
            out << " x " << get_expected();
        if(get_expected() == -1)
            out << " ...";
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



}
