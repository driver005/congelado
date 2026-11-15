-- Part of the merged plugins/ project — see plugins/xmake.lua. An auth-app task worker (IWorker),
-- built into build/workers alongside the other worker plugins the worker host scans.
target("auth_pw_hash_worker_plugin")
set_kind("shared")
apply_common_layer_settings({
	layer = "auth_pw_hash_worker_plugin",
	plugin_includedir = true,
	targetdir = path.join(core_root, "build", "workers"),
})
add_deps("congelado_sdk")
add_files("bin/**.cc")
target_end()

-- auth_pw_hash_worker_test: recompiles bin/**.cc with CONGELADO_TEST defined. See
-- apply_test_target in xmake/common.lua.
apply_test_target({
	name = "auth_pw_hash_worker",
	layer = "auth_pw_hash_worker_plugin",
	deps = { "congelado_sdk" },
	files = { "bin/**.cc" },
})
