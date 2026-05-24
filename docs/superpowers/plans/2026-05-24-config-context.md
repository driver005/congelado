# Config & Context Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Add a typed, file-backed `core::config::Config` struct hierarchy (TOML + JSON) that is loaded once at startup and scoped down to `PluginConfigView` when passed to plugins.

**Architecture:** Three new module partitions under `core_config` (types, loader, re-export root). The loader detects format by extension, parses into `Config` structs, and returns `std::expected<Config, std::string>`. Plugins receive a nullable `PluginConfigView*` (parallel C-string arrays) in `on_load()` — no access to host internals.

**Tech Stack:** C++26 modules, tomlplusplus 3.4.0 (Conan), simdjson 4.2.4 (already present), Catch2 3.7.1 (already present), xmake build system.

---

### Task 1: Build — add tomlplusplus + test target

**Files:**
- Modify: `xmake.lua`

- [ ] **Step 1: Add tomlplusplus Conan require**

In `xmake.lua`, after line 86 (`add_requires("conan::libffi/3.4.4"...)`), add:

```lua
add_requires("conan::tomlplusplus/3.4.0", { alias = "tomlplusplus", configs = conan })
```

- [ ] **Step 2: Add tomlplusplus to congelado_lib packages**

In the `add_packages(...)` call inside `target("congelado_lib")` (lines 152–167), add `"tomlplusplus"` to the list:

```lua
add_packages(
    "fmt",
    "simdjson",
    "tomlplusplus",
    "grpc",
    "protobuf",
    "asio",
    "openssl",
    "nghttp2",
    "ngtcp2",
    "nghttp3",
    "backward",
    "libffi",
    "microsoft-gsl",
    "range-v3",
    { public = true }
)
```

- [ ] **Step 3: Add config test target**

After the commented-out test block at the bottom of `xmake.lua` (after line 214), add:

```lua
target("config_test")
    set_kind("binary")
    add_files("tests/core/config/config_test.cc")
    add_packages("catch2")
    add_deps("congelado_lib")
    add_cxflags("-fpermissive")
    add_tests("default")
target_end()
```

- [ ] **Step 4: Install tomlplusplus via Conan**

```bash
cd /home/default/cc/congelado
xmake f --yes 2>&1 | tail -20
```

Expected: Conan installs `tomlplusplus/3.4.0`, no errors.

- [ ] **Step 5: Commit**

```bash
git add xmake.lua
git commit -m "build: add tomlplusplus 3.4.0 dependency and config test target"
```

---

### Task 2: types.cppm — Config struct hierarchy

**Files:**
- Create: `include/core/config/types.cppm`
- Create: `tests/core/config/config_test.cc`

- [ ] **Step 1: Write the failing test**

Create `tests/core/config/config_test.cc`:

```cpp
#include <catch2/catch_test_macros.hpp>
#include <filesystem>
#include <fstream>
#include <string>
import core_config;

TEST_CASE("Config default values") {
    core::config::Config cfg{};

    CHECK(cfg.server.host == "localhost");
    CHECK(cfg.server.port == 8080);
    CHECK(cfg.server.threads == 1);
    CHECK(cfg.server.max_connections == 1024);
    CHECK(cfg.server.timeout_ms == 30'000);
    CHECK(cfg.server.tls.cert == "server.crt");
    CHECK(cfg.server.tls.key == "server.key");
    CHECK(cfg.logger.file == "app.log");
    CHECK(cfg.logger.level == "info");
    CHECK(cfg.plugins.empty());
}
```

- [ ] **Step 2: Run test to verify it fails**

```bash
xmake build config_test 2>&1 | tail -20
```

Expected: compile error — `core_config` module not found.

- [ ] **Step 3: Create types.cppm**

Create `include/core/config/types.cppm`:

