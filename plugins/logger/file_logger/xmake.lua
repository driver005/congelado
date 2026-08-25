-- Part of the merged plugins/ project — see plugins/xmake.lua.
target("file_logger")
set_kind("shared")
apply_common_layer_settings({
	layer = "file_logger",
	plugin_includedir = true,
	targetdir = shared_plugin_dir,
})
add_deps("congelado_sdk")
add_files("bin/**.cc")
remove_files("src/build.cc")
target_end()

-- file_logger_test: recompiles bin/**.cc with CONGELADO_TEST defined. See apply_test_target in
-- xmake/common.lua.
apply_test_target({
	name = "file_logger",
	layer = "file_logger",
	deps = { "congelado_sdk" },
	files = { "bin/**.cc" },
	remove = { "src/build.cc" },
})
