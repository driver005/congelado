set_project("congelado")
set_version("0.1.0")

add_rules("mode.debug", "mode.release")
set_defaultmode("debug")

-- Generic build-script feature (wire_build_script) — needed by both xmake/plugin.lua's loop
-- and congelado_worker's own target below, so it's included first.
includes("xmake/buildscript.lua")

-- `xmake build-all` — the single-invocation entry point (builds standalone plugin projects in
-- order, then this project). See xmake/tasks/build_all.lua for why plugins need to be separate
-- xmake processes at all.
includes("xmake/tasks/build_all.lua")

--  TODO: use when the time is ready for now it is complining a lot!!!!
-- set_runtimes("c++_shared")
-- add_cxflags("-fexperimental-library")
-- add_ldflags("-lc++exp")

-- TODO: please add again
-- set_warnings("all", "extra", "error")

-- congelado_include/congelado_sdk (conan requires, toolchain setup, both target defs) live in
-- xmake/core_layers.lua — shared with every standalone plugin project (plugins/<name>/xmake.lua),
-- each of which builds its own copy of these two layers from source rather than depending on a
-- prebuilt package (xmake has no mechanism to propagate C++20 module interfaces through
-- add_packages() — see xmake/core_layers.lua's own comment).
includes("xmake/core_layers.lua")
setup_core_layers(os.projectdir())
-- Root project still needs cli11 (congelado_cli) and backward (congelado_worker, congelado);
-- catch2/cpython/lua/opentelemetrycpp no longer apply here — every plugin that used to need them
-- inline now requires its own subset directly (see e.g. plugins/xmake.lua's own
-- setup_extra_requires() call).
setup_extra_requires({ "cli11" })

-- Every plugin now lives in its own standalone xmake project (plugins/<name>/xmake.lua, built by
-- the orchestrator — xmake/tasks/build_all.lua) rather than as a target discovered inline here.
-- That leaves nothing between congelado_sdk and the three entry-point binaries below — the old
-- congelado_lib layer (plugins/**.cppm + src/**.cc) is gone: src/**.cc was always just the three
-- mains, and plugins/**.cppm's only real content (engine's core/model/worker modules) moved to
-- plugins/engine/xmake.lua. The three binaries below depend on congelado_sdk directly.
includes("xmake/ffi.lua")

target("congelado")
set_kind("binary")
set_policy("build.sanitizer.address", true)
add_files("src/main.cc")
add_deps("congelado_sdk")
add_packages("simdjson")
add_rpathdirs("$ORIGIN")
target_end()

target("congelado_worker")
set_kind("binary")
set_policy("build.sanitizer.address", true)
add_files("src/worker_main.cc")
-- worker_main.cc's own module family ("worker" + its :poll/:execution/:status partitions,
-- "model", and build.cc's generated client SDK) lives under plugins/engine/, which is engine's
-- own standalone project (see plugins/engine/xmake.lua) — so congelado_worker compiles its own
-- copy of just the subset it actually needs directly. The generated/ files only exist once the
-- orchestrator has already run engine's project (see xmake/tasks/build_all.lua) — that's the
-- entire point of the split: engine's project fully finishes, in its own separate invocation,
-- before this target's own module scan ever starts.
add_files("plugins/engine/src/model/**.cppm")
add_files("plugins/engine/src/worker/**.cppm")
add_files("plugins/engine/src/generated/client/**.cppm")
add_deps("congelado_sdk")
add_packages("backward")
add_rpathdirs("$ORIGIN")
target_end()

target("congelado_cli")
set_kind("binary")
set_policy("build.sanitizer.address", true)
add_files("src/cli_main.cc")
add_deps("congelado_sdk")
add_packages("cli11")
add_rpathdirs("$ORIGIN")
target_end()

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
