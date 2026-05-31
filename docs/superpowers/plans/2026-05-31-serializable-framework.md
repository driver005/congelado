# Serializable\<T\> Framework Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Replace ~500 lines of per-type simdjson/toml/rfl serialization boilerplate across 13 model types with a single `Serializable<T>` specialization per type that auto-generates all backends.

**Architecture:** `include/ser/ser.cppm` (standalone module `ser`) exports NTTP field descriptors, `FieldConverter<VT>` extension points, and constrained generic backends (`rfl::Reflector<ISerializable T>`, `simdjson::tag_invoke<ISerializable T>`, `model::from_toml<ISerializable T>`). Each model partition deletes hand-written serializers and adds `Serializable<T>::fields()`. `model.cppm` re-exports `ser` so all consumers get the framework automatically.

**Tech Stack:** C++26 modules, clang 22, xmake, simdjson 4.2.4, toml++ 3.4.0, reflect-cpp 0.23.0, stduuid 1.2.3, `rfl::internal::StringLiteral` as NTTP string type, magic_enum (bundled inside rfl).

---

## File Map

| Action | File | Responsibility |
|--------|------|---------------|
| Create | `include/ser/ser.cppm` | Entire framework: field descriptors, FieldConverter, generic backends |
| Modify | `xmake.lua` | Move `add_requires(reflect-cpp)` to root scope; add ser.cppm |
| Modify | `include/model/model.cppm` | Replace `:rfl_reflectors` with `export import ser;` |
| Delete | `include/model/rfl_reflectors.cppm` | Replaced by generic rfl::Reflector in ser.cppm |
| Modify | `include/model/common/timestamps.cppm` | Add optional<tp> setter overloads; add Serializable<ExecutionTimings> |
| Modify | `include/model/common/policies.cppm` | Remove simdjson/toml; add Serializable for 3 types |
| Modify | `include/model/workflow/dag.cppm` | Remove simdjson/toml; add Serializable for 4 types |
| Modify | `include/model/task/definition.cppm` | Remove simdjson/toml; add Serializable<TaskDef> |
| Modify | `include/model/task/instance.cppm` | Remove simdjson/toml; add Serializable<TaskInstance> |
| Modify | `include/model/workflow/definition.cppm` | Remove simdjson/toml; add Serializable<WorkflowDef> |
| Modify | `include/model/workflow/event.cppm` | Remove simdjson/toml; add Serializable<WorkflowEvent> |
| Modify | `include/model/workflow/exec.cppm` | Remove simdjson/toml; add Serializable<WorkflowExecution> |

---

## Critical Notes Before Starting

**magic_enum include path**: rfl bundles magic_enum. Find the header:
```bash
find ~/.conan2 -name "magic_enum.hpp" -path "*rfl*" 2>/dev/null | head -3
# Also try: find /usr /opt -name "magic_enum.hpp" 2>/dev/null | head -3
```
Use the found path in the global fragment. If unavailable, add `conan::magic_enum/0.9.7` to xmake.lua.

**rfl::make_field + fold**: `(rfl::make_field<Fd::name>(v) + ...)` right-fold over a pack creates `NamedTuple<Field<N1,V1>, Field<N2,V2>, ...>`. This requires ≥1 field. All model types have ≥1 field.

**uint32_t simdjson**: simdjson ondemand does not support `get(uint32_t&)` directly. The `FieldConverter<uint32_t>` specialization reads `uint64_t` and casts.

**ExecutionTimings setter mismatch**: `get_scheduled_at()` returns `optional<time_point>` but `set_scheduled_at` takes `time_point`. Task 1 adds overloaded setters accepting `optional<time_point>`.

---

## Task 1: Fix xmake.lua — scope bug + add ser.cppm

**Files:**
- Modify: `xmake.lua`

The `add_requires("conan::reflect-cpp/0.23.0", ...)` block currently sits inside `target("congelado_lib")` (no `target_end()` before it at line 183). xmake forbids `requires()` inside targets.

- [ ] **Step 1: Move add_requires to root scope**

In `xmake.lua`, cut the entire `add_requires("conan::reflect-cpp/0.23.0", ...)` block (lines 183–196) and paste it BEFORE the `target("congelado_lib")` line (line 122). The final order should be:

```lua
-- all other add_requires at root scope...
add_requires("conan::catch2/3.7.1", { alias = "catch2", configs = conan })
-- ... other add_requires ...

add_requires("conan::reflect-cpp/0.23.0", {
    alias = "reflectcpp",
    configs = {
        settings_build = conan.settings_build,
        settings = conan.settings,
        conf = conan.conf,
        build = "missing",
        options = {
            "reflect-cpp/*:toml=True",
            "reflect-cpp/*:msgpack=True",
            "reflect-cpp/*:xml=True",
        }
    }
})

target("congelado_lib")
```

- [ ] **Step 2: Add ser.cppm to congelado_lib sources**

Inside `target("congelado_lib")`, find the `add_files("include/**.cppm", ...)` line and add ser.cppm alongside it:

```lua
add_files("include/**.cppm", { public = true })
add_files("include/ser/ser.cppm", { public = true })
```

(If `include/ser/` is already under `include/`, the glob will pick it up automatically — skip the explicit line if so. Verify by checking the glob matches `include/ser/ser.cppm`.)

- [ ] **Step 3: Verify xmake.lua is valid**

```bash
cd /home/default/cc/congelado
xmake check 2>&1 | head -20
```

Expected: no "requires() cannot be called in target()" error. Warnings about missing packages are OK.

- [ ] **Step 4: Commit**

```bash
git add xmake.lua
git commit -m "fix(build): move add_requires(reflect-cpp) to root scope; reflectcpp was inside target() causing xmake error"
```

---

## Task 2: Write include/ser/ser.cppm

**Files:**
- Create: `include/ser/ser.cppm`

This is the entire framework in one file. Write it completely before building — partial states won't compile.

- [ ] **Step 1: Create directory**

```bash
mkdir -p /home/default/cc/congelado/include/ser
```

- [ ] **Step 2: Write the full file**

Create `/home/default/cc/congelado/include/ser/ser.cppm` with this exact content:

