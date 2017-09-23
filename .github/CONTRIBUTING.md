# A few notes on contributions

If you want to add code, please make sure it passes the clang-format style (I am using LLVM 4.0):

```bash
git ls-files -- '.cpp' '.hpp' | xargs clang-format -i -style=file
```

It is also a good idea to check this with `clang-tidy`; automatic fixes can be made using `-DCLANG_TIDY_FIX-ON` (resets to `OFF` when rerunning CMake).
