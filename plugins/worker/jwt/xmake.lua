-- Part of the merged plugins/ project — see plugins/xmake.lua. A shared, reusable JWT worker
-- (IWorker) — HS256 signing via OpenSSL, the counterpart to Conductor's Get-Signed-JWT system task.
-- Built into build/workers. Impl in src/jwt_worker.cppm; the plugin's bin/*.cc holds only the
-- Plugin class.
target("jwt_worker_plugin")
set_kind("shared")
apply_common_layer_settings({
	layer = "jwt_worker_plugin",
	plugin_includedir = true,
	targetdir = path.join(core_root, "build", "workers"),
})
add_deps("congelado_sdk")
add_packages("openssl")
add_files("src/**.cppm")
add_files("bin/**.cc")
target_end()

-- jwt_worker_test: recompiles src/**.cppm + bin/**.cc with CONGELADO_TEST defined. See
-- apply_test_target in xmake/common.lua.
apply_test_target({
	name = "jwt_worker",
	layer = "jwt_worker_plugin",
	core_packages = { "openssl" },
	deps = { "congelado_sdk" },
	files = { "src/**.cppm", "bin/**.cc" },
})
