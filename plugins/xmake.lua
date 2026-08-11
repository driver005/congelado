-- Included from root xmake.lua (`includes("plugins")`), NOT a separate project — every plugin
-- target below inherits the root's toolchain/set_languages/build.c++.modules policy/cxflags, and
-- can add_deps("congelado_sdk") since that target is a sibling in the same tree (xmake's own
-- "Multi-level Directories" inheritance model). engine and otel_otlp used to need their own
-- separate xmake process (see git history) — engine because congelado_worker's scan couldn't see
-- an `import` of its generated client SDK before build.cc wrote it (a ROOT-project-scan
-- constraint, not about sharing with sibling plugins), otel_otlp because "opentelemetrycpp"
-- transitively needs libcurl's conan recipe, which needs conan >=2.21.0 (now this repo's
-- documented floor, see README.md) — neither isolation reason applies to plain inclusion here.
shared_plugin_dir = path.join(core_root, "build", "plugins")

-- "Extra" packages: not part of core_packages (congelado_include/congelado_sdk never link
-- these), each used by only one or two specific plugins here (otel_otlp_plugin, the
-- python/lua bridge plugins, kafka/rabbitmq/redis events plugins). cpython/lua are already
-- add_requires()'d by root xmake.lua (same alias/version) — not repeated here.
local conan = build_conan_settings()
add_requires("conan::opentelemetry-cpp/1.26.0", {
	alias = "opentelemetrycpp",
	configs = {
		settings_build = conan.settings_build,
		settings = conan.settings,
		conf = conan.conf,
		build = "missing",
		options = {
			"opentelemetry-cpp/*:with_otlp_grpc=False",
			"opentelemetry-cpp/*:with_otlp_http=True",
		},
	},
})
add_requires("conan::libcurl/8.15.0", { alias = "libcurl", configs = conan })
add_requires("conan::rabbitmq-c/0.15.0", { alias = "rabbitmqc", configs = conan })
add_requires("conan::librdkafka/2.14.2", { alias = "librdkafka", configs = conan })
add_requires("conan::hiredis/1.3.0", { alias = "hiredis", configs = conan })

-- serde/ groups every ISerdeFormat-implementing plugin (json, toml, sql_postgres); bridge/
-- groups every IBridge-implementing plugin (python_bridge, lua_bridge) — same target names as
-- before (json_plugin, toml_plugin, sql_postgres_plugin, python_bridge_plugin,
-- lua_bridge_plugin), just moved a directory level deeper for grouping. Neither subfolder needs
-- its own xmake.lua: includes() just needs the right relative path to each target's own file.
includes("serde/json")
includes("generator/openapi_generator")
includes("engine")
includes("protocol/http2")
includes("logger/file_logger")
includes("serde/toml")
includes("database/postgres")
includes("search/elasticsearch")
includes("events/memory")
includes("events/rabbitmq")
includes("events/kafka")
includes("cache/redis")
includes("serde/sql_postgres")
includes("bridge/python_bridge")
includes("bridge/lua_bridge")
includes("extensions/otel_otlp")
