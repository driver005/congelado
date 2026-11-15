-- Part of the merged plugins/ project — see plugins/xmake.lua. The `events` worker (IWorker) —
-- publishes an event to the process event bus (core::events::publish); the host-wired IEventSink
-- plugins (kafka/rabbitmq/redis/memory) are the actual producers, so this worker links no broker
-- library. Built into build/workers. Impl in src/events_worker.cppm; the plugin's bin/*.cc holds
-- only the Plugin class.
target("events_worker_plugin")
set_kind("shared")
apply_common_layer_settings({
	layer = "events_worker_plugin",
	plugin_includedir = true,
	targetdir = path.join(core_root, "build", "workers"),
})
add_deps("congelado_sdk")
add_files("src/**.cppm")
add_files("bin/**.cc")
target_end()

-- events_worker_test: recompiles src/**.cppm + bin/**.cc with CONGELADO_TEST defined. See
-- apply_test_target in xmake/common.lua.
apply_test_target({
	name = "events_worker",
	layer = "events_worker_plugin",
	deps = { "congelado_sdk" },
	files = { "src/**.cppm", "bin/**.cc" },
})
