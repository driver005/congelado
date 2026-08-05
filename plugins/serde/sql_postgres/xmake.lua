-- Part of the merged plugins/ project — see plugins/xmake.lua.
target("sql_postgres_plugin")
set_kind("shared")
apply_common_layer_settings({ plugin_includedir = true })
add_deps("congelado_sdk")
add_packages("sqlgen", "reflectcpp")
add_files("src/**.cc")
remove_files("src/build.cc")
-- Same libpq/GSSAPI gap as plugins/postgres — see that project's own comment.
if not is_plat("windows", "mingw") then
	add_syslinks("gssapi_krb5")
end
add_rpathdirs("$ORIGIN")
set_targetdir(shared_plugin_dir)
if is_plat("linux", "macosx") then
	add_cxflags("-ffile-prefix-map=$(projectdir)=.", "-fmacro-prefix-map=$(projectdir)=.")
end
target_end()
