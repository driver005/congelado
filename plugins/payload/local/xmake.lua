-- Part of the merged plugins/ project — see plugins/xmake.lua. The local-disk payload-storage
-- backend: the default `payload_storage` capability (was engine::LocalPayloadStorage). The impl
-- lives in src/local_storage.cppm; the plugin's bin/*.cc holds only the Plugin class.
target("payload_local_plugin")
set_kind("shared")
apply_common_layer_settings({
	layer = "payload_local_plugin",
	plugin_includedir = true,
	targetdir = shared_plugin_dir,
})
add_deps("congelado_sdk")
add_files("src/**.cppm")
add_files("bin/**.cc")
target_end()

-- payload_local_test: recompiles src/**.cppm + bin/**.cc with CONGELADO_TEST defined. See
-- apply_test_target in xmake/common.lua.
apply_test_target({
	name = "payload_local",
	layer = "payload_local_plugin",
	deps = { "congelado_sdk" },
	files = { "src/**.cppm", "bin/**.cc" },
})
