-- Part of the merged plugins/ project — see plugins/xmake.lua. The external worker-manager plugin:
-- the full worker runtime (worker_runtime module = engine's old worker/ http2 poll + inbound API)
-- plus the IWorkerManager the worker host resolves. Stays a shared .so (loaded by the worker host
-- exe), which is the whole difference from the old engine_worker binary.

-- moduleonly (same reasoning as shared_model / the old worker_module): the worker_runtime .cppm
-- are compiled once here and shared by both the plugin .so and the worker_api codegen binary,
-- instead of each add_files()'ing the same glob (which trips xmake's module BMI double-credit bug).
target("worker_runtime_module")
set_kind("moduleonly")
-- -fPIC: these objects (call_engine's lambda/std::move_only_function, std::span) are linked into
-- the plugin .so below, which requires position-independent code — moduleonly skips
-- apply_common_layer_settings (where every other layer gets -fPIC), so set it explicitly.
add_cxflags("-fPIC")
add_deps("congelado_sdk", "shared_model")
add_files("src/**.cppm", { public = true })
target_end()

-- Build-time-only codegen binary (bin/build.cc has main()): generates the worker's OpenAPI doc +
-- congelado_worker_api client SDK. default(false) — only built via the plugin's add_deps below.
target("worker_api")
set_kind("binary")
set_default(false)
add_files("bin/build.cc")
apply_common_layer_settings({ layer = "worker_api", fence = true })
add_deps("congelado_include", "congelado_sdk", "json_plugin", "openapi_generator", "shared_model",
	"worker_runtime_module")
add_rpathdirs("$ORIGIN")
target_end()

target("worker_manager_external_plugin")
set_kind("shared")
apply_common_layer_settings({
	layer = "worker_manager_external_plugin",
	plugin_includedir = true,
	targetdir = shared_plugin_dir,
})
-- worker_api is a real add_deps() (not just built imperatively from before_build) so xmake's
-- scheduler builds it exactly once, in order, before this target's before_build runs its codegen —
-- same race-avoidance reasoning as the engine's engine_worker_lib.
add_deps("congelado_sdk", "shared_model", "worker_runtime_module", "worker_api")
-- Only the plugin entry .cc — bin/build.cc (own main()) is compiled by the worker_api target above,
-- and the .cppm are compiled by worker_runtime_module.
add_files("bin/external_worker_manager.cc")
before_build(function(target)
	import("build_tool")
	build_tool(target, "worker_api", "bin")
end)
target_end()

-- worker_manager_external_test: recompiles worker_runtime_module's src/**.cppm plus
-- bin/external_worker_manager.cc with CONGELADO_TEST defined. See apply_test_target in
-- xmake/common.lua. Skips worker_api's codegen step (the test binary doesn't need the
-- generated client SDK, same as the plugin target itself doesn't).
apply_test_target({
	name = "worker_manager_external",
	layer = "worker_manager_external_plugin",
	deps = { "congelado_sdk", "shared_model" },
	files = { "src/**.cppm", "bin/external_worker_manager.cc" },
})
