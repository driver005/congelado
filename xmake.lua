-- Project Metadata
set_project("congelado")
set_version("0.1.0")

-- Define Build Options (Equivalent to cmake/options.cmake)
option("build_static")
set_default(false)
set_showmenu(true)
set_description("Build static library and static executable")
option_end()

-- Add Packages (Xmake's built-in package manager can also use Conan)
-- We will use XRepo/Conan for your specific requirements
add_requires("conan::fmt/12.0.0", { alias = "fmt" })
add_requires("conan::simdjson/4.2.4", { alias = "simdjson" })
add_requires("conan::grpc/1.78.1", { alias = "grpc" })
add_requires("conan::protobuf/6.33.5", { alias = "protobuf" })

-- Global Toolchain & Reproducibility Settings
set_languages("c++23", "c11")
set_warnings("all", "extra", "error")

-- Reproducibility: Equivalent to your -ffile-prefix-map and fixed epochs
if is_plat("linux", "macosx") then
	add_cxflags("-ffile-prefix-map=$(projectdir)=.", "-fmacro-prefix-map=$(projectdir)=.")
end

-- Define the Shared Library
target("congelado")
set_kind("shared")
add_files("src/main.cc")
add_includedirs("include", { public = true })
add_includedirs("src")
add_packages("fmt", "simdjson", "grpc", "protobuf")

-- Define the Static Library (Conditional)
if has_config("build_static") then
	target("congelado_static")
	set_kind("static")
	add_files("src/main.cc")
	add_includedirs("include", { public = true })
	add_includedirs("src")
	add_packages("fmt", "simdjson", "grpc", "protobuf")
end

-- The Primary Executable
target("congelado_exec")
set_kind("binary")
add_files("src/main.cc")
if has_config("build_static") then
	add_deps("congelado_static")
else
	add_deps("congelado")
end
add_packages("fmt", "simdjson", "grpc", "protobuf")

-- Fully Static Executable
if has_config("build_static") then
	target("congelado_static_exec")
	set_kind("binary")
	add_files("src/main.cc")
	add_deps("congelado_static")
	add_packages("fmt", "simdjson", "grpc", "protobuf")
	add_ldflags("-static", { force = true })
end
