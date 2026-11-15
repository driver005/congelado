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
add_files("bin/**.cc")
remove_files("src/build.cc")
target_end()

-- toml_plugin_test: recompiles bin/**.cc with CONGELADO_TEST defined. See apply_test_target in
-- xmake/common.lua.
apply_test_target({
	name = "toml_plugin",
	layer = "toml_plugin",
	core_packages = { "reflectcpp" },
	deps = { "congelado_sdk" },
	files = { "bin/**.cc" },
	remove = { "src/build.cc" },
})
