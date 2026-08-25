-- Part of the merged plugins/ project — see plugins/xmake.lua. A single task worker, now a proper
-- IWorker plugin (was plugins/engine/worker/internal/transform under the old CONGELADO_TASK ABI).
target("transform_worker_plugin")
set_kind("shared")
apply_common_layer_settings({
	layer = "transform_worker_plugin",
	plugin_includedir = true,
	targetdir = path.join(core_root, "build", "workers"),
})
add_deps("congelado_sdk")
add_files("bin/**.cc")
target_end()

-- transform_worker_test: recompiles bin/**.cc with CONGELADO_TEST defined. See apply_test_target
-- in xmake/common.lua.
apply_test_target({
	name = "transform_worker",
	layer = "transform_worker_plugin",
	deps = { "congelado_sdk" },
	files = { "bin/**.cc" },
})
