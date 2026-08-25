-- Part of the merged plugins/ project — see plugins/xmake.lua.
target("cron_local_plugin")
set_kind("shared")
apply_common_layer_settings({
	layer = "cron_local_plugin",
	plugin_includedir = true,
	targetdir = shared_plugin_dir,
})
add_deps("congelado_sdk")
add_files("bin/**.cc")
target_end()

-- cron_local_test: recompiles bin/**.cc with CONGELADO_TEST defined. See apply_test_target in
-- xmake/common.lua.
apply_test_target({
	name = "cron_local",
	layer = "cron_local_plugin",
	deps = { "congelado_sdk" },
	files = { "bin/**.cc" },
})
