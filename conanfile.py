from conans import ConanFile, CMake
from conans.tools import load
import re

def get_version():
    try:
        content = load("include/CLI/Version.hpp")
        version = re.search(r'#define CLI11_VERSION "(.*)"', content).group(1)
        return version
    except Exception:
        return None

class CLI11Conan(ConanFile):
    name = "CLI11"
    version = get_version()
    description = "Command Line Interface toolkit for C++11"
    topics = ("cli", "c++11", "parser", "cli11")
    url = "https://github.com/CLIUtils/CLI11"
    homepage = "https://github.com/CLIUtils/CLI11"
    author = "Henry Schreiner <hschrein@cern.ch>"
    license = "BSD-3-Clause"

    settings = "os", "compiler", "arch", "build_type"
    exports_sources = "LICENSE", "README.md", "include/*", "extern/*", "cmake/*", "CMakeLists.txt", "tests/*"

    def build(self): # this is not building a library, just tests
        cmake = CMake(self)
        cmake.definitions["CLI11_EXAMPLES"] = "OFF"
        cmake.definitions["CLI11_SINGLE_FILE"] = "OFF"
        cmake.configure()
        cmake.build()
        cmake.test()
        cmake.install()

    def package_id(self):
        self.info.header_only()
