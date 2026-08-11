-- Part of the merged plugins/ project — see plugins/xmake.lua.
target("sql_postgres_plugin")
set_kind("shared")
apply_common_layer_settings({
	layer = "sql_postgres_plugin",
	plugin_includedir = true,
	targetdir = shared_plugin_dir,
	core_packages = { "reflectcpp" },
})
add_deps("congelado_sdk")
add_files("src/**.cc")
remove_files("src/build.cc")
-- Same libpq/GSSAPI gap as plugins/postgres — see that project's own comment.
if not is_plat("windows", "mingw") then
	add_syslinks("gssapi_krb5")
end
target_end()