```cpp
export module core_config:types;

import std;

export namespace core::config {

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
    std::string type;
    std::unordered_map<std::string, std::string> fields;
};

struct Config {
    ServerConfig server;
    LoggerConfig logger;
    std::unordered_map<std::string, PluginConfig> plugins;
};

} // namespace core::config
```

- [ ] **Step 4: Create config.cppm (root re-export)**

Create `include/core/config/config.cppm`:

```cpp
export module core_config;

export import :types;
export import :loader;
```

- [ ] **Step 5: Create loader.cppm stub** (so the root module compiles before Task 4 fills it in)

Create `include/core/config/loader.cppm`:

```cpp
export module core_config:loader;

import std;
import :types;

export namespace core::config {

[[nodiscard]] std::expected<Config, std::string>
load(const std::filesystem::path &path = {});

} // namespace core::config
```

- [ ] **Step 6: Run test to verify it passes**

```bash
xmake build config_test && xmake test config_test 2>&1 | tail -20
```

Expected: `Config default values` PASSED.

- [ ] **Step 7: Commit**

```bash
git add include/core/config/types.cppm include/core/config/config.cppm \
        include/core/config/loader.cppm tests/core/config/config_test.cc
git commit -m "feat(config): add Config struct hierarchy and module scaffold"
```

---

### Task 3: loader.cppm — empty path returns defaults

**Files:**
- Modify: `include/core/config/loader.cppm`

- [ ] **Step 1: Write the failing test**

Add to `tests/core/config/config_test.cc` (inside the file, after the previous test):

```cpp
TEST_CASE("load empty path returns defaults") {
    auto result = core::config::load({});

    REQUIRE(result.has_value());
    CHECK(result->server.host == "localhost");
    CHECK(result->server.port == 8080);
    CHECK(result->server.threads == 1);
    CHECK(result->server.max_connections == 1024);
    CHECK(result->server.timeout_ms == 30'000);
    CHECK(result->server.tls.cert == "server.crt");
    CHECK(result->server.tls.key == "server.key");
    CHECK(result->logger.file == "app.log");
    CHECK(result->logger.level == "info");
    CHECK(result->plugins.empty());
}
```

- [ ] **Step 2: Run test to verify it fails**

```bash
xmake build config_test && xmake test config_test 2>&1 | tail -20
```

Expected: linker error — `load` defined but body not implemented (undefined reference or link failure).

- [ ] **Step 3: Implement load() with empty-path case**

Replace `include/core/config/loader.cppm` entirely:

```cpp
module;

#include <toml++/toml.hpp>
#include <simdjson.h>

export module core_config:loader;

import std;
import :types;

namespace core::config {

static std::expected<Config, std::string>
parse_toml(const std::filesystem::path &path);

static std::expected<Config, std::string>
parse_json(const std::filesystem::path &path);

} // namespace core::config

export namespace core::config {

[[nodiscard]] std::expected<Config, std::string>
load(const std::filesystem::path &path) {
    if (path.empty()) return Config{};

    auto ext = path.extension().string();
    if (ext == ".toml") return parse_toml(path);
    if (ext == ".json") return parse_json(path);
    return std::unexpected(std::format("unknown config extension '{}': use .toml or .json", ext));
}

} // namespace core::config

namespace core::config {

static std::expected<Config, std::string>
parse_toml(const std::filesystem::path &path) {
    return std::unexpected("TOML parser not yet implemented");
}

static std::expected<Config, std::string>
parse_json(const std::filesystem::path &path) {
    return std::unexpected("JSON parser not yet implemented");
}

} // namespace core::config
```

- [ ] **Step 4: Run test to verify it passes**

```bash
xmake build config_test && xmake test config_test 2>&1 | tail -20
```

Expected: `load empty path returns defaults` PASSED.

- [ ] **Step 5: Commit**

```bash
git add include/core/config/loader.cppm tests/core/config/config_test.cc
git commit -m "feat(config): implement load() with empty-path default and extension dispatch"
```

---

### Task 4: loader.cppm — TOML parsing

