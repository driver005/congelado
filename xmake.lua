set_project("congelado")
set_version("0.1.0")

add_rules("mode.debug", "mode.release")
set_defaultmode("debug")

if is_plat("linux") then
	set_toolchains("clang")
	add_ldflags("-fuse-ld=lld")
elseif is_plat("windows", "mingw") then
	set_config("sdk", "/opt/llvm-mingw")
	set_toolchains("clang")
end

--  TODO: use when the time is ready for now it is complining a lot!!!!
-- set_runtimes("c++_shared")
-- add_cxflags("-fexperimental-library")
-- add_ldflags("-lc++exp")

local posix_module_files = {
	"include/modules/socket.cppm",
	"include/modules/net.cppm",
	"include/modules/netdb.cppm",
	"include/modules/unistd.cppm",
	"include/modules/fcntl.cppm",
	"include/modules/errno.cppm",
	"include/modules/cstring.cppm",
	"include/transport/base/leverage/uring.cppm",
}

local conan = {
	settings_build = {
		"os=Linux",
		"compiler=clang",
		"compiler.version=20",
		"compiler.cppstd=gnu26",
		"compiler.libcxx=libc++",
	},
	options = {
		"openssl/*:enable_quic=True",
	},
	build = "missing",
}

if is_plat("windows", "mingw") then
	conan.settings = {
		"os=Windows",
		"compiler=clang",
		"compiler.version=20",
		"compiler.cppstd=gnu26",
		"compiler.libcxx=libc++",
	}

	conan.conf = {
		"tools.build:compiler_executables={'c':'/opt/llvm-mingw/bin/clang','cpp':'/opt/llvm-mingw/bin/clang++','rc':'/opt/llvm-mingw/bin/x86_64-w64-mingw32-windres'}",
		"tools.build:cflags=['--target=x86_64-w64-mingw32']",
		"tools.build:cxxflags=['--target=x86_64-w64-mingw32']",
		"tools.build:exelinkflags=['--target=x86_64-w64-mingw32','-fuse-ld=lld']",
		"tools.cmake.cmaketoolchain:system_name=Windows",
		"tools.cmake.cmaketoolchain:generator=Unix Makefiles",
	}
else
	conan.settings = {
		"os=Linux",
		"compiler=clang",
		"compiler.version=20",
		"compiler.cppstd=gnu26",
		"compiler.libcxx=libc++",
	}

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
-- add_requires("conan::grpc/1.78.1", { alias = "grpc", configs = conan })
add_requires("conan::catch2/3.7.1", { alias = "catch2", configs = conan })
add_requires("conan::backward-cpp/1.6", { alias = "backward", configs = conan })
add_requires("conan::libffi/3.4.4", { alias = "libffi", configs = conan })
add_requires("conan::tomlplusplus/3.4.0", { alias = "tomlplusplus", configs = conan })
add_requires("conan::stduuid/1.2.3", { alias = "stduuid", configs = conan })
add_requires("microsoft-gsl", { configs = conan })
add_requires("range-v3", { configs = conan })

set_languages("c++26", "c11")
set_warnings("all", "extra", "error")

set_policy("build.c++.modules", true)

if is_plat("linux", "macosx") then
	-- Ensures debug paths are relative for reproducibility
	add_cxflags("-ffile-prefix-map=$(projectdir)=.", "-fmacro-prefix-map=$(projectdir)=.")
elseif is_plat("mingw") then
	add_cxflags("-Wno-unknown-pragmas")

	after_build(function(target)
		local conan_root = path.join(os.getenv("HOME"), ".conan2", "p", "b")
		for _, dir in ipairs(os.dirs(path.join(conan_root, "proto*", "p", "lib"))) do
			for _, lib in ipairs({ "utf8_validity", "utf8_range" }) do
				local src = path.join(dir, "lib" .. lib .. ".a")
				local dst = path.join(dir, "liblib" .. lib .. ".a")
				if os.isfile(src) and not os.isfile(dst) then
					os.ln(src, dst)
				end
			end
		end
	end)
end

if is_arch("x86_64") then
	add_cxflags("-mbmi2")
end

