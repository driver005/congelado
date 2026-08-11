-- Part of the merged plugins/ project — see plugins/xmake.lua.
target("file_logger")
set_kind("shared")
apply_common_layer_settings({
	layer = "file_logger",
	plugin_includedir = true,
	targetdir = shared_plugin_dir,
})
add_deps("congelado_sdk")
add_files("src/**.cc")
remove_files("src/build.cc")
target_end()
