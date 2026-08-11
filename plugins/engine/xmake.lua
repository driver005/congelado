includes(path.join(core_root, "xmake/build.lua"))

target("engine_model")
set_kind("moduleonly")
add_deps("congelado_sdk")
add_files("model/**.cppm", { public = true })
target_end()

-- moduleonly, same reasoning as engine_model above: worker_api and engine_worker_lib both need
-- the "worker" module compiled, and independently add_files()'ing the identical worker/**.cppm
-- glob in two sibling targets (no dependency edge between them) silently dropped one side's
-- objects — xmake's module BMI cache credited the second target's compile as already satisfied
-- by the first's, without actually producing the second target's own object files (confirmed:
-- every compiled worker/*.cppm.o landed only under build/.objs/worker_api/, never under
-- build/.objs/engine_worker_lib/, causing "undefined symbol: worker::..." at engine_worker's link
-- time). moduleonly is xmake's actual supported mechanism for sharing module compilation across
-- independent consumers — engine_model already proves it works correctly for model/**.cppm.
target("worker_module")
set_kind("moduleonly")
add_deps("congelado_sdk", "engine_model")
add_files("worker/**.cppm", { public = true })
target_end()

-- default(false): only ever built via engine_lib's/engine_worker's real add_deps (below), never
-- as part of a bare `xmake build`'s default set — keeps a bare full build from being one more
-- place that could independently schedule these.
target("engine_api")
set_kind("binary")
set_default(false)
add_files("src/build.cc")
add_files("src/**.cppm", { public = true })
apply_common_layer_settings({ layer = "engine_api", fence = true })
add_deps("congelado_include", "congelado_sdk", "json_plugin", "openapi_generator", "engine_model")
add_rpathdirs("$ORIGIN")
target_end()

target("worker_api")
set_kind("binary")
set_default(false)
add_files("worker/build.cc")
apply_common_layer_settings({ layer = "worker_api", fence = true })
add_deps("congelado_include", "congelado_sdk", "json_plugin", "openapi_generator", "engine_model", "worker_module")
add_rpathdirs("$ORIGIN")
target_end()

target("engine_lib")
-- static, not shared: only "engine" below ever add_deps()'s this, so there's no reason for it to
-- be its own .so. As a separate shared lib it caused two real problems: heart's
-- SharedLibrary::scan() (non-recursive directory_iterator over shared_plugin_dir) dlopen()'d
-- libengine_lib.so as its own independent plugin — RTLD_LOCAL kept that separate handle's symbols
-- invisible to libengine.so's own load, leaving libengine.so's reference to it unresolved even
-- though a copy was loaded in-process; and moving its targetdir off shared_plugin_dir to dodge
-- that just broke libengine.so's rpath resolution instead ("cannot open shared object file").
-- static sidesteps both: it gets absorbed into libengine.so at link time, so there's no separate
-- file to scan or fail to find, and the vague-linkage vtable issue that caused the very first
-- version of this bug (LocalPayloadStorage's dtor) also goes away, since everything's back to one
-- link unit instead of split across a DSO boundary.
set_kind("static")
apply_common_layer_settings({
	layer = "engine",
	core_packages = { "lua" },
	fence = true,
})
-- No add_deps("engine_api", ...) here anymore: engine_api now add_deps()'s engine_lib (to reuse
-- its compiled objects instead of recompiling src/**.cppm itself), so the reverse edge would be a
-- cycle. The engine_api/worker_api before_build orchestration moved to engine_worker_lib, the
-- actual consumer of their generated/ output — see that target's own comment.
add_deps("congelado_sdk", "engine_model")
add_files("src/**.cppm", { public = true })
target_end()

target("engine")
set_kind("shared")
apply_common_layer_settings({ layer = "engine", targetdir = shared_plugin_dir })
add_files("src/engine.cc")
add_deps("engine_lib")
target_end()

target("engine_worker_lib")
-- static, not shared — same reasoning as engine_lib above: only "engine_worker" below ever
-- add_deps()'s this, so there's no reason for it to be its own .so (and the same
-- scan()/RTLD_LOCAL/rpath problems that hit libengine_lib.so as a shared lib would hit this too).
set_kind("static")
apply_common_layer_settings({ layer = "engine_worker_lib", fence = true })
-- engine_api/worker_api are real add_deps() here (not just built imperatively from before_build
-- below) so xmake's own scheduler builds each exactly once, in order, before this target's
-- before_build runs — running the SAME binary target twice concurrently (once via the top-level
-- scheduler building it as an ordinary default target, once via build_tool()'s nested `xmake
-- build` subprocess) raced two clang processes against the same partition .pcm output paths and
-- reliably corrupted the BMI (clang-22 ICEs: "malformed or corrupted precompiled file", "double
-- free or corruption", non-deterministic file each run). Lives here (not on engine_lib, where it
-- used to) because this target is the actual consumer of engine_api's generated/engine/client
-- output below — engine_lib itself never reads it.
add_deps("congelado_sdk", "engine_model", "worker_module", "engine_api", "worker_api")
add_files("generated/engine/client/**.cppm", { public = true })
before_build(function(target)
	import("build_tool")
	build_tool(target, "engine_api")
	build_tool(target, "worker_api", "worker")
end)
target_end()

target("engine_worker")
set_kind("binary")
set_policy("build.sanitizer.address", true)
add_files(path.join(core_root, "src/worker_main.cc"))
add_deps("engine_worker_lib", "engine_api")
add_packages("backward")
add_rpathdirs("$ORIGIN")
target_end()
--
-- CONGELADO_TASK worker bundles, one plain shared lib per worker/internal/ subfolder.
local worker_groups = {}
for _, f in ipairs(os.files(path.join(os.scriptdir(), "worker/internal/**/*.cc"))) do
	local name = path.basename(path.directory(f))
	worker_groups[name] = worker_groups[name] or {}
	table.insert(worker_groups[name], f)
end
for name, files in pairs(worker_groups) do
	target(name)
	set_kind("shared")
	apply_common_layer_settings({ layer = name, targetdir = path.join(core_root, "build", "workers") })
	add_includedirs(path.join(core_root, "sdk/worker/include"), path.join(core_root, "include"))
	add_files(table.unpack(files))
	target_end()
end
