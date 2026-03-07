set_project("congelado")
set_version("0.1.0")

-- 1. Global Scoped Settings
add_rules("mode.debug", "mode.release")
set_defaultmode("debug")

set_toolchains("clang")

local conan = {
	settings = {
		"compiler=clang",
		"compiler.version=20",
		"compiler.cppstd=gnu23",
		-- "build_type=Debug",
	},
	settings_build = {
		"compiler=clang",
		"compiler.version=20",
		"compiler.cppstd=gnu23",
		-- "build_type=Debug",
	},
	conf = { "tools.build:compiler_executables={'c':'clang','cpp':'clang++'}" },
	conf_build = { "tools.build:compiler_executables={'c':'clang','cpp':'clang++'}" },
	-- env = { "CC=clang", "CXX=clang++" },
	-- env_build = { "CC=clang", "CXX=clang++" },
	build = "missing",
}

-- add_requires("conan::fmt/12.0.0", { alias = "fmt", configs = conan })
add_requires("conan::simdjson/4.2.4", { alias = "simdjson", configs = conan })
add_requires("conan::protobuf/6.33.5", { alias = "protobuf", configs = conan })
add_requires("conan::grpc/1.78.1", { alias = "grpc", configs = conan })
add_requires("conan::catch2/3.7.1", { alias = "catch2", configs = conan })

-- 3. Global Compilation Settings
set_languages("c++23", "c11")
set_warnings("all", "extra", "error")

if is_plat("linux", "macosx") then
	-- Ensures debug paths are relative for reproducibility
	add_cxflags("-ffile-prefix-map=$(projectdir)=.", "-fmacro-prefix-map=$(projectdir)=.")
end

-- Use the policies exactly as named in your 'xmake l' output
set_policy("build.c++.modules", true)
set_policy("build.c++.modules.std", true) -- Explicitly ensure this is on
set_policy("generator.compile_commands", true)

target("congelado_lib")
set_kind("static")
set_policy("build.c++.modules.std", true)
add_cxflags("-fpermissive")

-- Explicitly tell Xmake .cmpp files are C++ module interfaces
add_files("include/**.cppm", { public = true })
add_files("src/**.cc")

remove_files("src/main.cc")
add_includedirs("include", { public = true })
add_packages("fmt", "simdjson", "grpc", "protobuf")

-- for _, benchfile in ipairs(os.files("benchmarks/**.cc")) do
-- 	add_tests(path.basename(benchfile), {
-- 		files = benchfile,
-- 		packages = "catch2",
-- 	})
-- end

-- 5. Main Application
target("congelado")
set_kind("binary")
add_files("src/main.cc")
add_deps("congelado_lib")
add_packages("fmt", "simdjson")
target_end()

-- for _, testfile in ipairs(os.files("tests/**.cc")) do
-- 	local testname = path.basename(testfile)
-- 	target(testname)
-- 	set_kind("binary")
-- 	add_files(testfile)
-- 	add_packages("catch2")
-- 	add_deps("congelado_lib")
-- 	add_tests("default")
-- end