```cpp
module;
#define UUID_SYSTEM_GENERATOR
#include <rfl.hpp>
#include <simdjson.h>
#include <toml++/toml.hpp>
#include <uuid.h>
// rfl bundles magic_enum — verify path with:
// find ~/.conan2 -name "magic_enum.hpp" -path "*rfl*" 2>/dev/null | head -1
// Adjust the include path below if needed:
#include <rfl/internal/magic_enum/magic_enum.hpp>

export module ser;

import std;

// ─── Member pointer traits ────────────────────────────────────────────────────

export namespace ser {

template<typename MFP>
struct MFPTraits;

template<typename C, typename R>
struct MFPTraits<R(C::*)() const> {
    using class_t = C;
    using value_t = std::remove_cvref_t<R>;
};

// ─── FieldDesc ────────────────────────────────────────────────────────────────

template<rfl::internal::StringLiteral Name, auto Getter, auto Setter>
struct FieldDesc {
    static constexpr auto name   = Name;
    static constexpr auto getter = Getter;
    static constexpr auto setter = Setter;
    using ClassType = typename MFPTraits<decltype(Getter)>::class_t;
    using ValueType = typename MFPTraits<decltype(Getter)>::value_t;
};

template<rfl::internal::StringLiteral Name, auto Getter, auto Setter>
constexpr auto field() { return FieldDesc<Name, Getter, Setter>{}; }

// ─── Serializable<T> + ISerializable concept ──────────────────────────────────

template<typename T>
struct Serializable;

template<typename T>
concept ISerializable = requires { { Serializable<T>::fields() }; };

} // namespace ser

// ─── Forward declaration of from_toml_impl ───────────────────────────────────

namespace ser {
    template<ISerializable T>
    std::expected<void, std::string> from_toml_impl(const toml::table&, T&);
}

// ─── FieldConverter<VT> primary + specializations ────────────────────────────

export namespace ser {

// Primary: handles simdjson-native primitives (std::string, bool, int64_t, uint64_t, double)
template<typename VT>
struct FieldConverter {
    using rfl_type = VT;

    static simdjson::error_code from_simdjson(simdjson::ondemand::value& v, VT& out) {
        return v.get(out);
    }

    static std::expected<VT, std::string> from_toml(const toml::table& t, std::string_view fname) {
        auto val = t[fname].value<VT>();
        if (!val) return std::unexpected{std::format("missing or invalid field '{}'", fname)};
        return *val;
    }

    static rfl_type to_rfl(const VT& v) { return v; }
    static VT from_rfl(const rfl_type& v) { return v; }
};

// ─── uint32_t ─────────────────────────────────────────────────────────────────

template<>
struct FieldConverter<std::uint32_t> {
    using rfl_type = std::uint32_t;

    static simdjson::error_code from_simdjson(simdjson::ondemand::value& v, std::uint32_t& out) {
        std::uint64_t tmp{};
        if (auto ec = v.get_uint64().get(tmp); ec) return ec;
        out = static_cast<std::uint32_t>(tmp);
        return simdjson::SUCCESS;
    }

    static std::expected<std::uint32_t, std::string> from_toml(
        const toml::table& t, std::string_view fname)
    {
        auto val = t[fname].value<std::int64_t>();
        if (!val) return std::unexpected{std::format("missing field '{}'", fname)};
        return static_cast<std::uint32_t>(*val);
    }

    static std::uint32_t to_rfl(std::uint32_t v) { return v; }
    static std::uint32_t from_rfl(std::uint32_t v) { return v; }
};

// ─── Enum types ───────────────────────────────────────────────────────────────

template<typename E> requires std::is_enum_v<E>
struct FieldConverter<E> {
    using rfl_type = E; // rfl handles enums natively via bundled magic_enum

    static simdjson::error_code from_simdjson(simdjson::ondemand::value& v, E& out) {
        std::string_view sv;
        if (auto ec = v.get_string().get(sv); ec) return ec;
        auto result = magic_enum::enum_cast<E>(sv);
        if (!result) return simdjson::INCORRECT_TYPE;
        out = *result;
        return simdjson::SUCCESS;
    }

    static std::expected<E, std::string> from_toml(const toml::table& t, std::string_view fname) {
        auto sv = t[fname].value<std::string>();
        if (!sv) return std::unexpected{std::format("missing field '{}'", fname)};
        auto result = magic_enum::enum_cast<E>(*sv);
        if (!result)
            return std::unexpected{std::format("invalid enum '{}' for field '{}'", *sv, fname)};
        return *result;
    }

    static E to_rfl(const E& v) { return v; }
    static E from_rfl(const E& v) { return v; }
};

// ─── uuids::uuid ──────────────────────────────────────────────────────────────

template<>
struct FieldConverter<uuids::uuid> {
    using rfl_type = std::string;

    static simdjson::error_code from_simdjson(simdjson::ondemand::value& v, uuids::uuid& out) {
        std::string_view sv;
        if (auto ec = v.get_string().get(sv); ec) return ec;
        auto id = uuids::uuid::from_string(sv);
        if (!id) return simdjson::INCORRECT_TYPE;
        out = *id;
        return simdjson::SUCCESS;
    }

    static std::expected<uuids::uuid, std::string> from_toml(
        const toml::table& t, std::string_view fname)
    {
        auto sv = t[fname].value<std::string>();
        if (!sv) return std::unexpected{std::format("missing field '{}'", fname)};
        auto id = uuids::uuid::from_string(*sv);
        if (!id) return std::unexpected{std::format("invalid UUID for field '{}'", fname)};
        return *id;
    }

    static std::string to_rfl(const uuids::uuid& v) { return uuids::to_string(v); }
    static uuids::uuid from_rfl(const std::string& s) {
        return uuids::uuid::from_string(s).value_or(uuids::uuid{});
    }
};

// ─── std::optional<uuids::uuid> ───────────────────────────────────────────────

template<>
struct FieldConverter<std::optional<uuids::uuid>> {
    using rfl_type = std::optional<std::string>;

    static simdjson::error_code from_simdjson(
        simdjson::ondemand::value& v, std::optional<uuids::uuid>& out)
    {
        if (v.is_null()) { out = std::nullopt; return simdjson::SUCCESS; }
        std::string_view sv;
        if (auto ec = v.get_string().get(sv); ec) return ec;
        auto id = uuids::uuid::from_string(sv);
        if (!id) return simdjson::INCORRECT_TYPE;
        out = *id;
        return simdjson::SUCCESS;
    }

    static std::expected<std::optional<uuids::uuid>, std::string> from_toml(
        const toml::table& t, std::string_view fname)
    {
        auto sv = t[fname].value<std::string>();
        if (!sv) return std::nullopt;
        auto id = uuids::uuid::from_string(*sv);
        if (!id) return std::unexpected{std::format("invalid UUID for field '{}'", fname)};
        return *id;
    }

    static rfl_type to_rfl(const std::optional<uuids::uuid>& v) {
        if (!v) return std::nullopt;
        return uuids::to_string(*v);
    }
    static std::optional<uuids::uuid> from_rfl(const rfl_type& s) {
        if (!s) return std::nullopt;
        return uuids::uuid::from_string(*s).value_or(uuids::uuid{});
    }
};

// ─── std::chrono::system_clock::time_point ───────────────────────────────────

using TP = std::chrono::system_clock::time_point;

template<>
struct FieldConverter<TP> {
    using rfl_type = std::int64_t;

    static simdjson::error_code from_simdjson(simdjson::ondemand::value& v, TP& out) {
        std::int64_t ms{};
        if (auto ec = v.get_int64().get(ms); ec) return ec;
        out = TP{std::chrono::milliseconds{ms}};
        return simdjson::SUCCESS;
    }

    static std::expected<TP, std::string> from_toml(const toml::table& t, std::string_view fname) {
        auto ms = t[fname].value<std::int64_t>();
        if (!ms) return std::unexpected{std::format("missing field '{}'", fname)};
        return TP{std::chrono::milliseconds{*ms}};
    }

    static std::int64_t to_rfl(const TP& v) {
        return std::chrono::duration_cast<std::chrono::milliseconds>(
            v.time_since_epoch()).count();
    }
    static TP from_rfl(std::int64_t ms) { return TP{std::chrono::milliseconds{ms}}; }
};

// ─── std::optional<time_point> ────────────────────────────────────────────────

template<>
struct FieldConverter<std::optional<TP>> {
    using rfl_type = std::optional<std::int64_t>;

    static simdjson::error_code from_simdjson(
        simdjson::ondemand::value& v, std::optional<TP>& out)
    {
        if (v.is_null()) { out = std::nullopt; return simdjson::SUCCESS; }
        std::int64_t ms{};
        if (auto ec = v.get_int64().get(ms); ec) return ec;
        out = TP{std::chrono::milliseconds{ms}};
        return simdjson::SUCCESS;
    }

    static std::expected<std::optional<TP>, std::string> from_toml(
        const toml::table& t, std::string_view fname)
    {
        auto ms = t[fname].value<std::int64_t>();
        if (!ms) return std::nullopt;
        return TP{std::chrono::milliseconds{*ms}};
    }

    static rfl_type to_rfl(const std::optional<TP>& v) {
        if (!v) return std::nullopt;
        return std::chrono::duration_cast<std::chrono::milliseconds>(
            v->time_since_epoch()).count();
    }
    static std::optional<TP> from_rfl(const rfl_type& ms) {
        if (!ms) return std::nullopt;
        return TP{std::chrono::milliseconds{*ms}};
    }
};

// ─── std::vector<std::string> ────────────────────────────────────────────────

template<>
struct FieldConverter<std::vector<std::string>> {
    using rfl_type = std::vector<std::string>;

    static simdjson::error_code from_simdjson(
        simdjson::ondemand::value& v, std::vector<std::string>& out)
    {
        simdjson::ondemand::array arr;
        if (auto ec = v.get_array().get(arr); ec) return ec;
        for (auto elem : arr) {
            std::string_view sv;
            if (auto ec = elem.get_string().get(sv); ec) return ec;
            out.emplace_back(sv);
        }
        return simdjson::SUCCESS;
    }

    static std::expected<std::vector<std::string>, std::string> from_toml(
        const toml::table& t, std::string_view fname)
    {
        const auto* arr = t[fname].as_array();
        if (!arr) return std::vector<std::string>{};
        std::vector<std::string> result;
        result.reserve(arr->size());
        for (const auto& elem : *arr) {
            auto sv = elem.value<std::string>();
            if (!sv)
                return std::unexpected{std::format("element of '{}' must be a string", fname)};
            result.push_back(std::move(*sv));
        }
        return result;
    }

    static rfl_type to_rfl(const std::vector<std::string>& v) { return v; }
    static std::vector<std::string> from_rfl(const rfl_type& v) { return v; }
};

// ─── std::optional<std::string> ──────────────────────────────────────────────

template<>
struct FieldConverter<std::optional<std::string>> {
    using rfl_type = std::optional<std::string>;

    static simdjson::error_code from_simdjson(simdjson::ondemand::value& v, std::optional<std::string>& out) {
        if (v.is_null()) { out = std::nullopt; return simdjson::SUCCESS; }
        std::string_view sv;
        if (auto ec = v.get_string().get(sv); ec) return ec;
        out = std::string{sv};
        return simdjson::SUCCESS;
    }

    static std::expected<std::optional<std::string>, std::string> from_toml(
        const toml::table& t, std::string_view fname)
    {
        auto sv = t[fname].value<std::string>();
        if (!sv) return std::nullopt;
        return *sv;
    }

    static rfl_type to_rfl(const std::optional<std::string>& v) { return v; }
    static std::optional<std::string> from_rfl(const rfl_type& v) { return v; }
};

// ─── std::unordered_map<std::string, std::string> ────────────────────────────

template<>
struct FieldConverter<std::unordered_map<std::string, std::string>> {
    using rfl_type = std::map<std::string, std::string>; // rfl serializes map as JSON object

    static simdjson::error_code from_simdjson(
        simdjson::ondemand::value& v,
        std::unordered_map<std::string, std::string>& out)
    {
        simdjson::ondemand::object obj;
        if (auto ec = v.get_object().get(obj); ec) return ec;
        for (auto f : obj) {
            std::string_view k, val;
            if (auto ec = f.unescaped_key().get(k); ec) return ec;
            if (auto ec = f.value().get_string().get(val); ec) return ec;
            out.emplace(k, val);
        }
        return simdjson::SUCCESS;
    }

    static std::expected<std::unordered_map<std::string, std::string>, std::string> from_toml(
        const toml::table& t, std::string_view fname)
    {
        const auto* sub = t[fname].as_table();
        if (!sub) return std::unordered_map<std::string, std::string>{};
        std::unordered_map<std::string, std::string> result;
        for (auto&& [k, v] : *sub) {
            auto sv = v.value<std::string>();
            if (!sv)
                return std::unexpected{std::format("value in '{}' must be a string", fname)};
            result.emplace(std::string{k.str()}, std::move(*sv));
        }
        return result;
    }

    static rfl_type to_rfl(const std::unordered_map<std::string, std::string>& v) {
        return rfl_type{v.begin(), v.end()};
    }
    static std::unordered_map<std::string, std::string> from_rfl(const rfl_type& m) {
        return std::unordered_map<std::string, std::string>{m.begin(), m.end()};
    }
};

} // namespace ser

// ─── NamedTuple builder (needs FieldConverter defined first) ──────────────────

export namespace ser {

template<typename T, typename... Fds>
auto build_named_tuple(const T& obj, std::tuple<Fds...>) {
    return std::apply([&](auto... fds) {
        return (rfl::make_field<decltype(fds)::name>(
            FieldConverter<typename decltype(fds)::ValueType>::to_rfl(
                (obj.*decltype(fds)::getter)())) + ...);
    }, std::tuple<Fds...>{});
}

template<typename T, typename NT, typename... Fds>
void apply_named_tuple_to(T& obj, const NT& nt, std::tuple<Fds...>) {
    std::apply([&](auto... fds) {
        ((obj.*decltype(fds)::setter)(
            FieldConverter<typename decltype(fds)::ValueType>::from_rfl(
                rfl::get<decltype(fds)::name>(nt))), ...);
    }, std::tuple<Fds...>{});
}

} // namespace ser

// ─── ISerializable FieldConverter specializations ────────────────────────────

export namespace ser {

template<typename VT> requires ISerializable<VT>
struct FieldConverter<VT> {
    using rfl_type = decltype(build_named_tuple(
        std::declval<const VT&>(), Serializable<VT>::fields()));

    static simdjson::error_code from_simdjson(simdjson::ondemand::value& v, VT& out) {
        return simdjson::tag_invoke(simdjson::deserialize_tag{}, v, out);
    }

    static std::expected<VT, std::string> from_toml(
        const toml::table& t, std::string_view fname)
    {
        const auto* sub = t[fname].as_table();
        if (!sub)
            return std::unexpected{std::format("field '{}' must be a TOML table", fname)};
        VT obj;
        if (auto r = from_toml_impl(*sub, obj); !r) return std::unexpected{r.error()};
        return obj;
    }

    static rfl_type to_rfl(const VT& v) {
        return build_named_tuple(v, Serializable<VT>::fields());
    }
    static VT from_rfl(const rfl_type& nt) {
        VT obj;
        apply_named_tuple_to(obj, nt, Serializable<VT>::fields());
        return obj;
    }
};

template<typename VT> requires ISerializable<VT>
struct FieldConverter<std::optional<VT>> {
    using InnerRfl = typename FieldConverter<VT>::rfl_type;
    using rfl_type = std::optional<InnerRfl>;

    static simdjson::error_code from_simdjson(
        simdjson::ondemand::value& v, std::optional<VT>& out)
    {
        if (v.is_null()) { out = std::nullopt; return simdjson::SUCCESS; }
        VT obj;
        if (auto ec = FieldConverter<VT>::from_simdjson(v, obj); ec) return ec;
        out = std::move(obj);
        return simdjson::SUCCESS;
    }

    static std::expected<std::optional<VT>, std::string> from_toml(
        const toml::table& t, std::string_view fname)
    {
        const auto* sub = t[fname].as_table();
        if (!sub) return std::nullopt;
        VT obj;
        if (auto r = from_toml_impl(*sub, obj); !r) return std::unexpected{r.error()};
        return obj;
    }

    static rfl_type to_rfl(const std::optional<VT>& v) {
        if (!v) return std::nullopt;
        return FieldConverter<VT>::to_rfl(*v);
    }
    static std::optional<VT> from_rfl(const rfl_type& nt) {
        if (!nt) return std::nullopt;
        return FieldConverter<VT>::from_rfl(*nt);
    }
};

template<typename VT> requires ISerializable<VT>
struct FieldConverter<std::vector<VT>> {
    using InnerRfl = typename FieldConverter<VT>::rfl_type;
    using rfl_type = std::vector<InnerRfl>;

    static simdjson::error_code from_simdjson(
        simdjson::ondemand::value& v, std::vector<VT>& out)
    {
        simdjson::ondemand::array arr;
        if (auto ec = v.get_array().get(arr); ec) return ec;
        for (auto elem : arr) {
            simdjson::ondemand::value elem_val;
            if (auto ec = elem.get(elem_val); ec) return ec;
            VT obj;
            if (auto ec = FieldConverter<VT>::from_simdjson(elem_val, obj); ec) return ec;
            out.push_back(std::move(obj));
        }
        return simdjson::SUCCESS;
    }

    static std::expected<std::vector<VT>, std::string> from_toml(
        const toml::table& t, std::string_view fname)
    {
        const auto* arr = t[fname].as_array();
        if (!arr) return std::vector<VT>{};
        std::vector<VT> result;
        result.reserve(arr->size());
        for (const auto& elem : *arr) {
            const auto* sub = elem.as_table();
            if (!sub)
                return std::unexpected{
                    std::format("element of '{}' must be a TOML table", fname)};
            VT obj;
            if (auto r = from_toml_impl(*sub, obj); !r) return std::unexpected{r.error()};
            result.push_back(std::move(obj));
        }
        return result;
    }

    static rfl_type to_rfl(const std::vector<VT>& v) {
        rfl_type result;
        result.reserve(v.size());
        for (const auto& e : v) result.push_back(FieldConverter<VT>::to_rfl(e));
        return result;
    }
    static std::vector<VT> from_rfl(const rfl_type& nt) {
        std::vector<VT> result;
        result.reserve(nt.size());
        for (const auto& e : nt) result.push_back(FieldConverter<VT>::from_rfl(e));
        return result;
    }
};

} // namespace ser

// ─── Per-field helpers ────────────────────────────────────────────────────────

namespace ser {

template<typename Fd>
simdjson::error_code extract_simdjson_field(
    simdjson::ondemand::object& obj, typename Fd::ClassType& out, Fd)
{
    using VT = typename Fd::ValueType;
    simdjson::ondemand::value field_val;
    if (auto ec = obj.find_field_unordered(Fd::name.string_view()).get(field_val); ec)
        return ec;
    VT value{};
    if (auto ec = FieldConverter<VT>::from_simdjson(field_val, value); ec) return ec;
    (out.*Fd::setter)(std::move(value));
    return simdjson::SUCCESS;
}

template<typename Fd>
std::expected<void, std::string> extract_toml_field(
    const toml::table& t, typename Fd::ClassType& out, Fd)
{
    using VT = typename Fd::ValueType;
    auto result = FieldConverter<VT>::from_toml(t, Fd::name.string_view());
    if (!result) return std::unexpected{result.error()};
    (out.*Fd::setter)(std::move(*result));
    return {};
}

// from_toml_impl definition (declared above, used by FieldConverter<ISerializable>)
export template<ISerializable T>
std::expected<void, std::string> from_toml_impl(const toml::table& t, T& obj) {
    std::expected<void, std::string> result{};
    std::apply([&](auto... fds) {
        ((result && (result = extract_toml_field(t, obj, fds))), ...);
    }, Serializable<T>::fields());
    return result;
}

// Helper for rfl backend
export template<ISerializable T>
auto rfl_build_from(const T& obj) {
    return build_named_tuple(obj, Serializable<T>::fields());
}

} // namespace ser

// ─── Generic rfl::Reflector ───────────────────────────────────────────────────

namespace rfl {

template<ser::ISerializable T>
struct Reflector<T> {
    using ReflType = decltype(ser::rfl_build_from(std::declval<const T&>()));

    static T to(const ReflType& nt) noexcept {
        T obj;
        ser::apply_named_tuple_to(obj, nt, ser::Serializable<T>::fields());
        return obj;
    }

    static ReflType from(const T& obj) {
        return ser::rfl_build_from(obj);
    }
};

} // namespace rfl

// ─── Generic simdjson::tag_invoke ────────────────────────────────────────────

namespace simdjson {

template<typename V, ser::ISerializable T>
auto tag_invoke(deserialize_tag, V& val, T& obj) {
    ondemand::object json_obj;
    if (auto ec = val.get_object().get(json_obj); ec) return ec;

    error_code result = SUCCESS;
    std::apply([&](auto... fds) {
        ((result == SUCCESS
            ? (result = ser::extract_simdjson_field(json_obj, obj, fds))
            : SUCCESS), ...);
    }, ser::Serializable<T>::fields());
    return result;
}

} // namespace simdjson

// ─── Generic model::from_toml ────────────────────────────────────────────────

namespace model {

export template<ser::ISerializable T>
[[nodiscard]] std::expected<void, std::string> from_toml(const toml::table& t, T& obj) {
    return ser::from_toml_impl(t, obj);
}

} // namespace model
```

