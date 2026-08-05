-- Part of the merged plugins/ project — see plugins/xmake.lua. Sharing this project with the
-- other 8 plugins is safe despite engine's build.cc codegen step: the hard constraint that forced
-- engine into its own process originally was specifically "congelado_worker (root project) must
-- not scan for an `import` of engine's generated client SDK before build.cc has actually written
-- it" — none of the other 8 plugins here import anything engine generates, so their scan doesn't
-- care. The root project stays fully separate and still builds last (after this whole merged
-- project, including engine, finishes), so that original ordering constraint still holds.
--
-- Two things stop being automatically true once engine shares a process (and therefore build.cc's
-- inherited execv cwd) with sibling folders that aren't plugins/engine/, both fixed here:
-- 1. build.cc itself uses paths relative to its OWN folder ("generated/openapi.json",
--    "../../build/plugins/libjson_plugin.so") — xmake/buildscript.lua's wire_build_scripts() now
--    pins its execv curdir explicitly to that folder, not whatever cwd the invoking project has.
-- 2. os.projectdir() no longer equals this folder (it now returns the merged project's own root,
--    plugins/) — os.scriptdir() (this file's own directory) is used below wherever engine's
--    original code relied on os.projectdir() meaning "this folder".
includes(path.join(core_root, "xmake/buildscript.lua"))

-- engine's own module interface files (core/, model/, worker/) + engine.cc's dlopen entry point
-- — one target, same as how congelado_lib used to hold the .cppm files while the separate
-- "engine" dlopen target held just engine.cc; there's no other target in this project that
-- would need them split apart.
target("engine")
set_kind("shared")
set_languages("c++26")
set_policy("build.c++.modules", true)
-- engine_build (wire_build_scripts, below) depends on this target and needs its BMI/build fully
-- finished before compiling build.cc's `import engine;` — matches congelado_include/congelado_sdk's
-- own pattern (xmake/core_layers.lua): the fence belongs on the target *others wait on*, not
-- (only) on the dependent tool itself.
set_policy("build.fence", true)
apply_common_layer_settings({ plugin_includedir = true })
add_deps("congelado_sdk")
-- "lua" needed for src/core/expr/lua_eval.cppm's direct lua_State* manipulation (condition
-- evaluation for SWITCH/DO_WHILE/EventHandler) — same package lua_bridge itself links against.
add_packages("sqlgen", "reflectcpp", "lua")
add_files("src/**.cppm", { public = true })
add_files("src/**.cc")
remove_files("src/generated/**.cppm")
-- build.cc is its own separate tool target (wire_build_scripts, below) — never part of
-- this shared-lib target. worker/internal/**.cc are the CONGELADO_TASK worker bundles,
-- each its own plain-shared-lib target (the loop further below) with different include
-- dirs (sdk/worker/include, not congelado_sdk) — including them here would double-compile
-- them under the wrong module/include settings.
remove_files("src/build.cc")
remove_files("src/worker/internal/**.cc")
add_rpathdirs("$ORIGIN")
set_targetdir(shared_plugin_dir)
if is_plat("linux", "macosx") then
	add_cxflags("-ffile-prefix-map=$(projectdir)=.", "-fmacro-prefix-map=$(projectdir)=.")
end
target_end()

-- build.cc: same wire_build_scripts mechanism as before, depending on this project's own local
-- "engine" target. json_plugin is now a SIBLING target in this same merged project (not a
-- separate process the orchestrator sequenced before this one), so it's add_deps()'d here too,
-- purely for build ordering — build.cc dlopens libjson_plugin.so at its own runtime and needs
-- that file to already exist on disk before it runs, same as it always has, just enforced via
-- xmake's own target graph now instead of the orchestrator's process ordering.
wire_build_scripts("engine", os.scriptdir(), { "engine", "json_plugin" })

-- CONGELADO_TASK worker bundles: engine's own worker subsystem (physically nested under
-- worker/internal/ in this same directory tree) — built here, not by the root project and not as
-- their own standalone projects. Plain shared libs, no module imports, no tie to
-- congelado_include/congelado_sdk at all (congelado/worker.h is a header-only C ABI macro) — so
-- unlike "engine" above, they don't need set_policy("build.c++.modules") or add_deps(congelado_sdk).
-- A new bundle folder (even with multiple .cc files, grouped by folder same as before) just needs
-- to exist — no xmake.lua of its own, no orchestrator edit.
local worker_groups = {}
for _, f in ipairs(os.files(path.join(os.scriptdir(), "src/worker/internal/**/*.cc"))) do
	local name = path.basename(path.directory(f))
	worker_groups[name] = worker_groups[name] or {}
	table.insert(worker_groups[name], f)
end
for name, files in pairs(worker_groups) do
	target(name)
	set_kind("shared")
	set_languages("c++26")
	add_cxflags("-fPIC")
	add_includedirs(path.join(core_root, "sdk/worker/include"), path.join(core_root, "include"))
	add_files(table.unpack(files))
	add_rpathdirs("$ORIGIN")
	set_targetdir(path.join(core_root, "build", "workers"))
	if is_plat("linux", "macosx") then
		add_cxflags("-ffile-prefix-map=$(projectdir)=.", "-fmacro-prefix-map=$(projectdir)=.")
	end
	target_end()
end
