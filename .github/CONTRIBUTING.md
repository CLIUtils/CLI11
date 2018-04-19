Thanks for considering to write a Pull Request (PR) for CLI11! Here are a few guidelines to get you started:

Make sure you are comfortable with the license; all contributions are licensed under the original license.

## Adding functionality
Make sure any new functions you add are are:

* Documented by `///` documentation for Doxygen
* Mentioned in the instructions in the README, though brief mentions are okay
* Explained in your PR (or previously explained in an Issue mentioned in the PR)
* Completely covered by tests

In general, make sure the addition is well thought out and does not increase the complexity of CLI11 needlessly.

## Things you should know:

* Once you make the PR, tests will run to make sure your code works on all supported platforms
* The test coverage is also measured, and that should remain 100%
* Formatting should be done with clang-format, otherwise the format check will not pass. However, it is trivial to apply this to your PR, so don't worry about this check. If you do have clang-format, just run `scripts/check_style.sh`
* Everything must pass clang-tidy as well, run with `-DCLANG_TIDY_FIX-ON` (make sure you use a single threaded build process!)

Note that the style check is really just:

```bash
git ls-files -- '.cpp' '.hpp' | xargs clang-format -i -style=file
```

And, if you want to always use it, feel free to install the git hook provided in scripts.

## For developers releasing to Conan.io

This is now done by the CI system on tagged releases. Previously, the steps to make a Conan.io release were:

```bash
conan remove '*' # optional, I like to be clean
conan create . cliutils/stable
conan upload "*" -r cli11 --all
```

Here I've assumed that the remote is `cli11`.
