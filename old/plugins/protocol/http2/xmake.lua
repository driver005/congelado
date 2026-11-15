-- Part of the merged plugins/ project — see plugins/xmake.lua.
target("http2")
set_kind("shared")
apply_common_layer_settings({
	layer = "http2",
	plugin_includedir = true,
	targetdir = shared_plugin_dir,
})
add_deps("congelado_sdk")
add_files("bin/**.cc")
remove_files("src/build.cc")
target_end()

-- http2_plugin_test: recompiles bin/**.cc with CONGELADO_TEST defined. See apply_test_target in
-- xmake/common.lua.
apply_test_target({
	name = "http2_plugin",
	layer = "http2",
	deps = { "congelado_sdk" },
	files = { "bin/**.cc" },
	remove = { "src/build.cc" },
})
