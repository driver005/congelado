-- Part of the merged plugins/ project — see plugins/xmake.lua. The in-process fallback cache
-- (ICache) as a loadable plugin — same in-memory store Connector keeps built in as LocalCache,
-- now selectable as the `local` cache backend. No external client library, so no extra
-- core_packages beyond the SDK.
target("local_cache_plugin")
set_kind("shared")
apply_common_layer_settings({
	layer = "local_cache_plugin",
	plugin_includedir = true,
	targetdir = shared_plugin_dir,
})
add_deps("congelado_sdk")
add_files("bin/**.cc")
target_end()

-- local_cache_test: recompiles bin/**.cc with CONGELADO_TEST defined. See apply_test_target in
-- xmake/common.lua.
apply_test_target({
	name = "local_cache",
	layer = "local_cache_plugin",
	deps = { "congelado_sdk" },
	files = { "bin/**.cc" },
})
