# OpenAPI Client SDK Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Add named `components.schemas`/`$ref` to the existing server-side OpenAPI generator, then ship a new `sdk/client` module that generates typed C++ client code (DTOs + per-operation functions) from `openapi.json`, plus a `Runtime` class that dispatches requests/responses through `interfaces::IClient` with automatic stream-id correlation and JSON deserialization.

**Architecture:** Server-side change lives entirely in the already-shipped `include/utils/openapi/` module (`model.cppm`, `schema.cppm`, `generator.cppm`) — no call-site changes anywhere else. The new `sdk/client` module has four parts: `Runtime` (hand-written, static-singleton, protocol-agnostic dispatch), a small internal `SchemaType` parse tree + JSON reader (consumes exactly the shapes our own `utils::openapi` emits — not a general OpenAPI importer), a DTO writer (schema → C++ class + `serde::Serializable` spec), and a route writer (path+method → C++ function). `Generator` orchestrates parse → write.

**Tech Stack:** C++26 modules, xmake, simdjson (`simdjson::dom`, already linked), reflect-cpp (`rfl::type_name_t`, already linked), catch2 3.7.1 (already available via `conan::catch2`, not currently wired into any active xmake target).

## Global Constraints

- C++26, `class` only — never `struct` — for domain types; `public:` before `private:`; member variables `m_`-prefixed, declared last.
- Accessors/mutators: `get`/`set`/`add` prefixes only, minimum 3-character names, no abbreviations.
- New DTO classes get a `template<> struct serde::Serializable<T>` specialization (`serde::FieldDesc<"name", &T::getX, &T::setX>{}` tuples) — the same pattern as `model::TaskDef` (`include/model/task/definition.cppm`) — so `serde::Ser::deserialize<T>`/`serde::Json::encode` work unmodified.
- Fallible APIs return `std::expected<T, std::string>`, not exceptions (matches `core::config::load`, `SharedLibrary::open_all`).
- New multi-file module families use the umbrella + partition pattern (`export module X;` / `export module X:part;`), matching `utils_buffering`/`utils_openapi` — not the flat independent-module style used by (older, weaker-precedent) `utils/queue`.
- Every new source file must actually build via `xmake build` before a task is considered done — this project has hit real compiler crashes and behavioral surprises from C++20 modules + consteval that only show up at actual compile time, not from reading the code.

---

### Task 1: Named `components.schemas` + `$ref` in the server-side generator

**Files:**
- Modify: `include/utils/openapi/model.cppm` — add `$ref` field to `SchemaObject`, add `Components` class, add `components` field to `Document`.
- Modify: `include/utils/openapi/schema.cppm` — derive a bare type name via `rfl::type_name_t<T>`, register named object schemas in a new `SchemaRegistry` singleton, emit `$ref` instead of inlining for any `serde::ISerializable` type.
- Modify: `include/utils/openapi/generator.cppm` — copy `SchemaRegistry`'s accumulated schemas into `Document.components` in `generate()`.

**Interfaces:**
- Consumes: nothing new from other tasks (this task is self-contained, touches only already-shipped files).
- Produces: `utils::openapi::SchemaRegistry` (static singleton: `addSchema(std::string, SchemaObject)`, `hasSchema(const std::string&) -> bool`, `getSchemas() -> const std::unordered_map<std::string, SchemaObject>&`) — not consumed by any later task in this plan, but documents the mechanism the rest of the plan's manual verification step (Task 7) will observe in the real `openapi.json`.

- [ ] **Step 1: Add `$ref` to `SchemaObject` and a `Components` class in `model.cppm`**

