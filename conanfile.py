from conan import ConanFile
from conan.tools.cmake import CMake, CMakeToolchain, cmake_layout
from conan.tools.files import copy

class PPPPP(ConanFile):
    name = "5p"
    version = "1.0.0"
    settings = "os", "compiler", "build_type", "arch"
    exports_sources = "src/*"
    generators = "CMakeDeps"

    def requirements(self):
        self.requires("boost/1.86.0")
        self.requires("pcapplusplus/23.09")
        self.requires("cli11/2.4.2")

    def build_requirements(self):
        self.test_requires("gtest/1.15.0")
        self.tool_requires("cppcheck/2.15.0")
        self.tool_requires("cmake/3.30.1")

    def layout(self):
        cmake_layout(self)

    def generate(self):
        tc = CMakeToolchain(self)
        tc.generate()

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()

    def package(self):
        cmake = CMake(self)
        cmake.install()

    def package_info(self):
        pass

    def imports(self):
        copy(self, "license*", dst="licenses", folder=True, ignore_case=True)