- [ ] **Step 3: Commit (framework written — does not build yet without a test target)**

```bash
git add include/ser/ser.cppm
git commit -m "feat(ser): add Serializable<T> framework — field descriptors, FieldConverter, generic backends"
```

---

## Task 3: Fix timestamps.cppm setter signature mismatch

**Files:**
- Modify: `include/model/common/timestamps.cppm`

`get_scheduled_at()` returns `const std::optional<time_point>&` but the setter `set_scheduled_at(time_point)` takes a non-optional. The generic framework derives setter parameter type from the getter return type. Add overloaded setters accepting `optional<time_point>`. `time_point` is implicitly convertible to `optional<time_point>`, so existing call sites don't break.

- [ ] **Step 1: Add optional setter overloads in timestamps.cppm**

Replace the three setter declarations in `ExecutionTimings`:

```cpp
// REPLACE these three lines:
void set_scheduled_at(std::chrono::system_clock::time_point tp) noexcept { m_scheduled_at = tp; }
void set_started_at(std::chrono::system_clock::time_point tp) noexcept   { m_started_at = tp; }
void set_completed_at(std::chrono::system_clock::time_point tp) noexcept { m_completed_at = tp; }

// WITH these (accept optional<tp> — time_point implicitly converts to optional<time_point>):
void set_scheduled_at(std::optional<std::chrono::system_clock::time_point> tp) noexcept { m_scheduled_at = tp; }
void set_started_at(std::optional<std::chrono::system_clock::time_point> tp) noexcept   { m_started_at = tp; }
void set_completed_at(std::optional<std::chrono::system_clock::time_point> tp) noexcept { m_completed_at = tp; }
```

