-- Part of the merged plugins/ project — see plugins/xmake.lua.
target("rabbitmq_events_plugin")
set_kind("shared")
-- Same "link a blocking C client library directly, bypass IClient/router" pattern
-- postgres_plugin (libpq) and elasticsearch_plugin (libcurl) already use.
apply_common_layer_settings({
	layer = "rabbitmq_events_plugin",
	plugin_includedir = true,
	targetdir = shared_plugin_dir,
	core_packages = { "rabbitmqc" },
})
add_deps("congelado_sdk")
add_files("src/**.cc")
target_end()
