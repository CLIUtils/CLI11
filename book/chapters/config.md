# Accepting configure files

## Reading a configure file

You can tell your app to allow configure files with `set_config("--config")`. There are arguments: the first is the option name. If empty, it will clear the config flag. The second item is the default file name. If that is specified, the config will try to read that file. The third item is the help string, with a reasonable default, and the final argument is a boolean (default: false) that indicates that the configuration file is required and an error will be thrown if the file
is not found and this is set to true.

## Configure file format

Here is an example configuration file, in INI format:

```ini
; Commments are supported, using a ;
; The default section is [default], case insensitive

value = 1
str = "A string"
vector = 1 2 3

; Section map to subcommands
[subcommand]
in_subcommand = Wow
sub.subcommand = true
```

Spaces before and after the name and argument are ignored. Multiple arguments are separated by spaces. One set of quotes will be removed, preserving spaces (the same way the command line works). Boolean options can be `true`, `on`, `1`, `yes`; or `false`, `off`, `0`, `no` (case insensitive). Sections (and `.` separated names) are treated as subcommands (note: this does not mean that subcommand was passed, it just sets the "defaults".

## Writing out a configure file

To print a configuration file from the passed arguments, use `.config_to_str(default_also=false, prefix="", write_description=false)`, where `default_also` will also show any defaulted arguments, `prefix` will add a prefix, and `write_description` will include option descriptions.

## Custom formats

{% hint style='info' %}
New in CLI11 1.6
{% endhint %}

You can invent a custom format and set that instead of the default INI formatter. You need to inherit from `CLI::Config` and implement the following two functions:

```cpp
std::string to_config(const CLI::App *app, bool default_also, bool, std::string) const;
std::vector<CLI::ConfigItem> from_config(std::istream &input) const;
```

The `CLI::ConfigItem`s that you return are simple structures with a name, a vector of parents, and a vector of results. A optionally customizable `to_flag` method on the formatter lets you change what happens when a ConfigItem turns into a flag.

Finally, set your new class as new config formatter:

```cpp
app.config_formatter(std::make_shared<NewConfig>());
```

See [`examples/json.cpp`](https://github.com/CLIUtils/CLI11/blob/master/examples/json.cpp) for a complete JSON config example.