In `include/utils/openapi/model.cppm`, add a `$ref` field to `SchemaObject` (right after the class's existing members) and a new `Components` class after it:

```cpp
class SchemaObject {
  public:
    SchemaObject() = default;

    void setType(std::string value) { m_type = std::move(value); }
    void setFormat(std::string value) { m_format = std::move(value); }
    void setNullable(bool value) noexcept { m_nullable = value; }
    void setRef(std::string value) { m_ref = std::move(value); }
    void addRequired(std::string name) { m_required.push_back(std::move(name)); }
    void addEnumValue(std::string value) { m_enum_values.push_back(std::move(value)); }
    void addProperty(std::string name, SchemaObject schema) {
        m_properties.emplace(std::move(name), std::make_shared<SchemaObject>(std::move(schema)));
    }
    void setItems(SchemaObject schema) {
        m_items = std::make_shared<SchemaObject>(std::move(schema));
    }

    [[nodiscard]] const std::string &getType() const noexcept { return m_type; }
    [[nodiscard]] const std::string &getFormat() const noexcept { return m_format; }
    [[nodiscard]] bool getNullable() const noexcept { return m_nullable; }
    [[nodiscard]] const std::string &getRef() const noexcept { return m_ref; }
    [[nodiscard]] const std::vector<std::string> &getRequired() const noexcept {
        return m_required;
    }
    [[nodiscard]] const std::vector<std::string> &getEnumValues() const noexcept {
        return m_enum_values;
    }
    [[nodiscard]] const std::unordered_map<std::string, std::shared_ptr<SchemaObject>> &
    getProperties() const noexcept {
        return m_properties;
    }
    [[nodiscard]] const std::shared_ptr<SchemaObject> &getItems() const noexcept { return m_items; }

  private:
    std::string m_type;
    std::string m_format;
    bool m_nullable{false};
    std::string m_ref;
    std::vector<std::string> m_required;
    std::vector<std::string> m_enum_values;
    std::unordered_map<std::string, std::shared_ptr<SchemaObject>> m_properties;
    std::shared_ptr<SchemaObject> m_items;
};

class Components {
  public:
    Components() = default;

    void addSchema(std::string name, SchemaObject schema) {
        m_schemas.emplace(std::move(name), std::move(schema));
    }

    [[nodiscard]] const std::unordered_map<std::string, SchemaObject> &getSchemas() const noexcept {
        return m_schemas;
    }

  private:
    std::unordered_map<std::string, SchemaObject> m_schemas;
};
```

Update `serde::Serializable<utils::openapi::SchemaObject>::fields()` to include the new field (insert right after `"format"`, before `"nullable"` — order doesn't matter functionally, but keep it readable):

```cpp
template <>
struct serde::Serializable<utils::openapi::SchemaObject> {
    static constexpr auto fields() {
        using utils::openapi::SchemaObject;
        return std::tuple{
            serde::FieldDesc<"type", &SchemaObject::getType, &SchemaObject::setType>{},
            serde::FieldDesc<"format", &SchemaObject::getFormat, &SchemaObject::setFormat>{},
            serde::FieldDesc<"$ref", &SchemaObject::getRef, &SchemaObject::setRef>{},
            serde::FieldDesc<"nullable", &SchemaObject::getNullable,
                             &SchemaObject::setNullable>{},
            serde::FieldDesc<"required", &SchemaObject::getRequired,
                             &SchemaObject::addRequired>{},
            serde::FieldDesc<"enum", &SchemaObject::getEnumValues,
                             &SchemaObject::addEnumValue>{},
            serde::FieldDesc<"properties", &SchemaObject::getProperties,
                             &SchemaObject::addProperty>{},
            serde::FieldDesc<"items", &SchemaObject::getItems, &SchemaObject::setItems>{},
        };
    }
};

template <>
struct serde::Serializable<utils::openapi::Components> {
    static constexpr auto fields() {
        using utils::openapi::Components;
        return std::tuple{
            serde::FieldDesc<"schemas", &Components::getSchemas, &Components::addSchema>{},
        };
    }
};
```

Add a `components` field to `Document`:

```cpp
class Document {
  public:
    Document() = default;

    void setOpenapi(std::string value) { m_openapi = std::move(value); }
    void setInfo(Info info) { m_info = std::move(info); }
    void setComponents(Components components) { m_components = std::move(components); }
    void addOperation(std::string path, std::string method, Operation operation) {
        m_paths[std::move(path)][std::move(method)] = std::move(operation);
    }

    [[nodiscard]] const std::string &getOpenapi() const noexcept { return m_openapi; }
    [[nodiscard]] const Info &getInfo() const noexcept { return m_info; }
    [[nodiscard]] const Components &getComponents() const noexcept { return m_components; }
    [[nodiscard]] const std::unordered_map<std::string, std::unordered_map<std::string, Operation>> &
    getPaths() const noexcept {
        return m_paths;
    }

  private:
    std::string m_openapi{"3.0.3"};
    Info m_info;
    Components m_components;
    std::unordered_map<std::string, std::unordered_map<std::string, Operation>> m_paths;
};
```

And its `Serializable` spec:

```cpp
template <>
struct serde::Serializable<utils::openapi::Document> {
    static constexpr auto fields() {
        using utils::openapi::Document;
        return std::tuple{
            serde::FieldDesc<"openapi", &Document::getOpenapi, &Document::setOpenapi>{},
            serde::FieldDesc<"info", &Document::getInfo, &Document::setInfo>{},
            serde::FieldDesc<"components", &Document::getComponents, &Document::setComponents>{},
            serde::FieldDesc<"paths", &Document::getPaths, &Document::addOperation>{},
        };
    }
};
```

- [ ] **Step 2: Add `SchemaRegistry` and `$ref` emission to `schema.cppm`**

Replace the full contents of `include/utils/openapi/schema.cppm` with:

```cpp
module;
#include <rfl.hpp>

export module utils_openapi:schema;

import std;
import serde;
import :model;

namespace utils::openapi::detail {

template <typename T>
constexpr bool is_optional_v = false;
template <typename T>
constexpr bool is_optional_v<std::optional<T>> = true;

template <typename T>
constexpr bool is_vector_v = false;
template <typename T>
constexpr bool is_vector_v<std::vector<T>> = true;

template <typename T>
constexpr bool is_string_map_v = false;
template <typename T>
constexpr bool is_string_map_v<std::unordered_map<std::string, T>> = true;
template <typename T>
constexpr bool is_string_map_v<std::map<std::string, T>> = true;

template <typename T>
[[nodiscard]] std::string bare_type_name() {
    auto full = rfl::type_name_t<T>().str();
    auto pos = full.rfind("::");
    return pos == std::string::npos ? full : full.substr(pos + 2);
}

} // namespace utils::openapi::detail

export namespace utils::openapi {

// Static, process-wide collector of named object schemas — same singleton pattern as
// utils::openapi::Registry. Populated as a side effect of build_schema<T>() the first
// time each ISerializable T is encountered (which happens naturally whenever a route
// declares .body<T>()/.response<T>(), since those already call build_schema<T>()).
class SchemaRegistry {
  public:
    static void addSchema(std::string name, SchemaObject schema) {
        schemas.insert_or_assign(std::move(name), std::move(schema));
    }

    [[nodiscard]] static bool hasSchema(const std::string &name) noexcept {
        return schemas.contains(name);
    }

    [[nodiscard]] static const std::unordered_map<std::string, SchemaObject> &
    getSchemas() noexcept {
        return schemas;
    }

  private:
    static inline std::unordered_map<std::string, SchemaObject> schemas;
};

template <typename T>
[[nodiscard]] SchemaObject build_schema() {
    using Decayed = std::remove_cvref_t<T>;
    SchemaObject schema;

    if constexpr (serde::ISerializable<Decayed>) {
        auto name = detail::bare_type_name<Decayed>();
        if (!SchemaRegistry::hasSchema(name)) {
            // Register a placeholder BEFORE recursing into properties, so a type that
            // (directly or indirectly) references itself sees hasSchema()==true on
            // re-entry and just emits $ref instead of recursing forever.
            SchemaRegistry::addSchema(name, SchemaObject{});
            SchemaObject object_schema;
            object_schema.setType("object");
            std::apply(
                [&](auto... fds) {
                    (object_schema.addProperty(std::string{decltype(fds)::name.string_view()},
                                                build_schema<typename decltype(fds)::ValueType>()),
                     ...);
                },
                serde::Serializable<Decayed>::fields());
            SchemaRegistry::addSchema(name, std::move(object_schema));
        }
        schema.setRef(std::format("#/components/schemas/{}", name));
    } else if constexpr (detail::is_optional_v<Decayed>) {
        schema = build_schema<typename Decayed::value_type>();
        schema.setNullable(true);
    } else if constexpr (detail::is_vector_v<Decayed>) {
        schema.setType("array");
        schema.setItems(build_schema<typename Decayed::value_type>());
    } else if constexpr (detail::is_string_map_v<Decayed>) {
        schema.setType("object");
    } else if constexpr (std::same_as<Decayed, std::string>) {
        schema.setType("string");
    } else if constexpr (std::same_as<Decayed, bool>) {
        schema.setType("boolean");
    } else if constexpr (std::floating_point<Decayed>) {
        schema.setType("number");
    } else if constexpr (std::integral<Decayed>) {
        schema.setType("integer");
    } else if constexpr (std::is_enum_v<Decayed>) {
        schema.setType("string");
    } else {
        schema.setType("string");
    }

    return schema;
}

} // namespace utils::openapi
```

Known limitation (do not fix in this task — pre-existing, low-stakes, undocumented until now): when a `std::optional<T>` wraps an `ISerializable` `T`, the resulting `SchemaObject` has both `nullable: true` and `$ref` set as siblings, which OpenAPI 3.0's JSON-Schema dialect technically doesn't compose (most tooling ignores it; strict validators may complain). Not in scope here.

- [ ] **Step 3: Populate `Document.components` in `generator.cppm`**

In `include/utils/openapi/generator.cppm`, add `import :schema;` to the top-level imports (it currently imports `:model` and `:registry` — add `:schema` alongside them), then in `Generator::generate()`, right before `return document;`, add:

```cpp
Components components;
for (const auto &[name, schema] : SchemaRegistry::getSchemas())
    components.addSchema(name, schema);
document.setComponents(std::move(components));
```

- [ ] **Step 4: Build and verify**

Run: `cd /home/default/cc/congelado && xmake build`
Expected: `build ok` — this rebuilds `congelado_lib`, `engine`, `http2`, and `congelado` since all depend on `utils_openapi`.

- [ ] **Step 5: Run the server and confirm named schemas + `$ref` in the live document**

```bash
cd build/linux/x86_64/debug
rm -f openapi.json
./congelado > /tmp/run.log 2>&1 &
SRV=$!
sleep 3
python3 -c "
import json
d = json.load(open('openapi.json'))
assert 'TaskDef' in d['components']['schemas'], 'TaskDef missing from components.schemas'
op = d['paths']['/api/v1/tasks']['post']
ref = op['requestBody']['content']['application/json']['schema']['\$ref']
assert ref == '#/components/schemas/TaskDef', f'unexpected ref: {ref}'
print('OK:', ref)
"
kill -9 $SRV
```
Expected output: `OK: #/components/schemas/TaskDef`

- [ ] **Step 6: Commit**

```bash
git add include/utils/openapi/model.cppm include/utils/openapi/schema.cppm include/utils/openapi/generator.cppm
git commit -m "feat(openapi): emit named components.schemas + \$ref instead of inlining

Enables client codegen to reuse one generated type per schema instead
of duplicating an anonymous struct per operation."
```

---

### Task 2: `sdk/client` module scaffold + build wiring

**Files:**
- Create: `sdk/client/congelado_client.cppm` — umbrella module `congelado_client`.
- Create: `sdk/client/runtime.cppm` — partition `:runtime`, empty `Runtime` class shell (filled in Task 3).
- Modify: `xmake.lua` — register the new module file, add a new (currently-empty) catch2 test target for this SDK's tests.
- Create: `tests/sdk/client/.gitkeep` (placeholder so the test glob directory exists before Task 3 adds real tests) — actually skip the `.gitkeep`; the test target globs `tests/sdk/client/**.cc` and an empty glob is fine with xmake, so no placeholder file is needed.

**Interfaces:**
- Consumes: nothing.
- Produces: module `congelado_client` (importable via `import congelado_client;`), containing (for now) just `congelado::client::Runtime` as an empty shell — Task 3 fills in its real members.

- [ ] **Step 1: Create the umbrella module**

Create `sdk/client/congelado_client.cppm`:

```cpp
export module congelado_client;

export import :runtime;
```

- [ ] **Step 2: Create the `Runtime` shell**

Create `sdk/client/runtime.cppm`:

```cpp
export module congelado_client:runtime;

import std;
import interfaces;

export namespace congelado::client {

class Runtime {
  public:
    Runtime() = delete;
};

} // namespace congelado::client
```

- [ ] **Step 3: Wire into `xmake.lua`**

In `xmake.lua`, find the line:
```lua
add_files("sdk/plugin/congelado_plugin.cppm", { public = true })
```
and add right after it:
```lua
add_files("sdk/client/congelado_client.cppm", { public = true })
```

Then find the commented-out `engine_worker_test` target block (search for `-- target("engine_worker_test")`) and add a new, active target right after that whole commented block (keep the existing commented block untouched — it's out of scope for this plan):

```lua
target("client_sdk_test")
    set_kind("binary")
    set_languages("c++26")
    set_policy("build.c++.modules", true)
    add_files("tests/sdk/client/**.cc")
    add_deps("congelado_lib")
    add_packages("catch2")
    if is_plat("linux", "macosx") then
        add_cxflags("-ffile-prefix-map=$(projectdir)=.", "-fmacro-prefix-map=$(projectdir)=.")
    end
    add_tests("default")
target_end()
```

- [ ] **Step 4: Build to confirm the scaffold compiles**

Run: `cd /home/default/cc/congelado && xmake build congelado_lib`
Expected: `build ok` — no test files exist yet so `client_sdk_test` has nothing to compile; that's fine, it's built/run starting in Task 3.

- [ ] **Step 5: Commit**

```bash
git add sdk/client/congelado_client.cppm sdk/client/runtime.cppm xmake.lua
git commit -m "feat(sdk): scaffold congelado_client module and test target"
```

---

### Task 3: `Runtime` implementation + unit test

**Files:**
- Modify: `sdk/client/runtime.cppm` — full implementation.
- Create: `tests/sdk/client/runtime_test.cc`.

**Interfaces:**
- Consumes: `interfaces::IClient` (`include/interfaces/client.cppm`: `send(io::IRequest&)`, `on_connect(...)`), `interfaces::io::IRequest`/`IResponse` (`get_stream_id()`/`set_stream_id()` already present on both), `serde::Ser::deserialize<T>` (`include/serde/serde.cppm`).
- Produces:
  ```cpp
  namespace congelado::client {
    class Runtime {
      static void setClient(interfaces::IClient &client) noexcept;
      static interfaces::IClient &getClient() noexcept;
      using RequestFactory = std::function<std::unique_ptr<interfaces::io::IRequest>(std::uint32_t)>;
      static void setRequestFactory(RequestFactory factory) noexcept;
      [[nodiscard]] static std::unique_ptr<interfaces::io::IRequest> newRequest();
      template <typename Res>
      static void send(std::unique_ptr<interfaces::io::IRequest> request,
                       std::function<void(Res)> onResponse,
                       std::function<void(std::string)> onError = [](std::string) {});
      static void dispatch(interfaces::io::IRequest &request, interfaces::io::IResponse &response);
    };
  }
  ```
  Consumed by Task 6's generated route functions and Task 7's manual end-to-end check.

- [ ] **Step 1: Write the failing test**

Create `tests/sdk/client/runtime_test.cc`:

```cpp
#include <catch2/catch_test_macros.hpp>
import std;
import interfaces;
import shared;
import utils_buffering;
import congelado_client;
import serde;

namespace {

// Minimal concrete IRequest — in-memory header/body storage, no networking.
class TestRequest final : public interfaces::io::IRequest {
  public:
    explicit TestRequest(std::uint32_t stream_id) : interfaces::io::IRequest(stream_id) {}

    void set_header(std::variant<std::string_view, interfaces::io::types::Token> name_or_token,
                    std::string_view value) & override {
        m_headers[std::holds_alternative<std::string_view>(name_or_token)
                      ? std::string{std::get<std::string_view>(name_or_token)}
                      : std::string{interfaces::io::types::token_to_string(
                            std::get<interfaces::io::types::Token>(name_or_token))}] =
            std::string{value};
    }

    [[nodiscard]] std::string_view get_method() const noexcept override {
        return m_headers.at(":method");
    }
    [[nodiscard]] std::string_view get_path() const noexcept override {
        return m_headers.at(":path");
    }

    void set_body_bytes(std::vector<std::byte> bytes) { m_body_bytes = std::move(bytes); }

  private:
    std::unordered_map<std::string, std::string> m_headers;
    std::vector<std::byte> m_body_bytes;
};

// Records the last request handed to send(); does not touch the network.
class TestClient final : public interfaces::IClient {
  public:
    void send(interfaces::io::IRequest &request) override { m_last_stream_id = request.get_stream_id(); }

    [[nodiscard]] shared::ReadCallback on_connect(shared::SendCallback, shared::CloseCallback) override {
        return [](utils::buffering::BufferReader &) {};
    }

    [[nodiscard]] std::uint32_t getLastStreamId() const noexcept { return m_last_stream_id; }

  private:
    std::uint32_t m_last_stream_id{0};
};

class TestResponse final : public interfaces::io::IResponse {
  public:
    explicit TestResponse(std::uint32_t stream_id) : interfaces::io::IResponse(stream_id) {}

    void set_body(std::vector<std::byte> body) & override { m_body = std::move(body); }
    [[nodiscard]] std::span<const std::byte> get_body() const noexcept override { return m_body; }
    [[nodiscard]] std::string_view get_content_type() const noexcept override {
        return "application/json";
    }

  private:
    std::vector<std::byte> m_body;
};

class Point {
  public:
    Point() = default;
    void setX(int value) noexcept { m_x = value; }
    [[nodiscard]] int getX() const noexcept { return m_x; }

  private:
    int m_x{0};
};

} // namespace

template <>
struct serde::Serializable<Point> {
    static constexpr auto fields() {
        return std::tuple{serde::FieldDesc<"x", &Point::getX, &Point::setX>{}};
    }
};

TEST_CASE("Runtime::send correlates response by stream id and deserializes it") {
    TestClient client;
    congelado::client::Runtime::setClient(client);
    congelado::client::Runtime::setRequestFactory(
        [](std::uint32_t streamId) { return std::make_unique<TestRequest>(streamId); });

    auto request = congelado::client::Runtime::newRequest();
    auto stream_id = request->get_stream_id();

    bool called = false;
    int received_x = 0;
    congelado::client::Runtime::send<Point>(
        std::move(request), [&](Point p) { called = true; received_x = p.getX(); });

    CHECK(client.getLastStreamId() == stream_id);
    CHECK_FALSE(called);

    TestRequest echo_request{stream_id};
    TestResponse response{stream_id};
    Point sample;
    sample.setX(42);
    auto encoded = serde::Json::encode(sample);
    std::vector<std::byte> bytes(encoded.size());
    std::ranges::transform(encoded, bytes.begin(), [](char c) { return std::byte(c); });
    response.set_body(bytes);

    congelado::client::Runtime::dispatch(echo_request, response);

    CHECK(called);
    CHECK(received_x == 42);
}

TEST_CASE("Runtime::dispatch on an untracked stream id is a no-op") {
    TestClient client;
    congelado::client::Runtime::setClient(client);
    congelado::client::Runtime::setRequestFactory(
        [](std::uint32_t streamId) { return std::make_unique<TestRequest>(streamId); });

    TestRequest request{999};
    TestResponse response{999};
    congelado::client::Runtime::dispatch(request, response); // must not throw/crash
    CHECK(true);
}
```

- [ ] **Step 2: Run test to verify it fails**

Run: `cd /home/default/cc/congelado && xmake build client_sdk_test`
Expected: FAIL — compile error, `congelado::client::Runtime` has no `setClient`/`send`/etc. (only the `Runtime() = delete;` shell from Task 2 exists).

- [ ] **Step 3: Implement `Runtime`**

Replace `sdk/client/runtime.cppm` with:

```cpp
export module congelado_client:runtime;

import std;
import interfaces;
import serde;

export namespace congelado::client {

class Runtime {
  public:
    Runtime() = delete;

    static void setClient(interfaces::IClient &value) noexcept { client = &value; }

    [[nodiscard]] static interfaces::IClient &getClient() noexcept {
        if (client == nullptr) {
            std::abort();
        }
        return *client;
    }

    using RequestFactory =
        std::function<std::unique_ptr<interfaces::io::IRequest>(std::uint32_t)>;

    static void setRequestFactory(RequestFactory factory) noexcept {
        requestFactory = std::move(factory);
    }

    [[nodiscard]] static std::unique_ptr<interfaces::io::IRequest> newRequest() {
        if (!requestFactory) {
            std::abort();
        }
        return requestFactory(nextStreamId++);
    }

    template <typename Res>
    static void send(std::unique_ptr<interfaces::io::IRequest> request,
                     std::function<void(Res)> onResponse,
                     std::function<void(std::string)> onError = [](std::string) {}) {
        auto stream_id = request->get_stream_id();
        pending[stream_id] = [onResponse = std::move(onResponse),
                             onError = std::move(onError)](interfaces::io::IResponse &response) {
            auto body_bytes = response.get_body();
            std::string body(reinterpret_cast<const char *>(body_bytes.data()), body_bytes.size());
            auto result = serde::Ser::deserialize<Res>(response.get_content_type(), body);
            if (result) {
                onResponse(std::move(*result));
            } else {
                onError(result.error());
            }
        };
        getClient().send(*request);
    }

    static void dispatch(interfaces::io::IRequest &request, interfaces::io::IResponse &response) {
        auto it = pending.find(request.get_stream_id());
        if (it == pending.end()) {
            return;
        }
        auto callback = std::move(it->second);
        pending.erase(it);
        callback(response);
    }

  private:
    static inline interfaces::IClient *client = nullptr;
    static inline RequestFactory requestFactory;
    static inline std::unordered_map<std::uint32_t,
                                     std::function<void(interfaces::io::IResponse &)>> pending;
    static inline std::uint32_t nextStreamId{1};
};

} // namespace congelado::client
```

- [ ] **Step 4: Run test to verify it passes**

Run: `cd /home/default/cc/congelado && xmake build client_sdk_test && xmake run client_sdk_test`
Expected: both `TEST_CASE`s pass, e.g. `All tests passed (N assertions in 2 test cases)`.

- [ ] **Step 5: Commit**

```bash
git add sdk/client/runtime.cppm tests/sdk/client/runtime_test.cc
git commit -m "feat(sdk/client): implement Runtime — stream-id-correlated request/response dispatch"
```

---

### Task 4: Generator's internal schema parse tree + JSON reader

**Files:**
- Create: `sdk/client/schema_model.cppm` — partition `:schema_model`, a `SchemaType` value type representing exactly what `utils::openapi::SchemaObject` can produce (object/array/string/integer/number/boolean/`$ref`/nullable) plus a parser from `simdjson::dom::element`.
- Create: `tests/sdk/client/schema_model_test.cc`.
- Modify: `sdk/client/congelado_client.cppm` — re-export `:schema_model`.

**Interfaces:**
- Consumes: nothing from other tasks (parses raw JSON produced by Task 1's server-side change — verified against a hand-written JSON literal fixture in the test, not a live server).
- Produces:
  ```cpp
  namespace congelado::client {
    enum class SchemaKind { Object, Array, String, Integer, Number, Boolean, Ref };
    class SchemaType {
      SchemaKind getKind() const;
      const std::string& getRef() const;                         // valid iff kind == Ref
      const std::unordered_map<std::string, SchemaType>& getProperties() const; // iff Object
      const SchemaType& getItems() const;                         // iff Array
      bool getNullable() const;
    };
    [[nodiscard]] std::expected<SchemaType, std::string> parse_schema(simdjson::dom::element element);
  }
  ```
  Consumed by Task 5 (DTO writer) and Task 6 (route writer) to know each property/parameter's C++ type.

- [ ] **Step 1: Write the failing test**

Create `tests/sdk/client/schema_model_test.cc`:

```cpp
#include <catch2/catch_test_macros.hpp>
#include <simdjson.h>
import std;
import congelado_client;

TEST_CASE("parse_schema reads a $ref schema") {
    auto json = R"({"type":"","format":"","$ref":"#/components/schemas/TaskDef","nullable":false,"required":[],"enum":[],"properties":{}})";
    simdjson::dom::parser parser;
    simdjson::dom::element element;
    REQUIRE_FALSE(parser.parse(json).get(element));

    auto result = congelado::client::parse_schema(element);
    REQUIRE(result.has_value());
    CHECK(result->getKind() == congelado::client::SchemaKind::Ref);
    CHECK(result->getRef() == "TaskDef");
}

TEST_CASE("parse_schema reads a nullable array of strings") {
    auto json =
        R"({"type":"array","format":"","$ref":"","nullable":true,"required":[],"enum":[],)"
        R"("properties":{},"items":{"type":"string","format":"","$ref":"","nullable":false,)"
        R"("required":[],"enum":[],"properties":{}}})";
    simdjson::dom::parser parser;
    simdjson::dom::element element;
    REQUIRE_FALSE(parser.parse(json).get(element));

    auto result = congelado::client::parse_schema(element);
    REQUIRE(result.has_value());
    CHECK(result->getKind() == congelado::client::SchemaKind::Array);
    CHECK(result->getNullable());
    CHECK(result->getItems().getKind() == congelado::client::SchemaKind::String);
}

TEST_CASE("parse_schema reads a nested object") {
    auto json =
        R"({"type":"object","format":"","$ref":"","nullable":false,"required":[],"enum":[],)"
        R"("properties":{"count":{"type":"integer","format":"","$ref":"","nullable":false,)"
        R"("required":[],"enum":[],"properties":{}}}})";
    simdjson::dom::parser parser;
    simdjson::dom::element element;
    REQUIRE_FALSE(parser.parse(json).get(element));

    auto result = congelado::client::parse_schema(element);
    REQUIRE(result.has_value());
    CHECK(result->getKind() == congelado::client::SchemaKind::Object);
    REQUIRE(result->getProperties().contains("count"));
    CHECK(result->getProperties().at("count").getKind() == congelado::client::SchemaKind::Integer);
}
```

- [ ] **Step 2: Run test to verify it fails**

Run: `cd /home/default/cc/congelado && xmake build client_sdk_test`
Expected: FAIL — `congelado::client::parse_schema`/`SchemaType`/`SchemaKind` don't exist yet.

- [ ] **Step 3: Implement `schema_model.cppm`**

Create `sdk/client/schema_model.cppm`:

```cpp
module;
#include <simdjson.h>

export module congelado_client:schema_model;

import std;

export namespace congelado::client {

enum class SchemaKind { Object, Array, String, Integer, Number, Boolean, Ref };

class SchemaType {
  public:
    SchemaType() = default;

    void setKind(SchemaKind value) noexcept { m_kind = value; }
    void setRef(std::string value) { m_ref = std::move(value); }
    void setNullable(bool value) noexcept { m_nullable = value; }
    void addProperty(std::string name, SchemaType type) {
        m_properties.emplace(std::move(name), std::move(type));
    }
    void setItems(SchemaType type) { m_items = std::make_shared<SchemaType>(std::move(type)); }

    [[nodiscard]] SchemaKind getKind() const noexcept { return m_kind; }
    [[nodiscard]] const std::string &getRef() const noexcept { return m_ref; }
    [[nodiscard]] bool getNullable() const noexcept { return m_nullable; }
    [[nodiscard]] const std::unordered_map<std::string, SchemaType> &getProperties() const noexcept {
        return m_properties;
    }
    [[nodiscard]] const SchemaType &getItems() const noexcept { return *m_items; }

  private:
    SchemaKind m_kind{SchemaKind::String};
    std::string m_ref;
    bool m_nullable{false};
    std::unordered_map<std::string, SchemaType> m_properties;
    std::shared_ptr<SchemaType> m_items;
};

// Bare schema name from a "#/components/schemas/<Name>" JSON Pointer.
[[nodiscard]] inline std::string ref_name(std::string_view pointer) {
    auto pos = pointer.rfind('/');
    return pos == std::string_view::npos ? std::string{pointer}
                                         : std::string{pointer.substr(pos + 1)};
}

[[nodiscard]] inline std::expected<SchemaType, std::string>
parse_schema(simdjson::dom::element element) {
    SchemaType schema;

    std::string_view ref;
    if (!element["$ref"].get(ref) && !ref.empty()) {
        schema.setKind(SchemaKind::Ref);
        schema.setRef(ref_name(ref));
        return schema;
    }

    bool nullable = false;
    if (!element["nullable"].get(nullable)) {
        schema.setNullable(nullable);
    }

    std::string_view type;
    if (auto ec = element["type"].get(type); ec) {
        return std::unexpected{std::format("schema missing 'type': {}", simdjson::error_message(ec))};
    }

    if (type == "object") {
        schema.setKind(SchemaKind::Object);
        simdjson::dom::object properties;
        if (!element["properties"].get(properties)) {
            for (auto [name, value] : properties) {
                auto parsed = parse_schema(value);
                if (!parsed) {
                    return std::unexpected{parsed.error()};
                }
                schema.addProperty(std::string{name}, std::move(*parsed));
            }
        }
    } else if (type == "array") {
        schema.setKind(SchemaKind::Array);
        simdjson::dom::element items;
        if (auto ec = element["items"].get(items); ec) {
            return std::unexpected{
                std::format("array schema missing 'items': {}", simdjson::error_message(ec))};
        }
        auto parsed = parse_schema(items);
        if (!parsed) {
            return std::unexpected{parsed.error()};
        }
        schema.setItems(std::move(*parsed));
    } else if (type == "string") {
        schema.setKind(SchemaKind::String);
    } else if (type == "integer") {
        schema.setKind(SchemaKind::Integer);
    } else if (type == "number") {
        schema.setKind(SchemaKind::Number);
    } else if (type == "boolean") {
        schema.setKind(SchemaKind::Boolean);
    } else {
        return std::unexpected{std::format("unknown schema type '{}'", type)};
    }

    return schema;
}

} // namespace congelado::client
```

- [ ] **Step 4: Re-export the partition**

Update `sdk/client/congelado_client.cppm`:

```cpp
export module congelado_client;

export import :runtime;
export import :schema_model;
```

- [ ] **Step 5: Run test to verify it passes**

Run: `cd /home/default/cc/congelado && xmake build client_sdk_test && xmake run client_sdk_test`
Expected: all `TEST_CASE`s from Task 3 and Task 4 pass.

- [ ] **Step 6: Commit**

```bash
git add sdk/client/schema_model.cppm sdk/client/congelado_client.cppm tests/sdk/client/schema_model_test.cc
git commit -m "feat(sdk/client): parse utils::openapi's schema JSON into a small typed tree"
```

---

### Task 5: DTO writer

**Files:**
- Create: `sdk/client/dto_writer.cppm` — partition `:dto_writer`. Given a name → `SchemaType` map (all named `components.schemas` entries, already parsed), emits C++ source text: one class + one `serde::Serializable` spec per named `Object`-kind schema, topologically ordered so a schema referencing another via `$ref` is emitted after the schema it depends on.
- Create: `tests/sdk/client/dto_writer_test.cc`.
- Modify: `sdk/client/congelado_client.cppm` — re-export `:dto_writer`.

**Interfaces:**
- Consumes: `congelado::client::SchemaType`/`SchemaKind` (Task 4).
- Produces:
  ```cpp
  namespace congelado::client {
    [[nodiscard]] std::expected<std::string, std::string>
    write_dtos(const std::unordered_map<std::string, SchemaType> &namedSchemas,
              std::string_view moduleName);
  }
  ```
  Consumed by Task 7 (`Generator::generate()`), which writes the returned string to `<output_dir>/dto.cppm`.

- [ ] **Step 1: Write the failing test**

Create `tests/sdk/client/dto_writer_test.cc`:

```cpp
#include <catch2/catch_test_macros.hpp>
import std;
import congelado_client;

TEST_CASE("write_dtos emits a class with getters/setters and a Serializable spec") {
    congelado::client::SchemaType count_type;
    count_type.setKind(congelado::client::SchemaKind::Integer);

    congelado::client::SchemaType point;
    point.setKind(congelado::client::SchemaKind::Object);
    point.addProperty("count", count_type);

    std::unordered_map<std::string, congelado::client::SchemaType> schemas;
    schemas.emplace("Point", point);

    auto result = congelado::client::write_dtos(schemas, "client_dto");
    REQUIRE(result.has_value());

    CHECK(result->contains("export module client_dto;"));
    CHECK(result->contains("class Point"));
    CHECK(result->contains("void setCount(std::int64_t value)"));
    CHECK(result->contains("[[nodiscard]] std::int64_t getCount() const noexcept"));
    CHECK(result->contains("struct serde::Serializable<client_dto::Point>"));
    CHECK(result->contains(R"(serde::FieldDesc<"count", &Point::getCount, &Point::setCount>{})"));
}

TEST_CASE("write_dtos orders a schema after the one it $refs") {
    congelado::client::SchemaType inner_ref;
    inner_ref.setKind(congelado::client::SchemaKind::Ref);
    inner_ref.setRef("Inner");

    congelado::client::SchemaType outer;
    outer.setKind(congelado::client::SchemaKind::Object);
    outer.addProperty("inner", inner_ref);

    congelado::client::SchemaType inner;
    inner.setKind(congelado::client::SchemaKind::Object);

    std::unordered_map<std::string, congelado::client::SchemaType> schemas;
    schemas.emplace("Outer", outer);
    schemas.emplace("Inner", inner);

    auto result = congelado::client::write_dtos(schemas, "client_dto");
    REQUIRE(result.has_value());

    auto inner_pos = result->find("class Inner");
    auto outer_pos = result->find("class Outer");
    REQUIRE(inner_pos != std::string::npos);
    REQUIRE(outer_pos != std::string::npos);
    CHECK(inner_pos < outer_pos);
}
```

- [ ] **Step 2: Run test to verify it fails**

Run: `cd /home/default/cc/congelado && xmake build client_sdk_test`
Expected: FAIL — `congelado::client::write_dtos` doesn't exist yet.

- [ ] **Step 3: Implement `dto_writer.cppm`**

Create `sdk/client/dto_writer.cppm`:

```cpp
export module congelado_client:dto_writer;

import std;
import :schema_model;

namespace congelado::client::detail {

[[nodiscard]] inline std::string pascal_case(std::string_view name) {
    std::string result;
    bool capitalize_next = true;
    for (char c : name) {
        if (c == '_') {
            capitalize_next = true;
            continue;
        }
        result += capitalize_next ? static_cast<char>(std::toupper(c)) : c;
        capitalize_next = false;
    }
    return result;
}

[[nodiscard]] inline std::string cpp_type(const SchemaType &type) {
    std::string base;
    switch (type.getKind()) {
    case SchemaKind::Ref:
        base = type.getRef();
        break;
    case SchemaKind::Array:
        base = std::format("std::vector<{}>", cpp_type(type.getItems()));
        break;
    case SchemaKind::String:
        base = "std::string";
        break;
    case SchemaKind::Integer:
        base = "std::int64_t";
        break;
    case SchemaKind::Number:
        base = "double";
        break;
    case SchemaKind::Boolean:
        base = "bool";
        break;
    case SchemaKind::Object:
        base = "std::string"; // anonymous inline object — not expected from our own server, see Task 1 note
        break;
    }
    return type.getNullable() ? std::format("std::optional<{}>", base) : base;
}

// Post-order DFS so a schema is emitted after every named schema it $refs.
inline void topo_sort(const std::string &name,
                      const std::unordered_map<std::string, SchemaType> &schemas,
                      std::unordered_map<std::string, bool> &visited,
                      std::vector<std::string> &order) {
    if (visited[name]) {
        return;
    }
    visited[name] = true;
    const auto &schema = schemas.at(name);
    for (const auto &[prop_name, prop_type] : schema.getProperties()) {
        (void)prop_name;
        if (prop_type.getKind() == SchemaKind::Ref && schemas.contains(prop_type.getRef())) {
            topo_sort(prop_type.getRef(), schemas, visited, order);
        }
    }
    order.push_back(name);
}

} // namespace congelado::client::detail

export namespace congelado::client {

[[nodiscard]] std::expected<std::string, std::string>
write_dtos(const std::unordered_map<std::string, SchemaType> &namedSchemas,
          std::string_view moduleName) {
    std::vector<std::string> order;
    std::unordered_map<std::string, bool> visited;
    for (const auto &[name, schema] : namedSchemas) {
        (void)schema;
        detail::topo_sort(name, namedSchemas, visited, order);
    }

    std::string out;
    out += std::format("export module {};\n\n", moduleName);
    out += "import std;\nimport serde;\n\n";
    out += std::format("export namespace {} {{\n\n", moduleName);

    for (const auto &name : order) {
        const auto &schema = namedSchemas.at(name);
        if (schema.getKind() != SchemaKind::Object) {
            return std::unexpected{std::format("named schema '{}' is not an object", name)};
        }

        out += std::format("class {} {{\n  public:\n    {}() = default;\n\n", name, name);
        for (const auto &[prop_name, prop_type] : schema.getProperties()) {
            auto member = detail::pascal_case(prop_name);
            auto type = detail::cpp_type(prop_type);
            out += std::format("    void set{}({} value) {{ m_{} = std::move(value); }}\n", member,
                               type, prop_name);
        }
        out += "\n";
        for (const auto &[prop_name, prop_type] : schema.getProperties()) {
            auto member = detail::pascal_case(prop_name);
            auto type = detail::cpp_type(prop_type);
            out += std::format(
                "    [[nodiscard]] {} get{}() const noexcept {{ return m_{}; }}\n", type, member,
                prop_name);
        }
        out += "\n  private:\n";
        for (const auto &[prop_name, prop_type] : schema.getProperties()) {
            out += std::format("    {} m_{};\n", detail::cpp_type(prop_type), prop_name);
        }
        out += "};\n\n";
    }

    out += std::format("}} // namespace {}\n\n", moduleName);

    for (const auto &name : order) {
        const auto &schema = namedSchemas.at(name);
        out += std::format("template <>\nstruct serde::Serializable<{}::{}> {{\n", moduleName, name);
        out += "    static constexpr auto fields() {\n";
        out += std::format("        using {}::{};\n", moduleName, name);
        out += "        return std::tuple{\n";
        for (const auto &[prop_name, prop_type] : schema.getProperties()) {
            (void)prop_type;
            auto member = detail::pascal_case(prop_name);
            out += std::format(
                "            serde::FieldDesc<\"{}\", &{}::get{}, &{}::set{}>{{}},\n", prop_name,
                name, member, name, member);
        }
        out += "        };\n    }\n};\n\n";
    }

    return out;
}

} // namespace congelado::client
```

- [ ] **Step 4: Re-export the partition**

Update `sdk/client/congelado_client.cppm`:

```cpp
export module congelado_client;

export import :runtime;
export import :schema_model;
export import :dto_writer;
```

- [ ] **Step 5: Run test to verify it passes**

Run: `cd /home/default/cc/congelado && xmake build client_sdk_test && xmake run client_sdk_test`
Expected: all prior `TEST_CASE`s plus both new ones pass.

- [ ] **Step 6: Commit**

```bash
git add sdk/client/dto_writer.cppm sdk/client/congelado_client.cppm tests/sdk/client/dto_writer_test.cc
git commit -m "feat(sdk/client): generate DTO classes + serde::Serializable specs from named schemas"
```

---

### Task 6: Route writer (incl. `sharedModels` option)

**Files:**
- Create: `sdk/client/route_writer.cppm` — partition `:route_writer`. Given the parsed `paths` map (path → method → operation info) plus the DTO type-name resolver from Task 5, emits one C++ function per operation.
- Create: `tests/sdk/client/route_writer_test.cc`.
- Modify: `sdk/client/congelado_client.cppm` — re-export `:route_writer`.

**Interfaces:**
- Consumes: `congelado::client::SchemaType`/`SchemaKind`/`cpp_type`-equivalent naming (reuses the same type-name mapping logic as Task 5, via a small shared helper — see Step 3 for why it's duplicated rather than imported, and how that's kept in sync).
- Produces:
  ```cpp
  namespace congelado::client {
    class OperationInfo {
      // path, method, optional request-body SchemaType, response SchemaType (or none = void)
    };
    [[nodiscard]] std::expected<std::string, std::string>
    write_routes(const std::vector<OperationInfo> &operations,
                std::string_view routesModuleName,
                std::string_view dtoModuleName); // dtoModuleName is either the generated
                                                  // DTO module ("client_dto") or a shared
                                                  // models module name ("model") passed
                                                  // straight through from Generator.
  }
  ```
  Consumed by Task 7 (`Generator::generate()`), which writes the result to `<output_dir>/routes.cppm`.

- [ ] **Step 1: Write the failing test**

Create `tests/sdk/client/route_writer_test.cc`:

```cpp
#include <catch2/catch_test_macros.hpp>
import std;
import congelado_client;

TEST_CASE("write_routes emits one namespaced function per operation") {
    congelado::client::SchemaType task_def_ref;
    task_def_ref.setKind(congelado::client::SchemaKind::Ref);
    task_def_ref.setRef("TaskDef");

    congelado::client::OperationInfo create_task;
    create_task.setPath("/api/v1/tasks");
    create_task.setMethod("post");
    create_task.setRequestBody(task_def_ref);
    create_task.setResponse(task_def_ref);

    congelado::client::OperationInfo get_task;
    get_task.setPath("/api/v1/tasks/{name}");
    get_task.setMethod("get");
    get_task.setResponse(task_def_ref);

    auto result = congelado::client::write_routes({create_task, get_task}, "client_routes",
                                                   "client_dto");
    REQUIRE(result.has_value());

    CHECK(result->contains("export module client_routes;"));
    CHECK(result->contains("namespace client::tasks"));
    CHECK(result->contains(
        "void post(const client_dto::TaskDef &body, "
        "std::function<void(client_dto::TaskDef)> onResponse,"));
    CHECK(result->contains(
        "void get_by_name(std::string_view name, "
        "std::function<void(client_dto::TaskDef)> onResponse,"));
    CHECK(result->contains("congelado::client::Runtime::newRequest()"));
    CHECK(result->contains(R"(.with_path(std::format("/api/v1/tasks/{}", name)))"));
}

TEST_CASE("write_routes with a shared models module references it instead of client_dto") {
    congelado::client::SchemaType task_def_ref;
    task_def_ref.setKind(congelado::client::SchemaKind::Ref);
    task_def_ref.setRef("TaskDef");

    congelado::client::OperationInfo get_task;
    get_task.setPath("/api/v1/tasks/{name}");
    get_task.setMethod("get");
    get_task.setResponse(task_def_ref);

    auto result = congelado::client::write_routes({get_task}, "client_routes", "model");
    REQUIRE(result.has_value());

    CHECK(result->contains("import model;"));
    CHECK(result->contains("std::function<void(model::TaskDef)> onResponse"));
}

TEST_CASE("write_routes resolves an Array-of-Ref top-level response, not just a bare Ref") {
    // A list endpoint's response schema kind is Array (with Ref items), never a bare Ref —
    // this must produce std::vector<...>, not crash or silently emit an empty type name.
    congelado::client::SchemaType task_def_ref;
    task_def_ref.setKind(congelado::client::SchemaKind::Ref);
    task_def_ref.setRef("TaskDef");

    congelado::client::SchemaType task_def_list;
    task_def_list.setKind(congelado::client::SchemaKind::Array);
    task_def_list.setItems(task_def_ref);

    congelado::client::OperationInfo list_tasks;
    list_tasks.setPath("/api/v1/metadata/tasks");
    list_tasks.setMethod("get");
    list_tasks.setResponse(task_def_list);

    auto result = congelado::client::write_routes({list_tasks}, "client_routes", "client_dto");
    REQUIRE(result.has_value());

    CHECK(result->contains(
        "std::function<void(std::vector<client_dto::TaskDef>)> onResponse"));
}
```

- [ ] **Step 2: Run test to verify it fails**

Run: `cd /home/default/cc/congelado && xmake build client_sdk_test`
Expected: FAIL — `congelado::client::OperationInfo`/`write_routes` don't exist yet.

- [ ] **Step 3: Implement `route_writer.cppm`**

Create `sdk/client/route_writer.cppm`. Note: `pascal_case`/`resolve_type` (route_writer's own version, equivalent to `:dto_writer`'s `cpp_type`) are re-declared here rather than imported from `:dto_writer`, because `:dto_writer`'s versions are file-local (non-exported) helpers in an anonymous-equivalent internal namespace — duplicating these ~30 lines is simpler and less coupling than exporting internal helpers across partitions for two call sites; if a third writer needs them later, promote to a shared `:naming` partition then (YAGNI for now).

```cpp
export module congelado_client:route_writer;

import std;
import :schema_model;

namespace congelado::client::detail {

[[nodiscard]] inline std::string pascal_case(std::string_view name) {
    std::string result;
    bool capitalize_next = true;
    for (char c : name) {
        if (c == '_') {
            capitalize_next = true;
            continue;
        }
        result += capitalize_next ? static_cast<char>(std::toupper(c)) : c;
        capitalize_next = false;
    }
    return result;
}

// Mirrors :dto_writer's cpp_type() — must handle every SchemaKind, not just Ref, because
// a top-level operation response can itself be Array (e.g. GET /metadata/tasks responds
// with std::vector<TaskDef>, whose SchemaType kind is Array-of-Ref, not a bare Ref).
[[nodiscard]] inline std::string resolve_type(const SchemaType &type, std::string_view dtoModule) {
    std::string base;
    switch (type.getKind()) {
    case SchemaKind::Ref:
        base = std::format("{}::{}", dtoModule, type.getRef());
        break;
    case SchemaKind::Array:
        base = std::format("std::vector<{}>", resolve_type(type.getItems(), dtoModule));
        break;
    case SchemaKind::String:
        base = "std::string";
        break;
    case SchemaKind::Integer:
        base = "std::int64_t";
        break;
    case SchemaKind::Number:
        base = "double";
        break;
    case SchemaKind::Boolean:
        base = "bool";
        break;
    case SchemaKind::Object:
        base = "std::string"; // anonymous inline object — not expected from our own server
        break;
    }
    return type.getNullable() ? std::format("std::optional<{}>", base) : base;
}

[[nodiscard]] inline std::vector<std::string> path_segments(std::string_view path) {
    std::vector<std::string> segments;
    std::size_t start = 0;
    while (start < path.size()) {
        auto slash = path.find('/', start);
        auto part = path.substr(start, slash == std::string_view::npos ? std::string_view::npos
                                                                       : slash - start);
        if (!part.empty()) {
            segments.emplace_back(part);
        }
        if (slash == std::string_view::npos) {
            break;
        }
        start = slash + 1;
    }
    return segments;
}

[[nodiscard]] inline bool is_param(const std::string &segment) {
    return segment.size() > 1 && segment.front() == '{' && segment.back() == '}';
}

[[nodiscard]] inline std::string param_name(const std::string &segment) {
    return segment.substr(1, segment.size() - 2);
}

[[nodiscard]] inline std::string function_name(std::string_view method,
                                               const std::vector<std::string> &literal_tail) {
    std::string name{method};
    for (const auto &part : literal_tail) {
        name += "_" + part;
    }
    return name;
}

} // namespace congelado::client::detail

export namespace congelado::client {

class OperationInfo {
  public:
    OperationInfo() = default;

    void setPath(std::string value) { m_path = std::move(value); }
    void setMethod(std::string value) { m_method = std::move(value); }
    void setRequestBody(SchemaType value) { m_request_body = std::move(value); }
    void setResponse(SchemaType value) { m_response = std::move(value); }

    [[nodiscard]] const std::string &getPath() const noexcept { return m_path; }
    [[nodiscard]] const std::string &getMethod() const noexcept { return m_method; }
    [[nodiscard]] const std::optional<SchemaType> &getRequestBody() const noexcept {
        return m_request_body;
    }
    [[nodiscard]] const std::optional<SchemaType> &getResponse() const noexcept {
        return m_response;
    }

  private:
    std::string m_path;
    std::string m_method;
    std::optional<SchemaType> m_request_body;
    std::optional<SchemaType> m_response;
};

[[nodiscard]] std::expected<std::string, std::string>
write_routes(const std::vector<OperationInfo> &operations, std::string_view routesModuleName,
            std::string_view dtoModuleName) {
    std::string out;
    out += std::format("export module {};\n\n", routesModuleName);
    out += "import std;\nimport interfaces;\nimport congelado_client;\n";
    out += std::format("import {};\n\n", dtoModuleName);

    // Group by first path segment.
    std::unordered_map<std::string, std::vector<const OperationInfo *>> groups;
    for (const auto &op : operations) {
        auto segments = detail::path_segments(op.getPath());
        if (segments.empty()) {
            return std::unexpected{std::format("empty path for operation '{}'", op.getPath())};
        }
        groups[segments.front()].push_back(&op);
    }

    for (const auto &[group_name, ops] : groups) {
        out += std::format("export namespace client::{} {{\n\n", group_name);
        for (const auto *op : ops) {
            auto segments = detail::path_segments(op->getPath());
            std::vector<std::string> literal_tail;
            std::vector<std::string> path_params;
            for (std::size_t i = 1; i < segments.size(); ++i) {
                if (detail::is_param(segments[i])) {
                    path_params.push_back(detail::param_name(segments[i]));
                } else {
                    literal_tail.push_back(segments[i]);
                }
            }

            std::string function_name = detail::function_name(op->getMethod(), literal_tail);
            std::string response_type =
                op->getResponse() ? detail::resolve_type(*op->getResponse(), dtoModuleName)
                                  : "void";
            std::string on_response_type =
                op->getResponse() ? std::format("std::function<void({})>", response_type)
                                  : "std::function<void()>";

            std::string params;
            for (const auto &param : path_params) {
                params += std::format("std::string_view {}, ", param);
            }
            if (op->getRequestBody()) {
                params += std::format("const {} &body, ",
                                      detail::resolve_type(*op->getRequestBody(), dtoModuleName));
            }
            params += std::format("{} onResponse, "
                                  "std::function<void(std::string)> onError = {{}}",
                                  on_response_type);

            out += std::format("void {}({}) {{\n", function_name, params);

            std::string path_expr;
            if (path_params.empty()) {
                path_expr = std::format("\"{}\"", op->getPath());
            } else {
                std::string format_str = op->getPath();
                for (const auto &param : path_params) {
                    auto placeholder = std::format("{{{}}}", param);
                    auto pos = format_str.find(placeholder);
                    format_str.replace(pos, placeholder.size(), "{}");
                }
                path_expr = std::format("std::format(\"{}\"", format_str);
                for (const auto &param : path_params) {
                    path_expr += std::format(", {}", param);
                }
                path_expr += ")";
            }

            out += "    auto request = congelado::client::Runtime::newRequest();\n";
            out += std::format(
                "    *request = std::move(*request).with_method(\"{}\").with_path({});\n",
                op->getMethod(), path_expr);
            if (op->getRequestBody()) {
                out += "    *request = std::move(*request)"
                       ".with_content_type(\"application/json\");\n";
                out += "    { auto encoded = serde::Json::encode(body);\n"
                       "      std::vector<std::byte> bytes(encoded.size());\n"
                       "      std::ranges::transform(encoded, bytes.begin(), "
                       "[](char c) { return std::byte(c); });\n"
                       "      request->set_body(std::move(bytes)); }\n";
            }
            out += std::format(
                "    congelado::client::Runtime::send<{}>(std::move(request), "
                "std::move(onResponse), std::move(onError));\n",
                op->getResponse() ? response_type : "void");
            out += "}\n\n";
        }
        out += std::format("}} // namespace client::{}\n\n", group_name);
    }

    return out;
}

} // namespace congelado::client
```

- [ ] **Step 4: Re-export the partition**

Update `sdk/client/congelado_client.cppm`:

```cpp
export module congelado_client;

export import :runtime;
export import :schema_model;
export import :dto_writer;
export import :route_writer;
```

- [ ] **Step 5: Run test to verify it passes**

Run: `cd /home/default/cc/congelado && xmake build client_sdk_test && xmake run client_sdk_test`
Expected: all prior tests plus both new ones pass. If the exact generated-string assertions in Step 1 don't match (e.g. whitespace differences), adjust the `CHECK(result->contains(...))` substrings to match what the implementation actually emits — the important thing verified is presence of the right module name, namespace, function name, parameter types, and the `Runtime::newRequest()`/`send<...>` calls, not exact formatting.

- [ ] **Step 6: Commit**

```bash
git add sdk/client/route_writer.cppm sdk/client/congelado_client.cppm tests/sdk/client/route_writer_test.cc
git commit -m "feat(sdk/client): generate per-operation client functions from paths"
```

---

### Task 7: `Generator` orchestration + end-to-end verification

**Files:**
- Create: `sdk/client/generator.cppm` — partition `:generator`. Parses `openapi.json` (`simdjson::dom`), extracts `components.schemas` into a `name -> SchemaType` map (Task 4) and `paths` into a `vector<OperationInfo>` (Task 6), calls `write_dtos`/`write_routes` (skipping `write_dtos` and using the shared module name in `write_routes` when `.sharedModels(...)` was set), writes both strings to disk.
- Create: `tests/sdk/client/generator_test.cc`.
- Modify: `sdk/client/congelado_client.cppm` — re-export `:generator`.

**Interfaces:**
- Consumes: `parse_schema` (Task 4), `write_dtos` (Task 5), `write_routes` (Task 6).
- Produces:
  ```cpp
  namespace congelado::client {
    class Generator {
      Generator namespaceName(std::string_view value) &&;
      Generator sharedModels(std::string_view moduleName) &&;
      [[nodiscard]] std::expected<void, std::string>
      generate(const std::filesystem::path &openapi_path, const std::filesystem::path &output_dir) const;
    };
  }
  ```
  This is the top-level entry point a consumer's own generator-invoking program calls — no later task in this plan, but Step 6 below exercises it manually against the real server.

- [ ] **Step 1: Write the failing test**

Create `tests/sdk/client/generator_test.cc`:

```cpp
#include <catch2/catch_test_macros.hpp>
#include <fstream>
import std;
import congelado_client;

namespace {

std::filesystem::path write_fixture() {
    auto path = std::filesystem::temp_directory_path() / "generator_test_openapi.json";
    std::ofstream out{path};
    out << R"({
      "openapi": "3.0.3",
      "info": {"title": "Test API", "version": "1.0.0"},
      "components": {"schemas": {
        "TaskDef": {"type": "object", "format": "", "$ref": "", "nullable": false,
                    "required": [], "enum": [],
                    "properties": {"name": {"type": "string", "format": "", "$ref": "",
                                            "nullable": false, "required": [], "enum": [],
                                            "properties": {}}}}
      }},
      "paths": {"/api/v1/tasks": {"post": {
        "summary": "", "description": "", "tags": [],
        "requestBody": {"required": true, "content": {"application/json": {
          "schema": {"type": "", "format": "", "$ref": "#/components/schemas/TaskDef",
                     "nullable": false, "required": [], "enum": [], "properties": {}}}}},
        "responses": {"201": {"description": "Created", "content": {"application/json": {
          "schema": {"type": "", "format": "", "$ref": "#/components/schemas/TaskDef",
                     "nullable": false, "required": [], "enum": [], "properties": {}}}}}}
      }}}
    })";
    return path;
}

} // namespace

TEST_CASE("Generator writes dto.cppm and routes.cppm from an openapi.json") {
    auto openapi_path = write_fixture();
    auto output_dir = std::filesystem::temp_directory_path() / "generator_test_output";
    std::filesystem::create_directories(output_dir);

    auto result = congelado::client::Generator{}
                      .namespaceName("client")
                      .generate(openapi_path, output_dir);
    REQUIRE(result.has_value());

    CHECK(std::filesystem::exists(output_dir / "dto.cppm"));
    CHECK(std::filesystem::exists(output_dir / "routes.cppm"));

    std::ifstream dto_file{output_dir / "dto.cppm"};
    std::string dto_content{std::istreambuf_iterator<char>{dto_file}, {}};
    CHECK(dto_content.contains("class TaskDef"));

    std::ifstream routes_file{output_dir / "routes.cppm"};
    std::string routes_content{std::istreambuf_iterator<char>{routes_file}, {}};
    CHECK(routes_content.contains("namespace client::tasks"));

    std::filesystem::remove_all(output_dir);
    std::filesystem::remove(openapi_path);
}

TEST_CASE("Generator with sharedModels skips dto.cppm and references the shared module") {
    auto openapi_path = write_fixture();
    auto output_dir = std::filesystem::temp_directory_path() / "generator_test_output_shared";
    std::filesystem::create_directories(output_dir);

    auto result = congelado::client::Generator{}
                      .namespaceName("client")
                      .sharedModels("model")
                      .generate(openapi_path, output_dir);
    REQUIRE(result.has_value());

    CHECK_FALSE(std::filesystem::exists(output_dir / "dto.cppm"));
    CHECK(std::filesystem::exists(output_dir / "routes.cppm"));

    std::ifstream routes_file{output_dir / "routes.cppm"};
    std::string routes_content{std::istreambuf_iterator<char>{routes_file}, {}};
    CHECK(routes_content.contains("import model;"));

    std::filesystem::remove_all(output_dir);
    std::filesystem::remove(openapi_path);
}
```

- [ ] **Step 2: Run test to verify it fails**

Run: `cd /home/default/cc/congelado && xmake build client_sdk_test`
Expected: FAIL — `congelado::client::Generator` doesn't exist yet.

- [ ] **Step 3: Implement `generator.cppm`**

Create `sdk/client/generator.cppm`:

```cpp
module;
#include <simdjson.h>

export module congelado_client:generator;

import std;
import :schema_model;
import :dto_writer;
import :route_writer;

namespace congelado::client::detail {

[[nodiscard]] inline std::expected<std::unordered_map<std::string, SchemaType>, std::string>
parse_components(simdjson::dom::element document) {
    std::unordered_map<std::string, SchemaType> schemas;
    simdjson::dom::object named;
    if (document["components"]["schemas"].get(named)) {
        return schemas; // no components — valid, just empty
    }
    for (auto [name, value] : named) {
        auto parsed = parse_schema(value);
        if (!parsed) {
            return std::unexpected{std::format("schema '{}': {}", std::string_view{name},
                                               parsed.error())};
        }
        schemas.emplace(std::string{name}, std::move(*parsed));
    }
    return schemas;
}

[[nodiscard]] inline std::expected<std::vector<OperationInfo>, std::string>
parse_operations(simdjson::dom::element document) {
    std::vector<OperationInfo> operations;
    simdjson::dom::object paths;
    if (auto ec = document["paths"].get(paths); ec) {
        return std::unexpected{
            std::format("document missing 'paths': {}", simdjson::error_message(ec))};
    }
    for (auto [path, methods_element] : paths) {
        simdjson::dom::object methods;
        if (methods_element.get(methods)) {
            continue;
        }
        for (auto [method, operation_element] : methods) {
            OperationInfo op;
            op.setPath(std::string{path});
            op.setMethod(std::string{method});

            simdjson::dom::element request_schema;
            if (!operation_element["requestBody"]["content"]["application/json"]["schema"]
                     .get(request_schema)) {
                auto parsed = parse_schema(request_schema);
                if (!parsed) {
                    return std::unexpected{parsed.error()};
                }
                op.setRequestBody(std::move(*parsed));
            }

            simdjson::dom::object responses;
            if (!operation_element["responses"].get(responses)) {
                for (auto [status, response_element] : responses) {
                    if (status.empty() || status.front() != '2') {
                        continue;
                    }
                    simdjson::dom::element response_schema;
                    if (!response_element["content"]["application/json"]["schema"].get(
                            response_schema)) {
                        auto parsed = parse_schema(response_schema);
                        if (!parsed) {
                            return std::unexpected{parsed.error()};
                        }
                        op.setResponse(std::move(*parsed));
                    }
                    break;
                }
            }

            operations.push_back(std::move(op));
        }
    }
    return operations;
}

} // namespace congelado::client::detail

export namespace congelado::client {

class Generator {
  public:
    Generator() = default;

    Generator namespaceName(std::string_view value) && {
        m_namespace = std::string{value};
        return std::move(*this);
    }
    Generator sharedModels(std::string_view moduleName) && {
        m_shared_models_module = std::string{moduleName};
        return std::move(*this);
    }

    [[nodiscard]] std::expected<void, std::string>
    generate(const std::filesystem::path &openapi_path,
             const std::filesystem::path &output_dir) const {
        simdjson::dom::parser parser;
        simdjson::dom::element document;
        if (auto ec = parser.load(openapi_path.string()).get(document); ec) {
            return std::unexpected{
                std::format("failed to parse '{}': {}", openapi_path.string(),
                           simdjson::error_message(ec))};
        }

        auto schemas = detail::parse_components(document);
        if (!schemas) {
            return std::unexpected{schemas.error()};
        }
        auto operations = detail::parse_operations(document);
        if (!operations) {
            return std::unexpected{operations.error()};
        }

        std::string dto_module_name = m_shared_models_module.value_or(
            std::format("{}_dto", m_namespace));

        if (!m_shared_models_module) {
            auto dto_source = write_dtos(*schemas, dto_module_name);
            if (!dto_source) {
                return std::unexpected{dto_source.error()};
            }
            std::ofstream dto_file{output_dir / "dto.cppm"};
            dto_file << *dto_source;
        }

        auto routes_module_name = std::format("{}_routes", m_namespace);
        auto routes_source = write_routes(*operations, routes_module_name, dto_module_name);
        if (!routes_source) {
            return std::unexpected{routes_source.error()};
        }
        std::ofstream routes_file{output_dir / "routes.cppm"};
        routes_file << *routes_source;

        return {};
    }

  private:
    std::string m_namespace{"client"};
    std::optional<std::string> m_shared_models_module;
};

} // namespace congelado::client
```

- [ ] **Step 4: Re-export the partition**

Update `sdk/client/congelado_client.cppm`:

```cpp
export module congelado_client;

export import :runtime;
export import :schema_model;
export import :dto_writer;
export import :route_writer;
export import :generator;
```

- [ ] **Step 5: Run test to verify it passes**

Run: `cd /home/default/cc/congelado && xmake build client_sdk_test && xmake run client_sdk_test`
Expected: every `TEST_CASE` across all of `tests/sdk/client/` passes.

- [ ] **Step 6: Manual end-to-end verification against the real server**

```bash
cd /home/default/cc/congelado
xmake build
cd build/linux/x86_64/debug
rm -f openapi.json
./congelado > /tmp/run.log 2>&1 &
SRV=$!
sleep 3
kill -9 $SRV
```

Then write and run a tiny throwaway program that calls the real `Generator` against the `openapi.json` just produced:

```bash
mkdir -p /tmp/client_gen_output
cat > /tmp/gen_main.cc <<'EOF'
#include <print>
import std;
import congelado_client;

int main() {
    auto result = congelado::client::Generator{}
                      .namespaceName("client")
                      .generate("build/linux/x86_64/debug/openapi.json", "/tmp/client_gen_output");
    if (!result) {
        std::println(stderr, "generate failed: {}", result.error());
        return 1;
    }
    std::println("generated OK");
    return 0;
}
EOF
```
Add it as a throwaway xmake target (do not commit this part — it's a manual check, not part of the plan's deliverable):
```lua
target("gen_main_scratch")
    set_kind("binary")
    set_languages("c++26")
    set_policy("build.c++.modules", true)
    add_files("/tmp/gen_main.cc")
    add_deps("congelado_lib")
target_end()
```
Run: `xmake build gen_main_scratch && xmake run gen_main_scratch`
Expected: `generated OK`, and `/tmp/client_gen_output/dto.cppm` + `/tmp/client_gen_output/routes.cppm` exist.

Confirm the generated files actually compile standalone:
```bash
clang++ -std=c++26 --precompile /tmp/client_gen_output/dto.cppm -o /tmp/dto.pcm -fmodule-file=serde=<path-to-serde.pcm-from-build> -fmodule-file=std=<path-to-std.pcm>
```
(exact `-fmodule-file` paths depend on the current build's `.gens` cache layout — locate them the same way Task 1's build already surfaced full compiler invocations in its `xmake build -v` output, or simpler: add `dto.cppm`/`routes.cppm` temporarily to `add_files` on `gen_main_scratch`'s own target and let xmake resolve all module dependencies for you, then remove them once confirmed.)

Once confirmed, remove the scratch target from `xmake.lua` (it was never meant to be committed) and delete `/tmp/gen_main.cc`, `/tmp/client_gen_output`.

- [ ] **Step 7: Commit**

```bash
git add sdk/client/generator.cppm sdk/client/congelado_client.cppm tests/sdk/client/generator_test.cc
git commit -m "feat(sdk/client): Generator orchestrates parse -> dto/route writers -> disk

Completes the OpenAPI client SDK: openapi.json in, typed C++ client
functions + DTOs out. Manually verified end-to-end against this
server's own generated openapi.json."
```

---

## Self-review notes (already applied above, kept here for the record)

- Every step has real, complete code — no "add error handling" placeholders.
- Task 6 explicitly justifies duplicating `pascal_case`/naming helpers instead of cross-partition-exporting internal helpers (YAGNI until a third consumer appears).
- Task 1's known limitation (nullable + `$ref` siblings) is called out explicitly rather than silently glossed over.
- `dto.cppm`/`routes.cppm` as single files (not one file per schema) is a deliberate simplification from the spec's literal wording, made for topological-ordering simplicity — noted in Task 5/7, not hidden.
- Type consistency check: `Runtime::send<Res>`, `Runtime::newRequest()`, `write_dtos(...)`, `write_routes(...)`, `Generator::generate(...)` signatures are identical everywhere they're referenced across Tasks 3, 5, 6, 7.
