-- Part of the merged plugins/ project — see plugins/xmake.lua. engine's build.cc dlopens this
-- .so at its own build time to bootstrap JSON serialization for the OpenAPI doc it writes.
target("json_plugin")
set_kind("shared")
apply_common_layer_settings({
	layer = "json_plugin",
	plugin_includedir = true,
	targetdir = shared_plugin_dir,
	core_packages = { "reflectcpp" },
})
add_deps("congelado_sdk")
add_files("src/**.cc")
remove_files("src/build.cc")
target_end()