**Files:**
- Modify: `include/core/config/loader.cppm`
- Modify: `tests/core/config/config_test.cc`

- [ ] **Step 1: Write the failing test**

Add to `tests/core/config/config_test.cc`:

```cpp
static std::filesystem::path write_tmp(const std::string &ext, const std::string &content) {
    auto p = std::filesystem::temp_directory_path() / ("congelado_test." + ext);
    std::ofstream{p} << content;
    return p;
}

TEST_CASE("load TOML parses server and logger fields") {
    auto path = write_tmp("toml", R"(
[server]
host            = "example.com"
port            = 9090
threads         = 4
max_connections = 512
timeout_ms      = 5000

[server.tls]
cert = "my.crt"
key  = "my.key"

[logger]
file  = "test.log"
level = "debug"
)");

    auto result = core::config::load(path);
    std::filesystem::remove(path);

    REQUIRE(result.has_value());
    CHECK(result->server.host            == "example.com");
    CHECK(result->server.port            == 9090);
    CHECK(result->server.threads         == 4);
    CHECK(result->server.max_connections == 512);
    CHECK(result->server.timeout_ms      == 5000);
    CHECK(result->server.tls.cert        == "my.crt");
    CHECK(result->server.tls.key         == "my.key");
    CHECK(result->logger.file            == "test.log");
    CHECK(result->logger.level           == "debug");
}

TEST_CASE("load TOML wrong type returns error") {
    auto path = write_tmp("toml", "[server]\nport = \"not_a_number\"\n");
    auto result = core::config::load(path);
    std::filesystem::remove(path);

    REQUIRE_FALSE(result.has_value());
    CHECK(result.error().find("server.port") != std::string::npos);
}
```

- [ ] **Step 2: Run test to verify it fails**

```bash
xmake build config_test && xmake test config_test 2>&1 | tail -20
```

Expected: `load TOML parses server and logger fields` FAILS with "TOML parser not yet implemented".

- [ ] **Step 3: Implement parse_toml**

Replace the `parse_toml` stub in `include/core/config/loader.cppm`:

```cpp
static std::expected<Config, std::string>
parse_toml(const std::filesystem::path &path) {
    Config cfg{};

    toml::table tbl;
    try {
        tbl = toml::parse_file(path.string());
    } catch (const toml::parse_error &e) {
        return std::unexpected(std::format("TOML parse error in '{}': {}", path.string(), e.what()));
    }

    // helper: check a node exists and is the right type, or return error
    auto require_int = [](toml::node_view<const toml::node> node,
                          const char *field_name,
                          std::uint32_t &out) -> std::optional<std::string> {
        if (!node) return std::nullopt;
        if (auto v = node.value<std::int64_t>()) { out = static_cast<std::uint32_t>(*v); return std::nullopt; }
        return std::string{field_name} + ": expected integer";
    };

    auto require_u16 = [](toml::node_view<const toml::node> node,
                          const char *field_name,
                          std::uint16_t &out) -> std::optional<std::string> {
        if (!node) return std::nullopt;
        if (auto v = node.value<std::int64_t>()) { out = static_cast<std::uint16_t>(*v); return std::nullopt; }
        return std::string{field_name} + ": expected integer";
    };

    // --- server ---
    if (auto sv = tbl["server"]["host"].value<std::string>()) cfg.server.host = *sv;
    if (auto e = require_u16(tbl["server"]["port"],            "server.port",            cfg.server.port))            return std::unexpected(*e);
    if (auto e = require_int(tbl["server"]["threads"],         "server.threads",         cfg.server.threads))         return std::unexpected(*e);
    if (auto e = require_int(tbl["server"]["max_connections"], "server.max_connections", cfg.server.max_connections)) return std::unexpected(*e);
    if (auto e = require_int(tbl["server"]["timeout_ms"],      "server.timeout_ms",      cfg.server.timeout_ms))      return std::unexpected(*e);
    if (auto sv = tbl["server"]["tls"]["cert"].value<std::string>()) cfg.server.tls.cert = *sv;
    if (auto sv = tbl["server"]["tls"]["key"].value<std::string>())  cfg.server.tls.key  = *sv;

    // --- logger ---
    if (auto sv = tbl["logger"]["file"].value<std::string>())  cfg.logger.file  = *sv;
    if (auto sv = tbl["logger"]["level"].value<std::string>()) cfg.logger.level = *sv;

    // --- plugins ---
    if (auto *plugins = tbl["plugins"].as_table()) {
        for (auto &[name, section] : *plugins) {
            auto *sec = section.as_table();
            if (!sec) continue;

            PluginConfig pc;
            pc.name = std::string{name.str()};

            for (auto &[k, v] : *sec) {
                if (auto sv = v.value<std::string>()) {
                    std::string key{k.str()};
                    if (key == "type") pc.type = *sv;
                    pc.fields[std::move(key)] = *sv;
                }
                // non-string fields in plugin sections silently ignored
            }

            cfg.plugins[pc.name] = std::move(pc);
        }
    }

    return cfg;
}
```