- [ ] **Step 2: Commit**

```bash
git add include/model/common/timestamps.cppm
git commit -m "fix(model): change ExecutionTimings setters to accept optional<time_point> for ser framework compatibility"
```

---

## Task 4: Remove rfl_reflectors.cppm + update model.cppm

**Files:**
- Delete: `include/model/rfl_reflectors.cppm`
- Modify: `include/model/model.cppm`

- [ ] **Step 1: Delete rfl_reflectors.cppm**

```bash
git rm include/model/rfl_reflectors.cppm
```

- [ ] **Step 2: Update model.cppm**

Replace the contents of `include/model/model.cppm` with:

```cpp
export module model;

export import :identifiers;
export import :timestamps;
export import :audit;
export import :policies;
export import :task_status;
export import :task_def;
export import :task_instance;
export import :workflow_status;
export import :workflow_dag;
export import :workflow_def;
export import :workflow_exec;
export import :workflow_event;
export import ser;
```

(Removed `:rfl_reflectors`, added `export import ser;` so all consumers of `import model;` automatically get the ser framework and its generic backends.)

- [ ] **Step 3: Commit**

```bash
git add include/model/model.cppm
git commit -m "refactor(model): remove rfl_reflectors partition; re-export ser module for framework visibility"
```

---

## Task 5: Migrate policies.cppm (RetryPolicy, TimeoutPolicy, RateLimitPolicy)

**Files:**
- Modify: `include/model/common/policies.cppm`

Replace the global fragment includes for simdjson/toml++, delete the `from_toml` and `tag_invoke` functions, and add `Serializable<T>` specializations.

- [ ] **Step 1: Write new policies.cppm**

```cpp
export module model:policies;

import std;
import ser;

export namespace model {

enum class RetryBackoff : std::uint8_t { FIXED, EXPONENTIAL };

class RetryPolicy {
  public:
    RetryPolicy(std::uint32_t max_attempts = 3, RetryBackoff backoff = RetryBackoff::FIXED,
                std::uint32_t interval_ms = 1000)
        : m_max_attempts{max_attempts}, m_backoff{backoff}, m_interval_ms{interval_ms} {}

    void set_max_attempts(std::uint32_t max_attempts) noexcept { m_max_attempts = max_attempts; }
    void set_backoff(RetryBackoff backoff) noexcept { m_backoff = backoff; }
    void set_interval_ms(std::uint32_t interval_ms) noexcept { m_interval_ms = interval_ms; }

    [[nodiscard]] std::uint32_t get_max_attempts() const noexcept { return m_max_attempts; }
    [[nodiscard]] RetryBackoff get_backoff() const noexcept { return m_backoff; }
    [[nodiscard]] std::uint32_t get_interval_ms() const noexcept { return m_interval_ms; }

    [[nodiscard]] std::expected<void, std::string> validate() const noexcept {
        if (m_max_attempts == 0) return std::unexpected{"max_attempts must be at least 1"};
        if (m_interval_ms == 0)  return std::unexpected{"interval_ms must be greater than 0"};
        return {};
    }

  private:
    std::uint32_t m_max_attempts;
    RetryBackoff m_backoff;
    std::uint32_t m_interval_ms;
};

enum class TimeoutAction : std::uint8_t { RETRY, FAIL_WORKFLOW, ALERT_ONLY };

class TimeoutPolicy {
  public:
    TimeoutPolicy(std::uint32_t timeout_ms = 30000, TimeoutAction action = TimeoutAction::FAIL_WORKFLOW)
        : m_timeout_ms{timeout_ms}, m_action{action} {}

    void set_timeout_ms(std::uint32_t timeout_ms) noexcept { m_timeout_ms = timeout_ms; }
    void set_action(TimeoutAction action) noexcept { m_action = action; }

    [[nodiscard]] std::uint32_t get_timeout_ms() const noexcept { return m_timeout_ms; }
    [[nodiscard]] TimeoutAction get_action() const noexcept { return m_action; }

    [[nodiscard]] std::expected<void, std::string> validate() const noexcept {
        if (m_timeout_ms == 0) return std::unexpected{"timeout_ms must be greater than 0"};
        return {};
    }

  private:
    std::uint32_t m_timeout_ms;
    TimeoutAction m_action;
};

class RateLimitPolicy {
  public:
    RateLimitPolicy(std::uint32_t max_concurrent = 10, std::uint32_t rate_limit_per_second = 100)
        : m_max_concurrent{max_concurrent}, m_rate_limit_per_second{rate_limit_per_second} {}

    void set_max_concurrent(std::uint32_t max_concurrent) noexcept { m_max_concurrent = max_concurrent; }
    void set_rate_limit_per_second(std::uint32_t rps) noexcept { m_rate_limit_per_second = rps; }

    [[nodiscard]] std::uint32_t get_max_concurrent() const noexcept { return m_max_concurrent; }
    [[nodiscard]] std::uint32_t get_rate_limit_per_second() const noexcept { return m_rate_limit_per_second; }

    [[nodiscard]] std::expected<void, std::string> validate() const noexcept {
        if (m_max_concurrent == 0)        return std::unexpected{"max_concurrent must be at least 1"};
        if (m_rate_limit_per_second == 0) return std::unexpected{"rate_limit_per_second must be at least 1"};
        return {};
    }

  private:
    std::uint32_t m_max_concurrent;
    std::uint32_t m_rate_limit_per_second;
};

} // namespace model

// ─── Serializable specializations ────────────────────────────────────────────

template<> struct ser::Serializable<model::RetryPolicy> {
    static constexpr auto fields() {
        return std::tuple{
            ser::field<"max_attempts",
                &model::RetryPolicy::get_max_attempts,
                &model::RetryPolicy::set_max_attempts>(),
            ser::field<"backoff",
                &model::RetryPolicy::get_backoff,
                &model::RetryPolicy::set_backoff>(),
            ser::field<"interval_ms",
                &model::RetryPolicy::get_interval_ms,
                &model::RetryPolicy::set_interval_ms>(),
        };
    }
};

template<> struct ser::Serializable<model::TimeoutPolicy> {
    static constexpr auto fields() {
        return std::tuple{
            ser::field<"timeout_ms",
                &model::TimeoutPolicy::get_timeout_ms,
                &model::TimeoutPolicy::set_timeout_ms>(),
            ser::field<"action",
                &model::TimeoutPolicy::get_action,
                &model::TimeoutPolicy::set_action>(),
        };
    }
};

template<> struct ser::Serializable<model::RateLimitPolicy> {
    static constexpr auto fields() {
        return std::tuple{
            ser::field<"max_concurrent",
                &model::RateLimitPolicy::get_max_concurrent,
                &model::RateLimitPolicy::set_max_concurrent>(),
            ser::field<"rate_limit_per_second",
                &model::RateLimitPolicy::get_rate_limit_per_second,
                &model::RateLimitPolicy::set_rate_limit_per_second>(),
        };
    }
};
```

