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
add_files("bin/**.cc")
remove_files("src/build.cc")
-- Same libpq/GSSAPI gap as plugins/postgres — see that project's own comment.
if not is_plat("windows", "mingw") then
	add_syslinks("gssapi_krb5")
end
target_end()

-- sql_postgres_test: recompiles bin/**.cc with CONGELADO_TEST defined. See apply_test_target in
-- xmake/common.lua.
apply_test_target({
	name = "sql_postgres",
	layer = "sql_postgres_plugin",
	core_packages = { "reflectcpp" },
	deps = { "congelado_sdk" },
	files = { "bin/**.cc" },
	remove = { "src/build.cc" },
})
-- Same libpq/GSSAPI gap as the production target above.
if not is_plat("windows", "mingw") then
	target("sql_postgres_test")
	add_syslinks("gssapi_krb5")
	target_end()
end
