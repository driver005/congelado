from conan import ConanFile
from conan.tools.cmake import cmake_layout, CMakeToolchain, CMakeDeps

class MyProjectConan(ConanFile):
  name = "congelado"
  version = "0.1.0"
  settings = "os", "compiler", "build_type", "arch"
  requires = (
    "fmt/[>=11 <12]",
    "simdjson/[>=3 <4]",
    "grpc/[>=1.72 <2]",
    "protobuf/[>=5 <7]",
  )
  tool_requires = (
    "ninja/[>=1 <2]",
  )

  def layout(self):
    cmake_layout(self)

  def generate(self):
    tc = CMakeToolchain(self)
    tc.cache_variables["protobuf_BUILD_PROTOC_BINARIES"] = True
    tc.generate()

    # Also generate CMakeDeps for dependencies
    deps = CMakeDeps(self)
    deps.generate()
    