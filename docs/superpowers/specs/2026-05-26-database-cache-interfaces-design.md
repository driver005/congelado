# IDatabase & ICache Interfaces Design

**Date:** 2026-05-26
**Status:** Approved

## Context

congelado is a stateless C++ server framework. Plugins implement business logic. This spec adds database and cache backend interfaces so plugins can dispatch queries/commands to persistent backend connections established at load time.

## Goals

- Generic backend interfaces supporting any database (PostgreSQL, SQLite, …) and any cache (Redis, Memcached, …)
- Explicit CRUD operations on `IDatabase`; explicit key-value operations on `ICache`
- Plugin-owned codec interfaces that convert domain types to/from `string_view` — keeps query logic in the app plugin, not the framework
- Persistent connections established on plugin load; `required()` flag forces app abort on load failure

## Architecture

### New Shared Callback Type

One `string_view`-based result callback added to `shared::flow`:

```cpp
// shared/flow.cppm
using QueryReadFn = std::move_only_function<void(std::string_view)>;
```

No `QuerySendFn` needed — payload is passed directly as `string_view` on each operation call.

### Flow

```
app plugin
  → IDbCodec<T>::encode_insert(T)   → std::string (SQL / wire payload)
  → IDatabase::insert(payload, cb)   → backend executes
  ← QueryReadFn cb(result)           ← std::string_view (result row)
  → IDbCodec<T>::decode(sv)          → T
```

Cache follows the same pattern with `ICacheCodec<T>::key()` + `encode()` driving `ICache::set()`.

### Module Partitions

All four new interfaces are partitions of `interfaces` and re-exported from `interfaces.cppm`:

| Partition | File | Implemented by |
|---|---|---|
| `interfaces:database` | `include/interfaces/database.cppm` | backend plugin |
| `interfaces:cache` | `include/interfaces/cache.cppm` | backend plugin |
| `interfaces:db_codec` | `include/interfaces/db_codec.cppm` | app plugin |
| `interfaces:cache_codec` | `include/interfaces/cache_codec.cppm` | app plugin |

## Interface Definitions

### `interfaces:database`

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

### `interfaces:cache`

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

### `interfaces:db_codec`

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

### `interfaces:cache_codec`

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

### `interfaces.cppm` additions

```cpp
export import :database;
export import :cache;
export import :db_codec;
export import :cache_codec;
```

## Design Decisions

- **Explicit CRUD on `IDatabase`** — `query/insert/update/remove` make intent clear; codec provides per-operation encoding
- **Explicit key-value ops on `ICache`** — `get/set/remove`; cache has no concept of rows so `IDbCodec`-style split would be unnecessary
- **No `on_connect` / `CloseCallback`** — connections are persistent from load; backend plugin manages reconnect internally
- **`required() = false` default** — opt-in abort; critical backends override to `true`
- **`string_view` payload** — lightweight, no ownership transfer; backend and codec own their string storage
- **Separate `IDbCodec` / `ICacheCodec`** — database results are rows (need 4 encode variants); cache results are blobs (single encode + `key()` sufficient)
