# Database & Cache Interfaces Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Add `IDatabase`, `ICache`, `IDbCodec<T>`, and `ICacheCodec<T>` interfaces as C++20 module partitions of `interfaces`, enabling plugins to dispatch typed CRUD and key-value operations to persistent backend connections.

**Architecture:** `IDatabase` exposes `query/insert/update/remove(string_view, QueryReadFn)` and `ICache` exposes `get/set/remove` — both are virtual base classes with no `on_connect` since connections are persistent from plugin load. Codec interfaces `IDbCodec<T>` and `ICacheCodec<T>` are CRTP-adjacent templates implemented by app plugins to convert domain types to/from `string_view`. A single new callback type `shared::QueryReadFn` carries `string_view` results back to callers.

**Tech Stack:** C++23 modules (`.cppm`), xmake (`add_files("include/**.cppm")` auto-picks up new files), clang++.

---

## File Map

| Action | Path | Responsibility |
|---|---|---|
| Modify | `include/shared/flow.cppm` | Add `QueryReadFn` callback type |
| Create | `include/interfaces/database.cppm` | `IDatabase` virtual interface |
| Create | `include/interfaces/cache.cppm` | `ICache` virtual interface |
| Create | `include/interfaces/db_codec.cppm` | `IDbCodec<T>` + `DbCodec` concept |
| Create | `include/interfaces/cache_codec.cppm` | `ICacheCodec<T>` + `CacheCodec` concept |
| Modify | `include/interfaces/interfaces.cppm` | Re-export all four new partitions |

---

## Task 1: Add `QueryReadFn` to `shared::flow`

**Files:**
- Modify: `include/shared/flow.cppm`

`QueryReadFn` carries `string_view` results from database/cache backends back to callers. It lives in `shared::` alongside the existing `ReadCallback`, `SendCallback`, etc.

- [ ] **Step 1: Add the type**

Open `include/shared/flow.cppm`. After line 13 (`using CompletionCallback = ...`), add:

```cpp
using QueryReadFn = std::move_only_function<void(std::string_view)>;
```

Full file after edit:

```cpp
export module shared:flow;

import std;
import utils_buffering;
import :handler;

export namespace shared {

using ReadCallback = std::move_only_function<void(utils::buffering::BufferReader &)>;
using SendCallback = std::move_only_function<void(utils::buffering::BufferNode &&)>;
using CloseCallback = std::move_only_function<void()>;
using ErrorCallback = std::move_only_function<void(int, int)>;
using CompletionCallback = std::move_only_function<void(int)>;
using QueryReadFn = std::move_only_function<void(std::string_view)>;

template <typename T>
concept FlowLayer = requires(SendCallback send, CloseCallback close) {
    { T(send, close) };
} && requires(T t) {
    { t.on_read() } -> std::convertible_to<ReadCallback>;
};


template <typename T, typename Controller, typename Leverager>
concept FlowBase =
    HandlerController<Controller> && requires(ReadCallback &&on_read, Leverager &leverager, Controller controller) {
        T{std::move(on_read), leverager, controller};
    } && requires(T t, int fd) {
        { t.on_send(fd) } -> std::convertible_to<SendCallback>;
    };

} // namespace shared
```

- [ ] **Step 2: Verify build**

```bash
xmake build 2>&1 | tail -5
```

Expected: no errors. If build fails, check that `import std;` is present (it is) — `std::move_only_function` and `std::string_view` are in `<functional>` and `<string_view>` which are covered by `import std;`.

- [ ] **Step 3: Commit**

```bash
git add include/shared/flow.cppm
git commit -m "feat(shared): add QueryReadFn string_view result callback"
```

---

## Task 2: Create `interfaces:database`

**Files:**
- Create: `include/interfaces/database.cppm`

`IDatabase` is the backend interface. Backend plugins (Postgres driver, SQLite, etc.) implement it. `required()` defaults `false`; override to `true` to force app abort on load failure. The four methods map to SQL CRUD semantics; the backend executes whatever `string_view` payload the codec produces.

- [ ] **Step 1: Create the file**

```cpp
export module interfaces:database;

import std;
import shared;

export namespace interfaces {

class IDatabase {
  public:
    virtual ~IDatabase() = default;

    [[nodiscard]] virtual std::string_view backend_name() const noexcept = 0;
    [[nodiscard]] virtual bool required() const noexcept { return false; }

    virtual void query(std::string_view payload, shared::QueryReadFn result) = 0;
    virtual void insert(std::string_view payload, shared::QueryReadFn result) = 0;
    virtual void update(std::string_view payload, shared::QueryReadFn result) = 0;
    virtual void remove(std::string_view payload, shared::QueryReadFn result) = 0;
};

} // namespace interfaces
```

- [ ] **Step 2: Verify build**

```bash
xmake build 2>&1 | tail -5
```

Expected: no errors. Note: xmake picks up `include/**.cppm` automatically — no xmake.lua change needed.

- [ ] **Step 3: Commit**

