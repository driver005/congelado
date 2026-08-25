-- Part of the merged plugins/ project — see plugins/xmake.lua. The auth-app `db_query` worker: an
-- IWorker that persists/reads a user through the host-injected connector. The connector/model calls
-- live in src/store.cppm (a module TU) so the plugin's bin/*.cc never imports connector directly.
target("auth_db_query_worker_plugin")
set_kind("shared")
apply_common_layer_settings({
	layer = "auth_db_query_worker_plugin",
	plugin_includedir = true,
	targetdir = path.join(core_root, "build", "workers"),
})
add_deps("congelado_sdk", "shared_model")
add_files("src/**.cppm")
add_files("bin/**.cc")
target_end()

-- auth_db_query_worker_test: recompiles src/**.cppm + bin/**.cc with CONGELADO_TEST defined. See
-- apply_test_target in xmake/common.lua.
apply_test_target({
	name = "auth_db_query_worker",
	layer = "auth_db_query_worker_plugin",
	deps = { "congelado_sdk", "shared_model" },
	files = { "src/**.cppm", "bin/**.cc" },
})
