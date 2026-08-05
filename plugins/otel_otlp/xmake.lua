-- Part of the merged plugins/ project — see plugins/xmake.lua. Needs conan >=2.21.0 (this
-- project's documented floor, see README.md) for opentelemetry-cpp's transitive libcurl recipe.
target("otel_otlp_plugin")
set_kind("shared")
apply_common_layer_settings({ plugin_includedir = true })
add_deps("congelado_sdk")
add_packages("sqlgen", "reflectcpp", "opentelemetrycpp")
add_files("src/**.cc")
remove_files("src/build.cc")
add_rpathdirs("$ORIGIN")
set_targetdir(shared_plugin_dir)
if is_plat("linux", "macosx") then
	add_cxflags("-ffile-prefix-map=$(projectdir)=.", "-fmacro-prefix-map=$(projectdir)=.")
end
target_end()