```bash
git add include/interfaces/database.cppm
git commit -m "feat(interfaces): add IDatabase virtual interface"
```

---

## Task 3: Create `interfaces:cache`

**Files:**
- Create: `include/interfaces/cache.cppm`

`ICache` is the backend interface for key-value stores. `get` retrieves by key, `set` stores key+value, `remove` deletes by key. All ops are async — result arrives via `QueryReadFn` callback. Same `required()` semantics as `IDatabase`.

- [ ] **Step 1: Create the file**

```cpp
export module interfaces:cache;

import std;
import shared;

export namespace interfaces {

class ICache {
  public:
    virtual ~ICache() = default;

    [[nodiscard]] virtual std::string_view backend_name() const noexcept = 0;
    [[nodiscard]] virtual bool required() const noexcept { return false; }

    virtual void get(std::string_view key, shared::QueryReadFn result) = 0;
    virtual void set(std::string_view key, std::string_view value, shared::QueryReadFn result) = 0;
    virtual void remove(std::string_view key, shared::QueryReadFn result) = 0;
};

} // namespace interfaces
```

- [ ] **Step 2: Verify build**

```bash
xmake build 2>&1 | tail -5
```

Expected: no errors.

- [ ] **Step 3: Commit**

```bash
git add include/interfaces/cache.cppm
git commit -m "feat(interfaces): add ICache virtual interface"
```

---

## Task 4: Create `interfaces:db_codec`

**Files:**
- Create: `include/interfaces/db_codec.cppm`

`IDbCodec<T>` is implemented by **app plugins** (not backend plugins). It converts a domain type `T` to per-operation SQL/wire strings and decodes result rows back to `T`. One `encode_*` method per CRUD operation — each produces a complete `string_view`-compatible payload. `decode` fills an existing `T` by reference (avoids allocation).

- [ ] **Step 1: Create the file**

```cpp
export module interfaces:db_codec;

import std;

export namespace interfaces {

template <typename T>
class IDbCodec {
  public:
    virtual ~IDbCodec() = default;

    [[nodiscard]] virtual std::string encode_query(T const &) const = 0;
    [[nodiscard]] virtual std::string encode_insert(T const &) const = 0;
    [[nodiscard]] virtual std::string encode_update(T const &) const = 0;
    [[nodiscard]] virtual std::string encode_remove(T const &) const = 0;

    virtual void decode(std::string_view result, T &out) const = 0;
};

template <typename Codec, typename T>
concept DbCodec = std::derived_from<Codec, IDbCodec<T>>;

} // namespace interfaces
```

- [ ] **Step 2: Verify build**

```bash
xmake build 2>&1 | tail -5
```

Expected: no errors.

- [ ] **Step 3: Commit**

```bash
git add include/interfaces/db_codec.cppm
git commit -m "feat(interfaces): add IDbCodec template and DbCodec concept"
```

---

## Task 5: Create `interfaces:cache_codec`

**Files:**
- Create: `include/interfaces/cache_codec.cppm`

`ICacheCodec<T>` is implemented by **app plugins**. It derives a cache key from `T` via `key()`, serializes `T` to a value string via `encode()`, and reconstructs `T` from a stored value via `decode()`. Compared to `IDbCodec<T>`, it has a single `encode` (no per-operation variants) but adds `key()` since cache access requires explicit key derivation.

- [ ] **Step 1: Create the file**

```cpp
export module interfaces:cache_codec;

import std;

export namespace interfaces {

template <typename T>
class ICacheCodec {
  public:
    virtual ~ICacheCodec() = default;

    [[nodiscard]] virtual std::string key(T const &) const = 0;
    [[nodiscard]] virtual std::string encode(T const &) const = 0;

    virtual void decode(std::string_view value, T &out) const = 0;
};

template <typename Codec, typename T>
concept CacheCodec = std::derived_from<Codec, ICacheCodec<T>>;

} // namespace interfaces
```

- [ ] **Step 2: Verify build**

```bash
xmake build 2>&1 | tail -5
```

Expected: no errors.

- [ ] **Step 3: Commit**

```bash
git add include/interfaces/cache_codec.cppm
git commit -m "feat(interfaces): add ICacheCodec template and CacheCodec concept"
```

---

## Task 6: Wire up `interfaces.cppm`

**Files:**
- Modify: `include/interfaces/interfaces.cppm`

Add four `export import` lines so consumers of `import interfaces;` automatically get all new types.

- [ ] **Step 1: Add the re-exports**

Full file after edit:

```cpp
export module interfaces;

export import :logger;
export import :io;
export import :protocol;
export import :status;
export import :request;
export import :response;
export import :database;
export import :cache;
export import :db_codec;
export import :cache_codec;
```

- [ ] **Step 2: Verify full build**

```bash
xmake build 2>&1 | tail -10
```

Expected: no errors. All six partitions plus four new ones resolve cleanly.

- [ ] **Step 3: Commit**

```bash
git add include/interfaces/interfaces.cppm
git commit -m "feat(interfaces): re-export database, cache, db_codec, cache_codec partitions"
```
