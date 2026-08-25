-- Part of the merged plugins/ project — see plugins/xmake.lua. The `client` worker (IWorker,
-- worker_type `http`) — issues a request to the host-configured downstream service using
-- core::client::Client + a Register it owns itself, built via the host-injected
-- interfaces::IProtocol::get_client(). No external HTTP library. Built into build/workers. Impl in
-- src/client_worker.cppm; the plugin's bin/*.cc holds only the Plugin class. congelado_sdk already
-- pulls in every include/**.cppm module (io_layer_http2, core_contract, etc.), so no extra deps.
target("client_worker_plugin")
set_kind("shared")
apply_common_layer_settings({
	layer = "client_worker_plugin",
	plugin_includedir = true,
	targetdir = path.join(core_root, "build", "workers"),
})
add_deps("congelado_sdk")
add_files("src/**.cppm")
add_files("bin/**.cc")
target_end()

-- client_worker_test: recompiles src/**.cppm + bin/**.cc with CONGELADO_TEST defined. See
-- apply_test_target in xmake/common.lua.
apply_test_target({
	name = "client_worker",
	layer = "client_worker_plugin",
	deps = { "congelado_sdk" },
	files = { "src/**.cppm", "bin/**.cc" },
})
