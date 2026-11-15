-- Part of the merged plugins/ project — see plugins/xmake.lua. The `client_pool` worker (IWorker) —
-- the polling counterpart to the `client` worker. Owns its own core::client::Register + client, built
-- via the host-injected interfaces::IProtocol::get_client(); retries are driven by the leverager's
-- timer, no thread blocking. Built into build/workers. Impl in src/client_pool_worker.cppm; the
-- plugin's bin/*.cc holds only the Plugin class. congelado_sdk already pulls in every
-- include/**.cppm module, so no extra deps.
target("client_pool_worker_plugin")
set_kind("shared")
apply_common_layer_settings({
	layer = "client_pool_worker_plugin",
	plugin_includedir = true,
	targetdir = path.join(core_root, "build", "workers"),
})
add_deps("congelado_sdk")
add_files("src/**.cppm")
add_files("bin/**.cc")
target_end()

-- client_pool_worker_test: recompiles src/**.cppm + bin/**.cc with CONGELADO_TEST defined. See
-- apply_test_target in xmake/common.lua.
apply_test_target({
	name = "client_pool_worker",
	layer = "client_pool_worker_plugin",
	deps = { "congelado_sdk" },
	files = { "src/**.cppm", "bin/**.cc" },
})
