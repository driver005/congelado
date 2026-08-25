-- Part of the merged plugins/ project — see plugins/xmake.lua.
target("memory_events_plugin")
set_kind("shared")
apply_common_layer_settings({
	layer = "memory_events_plugin",
	plugin_includedir = true,
	targetdir = shared_plugin_dir,
})
add_deps("congelado_sdk")
add_files("bin/**.cc")
target_end()

-- memory_events_test: recompiles bin/**.cc with CONGELADO_TEST defined. See apply_test_target in
-- xmake/common.lua.
apply_test_target({
	name = "memory_events",
	layer = "memory_events_plugin",
	deps = { "congelado_sdk" },
	files = { "bin/**.cc" },
})
