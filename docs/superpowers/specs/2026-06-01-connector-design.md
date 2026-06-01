# Connector Design

**Date:** 2026-06-01
**Status:** Approved

## Summary

New `include/connector/` module providing a typed, cache-first database accessor for any `serde::ISerializable<T>` model. Abstracts the pattern: check cache → query DB → populate cache. Provides local in-memory fallbacks when cache or DB is unavailable.

---

## Architecture

### New files

```
include/connector/
  local_cache.cppm   — module connector:local_cache — LocalCache : ICache (in-memory, thread-safe)
  connector.cppm     — module connector             — Connector<T> + re-exports
```

### Modified files

```
include/serde/serde.cppm   — add FieldOptions, FieldOptionsDb, extend Serializable<T> + ISerializable
```

---

## Serde Extensions

### `FieldOptionsDb`

Compile-time field-level DB metadata. Structural type — valid as NTTP.

```cpp
struct FieldOptionsDb {
    bool primary_key = false;
    bool unique      = false;
    bool nullable    = true;
    bool skip_insert = false;
    bool skip_update = false;
    std::string_view ref_table  = {};
    std::string_view ref_column = {};

    static constexpr FieldOptionsDb init() { return {}; }

    constexpr FieldOptionsDb pk()        const { auto o=*this; o.primary_key=true; return o; }
    constexpr FieldOptionsDb not_null()  const { auto o=*this; o.nullable=false;   return o; }
    constexpr FieldOptionsDb no_insert() const { auto o=*this; o.skip_insert=true; return o; }
    constexpr FieldOptionsDb no_update() const { auto o=*this; o.skip_update=true; return o; }
    constexpr FieldOptionsDb references(std::string_view t, std::string_view c) const {
        auto o=*this; o.ref_table=t; o.ref_column=c; return o;
    }
};
```

### `FieldOptions`

Top-level options container — extensible for future backends (cache, search, etc.).

```cpp
struct FieldOptions {
    FieldOptionsDb db{};
    // FieldOptionsCache cache{};  — future

    static constexpr FieldOptions init() { return {}; }

    constexpr FieldOptions with_db(FieldOptionsDb d) const { auto o=*this; o.db=d; return o; }
};
```

### `FieldDesc` — gains `Opts` NTTP

```cpp
template <rfl::internal::StringLiteral Name, auto Getter, auto Setter,
          FieldOptions Opts = FieldOptions{}>
struct FieldDesc {
    static constexpr auto name    = Name;
    static constexpr auto getter  = Getter;
    static constexpr auto setter  = Setter;
    static constexpr auto options = Opts;
    using ClassType  = typename MFPTraits<decltype(Getter)>::class_t;
    using ValueType  = typename MFPTraits<decltype(Getter)>::value_t;
};

template <rfl::internal::StringLiteral Name, auto Getter, auto Setter,
          FieldOptions Opts = FieldOptions{}>
constexpr auto field() {
    return FieldDesc<Name, Getter, Setter, Opts>{};
}
```

### `Serializable<T>` — gains `table_name()`

```cpp
template <typename T>
struct Serializable {
    static auto fields();                                  // existing
    static constexpr std::string_view table_name();        // new
};
```

### `ISerializable` concept — tightened

```cpp
template <typename T>
concept ISerializable = requires {
    { Serializable<T>::fields() };
    { Serializable<T>::table_name() } -> std::convertible_to<std::string_view>;
};
```

### Model specialization example

```cpp
template <> struct Serializable<TaskInstance> {
    static constexpr std::string_view table_name() { return "task_instances"; }
    static auto fields() {
        return std::make_tuple(
            field<"id",          &T::id,          &T::set_id,
                  FieldOptions::init().with_db(FieldOptionsDb::init().pk().not_null())>(),
            field<"workflow_id", &T::workflow_id, &T::set_workflow_id,
                  FieldOptions::init().with_db(FieldOptionsDb::init().references("workflows","id"))>(),
            field<"created_at",  &T::created_at,  &T::set_created_at,
                  FieldOptions::init().with_db(FieldOptionsDb::init().no_update())>(),
            field<"name",        &T::name,        &T::set_name>()
        );
    }
};
```

The connector finds the PK field by scanning `fields()` for `options.db.primary_key == true` at compile time — no separate `pk_field()` needed.

---

## `LocalCache` — `include/connector/local_cache.cppm`

Concrete `ICache` implementation backed by an in-memory `std::unordered_map`. Used when no external cache is provided.