- [ ] **Step 4: Run tests to verify they pass**

```bash
xmake build config_test && xmake test config_test 2>&1 | tail -20
```

Expected: all TOML tests PASSED.

- [ ] **Step 5: Commit**

```bash
git add include/core/config/loader.cppm tests/core/config/config_test.cc
git commit -m "feat(config): implement TOML parser with type validation"
```

---

### Task 5: loader.cppm — JSON parsing

**Files:**
- Modify: `include/core/config/loader.cppm`
- Modify: `tests/core/config/config_test.cc`

- [ ] **Step 1: Write the failing test**

Add to `tests/core/config/config_test.cc`:

```cpp
TEST_CASE("load JSON parses server and logger fields") {
    auto path = write_tmp("json", R"({
  "server": {
    "host": "json-host.com",
    "port": 7070,
    "threads": 2,
    "max_connections": 256,
    "timeout_ms": 10000,
    "tls": { "cert": "j.crt", "key": "j.key" }
  },
  "logger": { "file": "json.log", "level": "warn" }
})");

    auto result = core::config::load(path);
    std::filesystem::remove(path);

    REQUIRE(result.has_value());
    CHECK(result->server.host            == "json-host.com");
    CHECK(result->server.port            == 7070);
    CHECK(result->server.threads         == 2);
    CHECK(result->server.max_connections == 256);
    CHECK(result->server.timeout_ms      == 10000);
    CHECK(result->server.tls.cert        == "j.crt");
    CHECK(result->server.tls.key         == "j.key");
    CHECK(result->logger.file            == "json.log");
    CHECK(result->logger.level           == "warn");
}

TEST_CASE("load JSON wrong type returns error") {
    auto path = write_tmp("json", R"({"server":{"port":"not_a_number"}})");
    auto result = core::config::load(path);
    std::filesystem::remove(path);

    REQUIRE_FALSE(result.has_value());
    CHECK(result.error().find("server.port") != std::string::npos);
}

TEST_CASE("load JSON with plugins section") {
    auto path = write_tmp("json", R"({
  "plugins": {
    "file_logger": { "type": "logger", "file": "app.log" },
    "my_client":   { "type": "client", "encryption_key": "abc123" }
  }
})");

    auto result = core::config::load(path);
    std::filesystem::remove(path);

    REQUIRE(result.has_value());
    REQUIRE(result->plugins.contains("file_logger"));
    CHECK(result->plugins.at("file_logger").type == "logger");
    CHECK(result->plugins.at("file_logger").fields.at("file") == "app.log");
    REQUIRE(result->plugins.contains("my_client"));
    CHECK(result->plugins.at("my_client").type == "client");
    CHECK(result->plugins.at("my_client").fields.at("encryption_key") == "abc123");
}

TEST_CASE("load TOML with plugins section") {
    auto path = write_tmp("toml", R"(
[plugins.file_logger]
type = "logger"
file = "app.log"

[plugins.my_client]
type           = "client"
encryption_key = "abc123"
)");

    auto result = core::config::load(path);
    std::filesystem::remove(path);

    REQUIRE(result.has_value());
    REQUIRE(result->plugins.contains("file_logger"));
    CHECK(result->plugins.at("file_logger").type == "logger");
    CHECK(result->plugins.at("file_logger").fields.at("file") == "app.log");
    REQUIRE(result->plugins.contains("my_client"));
    CHECK(result->plugins.at("my_client").fields.at("encryption_key") == "abc123");
}

TEST_CASE("load unknown extension returns error") {
    auto result = core::config::load("config.xml");
    REQUIRE_FALSE(result.has_value());
    CHECK(result.error().find(".xml") != std::string::npos);
}
```

