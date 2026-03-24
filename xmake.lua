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
	},
	settings_build = {
		"os=Linux",
		"compiler=clang",
		"compiler.version=20",
		"compiler.cppstd=gnu23",
	},
	options = {
		"openssl/*:enable_quic=True",
	},
	conf = { "tools.build:compiler_executables={'c':'clang','cpp':'clang++'}" },
	build = "missing",
}

if is_plat("windows") then
	table.insert(conan.settings, "os=Windows")
	table.insert(conan.settings, "arch=x86_64")
	table.insert(conan.settings, "compiler.runtime=mingw")
	conan.conf = {
		"tools.build:cflags=['--target=x86_64-w64-mingw32']",
		"tools.build:cxxflags=['--target=x86_64-w64-mingw32']",
		"tools.build:exelinkflags=['--target=x86_64-w64-mingw32','-fuse-ld=lld']",
		"tools.build:compiler_executables={'c':'clang','cpp':'clang++'}",
	}
else
	table.insert(conan.settings, "os=Linux")
	conan.conf = {
		"tools.build:compiler_executables={'c':'clang','cpp':'clang++'}",
	}
end

-- add_requires("conan::fmt/12.0.0", { alias = "fmt", configs = conan })
add_requires("conan::asio/1.36.0", { alias = "asio", configs = conan })
add_requires("conan::openssl/3.6.1", { alias = "openssl", configs = conan })
add_requires("conan::libnghttp2/1.66.0", { alias = "nghttp2", configs = conan })
add_requires("conan::nghttp3/1.12.0", { alias = "nghttp3", configs = conan })
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

local posix_module_files = {
	"include/module/socket.cppm",
	"include/module/netinet.cppm",
	"include/module/arpa.cppm",
	"include/module/netdb.cppm",
	"include/module/unistd.cppm",
	"include/module/fcntl.cppm",
	"include/module/errno.cppm",
	"include/module/cstring.cppm",
}

target("congelado_lib")
set_kind("static")
set_policy("build.c++.modules.std", true)
-- add_cxflags("-fpermissive", "-pthread", "-ldl")

-- Explicitly tell Xmake .cmpp files are C++ module interfaces
add_files("include/**.cppm", { public = true })
add_files("src/**.cc")

remove_files("src/main.cc")

if is_plat("windows") then
	remove_files("include/**/posix.cppm")
	for _, f in ipairs(posix_module_files) do
		remove_files(f)
	end
else
	remove_files("include/**/win32.cppm")
	remove_files("include/modules/winsock2.cppm")
end
add_includedirs("include", { public = true })
add_packages(
	"fmt",
	"simdjson",
	"grpc",
	"protobuf",
	"asio",
	"openssl",
	"nghttp2",
	"ngtcp2",
	"nghttp3",
	{ public = true }
)

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

for _, testfile in ipairs(os.files("tests/**.cc")) do
	local testname = path.basename(testfile)
	target(testname)
	set_kind("binary")
	set_policy("build.c++.modules.std", true)
	add_files(testfile)
	add_packages("catch2")
	add_deps("congelado_lib")
	-- Templates are instantiated in the test binary, not the lib.
	-- The lib provides module interfaces; the binary emits the symbols.
	add_cxflags("-fpermissive")
	add_tests("default")
	target_end()
end
