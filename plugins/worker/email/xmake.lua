-- Part of the merged plugins/ project — see plugins/xmake.lua. A shared, reusable email worker
-- (IWorker) — SMTP send via libcurl, the counterpart to Conductor's SendGrid/email system task.
-- Built into build/workers. Impl in src/email_worker.cppm; the plugin's bin/*.cc holds only the
-- Plugin class.
target("email_worker_plugin")
set_kind("shared")
apply_common_layer_settings({
	layer = "email_worker_plugin",
	plugin_includedir = true,
	targetdir = path.join(core_root, "build", "workers"),
})
add_deps("congelado_sdk")
add_packages("libcurl")
add_files("src/**.cppm")
add_files("bin/**.cc")
target_end()

-- email_worker_test: recompiles src/**.cppm + bin/**.cc with CONGELADO_TEST defined. See
-- apply_test_target in xmake/common.lua.
apply_test_target({
	name = "email_worker",
	layer = "email_worker_plugin",
	core_packages = { "libcurl" },
	deps = { "congelado_sdk" },
	files = { "src/**.cppm", "bin/**.cc" },
})
