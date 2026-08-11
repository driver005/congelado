-- Part of the merged plugins/ project — see plugins/xmake.lua.
target("elasticsearch_plugin")
set_kind("shared")
-- Same "link a client library directly, bypass IClient/router" pattern postgres_plugin uses for
-- libpq — no ready-made non-router HTTP client exists in this codebase to build ES's REST calls
-- on (see this plugin's own doc comment), so libcurl gets linked here explicitly rather than
-- reusing otel_otlp_plugin's transitive one (that would pull in the whole opentelemetry-cpp
-- dependency tree just for headers).
apply_common_layer_settings({
	layer = "elasticsearch_plugin",
	plugin_includedir = true,
	targetdir = shared_plugin_dir,
	core_packages = { "libcurl" },
})
add_deps("congelado_sdk")
add_files("src/**.cc")
target_end()
