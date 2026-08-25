-- Part of the merged plugins/ project — see plugins/xmake.lua. A shared, reusable database worker
-- (IWorker) that runs raw queries against the host-injected IDatabase — built into build/workers.
-- Impl in src/database_worker.cppm; the plugin's bin/*.cc holds only the Plugin class.
target("database_worker_plugin")
set_kind("shared")
apply_common_layer_settings({
	layer = "database_worker_plugin",
	plugin_includedir = true,
	targetdir = path.join(core_root, "build", "workers"),
})
add_deps("congelado_sdk")
add_files("src/**.cppm")
add_files("bin/**.cc")
target_end()

-- database_worker_test: recompiles src/**.cppm + bin/**.cc with CONGELADO_TEST defined. See
-- apply_test_target in xmake/common.lua.
apply_test_target({
	name = "database_worker",
	layer = "database_worker_plugin",
	deps = { "congelado_sdk" },
	files = { "src/**.cppm", "bin/**.cc" },
})
