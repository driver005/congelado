# OpenAPI client SDK: generator + Runtime

## Context

The server side already auto-generates an OpenAPI document from route registration (`utils::openapi` — see route metadata via `ApiRoute`/`ApiRouter`, `Registry`, `Generator`, served at `/openapi.json` and written to disk on startup). The next step is a **client-side SDK**: given that `openapi.json`, generate C++ client code (typed request functions + response DTOs) so a consumer just calls a function and supplies a response handler — no manual request building, no manual response deserialization, no manual request/response correlation.

This lives under `sdk/client/`, parallel to the existing `sdk/plugin/` and `sdk/worker/` SDK pieces. Unlike those (macro-based C ABI headers), this is a real C++ module since it contains actual parsing/codegen logic, not preprocessor macros.

Scope note: **how/when the generator is invoked is the consumer's problem.** `sdk/client` ships the `Generator` type as a library; it does not ship a `main.cc` or an xmake binary target for running it.

## Server-side generator change: named schemas + `$ref`

Today `utils::openapi`'s schema builder always inlines schemas — no `components.schemas`, no `$ref`, no dedup. That was an intentional simplification when the server-side generator was first built, but it blocks client codegen: without a name, generated client code can't know to emit one reusable `TaskDef` struct instead of a fresh anonymous one per operation.

**Change**: `build_schema<T>()` (in `utils/openapi/schema.cppm`) now:
1. Derives a bare type name for any `serde::ISerializable` `T` via `rfl::type_name_t<T>().str()`, stripped to the last `::`-delimited segment (`model::TaskDef` → `"TaskDef"`).
2. On first encountering a given `T`, registers its full object schema once under a new static `Components` singleton (same pattern as `utils::openapi::Registry`) keyed by that name, in `utils/openapi/model.cppm`.
3. Every reference to that `T` (including recursive/repeated ones across different operations) emits `{"$ref": "#/components/schemas/TaskDef"}` instead of re-inlining the object schema.

`SchemaObject` gains a `$ref` string field (empty when not a ref; when set, it's the only field OpenAPI readers should look at, but the others still serialize as their defaults — acceptable, matches the project's existing "always populate every field" simplification for this type). `Document` gains a `components` field (`Components` — `{schemas: unordered_map<string, SchemaObject>}`) alongside `paths`, populated from the same static singleton at `Generator::generate()` time.

This only touches already-shipped files: `utils/openapi/model.cppm`, `schema.cppm`, `generator.cppm`. No changes to `route.cppm`, `registry.cppm`, or any call site in `engine/routes.cppm` — `.body<T>()`/`.response<T>()` keep calling `build_schema<T>()` exactly as before; the naming/dedup happens inside `build_schema` itself.

## `sdk/client` module

New module `congelado_client`, file `sdk/client/congelado_client.cppm`, registered in `xmake.lua` the same way `sdk/plugin/congelado_plugin.cppm` is (`add_files("sdk/client/congelado_client.cppm", { public = true })`).

### `Runtime`

A single hand-written class (not generated), static-singleton style (matches `core::logger::LoggerRegistry` and `utils::openapi::Registry` already in this codebase):

```cpp
namespace congelado::client {

class Runtime {
  public:
    static void setClient(interfaces::IClient &client) noexcept;
    static interfaces::IClient &getClient() noexcept;  // aborts if unset — precondition

    // interfaces::io::IRequest's base-class virtuals (set_header, get_body, ...) all
    // std::abort() — it's a CRTP base, not directly constructible/usable. A real request
    // must be a concrete protocol subtype (e.g. io::layer::http2::HttpRequest), which
    // Runtime — deliberately protocol-agnostic — cannot construct itself. The consumer
    // supplies a factory once at startup alongside the client.
    using RequestFactory = std::function<std::unique_ptr<interfaces::io::IRequest>(std::uint32_t streamId)>;
    static void setRequestFactory(RequestFactory factory) noexcept;

    // Assigns the next stream id and asks the factory for a concrete request carrying it.
    [[nodiscard]] static std::unique_ptr<interfaces::io::IRequest> newRequest();

    template <typename Res>
    static void send(std::unique_ptr<interfaces::io::IRequest> request,
                     std::function<void(Res)> onResponse,
                     std::function<void(std::string)> onError = [](std::string) {});

    // Wired by the consumer as the ReceiveDispatchFn passed to IProtocol::get_client(dispatch).
    static void dispatch(interfaces::io::IRequest &request, interfaces::io::IResponse &response);

  private:
    static inline interfaces::IClient *client = nullptr;
    static inline RequestFactory requestFactory;
    static inline std::unordered_map<std::uint32_t,
                                     std::function<void(interfaces::io::IResponse &)>> pending;
    static inline std::uint32_t nextStreamId{1};
};

} // namespace congelado::client
```

