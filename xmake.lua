set_project("congelado")
set_version("0.1.0")

add_rules("mode.debug", "mode.release")
set_defaultmode("debug")

-- Global (no `local`): xmake/common.lua's apply_common_layer_settings() reads this to build
-- absolute include paths, since a bare relative "include" would resolve against whichever
-- project is calling it — wrong for plugins/xmake.lua, a separate project one directory down.
-- This (the actual repo root) is the root project's own os.projectdir().
core_root = os.projectdir()
-- import()'able custom modules (xmake/modules/build_tool.lua) for use inside before_build()/
-- after_build() script blocks — description-scope globals (xmake/common.lua, xmake/build.lua)
-- aren't visible there, only import() reaches into that sandboxed scope.
add_moduledirs("xmake/modules")
core_packages = {
	"simdjson",
	"tomlplusplus",
	"reflectcpp",
	"protobuf",
	"asio",
	"openssl",
	"nghttp2",
	"nghttp3",
	"backward",
	"libffi",
	"microsoft-gsl",
	"range-v3",
	"stduuid",
	"sqlgen",
	"cpython",
	"lua",
}

-- core_packages / apply_common_layer_settings() (shared by the three layered shared libs below,
-- and by plugins/xmake.lua's own separate project) live in xmake/common.lua; build_conan_settings()
-- lives in xmake/conan.lua — both included early since build_conan_settings() is needed below,
-- before any add_requires() call.
includes("xmake/conan.lua")
includes("xmake/common.lua")

if is_plat("linux") then
	set_toolchains("clang")
	add_ldflags("-fuse-ld=lld")
	-- liburing < 2.15 unconditionally defines IOURINGINLINE as "static inline", which a
	-- C++20 module partition can't export (internal linkage). Newer liburing guards this
	-- with #ifndef and picks plain "inline" under C++20+; predefine it ourselves so older
	-- distro packages (e.g. Ubuntu 25.10's liburing-dev 2.11) behave the same way.
	add_defines("IOURINGINLINE=inline")
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

local conan = build_conan_settings()

-- add_requires("conan::fmt/12.0.0", { alias = "fmt", configs = conan })
add_requires("conan::asio/1.36.0", { alias = "asio", configs = conan })
add_requires("conan::openssl/3.6.1", { alias = "openssl", configs = conan })
add_requires("conan::libnghttp2/1.66.0", { alias = "nghttp2", configs = conan })
add_requires("conan::nghttp3/1.12.0", { alias = "nghttp3", configs = conan })
add_requires("conan::simdjson/4.2.4", { alias = "simdjson", configs = conan })
add_requires("conan::protobuf/6.33.5", { alias = "protobuf", configs = conan })
-- add_requires("conan::grpc/1.78.1", { alias = "grpc", configs = conan })
add_requires("conan::catch2/3.7.1", { alias = "catch2", configs = conan })
add_requires("conan::cli11/2.4.2", { alias = "cli11", configs = conan })
add_requires("conan::backward-cpp/1.6", { alias = "backward", configs = conan })
add_requires("conan::libffi/3.4.4", { alias = "libffi", configs = conan })
add_requires("conan::tomlplusplus/3.4.0", { alias = "tomlplusplus", configs = conan })
add_requires("conan::stduuid/1.2.3", { alias = "stduuid", configs = conan })
add_requires("conan::cpython/3.12.7", { alias = "cpython", configs = conan })
add_requires("conan::lua/5.4.7", { alias = "lua", configs = conan })
add_requires("conan::reflect-cpp/0.23.0", {
	alias = "reflectcpp",
	configs = {
		settings_build = conan.settings_build,
		settings = conan.settings,
		conf = conan.conf,
		build = "missing",
		-- Real option names, per reflect-cpp's own conanfile.py — NOT "toml"/"msgpack"/"xml"/
		-- "json" (those don't exist on this recipe; conan silently accepts-and-ignores unknown
		-- options rather than erroring, so the wrong names here previously built reflect-cpp
		-- with every optional format defaulted off, which is how toml_plugin ended up linking
		-- against a reflect-cpp build missing rfl::toml::Writer entirely). There's no
		-- with_json option at all — JSON (via yyjson) is unconditional in every reflect-cpp
		-- build. with_msgpack/with_xml deliberately left off here (default False, matching
		-- their previous no-op state) — nothing in this codebase consumes either format yet,
		-- and enabling with_msgpack pulls in msgpack-c/6.0.0, whose own CMakeLists.txt fails
		-- to configure against this toolchain's CMake ("Compatibility with CMake < 3.5 has
		-- been removed") — an unrelated packaging bug, not something to drag in for an unused
		-- format. Add with_msgpack/with_xml back here if/when something actually needs them.
		options = {
			"reflect-cpp/*:with_toml=True",
		},
	},
})
add_requires("conan::sqlgen/0.4.0", { alias = "sqlgen", configs = conan })
add_requires("microsoft-gsl", { configs = conan })
add_requires("range-v3", { configs = conan })

set_languages("c++26", "c11")
-- TODO: please add again
-- set_warnings("all", "extra", "error")

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

-- congelado_include: include/** only (interfaces, io layer, utils, model, shared) — the
-- bottommost layer. Nothing here imports anything from sdk/ or plugins/.
target("congelado_include")
set_kind("shared")
-- Cross-target C++ module BMI tracking doesn't reliably gate a dependent's start on this
-- target's completion under "xmake build"'s default cross-target parallelism — congelado_sdk
-- can start compiling before congelado_include's BMIs are actually done, failing with
-- "module file not found" (see docker/Dockerfile.server's build-one-target-at-a-time
-- workaround for the same root cause). build.fence forces every dependent target to wait.
apply_common_layer_settings({
	core_packages = core_packages,
	layer = "include",
	fence = true,
})
add_includedirs(path.join(core_root, "include"), { public = true })

add_files("include/**.cppm", { public = true })

if is_plat("windows", "mingw") then
	remove_files("include/**/posix.cppm")
	for _, f in ipairs(posix_module_files) do
		remove_files(f)
	end
else
	remove_files("include/**/win32.cppm")
	remove_files("include/modules/winsock2.cppm")
end
target_end()

-- congelado_sdk: sdk/** (plugin ABI, client SDK codegen, heart, worker) — built on top of
-- congelado_include rather than merged with it; every sdk/** file imports include/**'s
-- modules (interfaces, core_*, io_*, utils_*, model, ...), never the other way round.
target("congelado_sdk")
set_kind("shared")
add_deps("congelado_include")
-- Same cross-target BMI race as congelado_include above — fence this one too so
-- dependents can't start before congelado_sdk's modules are actually built.
apply_common_layer_settings({
	layer = "sdk",
	plugin_includedir = true,
	core_packages = core_packages,
	fence = true,
})
-- add_includedirs(path.join(core_root, "include"), { public = true })
-- add_includedirs(path.join(core_root, "sdk/plugin/include"), { public = true })

add_files("sdk/**.cppm", { public = true })
add_includedirs("sdk/plugin/include", { public = true })
target_end()

-- No congelado_lib layer: that was a pre-plugin-split leftover from the a5e3e23-era layout
-- (plugins/**.cppm + src/**.cc merged into one shared lib). Once every plugin became its own
-- runtime-loadable target (plugins/*/xmake.lua), the only thing actually left in "plugins/**.cppm"
-- was engine's own model/worker/generated-client module partitions, and src/**.cc was always
-- just the three binaries below — so each depends on congelado_sdk directly instead.
target("congelado")
set_kind("binary")
set_policy("build.sanitizer.address", true)
add_files("src/main.cc")
add_deps("congelado_sdk")
add_packages("simdjson")
add_rpathdirs("$ORIGIN")
target_end()

target("congelado_cli")
set_kind("binary")
set_policy("build.sanitizer.address", true)
add_files("src/cli_main.cc")
add_deps("congelado_sdk")
add_deps("engine_model")
add_packages("cli11")
add_rpathdirs("$ORIGIN")
target_end()

includes("plugins")

-- target("engine_worker_test")
--     set_kind("binary")
--     set_languages("c++26")
--     set_policy("build.c++.modules", true)
--     add_cxflags("-fPIC")
--     add_defines("CLANG_ITERATE_MODULES", "CONGELADO_MODE_HOST", "CONGELADO_MODE_GUEST", "CONGELADO_IMPL")
--     add_files("tests/core/engine/engine_worker_test.cc")
--     add_deps("congelado_lib")
--     add_packages("catch2")
--     if is_plat("linux", "macosx") then
--         add_cxflags("-ffile-prefix-map=$(projectdir)=.", "-fmacro-prefix-map=$(projectdir)=.")
--         add_syslinks("uuid")
--     end
--     add_tests("default")
-- target_end()

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
