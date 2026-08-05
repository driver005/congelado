-- Part of the merged plugins/ project — see plugins/xmake.lua. engine's build.cc dlopens this
-- .so at its own build time to bootstrap JSON serialization for the OpenAPI doc it writes.
target("json_plugin")
set_kind("shared")
apply_common_layer_settings({ plugin_includedir = true })
add_deps("congelado_sdk")
add_files("src/**.cc")
remove_files("src/build.cc")
add_rpathdirs("$ORIGIN")
set_targetdir(shared_plugin_dir)
if is_plat("linux", "macosx") then
	add_cxflags("-ffile-prefix-map=$(projectdir)=.", "-fmacro-prefix-map=$(projectdir)=.")
end
target_end()