- [ ] **Step 2: Build**

```bash
cd /home/default/cc/congelado
xmake build congelado_lib 2>&1 | tail -30
```

Expected: policies.cppm compiles without errors. If magic_enum include path is wrong in ser.cppm, you'll see "file not found" — find the correct path with `find ~/.conan2 -name "magic_enum.hpp" 2>/dev/null` and update ser.cppm's global fragment.

- [ ] **Step 3: Commit**

```bash
git add include/model/common/policies.cppm
git commit -m "refactor(model): migrate RetryPolicy/TimeoutPolicy/RateLimitPolicy to Serializable<T> framework"
```

---

## Task 6: Migrate timestamps.cppm (ExecutionTimings)

**Files:**
- Modify: `include/model/common/timestamps.cppm`

- [ ] **Step 1: Write new timestamps.cppm**

```cpp
export module model:timestamps;

import std;
import ser;

export namespace model {

class ExecutionTimings {
  public:
    ExecutionTimings() = default;

    void set_scheduled_at(std::optional<std::chrono::system_clock::time_point> tp) noexcept { m_scheduled_at = tp; }
    void set_started_at(std::optional<std::chrono::system_clock::time_point> tp) noexcept   { m_started_at = tp; }
    void set_completed_at(std::optional<std::chrono::system_clock::time_point> tp) noexcept { m_completed_at = tp; }

    [[nodiscard]] const std::optional<std::chrono::system_clock::time_point>& get_scheduled_at() const noexcept {
        return m_scheduled_at;
    }
    [[nodiscard]] const std::optional<std::chrono::system_clock::time_point>& get_started_at() const noexcept {
        return m_started_at;
    }
    [[nodiscard]] const std::optional<std::chrono::system_clock::time_point>& get_completed_at() const noexcept {
        return m_completed_at;
    }

    [[nodiscard]] std::expected<void, std::string> validate() const noexcept {
        if (m_scheduled_at && m_started_at && *m_started_at < *m_scheduled_at)
            return std::unexpected{"started_at must not be before scheduled_at"};
        if (m_started_at && m_completed_at && *m_completed_at < *m_started_at)
            return std::unexpected{"completed_at must not be before started_at"};
        return {};
    }

  private:
    std::optional<std::chrono::system_clock::time_point> m_scheduled_at;
    std::optional<std::chrono::system_clock::time_point> m_started_at;
    std::optional<std::chrono::system_clock::time_point> m_completed_at;
};

} // namespace model

template<> struct ser::Serializable<model::ExecutionTimings> {
    static constexpr auto fields() {
        return std::tuple{
            ser::field<"scheduled_at",
                &model::ExecutionTimings::get_scheduled_at,
                &model::ExecutionTimings::set_scheduled_at>(),
            ser::field<"started_at",
                &model::ExecutionTimings::get_started_at,
                &model::ExecutionTimings::set_started_at>(),
            ser::field<"completed_at",
                &model::ExecutionTimings::get_completed_at,
                &model::ExecutionTimings::set_completed_at>(),
        };
    }
};
```

- [ ] **Step 2: Build**

```bash
xmake build congelado_lib 2>&1 | tail -30
```

Expected: clean build.

- [ ] **Step 3: Commit**

```bash
git add include/model/common/timestamps.cppm
git commit -m "refactor(model): migrate ExecutionTimings to Serializable<T> framework"
```

---

## Task 7: Migrate dag.cppm (InputMapping, OutputMapping, TaskEdge, TaskNode)

**Files:**
- Modify: `include/model/workflow/dag.cppm`

Note: `TaskEdge` has `get_mappings()` → `vector<InputMapping>` (ISerializable), and `get_condition()` → `optional<string>`. `TaskNode` has `get_edges()` → `vector<TaskEdge>` (ISerializable). The setter for TaskNode is `set_task_def_name` but the TOML key and JSON key is `"task_def_name"`.

- [ ] **Step 1: Write new dag.cppm**

```cpp
export module model:workflow_dag;

import std;
import ser;

export namespace model {

class InputMapping {
  public:
    InputMapping() = default;
    InputMapping(std::string source, std::string target)
        : m_source{std::move(source)}, m_target{std::move(target)} {}

    void set_source(std::string source) { m_source = std::move(source); }
    void set_target(std::string target) { m_target = std::move(target); }

    [[nodiscard]] const std::string& get_source() const noexcept { return m_source; }
    [[nodiscard]] const std::string& get_target() const noexcept { return m_target; }

    [[nodiscard]] std::expected<void, std::string> validate() const noexcept {
        if (m_source.empty()) return std::unexpected{"InputMapping source must not be empty"};
        if (m_target.empty()) return std::unexpected{"InputMapping target must not be empty"};
        return {};
    }

  private:
    std::string m_source;
    std::string m_target;
};

class OutputMapping {
  public:
    OutputMapping() = default;
    OutputMapping(std::string source, std::string target)
        : m_source{std::move(source)}, m_target{std::move(target)} {}

    void set_source(std::string source) { m_source = std::move(source); }
    void set_target(std::string target) { m_target = std::move(target); }

    [[nodiscard]] const std::string& get_source() const noexcept { return m_source; }
    [[nodiscard]] const std::string& get_target() const noexcept { return m_target; }

    [[nodiscard]] std::expected<void, std::string> validate() const noexcept {
        if (m_source.empty()) return std::unexpected{"OutputMapping source must not be empty"};
        if (m_target.empty()) return std::unexpected{"OutputMapping target must not be empty"};
        return {};
    }

  private:
    std::string m_source;
    std::string m_target;
};

class TaskEdge {
  public:
    TaskEdge() = default;

    void add_mapping(InputMapping mapping) { m_mappings.push_back(std::move(mapping)); }
    void set_to(std::string to)                          { m_to = std::move(to); }
    void set_from(std::string from)                      { m_from = std::move(from); }
    void set_condition(std::optional<std::string> cond)  { m_condition = std::move(cond); }
    void set_mappings(std::vector<InputMapping> mappings){ m_mappings = std::move(mappings); }

    [[nodiscard]] const std::string& get_from() const noexcept                     { return m_from; }
    [[nodiscard]] const std::string& get_to() const noexcept                       { return m_to; }
    [[nodiscard]] const std::vector<InputMapping>& get_mappings() const noexcept   { return m_mappings; }
    [[nodiscard]] const std::optional<std::string>& get_condition() const noexcept { return m_condition; }

    [[nodiscard]] std::expected<void, std::string> validate() const noexcept {
        if (m_from.empty()) return std::unexpected{"TaskEdge from must not be empty"};
        if (m_to.empty())   return std::unexpected{"TaskEdge to must not be empty"};
        for (auto const& m : m_mappings)
            if (auto r = m.validate(); !r) return r;
        return {};
    }

  private:
    std::string m_from;
    std::string m_to;
    std::optional<std::string> m_condition;
    std::vector<InputMapping> m_mappings;
};

class TaskNode {
  public:
    TaskNode() = default;

    void add_edge(TaskEdge edge)                 { m_edges.push_back(std::move(edge)); }
    void set_task_def_name(std::string def_name) { m_def_name = std::move(def_name); }
    void set_edges(std::vector<TaskEdge> edges)  { m_edges = std::move(edges); }

    [[nodiscard]] const std::string& get_def_name() const noexcept        { return m_def_name; }
    [[nodiscard]] const std::vector<TaskEdge>& get_edges() const noexcept { return m_edges; }

    [[nodiscard]] std::expected<void, std::string> validate() const noexcept {
        if (m_def_name.empty()) return std::unexpected{"TaskNode def_name must not be empty"};
        for (auto const& e : m_edges)
            if (auto r = e.validate(); !r) return r;
        return {};
    }

  private:
    std::string m_def_name;
    std::vector<TaskEdge> m_edges;
};

} // namespace model

template<> struct ser::Serializable<model::InputMapping> {
    static constexpr auto fields() {
        return std::tuple{
            ser::field<"source",
                &model::InputMapping::get_source,
                &model::InputMapping::set_source>(),
            ser::field<"target",
                &model::InputMapping::get_target,
                &model::InputMapping::set_target>(),
        };
    }
};

template<> struct ser::Serializable<model::OutputMapping> {
    static constexpr auto fields() {
        return std::tuple{
            ser::field<"source",
                &model::OutputMapping::get_source,
                &model::OutputMapping::set_source>(),
            ser::field<"target",
                &model::OutputMapping::get_target,
                &model::OutputMapping::set_target>(),
        };
    }
};

template<> struct ser::Serializable<model::TaskEdge> {
    static constexpr auto fields() {
        return std::tuple{
            ser::field<"from",
                &model::TaskEdge::get_from,
                &model::TaskEdge::set_from>(),
            ser::field<"to",
                &model::TaskEdge::get_to,
                &model::TaskEdge::set_to>(),
            ser::field<"condition",
                &model::TaskEdge::get_condition,
                &model::TaskEdge::set_condition>(),
            ser::field<"mappings",
                &model::TaskEdge::get_mappings,
                &model::TaskEdge::set_mappings>(),
        };
    }
};

template<> struct ser::Serializable<model::TaskNode> {
    static constexpr auto fields() {
        return std::tuple{
            ser::field<"task_def_name",
                &model::TaskNode::get_def_name,
                &model::TaskNode::set_task_def_name>(),
            ser::field<"edges",
                &model::TaskNode::get_edges,
                &model::TaskNode::set_edges>(),
        };
    }
};
```

