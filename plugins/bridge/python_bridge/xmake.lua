-- Part of the merged plugins/ project — see plugins/xmake.lua. "cpython" package requirement is
-- configured once at the parent's setup_extra_requires() call.
target("python_bridge_plugin")
set_kind("shared")
apply_common_layer_settings({
	layer = "python_bridge_plugin",
	plugin_includedir = true,
	targetdir = shared_plugin_dir,
	core_packages = { "cpython" },
})
add_deps("congelado_sdk")
add_files("bin/**.cc")
remove_files("src/build.cc")
-- conan's cpython package doesn't always auto-link libpython3.12.so — explicit syslink, same
-- as the old inline discovery's per-target special case.
if not is_plat("windows", "mingw") then
	add_syslinks("python3.12")
end
target_end()

-- python_bridge_test: recompiles bin/**.cc with CONGELADO_TEST defined. See apply_test_target in
-- xmake/common.lua. apply_common_layer_settings's PACKAGE_SYSLINKS table already covers the
-- cpython -> python3.12 syslink automatically, same as the production target above.
apply_test_target({
	name = "python_bridge",
	layer = "python_bridge_plugin",
	core_packages = { "cpython" },
	deps = { "congelado_sdk" },
	files = { "bin/**.cc" },
	remove = { "src/build.cc" },
})
