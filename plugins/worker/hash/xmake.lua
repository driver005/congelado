-- Part of the merged plugins/ project — see plugins/xmake.lua. A shared, reusable hashing worker
-- (IWorker) using OpenSSL's EVP digest API — built into build/workers. Impl in src/hash_worker.cppm;
-- the plugin's bin/*.cc holds only the Plugin class.
target("hash_worker_plugin")
set_kind("shared")
apply_common_layer_settings({
	layer = "hash_worker_plugin",
	plugin_includedir = true,
	targetdir = path.join(core_root, "build", "workers"),
})
add_deps("congelado_sdk")
add_packages("openssl")
add_files("src/**.cppm")
add_files("bin/**.cc")
target_end()

-- hash_worker_test: recompiles src/**.cppm + bin/**.cc with CONGELADO_TEST defined. See
-- apply_test_target in xmake/common.lua.
apply_test_target({
	name = "hash_worker",
	layer = "hash_worker_plugin",
	core_packages = { "openssl" },
	deps = { "congelado_sdk" },
	files = { "src/**.cppm", "bin/**.cc" },
})
