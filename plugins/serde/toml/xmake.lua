-- Part of the merged plugins/ project — see plugins/xmake.lua.
target("toml_plugin")
set_kind("shared")
apply_common_layer_settings({
	layer = "toml_plugin",
	plugin_includedir = true,
	targetdir = shared_plugin_dir,
	core_packages = { "reflectcpp" },
})
add_deps("congelado_sdk")
add_files("src/**.cc")
remove_files("src/build.cc")
target_end()
