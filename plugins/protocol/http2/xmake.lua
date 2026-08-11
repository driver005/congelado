-- Part of the merged plugins/ project — see plugins/xmake.lua.
target("http2")
set_kind("shared")
apply_common_layer_settings({
	layer = "http2",
	plugin_includedir = true,
	targetdir = shared_plugin_dir,
})
add_deps("congelado_sdk")
add_files("src/**.cc")
remove_files("src/build.cc")
target_end()
