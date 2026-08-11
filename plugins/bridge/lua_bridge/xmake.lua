-- Part of the merged plugins/ project — see plugins/xmake.lua. "lua" package requirement is
-- configured once at the parent's setup_extra_requires() call.
target("lua_bridge_plugin")
set_kind("shared")
apply_common_layer_settings({
	layer = "lua_bridge_plugin",
	plugin_includedir = true,
	targetdir = shared_plugin_dir,
	core_packages = { "lua" },
})
add_deps("congelado_sdk")
add_files("src/**.cc")
remove_files("src/build.cc")
target_end()
