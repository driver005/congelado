-- Part of the merged plugins/ project — see plugins/xmake.lua. Needs conan >=2.21.0 (this
-- project's documented floor, see README.md) for opentelemetry-cpp's transitive libcurl recipe.
target("otel_otlp_plugin")
set_kind("shared")
apply_common_layer_settings({
	layer = "otel_otlp_plugin",
	plugin_includedir = true,
	targetdir = shared_plugin_dir,
	core_packages = { "opentelemetrycpp" },
})
add_deps("congelado_sdk")
add_files("bin/**.cc")
remove_files("src/build.cc")
target_end()

-- otel_otlp_test: recompiles bin/**.cc with CONGELADO_TEST defined. See apply_test_target in
-- xmake/common.lua.
apply_test_target({
	name = "otel_otlp",
	layer = "otel_otlp_plugin",
	core_packages = { "opentelemetrycpp" },
	deps = { "congelado_sdk" },
	files = { "bin/**.cc" },
	remove = { "src/build.cc" },
})
