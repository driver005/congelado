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
add_files("bin/**.cc")
remove_files("src/build.cc")
target_end()

-- lua_bridge_test: recompiles bin/**.cc with CONGELADO_TEST defined. See apply_test_target in
-- xmake/common.lua.
apply_test_target({
	name = "lua_bridge",
	layer = "lua_bridge_plugin",
	core_packages = { "lua" },
	deps = { "congelado_sdk" },
	files = { "bin/**.cc" },
	remove = { "src/build.cc" },
})
