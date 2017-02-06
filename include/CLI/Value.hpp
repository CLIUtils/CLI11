#pragma once

// Distributed under the LGPL version 3.0 license.  See accompanying
// file LICENSE or https://github.com/henryiii/CLI11 for details.

#include <string>
#include <memory>

#include "CLI/Error.hpp"

namespace CLI {

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

}
