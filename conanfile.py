from conans import ConanFile, CMake

class HelloConan(ConanFile):
    name = "CLI11"
    version = "1.3.0"
    url = "https://github.com/CLIUtils/CLI11"
    settings = "os", "compiler", "arch", "build_type"
    license = "BSD 3 clause"
    description = "Command Line Interface toolkit for C++11"

    exports_sources = "LICENCE",  "include/*", "cmake/*", "CMakeLists.txt", "tests/*"

    def build(self): # this is not building a library, just tests
        cmake = CMake(self)
        cmake.definitions["CLI_EXAMPLES"] = "OFF"
        cmake.definitions["CLI_SINGLE_FILE"] = "OFF"
        cmake.configure()
        cmake.build()
        cmake.test()
        cmake.install()

    def package_id(self):
        self.info.header_only()
