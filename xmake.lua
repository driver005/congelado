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
	"llvm",
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
-- boost::ut (boost-ext/ut) — imported as a real C++20 module (`import boost.ut;`), not the
-- `#include <boost/ut.hpp>` header form. {modules = true} just stops the package defining
-- BOOST_UT_DISABLE_MODULE; the package itself never add_files()'s its own ut.cppm for
-- consumers (headeronly kind), so boost_ut_module below vendors that one file in ourselves.
add_requires("boost_ut", { configs = { modules = true } })
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

-- libsolv: real, self-built C library (openSUSE's SAT-based dependency resolver) — not on
-- ConanCenter (checked: no recipe), so built from source via CMake inside on_install, same
-- shallow-git-clone approach used elsewhere in this file. All of libsolv's distro
-- package-format backends (RPM/DEB/Conda/Arch/APK/AppStream/etc, all default OFF upstream) are
-- left disabled — only the general-purpose SAT solver (pool/repo/queue/solver/transaction) is
-- needed, matching how plugify's own libsolv_dependency_resolver.* uses it. zlib is libsolv's
-- one unconditional dependency, pulled via xmake's own bundled "zlib" package (not conan — no
-- add_requires needed for it, add_deps below resolves it automatically).
package("libsolv")
	set_kind("library")
	set_homepage("https://github.com/openSUSE/libsolv")
	set_description("Self-built libsolv (SAT dependency resolver), core solver only")
	add_versions("main", "")
	add_deps("zlib")

	on_install(function(package)
		local clone_dir = path.join(os.tmpdir(), "libsolv_checkout_" .. os.time())
		os.tryrm(clone_dir)
		os.vrunv("git", {
			"clone",
			"--depth", "1",
			"https://github.com/openSUSE/libsolv.git",
			clone_dir,
		})
		os.cd(clone_dir)
		import("package.tools.cmake").install(package, {
			"-DENABLE_STATIC=ON",
			"-DDISABLE_SHARED=ON",
			"-DCMAKE_POSITION_INDEPENDENT_CODE=ON",
		})
	end)

	on_test(function(package)
		assert(package:has_cxxfuncs("pool_create", { includes = "solv/pool.h" }))
	end)
package_end()

add_requires("libsolv")
add_requires("conan::glaze/8.0.0", { alias = "glaze", configs = conan })
add_requires("conan::valijson/1.1.0", { alias = "valijson", configs = conan })

-- plugify_plg: a self-fetched copy of plugify's own include/plg/ (untrustedmodders/plugify) —
-- not committed to this repo. Fetched via a shallow, sparse `git clone` (only include/plg —
-- not the rest of that repo) inside on_install, resolved like any other xmake dependency.
-- Pulled in whole, as-is: every file already lives under namespace plg (not std), so there's
-- no ODR-collision concern the way vendoring libc++'s own std::string/vector directly would
-- have had, and every file is a plain header — no out-of-line .cpp translation units, so no
-- linkage step at all (unlike llvm::SmallString/SmallVector's ADT, which needed a separately-
-- linked Support library closure; that path and a raw libc++ vendor were both tried and
-- dropped for this reason). No CMake-generated headers involved either — plugify's own headers
-- are hand-written, not templated by a build system, so nothing here needs a stand-in file.
package("plugify_plg")
	set_kind("library", { headeronly = true })
	set_homepage("https://github.com/untrustedmodders/plugify")
	set_description("Self-fetched copy of plugify's include/plg/ (plg::string, plg::vector, plg::any, ...)")
	add_versions("main", "")

	on_install(function(package)
		local clone_dir = path.join(os.tmpdir(), "plugify_plg_checkout_" .. os.time())
		os.tryrm(clone_dir)
		os.vrunv("git", {
			"clone",
			"--depth", "1",
			"--filter=blob:none",
			"--sparse",
			"https://github.com/untrustedmodders/plugify.git",
			clone_dir,
		})
		os.vrunv("git", { "-C", clone_dir, "sparse-checkout", "set", "include/plg" })

		local dest = path.join(package:installdir("include"), "plg")
		os.mkdir(dest)
		os.cp(path.join(clone_dir, "include", "plg", "*"), dest)
		os.tryrm(clone_dir)
	end)

	on_test(function(package)
		assert(package:has_cxxincludes("plg/string.hpp", { configs = { languages = "c++20" } }))
	end)
package_end()

add_requires("plugify_plg")

-- Resolves the plugify_plg package's installdir and adds it as a public includedir to
-- whichever target needs `#include <plg/string.hpp>` etc. Same on_load-resolution technique
-- as boost_ut_module above.
function apply_plugify_plg_includedir(target)
	local pkg = target:pkg("plugify_plg")
	target:add("includedirs", pkg:installdir("include"), { public = true })
end

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

-- boost_ut's package (headeronly kind) never exposes its ut.cppm as a buildable module unit
-- to consumers, even with {modules = true} set above — that config only stops the header
-- itself from disabling `export module boost.ut;`. So this vendors that one file directly:
-- a moduleonly target whose sole source is the installed package's ut.cppm, resolved at
-- on_load() from the package's own installdir. Every CONGELADO_TEST sibling target below
-- (and every plugin's, per apply_test_target in xmake/common.lua) add_deps() this instead of
-- re-deriving the path itself.
target("boost_ut_module")
set_kind("moduleonly")
set_languages("c++26")
add_packages("boost_ut")
on_load(function(target)
	local pkg = target:pkg("boost_ut")
	target:add("files", path.join(pkg:installdir(), "include", "boost", "ut.cppm"))
end)
target_end()

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
	core_packages = table.join(core_packages, { "plugify_plg" }),
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
on_load(apply_plugify_plg_includedir)
target_end()

-- congelado_include_test: recompiles include/**.cppm with CONGELADO_TEST defined, so every
-- file's inline `#ifdef CONGELADO_TEST` boost::ut suite gets built and run. See
-- apply_test_target in xmake/common.lua.
apply_test_target({
	name = "congelado_include",
	layer = "include",
	core_packages = table.join(core_packages, { "plugify_plg" }),
	includedirs = { path.join(core_root, "include") },
	files = { "include/**.cppm" },
	remove = is_plat("windows", "mingw") and table.join({ "include/**/posix.cppm" }, posix_module_files)
		or { "include/**/win32.cppm", "include/modules/winsock2.cppm" },
	on_load = apply_plugify_plg_includedir,
})

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

-- congelado_sdk_test: recompiles sdk/**.cppm with CONGELADO_TEST defined. See
-- congelado_include_test above / apply_test_target in xmake/common.lua.
apply_test_target({
	name = "congelado_sdk",
	layer = "sdk",
	core_packages = core_packages,
	deps = { "congelado_include" },
	includedirs = { "sdk/plugin/include" },
	files = { "sdk/**.cppm" },
})

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
add_deps("shared_model")
add_packages("cli11")
add_rpathdirs("$ORIGIN")
target_end()

-- The worker executable: an SDK-level binary (sources under sdk/worker/) that loads the defined
-- plugins + task-worker .so's, polls the engine over the OpenAPI-generated congelado_api client, and
-- dispatches each claimed task through interfaces::IWorker on a contract thread pool. Same deps the
-- old engine_worker target used (engine_worker_lib for the worker module, engine_api for the
-- generated client) — those targets live in plugins/engine and resolve across the global graph.
target("worker")
set_kind("binary")
set_policy("build.sanitizer.address", true)
add_files("sdk/worker/worker_main.cc")
add_deps("engine_worker_lib", "engine_api")
add_packages("backward")
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