- [ ] **Step 2: Run test to verify JSON tests fail**

```bash
xmake build config_test && xmake test config_test 2>&1 | tail -20
```

Expected: JSON tests FAIL with "JSON parser not yet implemented". TOML + plugin tests pass.

- [ ] **Step 3: Implement parse_json**

Replace the `parse_json` stub in `include/core/config/loader.cppm`:

```cpp
static std::expected<Config, std::string>
parse_json(const std::filesystem::path &path) {
    Config cfg{};

    simdjson::dom::parser parser;
    simdjson::dom::element doc;
    if (auto ec = parser.load(path.string()).get(doc); ec)
        return std::unexpected(std::format("JSON parse error in '{}': {}", path.string(), simdjson::error_message(ec)));

    // helper: get uint field, return error message if wrong type
    auto get_uint = [](simdjson::dom::element obj, const char *key,
                       std::uint64_t &out) -> std::optional<std::string> {
        simdjson::dom::element val;
        auto ec = obj[key].get(val);
        if (ec == simdjson::NO_SUCH_FIELD) return std::nullopt;
        if (ec) return std::nullopt; // other lookup error — skip
        std::uint64_t v;
        if (val.get(v) == simdjson::INCORRECT_TYPE)
            return std::string{key} + ": expected integer";
        if (!val.get(v)) out = v;
        return std::nullopt;
    };

    // --- server ---
    simdjson::dom::element server;
    if (!doc["server"].get(server)) {
        std::string_view host;
        if (!server["host"].get_string().get(host)) cfg.server.host = std::string{host};

        std::uint64_t tmp = 0;
        if (auto e = get_uint(server, "port",            tmp)) return std::unexpected(*e);
        if (tmp) cfg.server.port = static_cast<std::uint16_t>(tmp);

        tmp = 0;
        if (auto e = get_uint(server, "threads",         tmp)) return std::unexpected(*e);
        if (tmp) cfg.server.threads = static_cast<std::uint32_t>(tmp);

        tmp = 0;
        if (auto e = get_uint(server, "max_connections", tmp)) return std::unexpected(*e);
        if (tmp) cfg.server.max_connections = static_cast<std::uint32_t>(tmp);

        tmp = 0;
        if (auto e = get_uint(server, "timeout_ms",      tmp)) return std::unexpected(*e);
        if (tmp) cfg.server.timeout_ms = static_cast<std::uint32_t>(tmp);

        simdjson::dom::element tls;
        if (!server["tls"].get(tls)) {
            std::string_view cert, key;
            if (!tls["cert"].get_string().get(cert)) cfg.server.tls.cert = std::string{cert};
            if (!tls["key"].get_string().get(key))   cfg.server.tls.key  = std::string{key};
        }
    }

    // --- logger ---
    simdjson::dom::element logger;
    if (!doc["logger"].get(logger)) {
        std::string_view file, level;
        if (!logger["file"].get_string().get(file))   cfg.logger.file  = std::string{file};
        if (!logger["level"].get_string().get(level)) cfg.logger.level = std::string{level};
    }

    // --- plugins ---
    simdjson::dom::element plugins_el;
    if (!doc["plugins"].get(plugins_el)) {
        simdjson::dom::object plugins_obj;
        if (!plugins_el.get(plugins_obj)) {
            for (auto [pname, psection] : plugins_obj) {
                simdjson::dom::object section_obj;
                if (psection.get(section_obj)) continue;

                PluginConfig pc;
                pc.name = std::string{pname};

                for (auto [k, v] : section_obj) {
                    std::string_view sv;
                    if (!v.get_string().get(sv)) {
                        std::string key{k};
                        if (key == "type") pc.type = std::string{sv};
                        pc.fields[std::move(key)] = std::string{sv};
                    }
                    // non-string plugin fields silently ignored
                }

                cfg.plugins[pc.name] = std::move(pc);
            }
        }
    }

    return cfg;
}
```

