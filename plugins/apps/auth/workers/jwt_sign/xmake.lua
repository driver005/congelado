-- Part of the merged plugins/ project — see plugins/xmake.lua. An auth-app task worker (IWorker),
-- built into build/workers alongside the other worker plugins the worker host scans.
target("auth_jwt_sign_worker_plugin")
set_kind("shared")
apply_common_layer_settings({
	layer = "auth_jwt_sign_worker_plugin",
	plugin_includedir = true,
	targetdir = path.join(core_root, "build", "workers"),
})
add_deps("congelado_sdk")
add_files("bin/**.cc")
-- Stage the auth app's def files (taskdefs/ + workflows/, two dirs up) into build/apps/auth so the
-- worker host finds and registers them on load. Attached to a real target's after_build so it
-- actually runs (a no-op phony target's build hooks don't fire reliably).
after_build(function(target)
	local app_dir = path.join(target:scriptdir(), "..", "..")
	local outdir = path.join(os.projectdir(), "build", "apps", "auth")
	os.mkdir(outdir)
	os.cp(path.join(app_dir, "taskdefs"), outdir)
	os.cp(path.join(app_dir, "workflows"), outdir)
end)
target_end()

-- auth_jwt_sign_worker_test: recompiles bin/**.cc with CONGELADO_TEST defined. See
-- apply_test_target in xmake/common.lua.
apply_test_target({
	name = "auth_jwt_sign_worker",
	layer = "auth_jwt_sign_worker_plugin",
	deps = { "congelado_sdk" },
	files = { "bin/**.cc" },
})
