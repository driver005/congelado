from conan import ConanFile
from conan.tools.cmake import cmake_layout, CMakeToolchain, CMakeDeps

class MyProjectConan(ConanFile):
    name = "congelado"
    version = "0.1.0"
    settings = "os", "compiler", "build_type", "arch"
    requires = (
      "fmt/[>=12 <14]",
      "simdjson/[>=4 <5]",
      "grpc/[>=1.78 <2]",
      "protobuf/[>=6 <8]",
    )

    def build_requirements(self):
        self.tool_requires("cmake/[>=4 <5]")
        self.tool_requires("ninja/[>=1 <2]")
        self.tool_requires("grpc/[>=1.78 <2]")
        self.tool_requires("protobuf/[>=6 <8]")

    def layout(self):
        cmake_layout(self)

    def configure(self):
        # Pass information to the dependencies instead of forcing CMake variables
        self.options["protobuf"].build_protoc = True 

    def generate(self):
        tc = CMakeToolchain(self)
        tc.variables["CMAKE_EXPERIMENTAL_CXX_IMPORT_STD"] = "d0edc3af-4c50-42ea-a356-e2862fe7a444"
        tc.variables["CMAKE_CXX_STANDARD"] = "23"
        tc.variables["CMAKE_CXX_STANDARD_REQUIRED"] = "ON"
        # Remove the manual protobuf_BUILD_PROTOC_BINARIES line here
        tc.generate()

        deps = CMakeDeps(self)
        deps.generate()
