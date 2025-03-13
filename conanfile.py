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
        """
        note that for boost/1.87.0 there will occur an error:
        error: ‘from_string’ is not a member of ‘boost::asio::ip::address’
            boost::asio::ip::address::from_string(ip_), port_);
        solution might be boost::asio::ip::make_address(ip_) (untested)
        """
        self.requires("boost/1.86.0")
        self.requires("pcapplusplus/24.09")
        self.requires("cli11/2.4.2")

    def build_requirements(self):
        self.test_requires("gtest/1.15.0")
        self.tool_requires("cppcheck/2.16.0")
        self.tool_requires("cmake/3.30.5")

    def layout(self):
        cmake_layout(self)

    def generate(self):
        tc = CMakeToolchain(self)
        tc.generate()

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()
        cmake.ctest(cli_args=["--output-on-failure"])
        # add includes so that headers are found
        include1 = os.path.join(self.source_folder, "includes")

        self.run(f"cppcheck --enable=all {self.source_folder} "\
                f"-I {include1} "\
                "--check-level=exhaustive "\
                "--suppress=missingIncludeSystem -i .git/ -i build/ -i bin/")


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