Note: `InputMapping` and `OutputMapping` originally lacked a default constructor. Added `InputMapping() = default` and `OutputMapping() = default` so `FieldConverter<VT>::from_toml_impl` can default-construct before populating via setters.

- [ ] **Step 2: Build**

```bash
xmake build congelado_lib 2>&1 | tail -30
```

- [ ] **Step 3: Commit**

```bash
git add include/model/workflow/dag.cppm
git commit -m "refactor(model): migrate InputMapping/OutputMapping/TaskEdge/TaskNode to Serializable<T> framework"
```

---

## Task 8: Migrate task/definition.cppm (TaskDef)

**Files:**
- Modify: `include/model/task/definition.cppm`

`TaskDef` has nested `RetryPolicy`, `TimeoutPolicy`, and `optional<RateLimitPolicy>` — all `ISerializable`. Also `vector<string>` for input/output keys, and an enum `TaskType`.

- [ ] **Step 1: Write new definition.cppm**

```cpp
export module model:task_def;

import std;
import :task_status;
import :policies;
import ser;

export namespace model {

class TaskDef {
  public:
    TaskDef() = default;

    void add_input_key(std::string key)  { m_input_keys.push_back(std::move(key)); }
    void add_output_key(std::string key) { m_output_keys.push_back(std::move(key)); }

    void set_name(std::string name)                                { m_name = std::move(name); }
    void set_type(TaskType type) noexcept                          { m_type = type; }
    void set_worker_type(std::string type)                         { m_worker_type = std::move(type); }
    void set_input_keys(std::vector<std::string> input)            { m_input_keys = std::move(input); }
    void set_output_keys(std::vector<std::string> output)          { m_output_keys = std::move(output); }
    void set_retry(RetryPolicy retry) noexcept                     { m_retry = retry; }
    void set_timeout(TimeoutPolicy timeout) noexcept               { m_timeout = timeout; }
    void set_rate_limit(std::optional<RateLimitPolicy> rate_limit) noexcept { m_rate_limit = rate_limit; }

    [[nodiscard]] const std::string& get_name() const noexcept                          { return m_name; }
    [[nodiscard]] TaskType get_type() const noexcept                                     { return m_type; }
    [[nodiscard]] const std::string& get_worker_type() const noexcept                   { return m_worker_type; }
    [[nodiscard]] const std::vector<std::string>& get_input_keys() const noexcept       { return m_input_keys; }
    [[nodiscard]] const std::vector<std::string>& get_output_keys() const noexcept      { return m_output_keys; }
    [[nodiscard]] const RetryPolicy& get_retry() const noexcept                         { return m_retry; }
    [[nodiscard]] const TimeoutPolicy& get_timeout() const noexcept                     { return m_timeout; }
    [[nodiscard]] const std::optional<RateLimitPolicy>& get_rate_limit() const noexcept { return m_rate_limit; }

    [[nodiscard]] std::expected<void, std::string> validate() const noexcept {
        if (m_name.empty())
            return std::unexpected{"TaskDef name must not be empty"};
        if (m_type == TaskType::SIMPLE && m_worker_type.empty())
            return std::unexpected{"TaskDef worker_type must not be empty for SIMPLE tasks"};
        if (auto result = m_retry.validate(); !result)   return result;
        if (auto result = m_timeout.validate(); !result) return result;
        if (m_rate_limit)
            if (auto result = m_rate_limit->validate(); !result) return result;
        return {};
    }

  private:
    std::string m_name;
    TaskType m_type{TaskType::SIMPLE};
    std::string m_worker_type;
    std::vector<std::string> m_input_keys;
    std::vector<std::string> m_output_keys;
    RetryPolicy m_retry;
    TimeoutPolicy m_timeout;
    std::optional<RateLimitPolicy> m_rate_limit;
};

} // namespace model

template<> struct ser::Serializable<model::TaskDef> {
    static constexpr auto fields() {
        return std::tuple{
            ser::field<"name",
                &model::TaskDef::get_name,
                &model::TaskDef::set_name>(),
            ser::field<"type",
                &model::TaskDef::get_type,
                &model::TaskDef::set_type>(),
            ser::field<"worker_type",
                &model::TaskDef::get_worker_type,
                &model::TaskDef::set_worker_type>(),
            ser::field<"input_keys",
                &model::TaskDef::get_input_keys,
                &model::TaskDef::set_input_keys>(),
            ser::field<"output_keys",
                &model::TaskDef::get_output_keys,
                &model::TaskDef::set_output_keys>(),
            ser::field<"retry",
                &model::TaskDef::get_retry,
                &model::TaskDef::set_retry>(),
            ser::field<"timeout",
                &model::TaskDef::get_timeout,
                &model::TaskDef::set_timeout>(),
            ser::field<"rate_limit",
                &model::TaskDef::get_rate_limit,
                &model::TaskDef::set_rate_limit>(),
        };
    }
};
```

- [ ] **Step 2: Build**

```bash
xmake build congelado_lib 2>&1 | tail -30
```

- [ ] **Step 3: Commit**

```bash
git add include/model/task/definition.cppm
git commit -m "refactor(model): migrate TaskDef to Serializable<T> framework"
```

---

## Task 9: Migrate task/instance.cppm (TaskInstance)

**Files:**
- Modify: `include/model/task/instance.cppm`

`TaskInstance` has `uuids::uuid` fields (`task_id`, `workflow_exec_id`), enum `TaskStatus`, `unordered_map<string,string>` for input/output data, and nested `ExecutionTimings` (ISerializable).

- [ ] **Step 1: Write new instance.cppm**

```cpp
export module model:task_instance;

import std;
import :identifiers;
import :timestamps;
import :task_status;
import ser;

export namespace model {

class TaskInstance {
  public:
    TaskInstance() = default;

    void add_input_data(std::string key, std::string value)  { m_input_data.emplace(std::move(key), std::move(value)); }
    void add_output_data(std::string key, std::string value) { m_output_data.emplace(std::move(key), std::move(value)); }

    void set_workflow_exec_id(ExecutionId execution_id)                            { m_workflow_exec_id = execution_id; }
    void set_def_name(std::string def_name)                                        { m_def_name = std::move(def_name); }
    void set_task_id(TaskId task_id)                                               { m_task_id = task_id; }
    void set_status(TaskStatus status) noexcept                                    { m_status = status; }
    void set_seq(std::uint32_t seq) noexcept                                       { m_seq = seq; }
    void set_input_data(std::unordered_map<std::string, std::string> data)         { m_input_data = std::move(data); }
    void set_output_data(std::unordered_map<std::string, std::string> data)        { m_output_data = std::move(data); }
    void set_timings(ExecutionTimings timing)                                      { m_timings = timing; }
    void set_retry_count(std::uint32_t count) noexcept                             { m_retry_count = count; }

    [[nodiscard]] const TaskId& get_task_id() const noexcept                                          { return m_task_id; }
    [[nodiscard]] const std::string& get_def_name() const noexcept                                    { return m_def_name; }
    [[nodiscard]] const ExecutionId& get_workflow_exec_id() const noexcept                            { return m_workflow_exec_id; }
    [[nodiscard]] TaskStatus get_status() const noexcept                                              { return m_status; }
    [[nodiscard]] std::uint32_t get_seq() const noexcept                                              { return m_seq; }
    [[nodiscard]] const std::unordered_map<std::string, std::string>& get_input_data() const noexcept { return m_input_data; }
    [[nodiscard]] const std::unordered_map<std::string, std::string>& get_output_data() const noexcept{ return m_output_data; }
    [[nodiscard]] const ExecutionTimings& get_timings() const noexcept                                { return m_timings; }
    [[nodiscard]] std::uint32_t get_retry_count() const noexcept                                      { return m_retry_count; }

    [[nodiscard]] std::expected<void, std::string> validate() const noexcept {
        if (m_def_name.empty())
            return std::unexpected{"TaskInstance def_name must not be empty"};
        if (auto result = m_timings.validate(); !result) return result;
        return {};
    }

  private:
    TaskId m_task_id;
    std::string m_def_name;
    ExecutionId m_workflow_exec_id;
    TaskStatus m_status{TaskStatus::SCHEDULED};
    std::uint32_t m_seq{0};
    std::unordered_map<std::string, std::string> m_input_data;
    std::unordered_map<std::string, std::string> m_output_data;
    ExecutionTimings m_timings;
    std::uint32_t m_retry_count{0};
};

} // namespace model

template<> struct ser::Serializable<model::TaskInstance> {
    static constexpr auto fields() {
        return std::tuple{
            ser::field<"task_id",
                &model::TaskInstance::get_task_id,
                &model::TaskInstance::set_task_id>(),
            ser::field<"workflow_exec_id",
                &model::TaskInstance::get_workflow_exec_id,
                &model::TaskInstance::set_workflow_exec_id>(),
            ser::field<"def_name",
                &model::TaskInstance::get_def_name,
                &model::TaskInstance::set_def_name>(),
            ser::field<"status",
                &model::TaskInstance::get_status,
                &model::TaskInstance::set_status>(),
            ser::field<"seq",
                &model::TaskInstance::get_seq,
                &model::TaskInstance::set_seq>(),
            ser::field<"retry_count",
                &model::TaskInstance::get_retry_count,
                &model::TaskInstance::set_retry_count>(),
            ser::field<"input_data",
                &model::TaskInstance::get_input_data,
                &model::TaskInstance::set_input_data>(),
            ser::field<"output_data",
                &model::TaskInstance::get_output_data,
                &model::TaskInstance::set_output_data>(),
            ser::field<"timings",
                &model::TaskInstance::get_timings,
                &model::TaskInstance::set_timings>(),
        };
    }
};
```

