-- Part of the merged plugins/ project — see plugins/xmake.lua.
target("postgres_plugin")
set_kind("shared")
apply_common_layer_settings({
	layer = "postgres_plugin",
	plugin_includedir = true,
	targetdir = shared_plugin_dir,
	core_packages = { "sqlgen" },
})
add_deps("congelado_sdk")
add_files("src/**.cppm")
add_files("bin/**.cc")
remove_files("src/build.cc")
-- libpq (pulled in transitively via sqlgen) auto-detects GSSAPI support at ITS OWN build time
-- but xmake never gets told to link libgssapi_krb5 for it — same explicit syslink the old
-- inline discovery used (xmake/plugin.lua's per-target special case, now moved here since this
-- plugin owns its own target definition).
if not is_plat("windows", "mingw") then
	add_syslinks("gssapi_krb5")
end
target_end()

-- postgres_test: recompiles src/**.cppm + bin/**.cc with CONGELADO_TEST defined. See
-- apply_test_target in xmake/common.lua.
apply_test_target({
	name = "postgres",
	layer = "postgres_plugin",
	core_packages = { "sqlgen" },
	deps = { "congelado_sdk" },
	files = { "src/**.cppm", "bin/**.cc" },
	remove = { "src/build.cc" },
})
-- Same libpq/GSSAPI gap as the production target above — reopen to add the syslink
-- apply_test_target() itself has no hook for.
if not is_plat("windows", "mingw") then
	target("postgres_test")
	add_syslinks("gssapi_krb5")
	target_end()
end
