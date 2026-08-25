-- Part of the merged plugins/ project — see plugins/xmake.lua. A shared, reusable LLM worker
-- (IWorker) — OpenAI-compatible chat completions via libcurl, the counterpart to Conductor's LLM
-- system tasks. Built into build/workers. Impl in src/llm_worker.cppm; the plugin's bin/*.cc holds
-- only the Plugin class.
target("llm_worker_plugin")
set_kind("shared")
apply_common_layer_settings({
	layer = "llm_worker_plugin",
	plugin_includedir = true,
	targetdir = path.join(core_root, "build", "workers"),
})
add_deps("congelado_sdk")
add_packages("libcurl")
add_files("src/**.cppm")
add_files("bin/**.cc")
target_end()

-- llm_worker_test: recompiles src/**.cppm + bin/**.cc with CONGELADO_TEST defined. See
-- apply_test_target in xmake/common.lua.
apply_test_target({
	name = "llm_worker",
	layer = "llm_worker_plugin",
	core_packages = { "libcurl" },
	deps = { "congelado_sdk" },
	files = { "src/**.cppm", "bin/**.cc" },
})
