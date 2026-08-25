-- Part of the merged plugins/ project — see plugins/xmake.lua.
target("redis_events_plugin")
set_kind("shared")
-- Same "link a blocking C client library directly, bypass IClient/router" pattern
-- postgres_plugin (libpq) and elasticsearch_plugin (libcurl) already use.
apply_common_layer_settings({
	layer = "redis_events_plugin",
	plugin_includedir = true,
	targetdir = shared_plugin_dir,
	core_packages = { "hiredis" },
})
add_deps("congelado_sdk")
add_files("bin/**.cc")
target_end()

-- redis_events_test: recompiles bin/**.cc with CONGELADO_TEST defined. See apply_test_target in
-- xmake/common.lua.
apply_test_target({
	name = "redis_events",
	layer = "redis_events_plugin",
	core_packages = { "hiredis" },
	deps = { "congelado_sdk" },
	files = { "bin/**.cc" },
})