- `newRequest()`: calls `requestFactory(nextStreamId++)` — the returned request already carries the right stream id (concrete request types take it in their constructor, e.g. `HttpRequest(std::uint32_t stream_id)`), so no separate `set_stream_id()` call is needed.
- `send<Res>()`: takes an already-built request (generated code calls `Runtime::newRequest()` then chains `.with_method(...).with_path(...).with_content_type(...).with_body(...)` — these fluent builders dispatch to the concrete subtype's overridden virtuals, so they work correctly on the object `newRequest()` returned, just not on a bare `IRequest`). Wraps `onResponse`/`onError` into a type-erased closure that runs `serde::Ser::deserialize<Res>(response.get_content_type(), response.get_body())` and dispatches to one or the other, stores it in `pending` keyed by `request->get_stream_id()`, then calls `getClient().send(*request)`.
- `dispatch()`: looks up `request.get_stream_id()` in `pending`; if found, invokes it with `response` and erases the entry; if not found, no-ops (a response for something no longer tracked is not an error — e.g. after a future timeout feature drops it).
- Both `IRequest` and `IResponse` already carry `get_stream_id()`/`set_stream_id()` (`interfaces/io/request.cppm`, `interfaces/io/response.cppm`) — this is the existing correlation mechanism, not new plumbing.

### `Generator`

```cpp
namespace congelado::client {

class Generator {
  public:
    Generator() = default;

    Generator namespaceName(std::string_view value) &&;   // default "client"
    Generator sharedModels(std::string_view moduleName) &&; // opt-in, see below

    [[nodiscard]] std::expected<void, std::string>
    generate(const std::filesystem::path &openapi_path,
             const std::filesystem::path &output_dir) const;
};

} // namespace congelado::client
```

Parses `openapi.json` with `simdjson::dom` (whole-document read-then-walk — simpler than `ondemand` for this one-shot, not-performance-sensitive use), then:

1. **DTOs** — for each `components.schemas.<Name>`:
   - **Default (no `sharedModels` set)**: emit `<output_dir>/dto/<name>.cppm` (module `<namespaceName>_dto`, e.g. `client_dto`) — one class per schema, same shape as `model::TaskDef` (private `m_`-prefixed members, `get`/`set` accessors, a `serde::Serializable<T>` specialization) so `serde::Ser::deserialize<T>`/`Runtime::send<T>` work unmodified. `$ref` properties map to the referenced DTO type (already-generated or forward-declared as needed).
   - **If `sharedModels("model")` (or similar) is set**: DTO generation is skipped entirely; generated route code does `import model;` and references `model::<SchemaName>` directly. Precondition (unverified at codegen time, since the generator only has JSON, not the target module's AST): the named module must already have a type of that exact name satisfying `serde::ISerializable`. If wrong, it fails at the generated code's compile time, not at generation time — acceptable fast-fail for a codegen mismatch. This is what lets an in-repo client (this same codebase) reuse `model::TaskDef`/`model::WorkflowDef` instead of duplicating them.

2. **Routes** — for each `paths.<path>.<method>`, emit one function into `<output_dir>/routes.cppm` (module `<namespaceName>_routes`, e.g. `client_routes`), grouped into `namespace <namespaceName>::<first-path-segment-after-common-prefix>` (e.g. `client::tasks`). Function name: snake_case of `<method>_<remaining-path-segments-with-params-dropped>` (e.g. `POST /api/v1/tasks/{name}/enqueue` → `post_enqueue(std::string_view name, ...)`). No `operationId` needed from the spec — purely mechanical from path + method, since our server-side generator doesn't emit `operationId` today and adding it is out of scope for this feature.
   - Request body param (if the operation has one): the DTO/shared-model type, taken by `const&`.
   - Path params (`{name}` segments): `std::string_view`, in path order, before the body param.
   - Always-present trailing params: `std::function<void(ResponseType)> onResponse`, `std::function<void(std::string)> onError = {}` (defaulted no-op).
   - Body: calls `Runtime::newRequest()`, chains `.with_method(...).with_path(interpolated)` and, if there's a request body, `.with_content_type("application/json")` + `.with_body(...)` with the JSON-encoded bytes, then calls `Runtime::send<ResponseType>(std::move(request), std::move(onResponse), std::move(onError))`.
   - If an operation has multiple response status codes, use the `2xx` one for the typed callback (first `2xx` found; if none, `void` response — callback becomes `std::function<void()>`).

## Consumer usage (once generated)

```cpp
congelado::client::Runtime::setClient(myHttp2Client);   // once, at startup
congelado::client::Runtime::setRequestFactory([](std::uint32_t streamId) {
    return std::make_unique<io::layer::http2::HttpRequest>(streamId);
});

client::tasks::post(def,
    [](client::dto::TaskDef created) { /* success */ },
    [](std::string error) { /* decode/transport error */ });
```

## Error handling

- `Runtime::getClient()` before `setClient()`, or `newRequest()` before `setRequestFactory()`: precondition violation → `std::abort()` (matches existing project convention, e.g. `IResponse`'s virtual defaults).
- Response deserialize failure: `onError` called with the decode error message, not `onResponse` with a default-constructed value.
- Stray/untracked stream id on `dispatch()`: silent no-op, not an error.
- `Generator::generate()` I/O or parse failure (bad JSON, unwritable output dir): returns `std::expected<void, std::string>` rather than throwing, matching `core::config::load`/`SharedLibrary::open_all`'s existing style.

## Testing / verification

- **Server-side schema change**: rebuild, run the app, `curl /openapi.json`, confirm `components.schemas.TaskDef` (etc.) exist and operations reference them via `$ref`. No regression in existing paths/methods (already covered by the prior session's verification).
- **Generator**: run it against this server's own real `openapi.json` (dogfooding, no separate fixture needed), generate into a scratch dir, confirm the emitted `.cppm` files compile standalone.
- **`Runtime`**: unit-style test with a fake `IClient` (records the request it was given, synchronously invokes a stored dispatch callback with a canned response) and a request factory returning a minimal concrete test `IRequest` subclass (in-memory header/body storage, no real networking); assert `Runtime::send<T>()`'s callback fires with the correctly deserialized value, keyed by the right stream id.
- **End-to-end** (optional/manual, not required for this spec to land): generate a client for the real engine API with `.sharedModels("model")`, run it against the live server from the earlier verification session. Flagged optional given network-sandbox flakiness hit earlier in this session.