- [ ] **Step 4: Run all tests to verify they pass**

```bash
xmake build config_test && xmake test config_test 2>&1 | tail -30
```

Expected: all tests PASSED.

- [ ] **Step 5: Commit**

```bash
git add include/core/config/loader.cppm tests/core/config/config_test.cc
git commit -m "feat(config): implement JSON parser and plugin section parsing"
```

---

### Task 6: plugin_api.h — add PluginConfigView, update on_load

**Files:**
- Modify: `include/core/ffi/plugin_api.h`
- Modify: `plugins/file_logger/file_logger.cc`

- [ ] **Step 1: Add PluginConfigView to plugin_api.h**

In `include/core/ffi/plugin_api.h`, after the `HostCallbacks` struct (after line 24) and before `IPluginHandler`, add:

```cpp
// Scoped, read-only view of one plugin's config section.
// Lifetime: valid only for the duration of on_load(). Do not store pointers.
struct PluginConfigView {
    const char * const *keys;
    const char * const *values;
    std::size_t         count;
};
```

- [ ] **Step 2: Update on_load signature in IPluginHandler**

In `include/core/ffi/plugin_api.h`, change line 41:

```cpp
// Before:
virtual void on_load(const HostCallbacks &) {}

// After:
virtual void on_load(const HostCallbacks &, const PluginConfigView *) {}
```

- [ ] **Step 3: Update file_logger.cc to match new signature**

In `plugins/file_logger/file_logger.cc`, change line 30:

```cpp
// Before:
void on_load(const core::ffi::HostCallbacks &host) override {
    host_    = host;
    stream_.open("app.log", std::ios::app);
}

// After:
void on_load(const core::ffi::HostCallbacks &host,
             const core::ffi::PluginConfigView *cfg) override {
    host_ = host;
    const char *log_file = "app.log";
    if (cfg) {
        for (std::size_t i = 0; i < cfg->count; ++i) {
            if (std::string_view{cfg->keys[i]} == "file") {
                log_file = cfg->values[i];
                break;
            }
        }
    }
    stream_.open(log_file, std::ios::app);
}
```

- [ ] **Step 4: Build to verify it compiles**

```bash
xmake build congelado_lib file_logger 2>&1 | tail -20
```

Expected: builds cleanly, no errors.

- [ ] **Step 5: Commit**

```bash
git add include/core/ffi/plugin_api.h plugins/file_logger/file_logger.cc
git commit -m "feat(config): add PluginConfigView and update on_load signature"
```

---

### Task 7: bridge.cppm — pass PluginConfig to on_load

**Files:**
- Modify: `include/core/ffi/bridge.cppm`

- [ ] **Step 1: Add core_config import to bridge.cppm**

In `include/core/ffi/bridge.cppm`, after `import shared;` (line 15), add:

```cpp
import core_config;
```

- [ ] **Step 2: Update FfiBridge::load() signature**

