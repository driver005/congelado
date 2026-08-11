-- Part of the merged plugins/ project — see plugins/xmake.lua.
target("cron_local_plugin")
set_kind("shared")
apply_common_layer_settings({
	layer = "cron_local_plugin",
	plugin_includedir = true,
	targetdir = shared_plugin_dir,
})
add_deps("congelado_sdk")
add_files("src/**.cc")
target_end()
