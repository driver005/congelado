-- Part of the merged plugins/ project — see plugins/xmake.lua.
target("rabbitmq_events_plugin")
set_kind("shared")
apply_common_layer_settings({ plugin_includedir = true })
add_deps("congelado_sdk")
-- Same "link a blocking C client library directly, bypass IClient/router" pattern
-- postgres_plugin (libpq) and elasticsearch_plugin (libcurl) already use.
add_packages("rabbitmqc")
add_files("src/**.cc")
add_rpathdirs("$ORIGIN")
set_targetdir(shared_plugin_dir)
if is_plat("linux", "macosx") then
	add_cxflags("-ffile-prefix-map=$(projectdir)=.", "-fmacro-prefix-map=$(projectdir)=.")
end
target_end()
