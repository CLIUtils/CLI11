#pragma once

// Distributed under the LGPL version 3.0 license.  See accompanying
// file LICENSE or https://github.com/henryiii/CLI11 for details.

#include <string>
#include <functional>
#include <vector>
#include <tuple>
#include <algorithm>

#include "CLI/StringTools.hpp"
#include "CLI/Split.hpp"
#include "CLI/Combiner.hpp"

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



}
