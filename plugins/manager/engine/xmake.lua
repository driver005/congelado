includes(path.join(core_root, "xmake/build.lua"))

-- The `model` module moved to plugins/shared/ (target shared_model, moduleonly) — shared between
-- the engine and the manager/worker_external plugin. See plugins/shared/xmake.lua.

-- default(false): only ever built via engine_lib's/engine_worker's real add_deps (below), never
-- as part of a bare `xmake build`'s default set — keeps a bare full build from being one more
-- place that could independently schedule these.
target("engine_api")
set_kind("binary")
set_default(false)
add_files("bin/build.cc")
add_files("src/**.cppm", { public = true })
apply_common_layer_settings({ layer = "engine_api", fence = true })
add_deps("congelado_include", "congelado_sdk", "json_plugin", "openapi_generator", "shared_model")
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
add_deps("congelado_sdk", "shared_model")
add_files("src/**.cppm", { public = true })
target_end()

target("engine")
set_kind("shared")
apply_common_layer_settings({ layer = "engine", targetdir = shared_plugin_dir })
add_files("bin/engine.cc")
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
add_deps("congelado_sdk", "shared_model", "engine_api")
add_files("generated/engine/client/**.cppm", { public = true })
before_build(function(target)
	import("build_tool")
	build_tool(target, "engine_api")
end)
target_end()

-- The worker executable moved to sdk/worker/ (root xmake.lua target("worker")); the individual
-- task workers (echo/transform) are now standalone IWorker plugins under plugins/worker/, and the
-- http2 poll runtime lives in the manager/worker_external plugin. Nothing worker-related is built
-- from this engine directory anymore.

-- engine_test: recompiles src/**.cppm with CONGELADO_TEST defined (same "cargo test" pattern as
-- engine_lib above, minus the generated-client/before_build machinery it doesn't need). See
-- apply_test_target in xmake/common.lua.
apply_test_target({
	name = "engine",
	layer = "engine",
	core_packages = { "lua" },
	deps = { "congelado_sdk", "shared_model" },
	files = { "src/**.cppm" },
})