target("congelado_lib")
set_policy("build.sanitizer.address", true)
set_kind("static")
add_cxflags("-fPIC")

if is_plat("windows", "mingw") then
	add_syslinks("ws2_32", "mswsock", "stdc++", "gcc_s")
	add_defines("_WIN32_WINNT=0x0A00")
	add_cxflags("--target=x86_64-w64-mingw32")
	add_ldflags("--target=x86_64-w64-mingw32", "-lstdc++exp")
else
	add_links("uring", "pthread", "dl", "ssl", "crypto")
end

add_defines("CLANG_ITERATE_MODULES")

add_files("include/**.cppm", { public = true })
add_files("src/**.cc")

remove_files("src/main.cc")

if is_plat("windows", "mingw") then
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
	"tomlplusplus",
	"grpc",
	"protobuf",
	"asio",
	"openssl",
	"nghttp2",
	"ngtcp2",
	"nghttp3",
	"backward",
	"libffi",
	"microsoft-gsl",
	"range-v3",
	"stduuid",
	{ public = true }
)

-- First-party protocol plugins: compiled with module support, linked against congelado_lib.
-- These can import any module from the main library and expose congelado_get_protocol().
local first_party_plugins = {["http2"] = true}

-- http2: first-party protocol plugin — needs module access to import io_layer_http2 etc.
target("http2")
set_kind("shared")
set_languages("c++26")
set_policy("build.c++.modules", true)
add_files("plugins/http2/http2.cc")
add_includedirs("include")
add_deps("congelado_lib")
if is_plat("linux", "macosx") then
	add_cxflags("-ffile-prefix-map=$(projectdir)=.", "-fmacro-prefix-map=$(projectdir)=.")
end
if is_plat("windows", "mingw") then
	add_cxflags("--target=x86_64-w64-mingw32")
end
target_end()

-- Third-party plugin targets — standalone shared libraries.
-- Each exposes congelado_get_plugin/congelado_destroy_plugin (see include/core/ffi/plugin_api.h).
-- C++ plugins inherit congelado::PluginBase and use CONGELADO_PLUGIN(T) (plugin_api.hpp).
-- NOT linked into the main binary; loaded at runtime via dlopen.
for _, pluginfile in ipairs(os.files("plugins/**/*.cc")) do
	local pluginname = path.basename(pluginfile)
	if first_party_plugins[pluginname] then goto continue end
	target(pluginname)
	set_kind("shared")
	set_languages("c++26")
	add_files(pluginfile)
	add_includedirs("include")
	add_cxflags("-fpermissive")
	if is_plat("linux", "macosx") then
		add_cxflags("-ffile-prefix-map=$(projectdir)=.", "-fmacro-prefix-map=$(projectdir)=.")
	end
	if is_plat("windows", "mingw") then
		add_cxflags("--target=x86_64-w64-mingw32")
	end
	target_end()
	::continue::
end

target("congelado")
set_kind("binary")
add_files("src/main.cc")
add_deps("congelado_lib")
add_packages("fmt", "simdjson")
target_end()

target("model_test")
set_kind("binary")
set_languages("c++26")
set_policy("build.c++.modules", true)
add_files("tests/model/model_test.cc")
add_deps("congelado_lib")
add_packages("catch2")
add_cxflags("-fpermissive")
if is_plat("linux", "macosx") then
	add_cxflags("-ffile-prefix-map=$(projectdir)=.", "-fmacro-prefix-map=$(projectdir)=.")
end
if is_plat("windows", "mingw") then
	add_cxflags("--target=x86_64-w64-mingw32")
end
add_tests("default")
target_end()

-- for _, benchfile in ipairs(os.files("benchmarks/**.cc")) do
-- 	add_tests(path.basename(benchfile), {
-- 		files = benchfile,
-- 		packages = "catch2",
-- 	})
-- end

-- 	for _, testfile in ipairs(os.files("tests/**.cc")) do
-- 		local testname = path.basename(testfile)
-- 		target(testname)
-- 		set_kind("binary")
-- 		add_files(testfile)
-- 		add_packages("catch2")
-- 		add_deps("congelado_lib")
-- 		add_cxflags("-fpermissive")
-- 		add_tests("default")
-- 		target_end()
-- 	end