In `include/core/ffi/bridge.cppm`, change the `load()` signature (line 47):

```cpp
// Before:
[[nodiscard]] static std::expected<std::shared_ptr<FfiBridge>, LoadError>
load(const std::filesystem::path &path) {

// After:
[[nodiscard]] static std::expected<std::shared_ptr<FfiBridge>, LoadError>
load(const std::filesystem::path &path,
     const core::config::PluginConfig *plugin_cfg = nullptr) {
```

- [ ] **Step 3: Build PluginConfigView and pass to on_load**

In `include/core/ffi/bridge.cppm`, replace the two lines (currently around line 80–81):

```cpp
// Before:
HostCallbacks callbacks = bridge->make_host_callbacks();
bridge->handler_->on_load(callbacks);

// After:
HostCallbacks callbacks = bridge->make_host_callbacks();

std::vector<const char *> pcv_keys, pcv_vals;
if (plugin_cfg) {
    for (auto &[k, v] : plugin_cfg->fields) {
        pcv_keys.push_back(k.c_str());
        pcv_vals.push_back(v.c_str());
    }
}
PluginConfigView pcv{
    plugin_cfg ? pcv_keys.data() : nullptr,
    plugin_cfg ? pcv_vals.data() : nullptr,
    plugin_cfg ? pcv_keys.size() : 0,
};
bridge->handler_->on_load(callbacks, plugin_cfg ? &pcv : nullptr);
```

- [ ] **Step 4: Update loader.cppm in core_plugin to forward PluginConfig**

In `include/core/manager/loader.cppm`, add the import and update `load()`:

```cpp
export module core_plugin:loader;

import std;
import core_ffi;
import core_config;
import :handle;

export namespace core::plugin {

[[nodiscard]]
inline std::expected<PluginHandle, LoadError>
load(const std::filesystem::path &path,
     const core::config::PluginConfig *plugin_cfg = nullptr) {
    auto result = core::ffi::FfiBridge::load(path, plugin_cfg);
    if (!result)
        return std::unexpected(std::move(result.error()));
    return std::move(*result);
}

} // namespace core::plugin
```

- [ ] **Step 5: Build to verify it compiles**

```bash
xmake build congelado_lib file_logger 2>&1 | tail -20
```

Expected: builds cleanly.

- [ ] **Step 6: Commit**

```bash
git add include/core/ffi/bridge.cppm include/core/manager/loader.cppm
git commit -m "feat(config): bridge passes PluginConfigView to on_load"
```

---

### Task 8: congelado.cppm + main.cc — wire Config end-to-end

**Files:**
- Modify: `include/congelado.cppm`
- Modify: `src/main.cc`

- [ ] **Step 1: Update Server to accept Config**

In `include/congelado.cppm`, add the import and update `Server`:

After `import hashmap;` add:
```cpp
import core_config;
```

Change the `Server` class constructor and `make_socket_flow()` to use `ServerConfig`:

```cpp
class Server {
  public:
    explicit Server(const core::config::Config &cfg = {})
        : m_config{cfg}, m_contract_group{}, m_thread_pool{m_contract_group, m_config.server.threads},
          m_leverager{}, m_table{}, m_socket_flow{make_socket_flow()} {}

  private:
    inline io::base::flow::sync::FlowSocket<core::contract::ContractGroup<>, io::base::socket::Protocol::TLS>
    make_socket_flow() {
        printf("Hello, Congelado!\n");
        io::base::flow::sync::FlowSocket<core::contract::ContractGroup<>, io::base::socket::Protocol::TLS> flow{
            io::base::socket::Endpoint{m_config.server.host, m_config.server.port},
            m_leverager, m_contract_group};
        flow.add_on_accept([&](shared::SendCallback send, shared::CloseCallback close) -> shared::ReadCallback {
            std::println("New connection accepted, creating HTTP/2 flow");
            return m_table.emplace_back(std::make_unique<io::layer::http2::Flow>(std::move(send), std::move(close)))
                ->on_read();
        });
        flow.build();
        return flow;
    }

    // m_config must be declared first — used in ContractThreadPool init and make_socket_flow()
    core::config::Config                m_config;
    core::contract::ContractGroup<>     m_contract_group;
    core::contract::ContractThreadPool<> m_thread_pool;
    io::base::leverage::Leverager<io::base::leverage::Context> m_leverager;
    std::deque<std::unique_ptr<io::layer::http2::Flow>> m_table;
    io::base::flow::sync::FlowSocket<core::contract::ContractGroup<>, io::base::socket::Protocol::TLS> m_socket_flow;
};
```