```
module connector:local_cache
namespace connector

class LocalCache : public interfaces::ICache {
    std::unordered_map<std::string, std::string> store_
    mutable std::shared_mutex mutex_

    backend_name() → "local"
    required()     → false
    get(key, cb)   → shared_lock → invoke cb(value) or cb("")
    set(key, val, cb) → unique_lock → store[key]=val → invoke cb("")
    remove(key, cb)   → unique_lock → erase → invoke cb("")
}
```

`QueryReadFn` callback receives empty string_view on miss/confirmation.

---

## `Connector<T>` — `include/connector/connector.cppm`

### Template constraint

```cpp
template <serde::ISerializable T>
class Connector { ... };
```

### Constructor

```cpp
Connector(interfaces::ICache* cache, interfaces::IDatabase* db);
// nullptr → use local_cache_ / local_store_ respectively
```

### Full API

```cpp
// Schema
void create(std::move_only_function<void(bool)> cb) noexcept;

// Read (cache-first)
void find(std::string_view pk,
          std::move_only_function<void(std::optional<T>)> cb)   noexcept;
void find_many(std::span<const std::string_view> pks,
               std::move_only_function<void(std::vector<T>)> cb) noexcept;
void find_all(std::move_only_function<void(std::vector<T>)> cb) noexcept;

// Write (write-through: cache + DB parallel)
void insert(const T& val,              std::move_only_function<void(bool)> cb) noexcept;
void insert_many(std::span<const T>,   std::move_only_function<void(bool)> cb) noexcept;
void update(const T& val,              std::move_only_function<void(bool)> cb) noexcept;
void upsert(const T& val,              std::move_only_function<void(bool)> cb) noexcept;
void remove(std::string_view pk,       std::move_only_function<void(bool)> cb) noexcept;
void remove_many(std::span<const std::string_view>,
                 std::move_only_function<void(bool)> cb)         noexcept;
```

### Internal storage

```cpp
private:
    interfaces::ICache*    cache_;           // external cache (nullable)
    interfaces::IDatabase* db_;              // external DB (nullable)
    LocalCache             local_cache_;     // fallback when cache_ == nullptr
    std::unordered_map<std::string, T> local_store_;    // fallback when db_ == nullptr
    std::shared_mutex      local_store_mutex_;
```

### Operation flows

**`find(pk, cb)`**
1. `active_cache().get(cache_key(pk))` → hit: `rfl::json::read<T>(val)` → cb
2. Miss → `active_db().query(select_sql(pk))` → decode JSON result via `rfl::json` → set cache → cb
3. DB miss or DB null + local_store_ miss → `cb(std::nullopt)`

**`insert/update/upsert(val, cb)`**
- Fire `active_cache().set(cache_key, json)` + `active_db().insert/update/upsert(sql)` concurrently
- Both callbacks must complete before invoking user cb(bool)
- If DB null: write `local_store_[pk] = val` under unique_lock

**`remove(pk, cb)`**
- Fire `active_cache().remove(cache_key)` + `active_db().remove(delete_sql)` concurrently
- If DB null: erase from `local_store_`

**`create(cb)`**
- DB only — generates `CREATE TABLE IF NOT EXISTS {table} (...)` from `Serializable<T>::fields()`
- Column types inferred from `ValueType` via a `cpp_to_sql_type<VT>()` helper
- `FieldOptionsDb` used for `PRIMARY KEY`, `UNIQUE`, `NOT NULL`, `REFERENCES` constraints
- No cache involvement

**`find_many(pks, cb)`**
- Per-PK cache lookup in parallel; collect misses → single `SELECT ... WHERE pk IN (...)` → decode → populate cache → merge results

**`find_all(cb)`**
- Cache bypass — always hits DB (no key to check against)
- `SELECT row_to_json(t) FROM {table} t`

### SQL generation

All SQL built at runtime from `Serializable<T>::fields()` + `Serializable<T>::table_name()`.

SELECT returns JSON via PostgreSQL `row_to_json()` — result decoded with `rfl::json::read<T>()` directly, no extra codec needed.

```
cache_key(pk)  →  "{table_name}:{pk}"
cache_value    →  rfl::json::write(obj)
```

### Active backend helpers

```cpp
ICache*    active_cache() { return cache_ ? cache_ : &local_cache_; }
IDatabase* active_db()    { return db_; }  // null → use local_store_ branch
```

---

## What is NOT in scope

- Query by non-PK fields (no `find_where` predicate)
- Transactions spanning multiple model types
- Cache TTL / expiry
- Schema migrations (`create` only, no `alter`/`drop`)
