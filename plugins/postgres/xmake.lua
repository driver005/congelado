-- Part of the merged plugins/ project — see plugins/xmake.lua.
target("postgres_plugin")
set_kind("shared")
apply_common_layer_settings({ plugin_includedir = true })
add_deps("congelado_sdk")
add_packages("sqlgen", "reflectcpp")
add_files("src/**.cc")
remove_files("src/build.cc")
-- libpq (pulled in transitively via sqlgen) auto-detects GSSAPI support at ITS OWN build time
-- but xmake never gets told to link libgssapi_krb5 for it — same explicit syslink the old
-- inline discovery used (xmake/plugin.lua's per-target special case, now moved here since this
-- plugin owns its own target definition).
if not is_plat("windows", "mingw") then
	add_syslinks("gssapi_krb5")
end
add_rpathdirs("$ORIGIN")
set_targetdir(shared_plugin_dir)
if is_plat("linux", "macosx") then
	add_cxflags("-ffile-prefix-map=$(projectdir)=.", "-fmacro-prefix-map=$(projectdir)=.")
end
target_end()
