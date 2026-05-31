# Serializable\<T\> Framework Design

**Date**: 2026-05-31  
**Scope**: Unified multi-format serialization for all model types in `congelado`

---

## Problem

13 model types × 3 serialization backends (simdjson, toml++, reflect-cpp) = ~500 lines of hand-written boilerplate spread across 9 files. Each new field or type requires 3 coordinated edits. The rfl reflectors and simdjson tag_invokes encode the same structural information redundantly.

---

## Solution

A `std::formatter`-like framework: user specializes `Serializable<T>` once with field descriptors, and the framework auto-generates all three backends via constrained generic templates.

---

## Architecture

### Files

```
include/ser/ser.cppm          ← new: framework (field descriptors, concept, backends)
include/model/model.cppm      ← remove: export import :rfl_reflectors
include/model/rfl_reflectors.cppm  ← DELETED (replaced by ser.cppm)
include/model/common/policies.cppm    ← remove hand-written backends, add Serializable<T>
include/model/common/timestamps.cppm  ← same
include/model/workflow/dag.cppm       ← same
include/model/task/definition.cppm    ← same
include/model/task/instance.cppm      ← same
include/model/workflow/definition.cppm ← same
include/model/workflow/event.cppm     ← same
include/model/workflow/exec.cppm      ← same
xmake.lua                     ← add include/ser/ser.cppm to congelado_lib
```

### Data Flow

```
User writes: Serializable<RetryPolicy>::fields()
                         │
          ┌──────────────┼──────────────┐
          ▼              ▼              ▼
  rfl::Reflector<T>  tag_invoke<T>  from_toml<T>
  (rfl→16 formats)  (simdjson)     (toml++)
```

---

## Section 1: Field Descriptor System

Uses `rfl::internal::StringLiteral<N>` as the NTTP string type — rfl already requires it for `rfl::make_field`, so no extra dependency.

```cpp
// Member pointer trait — extracts class and value types from getter
template<typename MFP> struct MFPTraits;
template<typename C, typename R>
struct MFPTraits<R(C::*)() const> {
    using class_t = C;
    using value_t = std::remove_cvref_t<R>;
};

// Single field descriptor
template<rfl::internal::StringLiteral Name, auto Getter, auto Setter>
struct FieldDesc {
    static constexpr auto name   = Name;
    static constexpr auto getter = Getter;
    static constexpr auto setter = Setter;
    using ClassType = typename MFPTraits<decltype(Getter)>::class_t;
    using ValueType = typename MFPTraits<decltype(Getter)>::value_t;
};

// Factory — user calls this inside fields()
template<rfl::internal::StringLiteral Name, auto Getter, auto Setter>
constexpr auto field() { return FieldDesc<Name, Getter, Setter>{}; }
```

### ISerializable Concept

```cpp
template<typename T>
concept ISerializable = requires {
    { Serializable<T>::fields() } -> /* returns std::tuple of FieldDesc instantiations */;
};
```

### User Specialization (example)

```cpp
template<> struct Serializable<model::RetryPolicy> {
    static constexpr auto fields() {
        return std::tuple{
            field<"max_attempts", &RetryPolicy::get_max_attempts, &RetryPolicy::set_max_attempts>(),
            field<"backoff",      &RetryPolicy::get_backoff,      &RetryPolicy::set_backoff>(),
            field<"interval_ms",  &RetryPolicy::get_interval_ms,  &RetryPolicy::set_interval_ms>(),
        };
    }
};
```

---

## Section 2: FieldConverter\<VT\> Extension Point

Handles per-value-type adaptation for each backend. Same extension-point shape as `std::formatter`.

```cpp
// Primary template — works for primitives (int, string, bool, float, enums)
template<typename VT>
struct FieldConverter {
    // simdjson: uses built-in ondemand get<VT>
    // toml:     uses node.value<VT>()
    // rfl:      identity (VT used directly in NamedTuple)
};
```

Required specializations (provided by `ser.cppm`):

| ValueType | simdjson wire | toml wire | rfl wire |
|-----------|--------------|-----------|----------|
| `uuids::uuid` | string → `uuid::from_string` | string → `uuid::from_string` | `std::string` |
| `std::chrono::system_clock::time_point` | int64 ms | int64 ms | `std::int64_t` |
| `std::optional<uuids::uuid>` | nullable string | optional string | `std::optional<std::string>` |
| `std::optional<T>` (other) | nullable | optional node | `std::optional<T>` |

Enums: `FieldConverter<E>` (constrained on `std::is_enum_v<E>`) uses rfl's public `rfl::enum_to_string<E>(v)` and `rfl::string_to_enum<E>(s)` utilities for all three backends. No `magic_enum` direct dependency — rfl already vendors it. If `rfl::string_to_enum` returns `std::optional<E>`, `FieldConverter` propagates errors as `std::unexpected`.

Users can add their own `FieldConverter<MyType>` specializations for custom value types.

---

## Section 3: Generic Backends

### rfl::Reflector (in namespace rfl)

```cpp
template<ser::ISerializable T>
struct Reflector<T> {
    // ReflType deduced: rfl::NamedTuple<rfl::Field<"a", VA>, rfl::Field<"b", VB>, ...>
    // where VA = FieldConverter<ValueType>::rfl_type
    using ReflType = decltype(ser::build_named_tuple(
        std::declval<const T&>(), Serializable<T>::fields()));

    static T to(const ReflType& nt) noexcept {
        T obj;
        // expands to: (obj.*Fd::setter)(rfl::get<Fd::name>(nt))  for each field
        ser::apply_named_tuple_to(obj, nt, Serializable<T>::fields());
        return obj;
    }

    static ReflType from(const T& obj) {
        return ser::build_named_tuple(obj, Serializable<T>::fields());
    }
};
```

