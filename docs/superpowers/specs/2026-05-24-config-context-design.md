# Config & Context Design

**Date:** 2026-05-24
**Status:** Approved

## Overview

A typed, file-backed configuration system for congelado. Supports TOML (tomlplusplus) and JSON (simdjson), detected by file extension. Plugins receive a scoped, read-only view of only their own named section. All config lives under `include/core/config/`.

## Files

```
include/core/config/
├── config.cppm    — export module core_config; re-exports partitions
├── types.cppm     — core_config:types; all structs
└── loader.cppm    — core_config:loader; load() → Config
```

## Struct Hierarchy

```cpp
struct TlsConfig {
    std::string cert = "server.crt";
    std::string key  = "server.key";
};

struct ServerConfig {
    std::string   host            = "localhost";
    std::uint16_t port            = 8080;
    TlsConfig     tls             = {};
    std::uint32_t threads         = 1;
    std::uint32_t max_connections = 1024;
    std::uint32_t timeout_ms      = 30'000;
};

struct LoggerConfig {
    std::string file  = "app.log";
    std::string level = "info";
};

struct PluginConfig {
    std::string name;
    std::string type;  // "logger" | "server" | "client"
    std::unordered_map<std::string, std::string> fields;
};

struct Config {
    ServerConfig server;
    LoggerConfig logger;
    std::unordered_map<std::string, PluginConfig> plugins;
};
```

All fields carry defaults — `Config{}` is valid with no file present.

## Loader

```cpp
// core_config:loader
std::expected<Config, std::string> load(const std::filesystem::path& path = {});
```

- Empty path → returns `Config{}` (all defaults, no error)
- `.toml` extension → tomlplusplus parser
- `.json` extension → simdjson parser
- Unknown extension → error string
- Unknown keys in file → silently ignored (forward-compatible for future CLI layer)
- Wrong value type for a known key → error string with key name

## Config File Layout (TOML)

```toml
[server]
host        = "localhost"
port        = 8080
threads     = 2
max_connections = 1024
timeout_ms  = 30000

[server.tls]
cert = "server.crt"
key  = "server.key"

[logger]
file  = "app.log"
level = "info"

[plugins.file_logger]
type = "logger"
file = "app.log"

[plugins.my_client]
type           = "client"
encryption_key = "abc123"
```

JSON follows the same structure with equivalent keys.

## Plugin Integration

`plugin_api.h` gains a C-compatible view type (no module dependency):

```cpp
struct PluginConfigView {
    const char * const *keys;
    const char * const *values;
    std::size_t         count;
};
```

`IPluginHandler::on_load` signature becomes:

```cpp
virtual void on_load(const HostCallbacks&, const PluginConfigView*) {}
```

`PluginConfigView*` is nullable — plugin receives `nullptr` if it has no named section in the config. `FfiBridge::load()` gains a `const PluginConfig*` parameter; it builds a `PluginConfigView` from `PluginConfig::fields` (parallel key/value arrays alive for the duration of the `on_load` call only — plugin must not store the pointers) and passes it to `on_load`.

Plugin access is scoped to its own `fields` map — no visibility into `ServerConfig`, TLS keys, or other plugins' sections.

## Build Integration

Add to `xmake.lua`:

```lua
add_requires("tomlplusplus 3.4.0")
-- inside congelado_lib target:
add_packages("tomlplusplus")
```

simdjson 4.2.4 already present — no changes needed.

## Runtime Integration

`main.cc` loads config once at startup:

```cpp
auto cfg = core::config::load("congelado.toml");
if (!cfg) { std::println(stderr, "config error: {}", cfg.error()); return 1; }
app::Server server{ *cfg };
```

`app::Server` / `congelado.cppm` constructor gains a `Config` parameter. Currently hardcoded `localhost:8080` and TLS paths move to `ServerConfig`. Plugin manager reads `Config::plugins` and passes each plugin's `PluginConfig` by name to `FfiBridge::load()`.

## Error Handling

- File not found → descriptive error from `load()`
- Parse error → `std::unexpected` with key name + expected type
- Missing required field → none; all fields have defaults
- `level` string in `LoggerConfig` stored as-is; host does not validate. Plugin interprets it.
- `app::Server` constructor receives a valid `Config` — no error handling needed there

## Non-Goals

- CLI argument parsing (next step — designed to layer on top of `Config` structs)
- Config hot-reload at runtime
- Encrypted config fields
- Schema validation beyond type checking