Note: `TaskId` and `ExecutionId` are both `uuids::uuid` aliases (from `:identifiers`). `FieldConverter<uuids::uuid>` handles them. If they are typedefs/aliases, this works automatically. If they are distinct strong types, add `FieldConverter` specializations for them matching the `uuids::uuid` pattern.

- [ ] **Step 2: Build**

```bash
xmake build congelado_lib 2>&1 | tail -30
```

- [ ] **Step 3: Commit**

```bash
git add include/model/task/instance.cppm
git commit -m "refactor(model): migrate TaskInstance to Serializable<T> framework"
```

---

## Task 10: Migrate workflow/definition.cppm (WorkflowDef)

**Files:**
- Modify: `include/model/workflow/definition.cppm`

`WorkflowDef` has `vector<TaskNode>` (ISerializable), `vector<string>` (input_params), `vector<OutputMapping>` (ISerializable), `optional<string>` (failure_workflow), and `optional<TimeoutPolicy>` (ISerializable).

- [ ] **Step 1: Write new definition.cppm**

```cpp
export module model:workflow_def;

import std;
import :workflow_dag;
import :policies;
import ser;

export namespace model {

class WorkflowDef {
  public:
    WorkflowDef() = default;

    void add_node(TaskNode node)                    { m_nodes.push_back(std::move(node)); }
    void add_input_param(std::string param)         { m_input_params.push_back(std::move(param)); }
    void add_output_mapping(OutputMapping mapping)  { m_output_mappings.push_back(std::move(mapping)); }

    void set_name(std::string name)                                         { m_name = std::move(name); }
    void set_version(std::uint32_t version) noexcept                        { m_version = version; }
    void set_nodes(std::vector<TaskNode> nodes)                             { m_nodes = std::move(nodes); }
    void set_input_params(std::vector<std::string> params)                  { m_input_params = std::move(params); }
    void set_output_mappings(std::vector<OutputMapping> mappings)           { m_output_mappings = std::move(mappings); }
    void set_failure_workflow(std::optional<std::string> failure_workflow)  { m_failure_workflow = std::move(failure_workflow); }
    void set_timeout(std::optional<TimeoutPolicy> timeout) noexcept         { m_timeout = timeout; }

    [[nodiscard]] std::uint32_t get_version() const noexcept                               { return m_version; }
    [[nodiscard]] const std::string& get_name() const noexcept                             { return m_name; }
    [[nodiscard]] const std::vector<TaskNode>& get_nodes() const noexcept                  { return m_nodes; }
    [[nodiscard]] const std::vector<std::string>& get_input_params() const noexcept        { return m_input_params; }
    [[nodiscard]] const std::vector<OutputMapping>& get_output_mappings() const noexcept   { return m_output_mappings; }
    [[nodiscard]] const std::optional<std::string>& get_failure_workflow() const noexcept  { return m_failure_workflow; }
    [[nodiscard]] const std::optional<TimeoutPolicy>& get_timeout() const noexcept         { return m_timeout; }

    [[nodiscard]] std::expected<void, std::string> validate() const noexcept {
        if (m_name.empty())    return std::unexpected{"WorkflowDef name must not be empty"};
        if (m_version == 0)    return std::unexpected{"WorkflowDef version must be at least 1"};
        if (m_nodes.empty())   return std::unexpected{"WorkflowDef must have at least one node"};
        for (auto const& node    : m_nodes)          if (auto r = node.validate(); !r)    return r;
        for (auto const& mapping : m_output_mappings) if (auto r = mapping.validate(); !r) return r;
        if (m_timeout) if (auto r = m_timeout->validate(); !r) return r;
        return {};
    }

  private:
    std::string m_name;
    std::uint32_t m_version{1};
    std::vector<TaskNode> m_nodes;
    std::vector<std::string> m_input_params;
    std::vector<OutputMapping> m_output_mappings;
    std::optional<std::string> m_failure_workflow;
    std::optional<TimeoutPolicy> m_timeout;
};

} // namespace model

template<> struct ser::Serializable<model::WorkflowDef> {
    static constexpr auto fields() {
        return std::tuple{
            ser::field<"name",
                &model::WorkflowDef::get_name,
                &model::WorkflowDef::set_name>(),
            ser::field<"version",
                &model::WorkflowDef::get_version,
                &model::WorkflowDef::set_version>(),
            ser::field<"nodes",
                &model::WorkflowDef::get_nodes,
                &model::WorkflowDef::set_nodes>(),
            ser::field<"input_params",
                &model::WorkflowDef::get_input_params,
                &model::WorkflowDef::set_input_params>(),
            ser::field<"output_mappings",
                &model::WorkflowDef::get_output_mappings,
                &model::WorkflowDef::set_output_mappings>(),
            ser::field<"failure_workflow",
                &model::WorkflowDef::get_failure_workflow,
                &model::WorkflowDef::set_failure_workflow>(),
            ser::field<"timeout",
                &model::WorkflowDef::get_timeout,
                &model::WorkflowDef::set_timeout>(),
        };
    }
};
```

`FieldConverter<std::optional<std::string>>` is already included in ser.cppm (written in Task 2).

- [ ] **Step 2: Build**

```bash
xmake build congelado_lib 2>&1 | tail -30
```

- [ ] **Step 3: Commit**

```bash
git add include/model/workflow/definition.cppm include/ser/ser.cppm
git commit -m "refactor(model): migrate WorkflowDef to Serializable<T> framework; add optional<string> FieldConverter"
```

---

## Task 11: Migrate workflow/event.cppm (WorkflowEvent)

**Files:**
- Modify: `include/model/workflow/event.cppm`

`WorkflowEvent` has `uuids::uuid exec_id`, enum `WorkflowEventType`, `optional<string> payload`, and `time_point issued_at`.

- [ ] **Step 1: Write new event.cppm**

```cpp
export module model:workflow_event;

import std;
import :identifiers;
import ser;

export namespace model {

enum class WorkflowEventType : std::uint8_t {
    PAUSE,
    RESUME,
    TERMINATE,
    RESTART,
    SIGNAL,
};

class WorkflowEvent {
  public:
    WorkflowEvent() = default;

    void set_exec_id(ExecutionId execution_id)                   { m_exec_id = execution_id; }
    void set_type(WorkflowEventType type) noexcept               { m_type = type; }
    void set_payload(std::optional<std::string> payload)         { m_payload = std::move(payload); }
    void set_issued_at(std::chrono::system_clock::time_point tp) noexcept { m_issued_at = tp; }

    [[nodiscard]] WorkflowEventType get_type() const noexcept                    { return m_type; }
    [[nodiscard]] const ExecutionId& get_exec_id() const noexcept                { return m_exec_id; }
    [[nodiscard]] const std::optional<std::string>& get_payload() const noexcept { return m_payload; }
    [[nodiscard]] const std::chrono::system_clock::time_point& get_issued_at() const noexcept { return m_issued_at; }

    [[nodiscard]] std::expected<void, std::string> validate() const noexcept {
        if (m_exec_id == ExecutionId{})
            return std::unexpected{"WorkflowEvent exec_id must not be nil"};
        return {};
    }

  private:
    ExecutionId m_exec_id;
    WorkflowEventType m_type{};
    std::optional<std::string> m_payload;
    std::chrono::system_clock::time_point m_issued_at;
};

} // namespace model

template<> struct ser::Serializable<model::WorkflowEvent> {
    static constexpr auto fields() {
        return std::tuple{
            ser::field<"exec_id",
                &model::WorkflowEvent::get_exec_id,
                &model::WorkflowEvent::set_exec_id>(),
            ser::field<"type",
                &model::WorkflowEvent::get_type,
                &model::WorkflowEvent::set_type>(),
            ser::field<"payload",
                &model::WorkflowEvent::get_payload,
                &model::WorkflowEvent::set_payload>(),
            ser::field<"issued_at",
                &model::WorkflowEvent::get_issued_at,
                &model::WorkflowEvent::set_issued_at>(),
        };
    }
};
```

