-- Merged standalone project for every plugin, including otel_otlp: congelado_include/congelado_sdk
-- get declared exactly once here and inherited (xmake's own multi-level-directories tree config)
-- by each includes()'d child below, instead of each plugin's own separate xmake process
-- recompiling them from source. engine is included here too — see plugins/engine/xmake.lua's own
-- comment for why that's safe (the constraint that used to force it into its own project was
-- specifically about the ROOT project's scan, not about sharing with sibling plugins). otel_otlp
-- used to stay its own separate project (see git history) because its "opentelemetrycpp"
-- requirement transitively needs libcurl's conan recipe, which needs conan >=2.21.0 — that's now
-- this project's documented conan floor (see README.md), so the isolation is no longer needed.
set_project("congelado_plugins")
add_rules("mode.debug", "mode.release")

-- Global (no `local`), not scoped to this file: every includes()'d child below needs to read
-- these — same existing pattern core_layers.lua already uses for core_packages/
-- apply_common_layer_settings.
core_root = path.normalize(path.join(os.projectdir(), ".."))
shared_plugin_dir = path.join(core_root, "build", "plugins")

includes(path.join(core_root, "xmake/core_layers.lua"))
setup_core_layers(core_root)
-- Union of every child's own extras (previously each configured separately in its own standalone
-- project) — configured once here since add_requires() must run in this project's single
-- description-domain pass, before any includes(). "opentelemetrycpp" is otel_otlp_plugin's own
-- requirement, folded in here now that this project's conan floor covers the libcurl recipe it
-- transitively needs (see README.md).
setup_extra_requires({ "cpython", "lua", "opentelemetrycpp", "libcurl", "rabbitmqc", "librdkafka", "hiredis" })

-- serde/ groups every ISerdeFormat-implementing plugin (json, toml, sql_postgres); bridge/
-- groups every IBridge-implementing plugin (python_bridge, lua_bridge) — same target names as
-- before (json_plugin, toml_plugin, sql_postgres_plugin, python_bridge_plugin,
-- lua_bridge_plugin), just moved a directory level deeper for grouping. Neither subfolder needs
-- its own xmake.lua: includes() just needs the right relative path to each target's own file.
includes("serde/json")
includes("openapi_generator")
includes("engine")
includes("http2")
includes("file_logger")
includes("serde/toml")
includes("postgres")
includes("search/elasticsearch")
includes("events/memory")
includes("events/rabbitmq")
includes("events/kafka")
includes("events/redis")
includes("serde/sql_postgres")
includes("bridge/python_bridge")
includes("bridge/lua_bridge")
includes("otel_otlp")
