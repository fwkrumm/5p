import os
from conan import ConanFile
from conan.tools.cmake import CMake, CMakeToolchain, cmake_layout
from conan.tools.files import copy

class PPPPP(ConanFile):
    name = "5p"
    version = "0.0.1"
    settings = "os", "compiler", "build_type", "arch"
    exports_sources = "*"
    generators = "CMakeDeps"

    def requirements(self):
        self.requires("boost/1.86.0")
        self.requires("pcapplusplus/23.09")
        self.requires("cli11/2.4.2")

    def build_requirements(self):
        #self.test_requires("gtest/1.15.0") # not yet used
        #self.tool_requires("cppcheck/2.15.0") # not yet used
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
        # deploy binary
        copy(self, "bin/*", src=self.source_folder,
             dst=os.path.join(self.package_folder, "bin"),
             keep_path=False)
        # deploy project license file
        copy(self, "LICENSE.txt", src=self.source_folder,
             dst=os.path.join(self.package_folder, "licenses", self.name),
             keep_path=False)

    def package_info(self):
        # deploy license files of dependencies
        for name, dep in self.dependencies.items():
            copy(self, "*license*",
                 src=dep.package_folder,
                 dst=os.path.join(self.package_folder, "licenses", name.ref.name),
                 ignore_case=True)
