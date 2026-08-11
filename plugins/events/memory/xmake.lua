-- Part of the merged plugins/ project — see plugins/xmake.lua.
target("memory_events_plugin")
set_kind("shared")
apply_common_layer_settings({
	layer = "memory_events_plugin",
	plugin_includedir = true,
	targetdir = shared_plugin_dir,
})
add_deps("congelado_sdk")
add_files("src/**.cc")
target_end()
