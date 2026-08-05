-- Shared setup for congelado_include/congelado_sdk (the two bottom layers every project —
-- the root project and every standalone plugin project alike — needs, per the "consume core
-- as source" design: xmake has no mechanism to propagate C++20 module interfaces through
-- add_packages()/a package system (confirmed empirically — see the plan doc for this session's
-- repro), so every project that needs congelado_include/congelado_sdk *declares* its own copy of
-- them rather than depending on a prebuilt package. They don't actually get recompiled N times
-- in practice, though: setup_core_layers() below also points every project's builddir at the
-- same shared repo-root build/ directory, so xmake's own incremental-rebuild cache (keyed on
-- source hash + compiler flags, not on which project/process built it) lets every project after
-- the first one just reuse the already-built object/BMI files directly.
--
-- Shared conan settings table (platform-dependent), used both by setup_core_layers() below and
-- by setup_extra_requires() — factored out so a standalone plugin project that only needs one
-- or two of the "extra" packages (opentelemetrycpp/catch2/cli11/cpython/lua — none of which
-- congelado_include/congelado_sdk themselves link, see core_packages below) doesn't have to
-- duplicate this table by hand.
function build_conan_settings()
	local conan = {
		settings_build = {
			"os=Linux",
			"compiler=clang",
			"compiler.version=20",
			"compiler.cppstd=gnu26",
			"compiler.libcxx=libstdc++11",
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
			"compiler.libcxx=libstdc++11",
		}

		conan.conf = {
			"tools.build:compiler_executables={'c':'clang','cpp':'clang++'}",
		}
	end
	return conan
end

-- "Extra" packages: not part of core_packages (congelado_include/congelado_sdk never link
-- these), each used by only one or two specific targets (otel_otlp_plugin, congelado_cli, the
-- python/lua bridge plugins, tests). add_requires() forces xmake to configure/install a package
-- for the WHOLE project regardless of whether any target actually uses it yet, and configuring
-- all five of these in every standalone plugin project is real, avoidable cost (verified: it's
-- what made a fresh plugins/serde/json project's `xmake f` attempt a full opentelemetry-cpp
-- from-source rebuild, which then failed, for a plugin that never uses OTel at all) — so each
-- caller passes only the names it actually needs. The root project needs all of them (it still
-- carries every plugin's various add_packages() calls); a standalone plugin project passes just
-- its own subset (e.g. {"opentelemetrycpp"} for otel_otlp_plugin, {} for most others).
function setup_extra_requires(names)
	local conan = build_conan_settings()
	local wanted = {}
	for _, name in ipairs(names or {}) do
		wanted[name] = true
	end

	if wanted.opentelemetrycpp then
		add_requires("conan::opentelemetry-cpp/1.26.0", {
			alias = "opentelemetrycpp",
			configs = {
				settings_build = conan.settings_build,
				settings = conan.settings,
				conf = conan.conf,
				build = "missing",
				options = {
					"opentelemetry-cpp/*:with_otlp_grpc=False",
					"opentelemetry-cpp/*:with_otlp_http=True",
				},
			},
		})
	end
	if wanted.catch2 then
		add_requires("conan::catch2/3.7.1", { alias = "catch2", configs = conan })
	end
	if wanted.cli11 then
		add_requires("conan::cli11/2.4.2", { alias = "cli11", configs = conan })
	end
	if wanted.cpython then
		add_requires("conan::cpython/3.12.7", { alias = "cpython", configs = conan })
	end
	if wanted.lua then
		add_requires("conan::lua/5.4.7", { alias = "lua", configs = conan })
	end
	if wanted.libcurl then
		add_requires("conan::libcurl/8.15.0", { alias = "libcurl", configs = conan })
	end
	if wanted.rabbitmqc then
		add_requires("conan::rabbitmq-c/0.15.0", { alias = "rabbitmqc", configs = conan })
	end
	if wanted.librdkafka then
		add_requires("conan::librdkafka/2.14.2", { alias = "librdkafka", configs = conan })
	end
	if wanted.hiredis then
		add_requires("conan::hiredis/1.3.0", { alias = "hiredis", configs = conan })
	end
end

-- core_root: absolute path to the repo root (where include/, sdk/, conan settings, etc. live) —
-- the root xmake.lua passes its own os.projectdir(); a standalone plugin project's xmake.lua
-- (e.g. plugins/engine/xmake.lua) passes path.join(os.projectdir(), "..", "..").
function setup_core_layers(core_root)
	-- Point this project's build output at the repo-root build/ dir every other project (root
	-- and every standalone plugin project) also points at — xmake's own incremental-rebuild
	-- staleness check (source hash + recorded compiler flags, stored alongside each object/BMI
	-- file) is agnostic to which xmake process wrote the cache, so once one project has built
	-- congelado_include/congelado_sdk, every other project sharing this same builddir sees them
	-- as already up to date and skips recompiling — confirmed via an isolated two-project repro
	-- this session (second project's build skipped scanning+compiling entirely, reusing the
	-- first project's object/BMI files directly, ~5x faster). This sidesteps xmake's confirmed
	-- inability to propagate C++20 module interfaces through its own package system
	-- (add_requires()/add_packages() — see this file's own top comment) — it's cache sharing at
	-- the build-output level, not package export. No-op for the root project itself (core_root
	-- already equals its own default builddir).
	set_config("builddir", path.join(core_root, "build"))

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

	-- Only what congelado_include/congelado_sdk themselves actually link (matches
	-- core_packages below) — the "extra" packages (opentelemetrycpp/catch2/cli11/cpython/lua)
	-- are setup_extra_requires()'s job, called separately by whichever project needs them.
	add_requires("conan::asio/1.36.0", { alias = "asio", configs = conan })
	add_requires("conan::openssl/3.6.1", { alias = "openssl", configs = conan })
	add_requires("conan::libnghttp2/1.66.0", { alias = "nghttp2", configs = conan })
	add_requires("conan::nghttp3/1.12.0", { alias = "nghttp3", configs = conan })
	add_requires("conan::simdjson/4.2.4", { alias = "simdjson", configs = conan })
	add_requires("conan::protobuf/6.33.5", { alias = "protobuf", configs = conan })
	add_requires("conan::backward-cpp/1.6", { alias = "backward", configs = conan })
	add_requires("conan::libffi/3.4.4", { alias = "libffi", configs = conan })
	add_requires("conan::tomlplusplus/3.4.0", { alias = "tomlplusplus", configs = conan })
	add_requires("conan::stduuid/1.2.3", { alias = "stduuid", configs = conan })
	add_requires("conan::reflect-cpp/0.23.0", {
		alias = "reflectcpp",
		configs = {
			settings_build = conan.settings_build,
			settings = conan.settings,
			conf = conan.conf,
			build = "missing",
			-- Option names must match the reflect-cpp recipe's actual attribute names (with_*
			-- prefix) — only with_toml is set, the one format this project's own code needs
			-- (rfl::toml::write, used by plugins/serde/toml).
			options = {
				"reflect-cpp/*:with_toml=True",
			},
		},
	})
	add_requires("conan::sqlgen/0.4.0", { alias = "sqlgen", configs = conan })
	add_requires("microsoft-gsl", { configs = conan })
	add_requires("range-v3", { configs = conan })

	set_languages("c++26", "c11")
	set_policy("build.c++.modules", true)

	if is_plat("linux", "macosx") then
		-- core_root, not $(projectdir): this add_cxflags is project-wide, so it also reaches
		-- congelado_include/congelado_sdk (declared further down in this same function) — and
		-- $(projectdir) differs per plugin project (each plugin's own directory), which put a
		-- project-specific string into congelado_include/sdk's own compile command line and
		-- silently defeated the whole point of sharing one builddir across projects (see this
		-- function's own builddir comment above): xmake's staleness check treats a different
		-- recorded command line as cache-invalid and recompiles from scratch every time, even
		-- though the source is byte-identical. core_root is the same absolute path in every
		-- project (repo root), so this flag's value — and therefore congelado_include/sdk's
		-- entire command line — now matches exactly everywhere, and the shared build cache
		-- actually gets reused.
		add_cxflags("-ffile-prefix-map=" .. core_root .. "=.", "-fmacro-prefix-map=" .. core_root .. "=.")
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

	-- Global (not local): congelado_lib, and every standalone plugin project's own target(s),
	-- need to call this too, from outside setup_core_layers()'s own scope.
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
	}

	function apply_common_layer_settings(opts)
		opts = opts or {}
		add_cxflags("-fPIC")

		if is_plat("windows", "mingw") then
			add_syslinks("ws2_32", "mswsock", "stdc++", "gcc_s")
			add_defines("_WIN32_WINNT=0x0A00")
			add_cxflags("--target=x86_64-w64-mingw32")
			add_ldflags("--target=x86_64-w64-mingw32", "-lstdc++exp")
		else
			-- stduuid (conan::stduuid, one of core_packages below) doesn't auto-link its own
			-- runtime backend on Linux (libuuid, providing uuid_generate) — same gap as
			-- gssapi_krb5/python3.12 elsewhere in this codebase's plugin projects. congelado_include
			-- itself uses it directly (serde/core.cppm, serde/converter.cppm, connector/connector.cppm),
			-- so every project needs this, not just specific plugins.
			add_links("c++", "uring", "pthread", "dl", "ssl", "crypto", "uuid")
		end

		add_defines("CLANG_ITERATE_MODULES")
		-- Tried passing this as a relative path ("../../include", identical text across every
		-- plugin project) to dodge xmake re-anchoring an absolute includedir onto *this* target's
		-- own os.projectdir() in the recorded compile command (see core_root's own comment above)
		-- — reverted: it made json_plugin's own target re-trigger a full second compile of every
		-- congelado_include/sdk module partition under its own name (177 extra compiles, ~50s
		-- slower), since a relative -I's meaning depends on the invoking cwd, which xmake doesn't
		-- guarantee is identical for every compile step within one target. Absolute is correct,
		-- even though it means this specific flag still isn't reused cache-wise across projects.
		add_includedirs(path.join(core_root, "include"), { public = true })
		if opts.plugin_includedir then
			add_includedirs(path.join(core_root, "sdk/plugin/include"), { public = true })
		end
		add_packages(table.unpack(table.join(core_packages, { { public = true } })))

		-- In any project that also requires "cpython" (currently just plugins/bridge/python_bridge),
		-- cpython's transitive dependency on util-linux installs its own uuid.h (plain-C, no
		-- `namespace uuids`) at a path that -isystem resolves ahead of stduuid's own uuid.h —
		-- congelado_include's own `#include <uuid.h>` (serde/core.cppm etc.) then picks up the
		-- wrong one and fails with "use of undeclared identifier 'uuids'". Adding stduuid's real
		-- include dir a second time via plain add_includedirs() doesn't help — clang deduplicates
		-- the repeated identical directory and keeps its -isystem position/classification
		-- regardless. Fix: generate a tiny shim directory (a genuinely new, never-elsewhere-used
		-- path, so there's nothing for clang to deduplicate against) containing just a
		-- forwarding uuid.h that #includes stduuid's real header by absolute path — added as a
		-- plain includedir, which then wins the search unambiguously since it's the only
		-- candidate at that unique path.
		on_load(function(target)
			local pkg = target:pkg("stduuid")
			if not pkg then
				return
			end
			local real_header
			for _, dir in ipairs(pkg:get("sysincludedirs") or pkg:get("includedirs") or {}) do
				local candidate = path.join(dir, "uuid.h")
				if os.isfile(candidate) then
					real_header = candidate
					break
				end
			end
			if not real_header then
				return
			end
			local shim_dir = path.join(os.projectdir(), "build", ".stduuid_shim")
			os.mkdir(shim_dir)
			io.writefile(path.join(shim_dir, "uuid.h"),
				string.format("// Generated by xmake/core_layers.lua — forwards to stduuid's real\n"
					.. "// uuid.h by absolute path, sidestepping a same-named uuid.h from another\n"
					.. "// resolved package (e.g. cpython's transitive util-linux dependency).\n"
					.. "#include \"%s\"\n", real_header))
			target:add("includedirs", shim_dir)
		end)
	end

	-- congelado_include: include/** only (interfaces, io layer, utils, model, shared) — the
	-- bottommost layer. Nothing here imports anything from sdk/ or plugins/.
	target("congelado_include")
	set_kind("shared")
	-- Cross-target C++ module BMI tracking doesn't reliably gate a dependent's start on this
	-- target's completion under "xmake build"'s default cross-target parallelism — congelado_sdk
	-- can start compiling before congelado_include's BMIs are actually done, failing with
	-- "module file not found". build.fence forces every dependent target to wait.
	set_policy("build.fence", true)
	apply_common_layer_settings()

	add_files(path.join(core_root, "include/**.cppm"), { public = true })

	if is_plat("windows", "mingw") then
		remove_files(path.join(core_root, "include/**/posix.cppm"))
		for _, f in ipairs(posix_module_files) do
			remove_files(path.join(core_root, f))
		end
	else
		remove_files(path.join(core_root, "include/**/win32.cppm"))
		remove_files(path.join(core_root, "include/modules/winsock2.cppm"))
	end
	target_end()

	-- congelado_sdk: sdk/** (plugin ABI, client SDK codegen, heart, worker) — built on top of
	-- congelado_include rather than merged with it.
	target("congelado_sdk")
	set_kind("shared")
	add_deps("congelado_include")
	set_policy("build.fence", true)
	apply_common_layer_settings({ plugin_includedir = true })

	add_files(path.join(core_root, "sdk/**.cppm"), { public = true })
	target_end()
end