- [ ] **Step 2: Update main.cc to load config and pass to Server + plugins**

Replace `src/main.cc` entirely:

```cpp
#include "backward.hpp"
#include <stdio.h>

import std;
import core_logger;
import core_plugin;
import core_config;
import congelado;
import core_server;

int main(int argc, char *argv[]) {
    backward::SignalHandling sh;

    // Load config — try "congelado.toml", fall back to all defaults if absent
    core::config::Config cfg_val{};
    if (std::filesystem::exists("congelado.toml")) {
        auto cfg_result = core::config::load("congelado.toml");
        if (!cfg_result) {
            std::println(stderr, "[main] config error: {}", cfg_result.error());
            return 1;
        }
        cfg_val = std::move(*cfg_result);
    }
    const auto &cfg = cfg_val;
    const auto &cfg = *cfg_result;

    // Try to load the file_logger plugin, passing its config section if present
    bool plugin_loaded = false;
    if (argc > 0) {
        auto plugin_dir = std::filesystem::path(argv[0]).parent_path();
#if defined(_WIN32)
        auto plugin_path = plugin_dir / "file_logger.dll";
#else
        auto plugin_path = plugin_dir / "libfile_logger.so";
#endif
        const core::config::PluginConfig *plugin_cfg = nullptr;
        if (cfg.plugins.contains("file_logger"))
            plugin_cfg = &cfg.plugins.at("file_logger");

        auto result = core::plugin::load(plugin_path, plugin_cfg);
        if (result) {
            auto logger = core::plugin::make_logger(*result);
            if (logger) {
                std::string init_resp = core::logger::LoggerRegistry::register_logger(std::move(logger));
                core::logger::info("LoggerRegistry", "Plugin logger ready. Handshake: {}", init_resp);
                plugin_loaded = true;
            } else {
                std::println(stderr, "[main] plugin '{}' has no ILogger capability", (*result)->name());
            }
        } else {
            std::println(stderr, "[main] plugin load failed: {}", result.error().detail);
        }
    }

    if (!plugin_loaded) {
        auto fallback  = std::make_shared<app::MyCustomFileLogger>(cfg.logger.file);
        auto init_resp = core::logger::LoggerRegistry::register_logger(fallback);
        core::logger::info("LoggerRegistry", "Fallback logger ready. Handshake: {}", init_resp);
    }

    app::Server server{cfg};

    std::promise<void>().get_future().wait();
    return 0;
}
```

- [ ] **Step 3: Build the full project**

```bash
xmake build congelado 2>&1 | tail -30
```

Expected: builds cleanly with no errors.

- [ ] **Step 4: Run config tests one more time to confirm nothing regressed**

```bash
xmake test config_test 2>&1 | tail -20
```

Expected: all tests PASSED.

- [ ] **Step 5: Smoke test the binary starts with defaults (no config file)**

```bash
timeout 3 ./build/linux/x86_64/debug/congelado 2>&1 || true
```

Expected: prints `Hello, Congelado!` and `New connection accepted...` or timeout after 3s — no crash, no missing-symbol errors.

- [ ] **Step 6: Commit**

```bash
git add include/congelado.cppm src/main.cc
git commit -m "feat(config): wire Config into Server and plugin loader end-to-end"
```