`build_named_tuple` chains `rfl::make_field<Name>(FieldConverter<VT>::to_rfl(getter(obj)))` calls using fold expressions over the field tuple.

### simdjson::tag_invoke (in namespace simdjson)

Constrained generic — lower overload priority than any specific `tag_invoke`, so existing specific overloads would win if any were left (there won't be after migration).

```cpp
template<typename V, ser::ISerializable T>
auto tag_invoke(deserialize_tag, V& val, T& obj) {
    ondemand::object json_obj;
    if (auto ec = val.get_object().get(json_obj); ec) return ec;

    error_code result = SUCCESS;
    std::apply([&](auto... fds) {
        ((result == SUCCESS && (result = ser::extract_simdjson_field(json_obj, obj, fds))) , ...);
    }, Serializable<T>::fields());
    return result;
}
```

`extract_simdjson_field` calls `FieldConverter<VT>::from_simdjson(json_obj[Name])` then `obj.*setter`.

### model::from_toml (in namespace model)

```cpp
template<ser::ISerializable T>
std::expected<void, std::string> from_toml(const toml::table& t, T& obj) {
    std::expected<void, std::string> result{};
    std::apply([&](auto... fds) {
        ((result && (result = ser::extract_toml_field(t, obj, fds))) , ...);
    }, Serializable<T>::fields());
    return result;
}
```

`extract_toml_field` calls `FieldConverter<VT>::from_toml(t[Name])` then `obj.*setter`. Returns `std::unexpected` on missing required fields or parse errors.

---

## Section 4: Migration Plan (13 types)

### Types to migrate

| File | Types |
|------|-------|
| `model/common/policies.cppm` | `RetryPolicy`, `TimeoutPolicy`, `RateLimitPolicy` |
| `model/common/timestamps.cppm` | `ExecutionTimings` |
| `model/workflow/dag.cppm` | `InputMapping`, `OutputMapping`, `TaskEdge`, `TaskNode` |
| `model/task/definition.cppm` | `TaskDef` |
| `model/task/instance.cppm` | `TaskInstance` |
| `model/workflow/definition.cppm` | `WorkflowDef` |
| `model/workflow/event.cppm` | `WorkflowEvent` |
| `model/workflow/exec.cppm` | `WorkflowExecution` |

### Per-file steps

1. Remove `module;` global fragment includes for `simdjson.h` / `toml++/toml.hpp` / `uuid.h` (if no longer needed by class body)
2. Remove `namespace simdjson { tag_invoke(...) }` block
3. Remove `namespace model { from_toml(...) }` free function(s)
4. Add `import ser;`
5. Add `Serializable<T>` specialization(s) after class definition

### Special handling

- `TaskEdge` contains `vector<InputMapping>` — `FieldConverter<vector<T>>` handles nested `ISerializable` via recursive `from_toml` / simdjson array iteration.
- `WorkflowExecution` contains `vector<TaskInstance>` — same.
- `optional<CorrelationId>` — handled by `FieldConverter<optional<uuids::uuid>>`.
- Enum fields (`WorkflowStatus`, `TaskStatus`, `TaskType`, etc.) — `FieldConverter<EnumT>` uses string↔enum mapping.

---

## Module Dependencies

```
ser.cppm
  ← #include <rfl.hpp>          (rfl::make_field, rfl::NamedTuple, rfl::Reflector)
  ← #include <simdjson.h>       (tag_invoke, ondemand::object)
  ← #include <toml++/toml.hpp>  (toml::table, toml::node)
  ← #include <uuid.h>           (uuids::uuid — for FieldConverter specialization)
  import std;

model partitions
  ← import ser;
  ← (no longer need simdjson/toml/uuid includes in global fragment for serialization)
```

---

## Constraints

- No P2996 (`-freflection` not available in clang 22)
- `rfl::internal::StringLiteral<N>` is a structural type — valid as NTTP
- Getter must be `const` member function with no arguments (enforced by `MFPTraits` match); return type may be `const R&` — `MFPTraits` applies `remove_cvref_t` so `value_t = R`
- Setter must be `void (T::*)(VT)` or `void (T::*)(VT&&)` — setter trait handles both via partial specialization
- All model classes unchanged (private members, getters/setters preserved)
- `FieldConverter<std::vector<U>>` where `ISerializable<U>`: simdjson recurses via existing `tag_invoke<U>` ADL; toml iterates `as_array()` and calls `from_toml<U>` recursively; rfl uses `std::vector<rfl_type_of<U>>`

## Pre-existing Bug to Fix

`add_requires("conan::reflect-cpp/0.23.0", ...)` in `xmake.lua` is currently inside the `target("congelado_lib")` block (no `target_end()` before it). xmake rejects `requires()` inside target scope. Implementation must move this `add_requires` call before the `target("congelado_lib")` line.

---

## Success Criteria

- `rfl::to_json(model_instance)` produces correct JSON for all 13 types
- `simdjson::ondemand::parser` deserializes all 13 types via generic `tag_invoke`
- `from_toml(table, obj)` works for all 13 types
- `rfl_reflectors.cppm` deleted; hand-written `tag_invoke` and `from_toml` functions deleted
- Build passes with existing xmake configuration
