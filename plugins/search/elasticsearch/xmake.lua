-- Part of the merged plugins/ project — see plugins/xmake.lua.
target("elasticsearch_plugin")
set_kind("shared")
apply_common_layer_settings({ plugin_includedir = true })
add_deps("congelado_sdk")
-- Same "link a client library directly, bypass IClient/router" pattern postgres_plugin uses for
-- libpq — no ready-made non-router HTTP client exists in this codebase to build ES's REST calls
-- on (see this plugin's own doc comment), so libcurl gets linked here explicitly rather than
-- reusing otel_otlp_plugin's transitive one (that would pull in the whole opentelemetry-cpp
-- dependency tree just for headers).
add_packages("libcurl")
add_files("src/**.cc")
add_rpathdirs("$ORIGIN")
set_targetdir(shared_plugin_dir)
if is_plat("linux", "macosx") then
	add_cxflags("-ffile-prefix-map=$(projectdir)=.", "-fmacro-prefix-map=$(projectdir)=.")
end
target_end()
