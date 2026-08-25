-- Part of the merged plugins/ project — see plugins/xmake.lua. The local (in-process,
-- connector-backed) worker_orchestrator backend: the engine-side dispatch/orchestration capability,
-- symmetric to worker_manager/worker_external. Loaded by the engine host into build/plugins. The
-- connector/model calls live in src/store.cppm (a module TU) so the plugin's bin/*.cc never imports
-- connector directly.
target("worker_orchestrator_local_plugin")
set_kind("shared")
apply_common_layer_settings({
	layer = "worker_orchestrator_local_plugin",
	plugin_includedir = true,
	targetdir = shared_plugin_dir,
})
add_deps("congelado_sdk", "shared_model")
add_files("src/**.cppm")
add_files("bin/**.cc")
target_end()

-- worker_orchestrator_local_test: recompiles src/**.cppm + bin/**.cc with CONGELADO_TEST
-- defined. See apply_test_target in xmake/common.lua.
apply_test_target({
	name = "worker_orchestrator_local",
	layer = "worker_orchestrator_local_plugin",
	deps = { "congelado_sdk", "shared_model" },
	files = { "src/**.cppm", "bin/**.cc" },
})
