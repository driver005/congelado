-- Part of the merged plugins/ project — see plugins/xmake.lua. Runtime-loadable OpenAPI codegen
-- plugin: bundles the server-side OpenAPI document generator (moved in from
-- include/utils/openapi/generator.cppm) and the typed client SDK codegen pipeline (moved in from
-- sdk/client/{document,schema_model,dto_writer,route_writer,generator}.cppm) behind
-- interfaces::IOpenApiGenerator, replacing what used to be two separate build-time-only tools.
target("openapi_generator")
set_kind("shared")
apply_common_layer_settings({ plugin_includedir = true })
add_deps("congelado_sdk")
add_files("src/**.cc", "src/**.cppm")
remove_files("src/build.cc")
add_rpathdirs("$ORIGIN")
set_targetdir(shared_plugin_dir)
if is_plat("linux", "macosx") then
	add_cxflags("-ffile-prefix-map=$(projectdir)=.", "-fmacro-prefix-map=$(projectdir)=.")
end
target_end()
