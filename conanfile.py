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

class HelloConan(ConanFile):
    name = "CLI11"
    version = get_version()
    url = "https://github.com/CLIUtils/CLI11"
    settings = "os", "compiler", "arch", "build_type"
    license = "BSD 3 clause"
    description = "Command Line Interface toolkit for C++11"

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
