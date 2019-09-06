# Validators

{% hint style='info' %}
Improved in CLI11 1.6
{% endhint %}

There are two forms of validators:

* `transform` validators: mutating
* `check` validators: non-mutating (recommended unless the parsed string must be mutated)

A transform validator comes in one form, a function with the signature `std::string(std::string)`.
The function will take a string and return the modified version of the string. If there is an error,
the function should throw a `CLI::ValidationError` with the appropriate reason as a message.

However, `check` validators come in two forms; either a simple function with the const version of the
above signature, `std::string(const std::string &)`, or a subclass of `struct CLI::Validator`. This
structure has two members that a user should set; one (`func`) is the function to add to the Option
(exactly matching the above function signature, since it will become that function), and the other is
`tname`, and is the type name to set on the Option (unless empty, in which case the typename will be
left unchanged).

Validators can be combined with `&` and `|`, and they have an `operator()` so that you can call them
as if they were a function. In CLI11, const static versions of the validators are provided so that
the user does not have to call a constructor also.

An example of a custom validator:

```cpp
struct LowerCaseValidator : public Validator {
    LowerCaseValidator() {
        tname = "LOWER";
        func = [](const std::string &str) {
            if(CLI::detail::to_lower(str) != str)
                return std::string("String is not lower case");
            else
                return std::string();
        };
    }
};
const static LowerCaseValidator Lowercase;
```

If you were not interested in the extra features of Validator, you could simply pass the lambda function above to the `->check()` method of `Option`.

The built-in validators for CLI11 are:

| Validator           | Description |
|---------------------|-------------|
| `ExistingFile`      | Check for existing file (returns error message if check fails) |
| `ExistingDirectory` | Check for an existing directory (returns error message if check fails) |
| `ExistingPath`      | Check for an existing path |
| `NonexistentPath`   | Check for an non-existing path |
| `Range(min=0, max)` |  Produce a range (factory). Min and max are inclusive. |