- [ ] **Step 2: Build**

```bash
xmake build congelado_lib 2>&1 | tail -30
```

- [ ] **Step 3: Commit**

```bash
git add include/model/workflow/event.cppm
git commit -m "refactor(model): migrate WorkflowEvent to Serializable<T> framework"
```

---

## Task 12: Migrate workflow/exec.cppm (WorkflowExecution)

**Files:**
- Modify: `include/model/workflow/exec.cppm`

`WorkflowExecution` has `exec_id` (uuid), `optional<correlation_id>` (optional<uuid>), `WorkflowStatus` (enum), `unordered_map<string,string>` variables, `vector<TaskInstance>` (ISerializable), and nested `ExecutionTimings` (ISerializable).

- [ ] **Step 1: Write new exec.cppm**

```cpp
export module model:workflow_exec;

import std;
import :identifiers;
import :timestamps;
import :workflow_status;
import :task_instance;
import ser;

export namespace model {

class WorkflowExecution {
  public:
    WorkflowExecution() = default;

    void add_task_instance(TaskInstance instance)  { m_task_instances.push_back(std::move(instance)); }
    void add_variable(std::string key, std::string value) { m_variables.emplace(std::move(key), std::move(value)); }

    void set_exec_id(ExecutionId execution_id)                                       { m_exec_id = execution_id; }
    void set_def_name(std::string name)                                              { m_def_name = std::move(name); }
    void set_def_version(std::uint32_t version) noexcept                             { m_def_version = version; }
    void set_status(WorkflowStatus status) noexcept                                  { m_status = status; }
    void set_correlation_id(std::optional<CorrelationId> correlation_id)             { m_correlation_id = correlation_id; }
    void set_variables(std::unordered_map<std::string, std::string> variables)       { m_variables = std::move(variables); }
    void set_task_instances(std::vector<TaskInstance> instances)                     { m_task_instances = std::move(instances); }
    void set_timings(ExecutionTimings timings)                                       { m_timings = timings; }

    [[nodiscard]] const std::string& get_def_name() const noexcept                                      { return m_def_name; }
    [[nodiscard]] const ExecutionId& get_exec_id() const noexcept                                       { return m_exec_id; }
    [[nodiscard]] std::uint32_t get_def_version() const noexcept                                        { return m_def_version; }
    [[nodiscard]] WorkflowStatus get_status() const noexcept                                            { return m_status; }
    [[nodiscard]] const std::optional<CorrelationId>& get_correlation_id() const noexcept               { return m_correlation_id; }
    [[nodiscard]] const std::unordered_map<std::string, std::string>& get_variables() const noexcept    { return m_variables; }
    [[nodiscard]] const std::vector<TaskInstance>& get_task_instances() const noexcept                  { return m_task_instances; }
    [[nodiscard]] const ExecutionTimings& get_timings() const noexcept                                  { return m_timings; }

    [[nodiscard]] std::expected<void, std::string> validate() const noexcept {
        if (m_def_name.empty())   return std::unexpected{"WorkflowExecution def_name must not be empty"};
        if (m_def_version == 0)   return std::unexpected{"WorkflowExecution def_version must be at least 1"};
        if (auto result = m_timings.validate(); !result) return result;
        for (auto const& instance : m_task_instances)
            if (auto result = instance.validate(); !result) return result;
        return {};
    }

  private:
    ExecutionId m_exec_id;
    std::string m_def_name;
    std::uint32_t m_def_version{1};
    WorkflowStatus m_status{WorkflowStatus::RUNNING};
    std::optional<CorrelationId> m_correlation_id;
    std::unordered_map<std::string, std::string> m_variables;
    std::vector<TaskInstance> m_task_instances;
    ExecutionTimings m_timings;
};

} // namespace model

template<> struct ser::Serializable<model::WorkflowExecution> {
    static constexpr auto fields() {
        return std::tuple{
            ser::field<"exec_id",
                &model::WorkflowExecution::get_exec_id,
                &model::WorkflowExecution::set_exec_id>(),
            ser::field<"def_name",
                &model::WorkflowExecution::get_def_name,
                &model::WorkflowExecution::set_def_name>(),
            ser::field<"def_version",
                &model::WorkflowExecution::get_def_version,
                &model::WorkflowExecution::set_def_version>(),
            ser::field<"status",
                &model::WorkflowExecution::get_status,
                &model::WorkflowExecution::set_status>(),
            ser::field<"correlation_id",
                &model::WorkflowExecution::get_correlation_id,
                &model::WorkflowExecution::set_correlation_id>(),
            ser::field<"variables",
                &model::WorkflowExecution::get_variables,
                &model::WorkflowExecution::set_variables>(),
            ser::field<"task_instances",
                &model::WorkflowExecution::get_task_instances,
                &model::WorkflowExecution::set_task_instances>(),
            ser::field<"timings",
                &model::WorkflowExecution::get_timings,
                &model::WorkflowExecution::set_timings>(),
        };
    }
};
```

Note: `CorrelationId` is `uuids::uuid` (from `:identifiers`). `FieldConverter<std::optional<uuids::uuid>>` handles it — verify that `CorrelationId` is a plain `using` alias for `uuids::uuid`, not a distinct type.

- [ ] **Step 2: Build**

```bash
xmake build congelado_lib 2>&1 | tail -30
```

Expected: clean build. All 13 types migrated.

- [ ] **Step 3: Commit**

```bash
git add include/model/workflow/exec.cppm
git commit -m "refactor(model): migrate WorkflowExecution to Serializable<T> framework — migration complete"
```

---

## Task 13: Final verification

- [ ] **Step 1: Full clean build**

```bash
cd /home/default/cc/congelado
xmake clean
xmake build congelado_lib 2>&1 | tail -40
```

Expected: zero errors.

- [ ] **Step 2: Verify rfl serialization works (manual spot check)**

Add a temporary check in `src/main.cc` (or run as a standalone snippet if possible):

```cpp
// Spot check: round-trip RetryPolicy through rfl JSON
model::RetryPolicy p{5, model::RetryBackoff::EXPONENTIAL, 2000};
auto json = rfl::to_json(p);
// Expected: {"max_attempts":5,"backoff":"EXPONENTIAL","interval_ms":2000}
// (Print json and verify structure — remove after check)
```

Run: `xmake run congelado 2>&1 | head -5` if main.cc has a print statement.

- [ ] **Step 3: Final commit**

```bash
git add -A
git status  # verify nothing unexpected
git commit -m "feat(ser): complete Serializable<T> migration — 13 model types, ~500 lines removed"
```

---

## Troubleshooting

**magic_enum not found**: If `#include <rfl/internal/magic_enum/magic_enum.hpp>` fails:
1. Run `find ~/.conan2 -name "magic_enum.hpp" 2>/dev/null` — use the found path.
2. If not bundled at all, add to xmake.lua: `add_requires("conan::magic_enum/0.9.7", { alias = "magic_enum", configs = conan })` and `add_packages("magic_enum", ...)`.

**`rfl::make_field<Fd::name>` compile error**: If `StringLiteral` NTTP doesn't work as template argument, try `rfl::make_field<Fd::name.string_view()>(...)` or wrap in a constexpr function that converts StringLiteral to rfl's internal format.

**NamedTuple + fold empty**: If a type has 0 fields (impossible given current model types, but guarded): `static_assert(sizeof...(Fds) > 0, "Serializable<T> must have at least one field");`

**simdjson `find_field_unordered` not available**: Some simdjson versions name it differently. Fall back to `obj[Fd::name.string_view()]` if the unordered variant doesn't compile.

**CorrelationId / TaskId are strong types**: If they are not plain `using` aliases for `uuids::uuid`, add `FieldConverter` specializations modeled on `FieldConverter<uuids::uuid>`.

**Optional<string> for payload/failure_workflow**: The primary `FieldConverter<VT>` doesn't handle `optional<VT>` for primitive VT. The `optional<string>` specialization described in Task 10 must be added to ser.cppm before these fields are used.
