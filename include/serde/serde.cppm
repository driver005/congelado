module;
#define UUID_SYSTEM_GENERATOR
#include <rfl.hpp>
#include <rfl/enums.hpp>
#include <simdjson.h>
#include <toml++/toml.hpp>
#include <uuid.h>

export module serde;

import std;


export namespace serde {

template <typename MFP>
struct MFPTraits;

template <typename C, typename R>
struct MFPTraits<R (C::*)() const> {
    using class_t = C;
    using value_t = std::remove_cvref_t<R>;
};

template <typename C, typename R>
struct MFPTraits<R (C::*)() const noexcept> {
    using class_t = C;
    using value_t = std::remove_cvref_t<R>;
};

// ─── FieldDesc ────────────────────────────────────────────────────────────────

template <rfl::internal::StringLiteral Name, auto Getter, auto Setter>
struct FieldDesc {
    static constexpr auto name = Name;
    static constexpr auto getter = Getter;
    static constexpr auto setter = Setter;
    using ClassType = typename MFPTraits<decltype(Getter)>::class_t;
    using ValueType = typename MFPTraits<decltype(Getter)>::value_t;
};

template <rfl::internal::StringLiteral Name, auto Getter, auto Setter>
constexpr auto field() {
    return FieldDesc<Name, Getter, Setter>{};
}

// ─── Serializable<T> + ISerializable concept ──────────────────────────────────

template <typename T>
struct Serializable;

template <typename T>
concept ISerializable = requires {
    { Serializable<T>::fields() };
};

} // namespace serde

// ─── Forward declaration of from_toml_impl ───────────────────────────────────

namespace serde {
template <ISerializable T>
std::expected<void, std::string> from_toml_impl(const toml::table &, T &);
}

// ─── FieldConverter<VT> primary + specializations ────────────────────────────

export namespace serde {

// Primary: handles simdjson-native primitives (std::string, bool, int64_t, uint64_t, double)
template <typename VT>
struct FieldConverter {
    using rfl_type = VT;

    static simdjson::error_code from_simdjson(simdjson::ondemand::value &v, VT &out) {
        return v.get(out);
    }

    static std::expected<VT, std::string> from_toml(const toml::table &t, std::string_view fname) {
        auto val = t[fname].value<VT>();
        if (!val)
            return std::unexpected{std::format("missing or invalid field '{}'", fname)};
        return *val;
    }

    static rfl_type to_rfl(const VT &v) { return v; }
    static VT from_rfl(const rfl_type &v) { return v; }
};

// ─── uint32_t ─────────────────────────────────────────────────────────────────

template <>
struct FieldConverter<std::uint32_t> {
    using rfl_type = std::uint32_t;

    static simdjson::error_code from_simdjson(simdjson::ondemand::value &v, std::uint32_t &out) {
        std::uint64_t tmp{};
        if (auto ec = v.get_uint64().get(tmp); ec)
            return ec;
        out = static_cast<std::uint32_t>(tmp);
        return simdjson::SUCCESS;
    }

    static std::expected<std::uint32_t, std::string> from_toml(const toml::table &t,
                                                               std::string_view fname) {
        auto val = t[fname].value<std::int64_t>();
        if (!val)
            return std::unexpected{std::format("missing field '{}'", fname)};
        return static_cast<std::uint32_t>(*val);
    }

    static std::uint32_t to_rfl(std::uint32_t v) { return v; }
    static std::uint32_t from_rfl(std::uint32_t v) { return v; }
};

// ─── Enum types ───────────────────────────────────────────────────────────────

template <typename E>
    requires std::is_enum_v<E>
struct FieldConverter<E> {
    using rfl_type = E;

    static simdjson::error_code from_simdjson(simdjson::ondemand::value &v, E &out) {
        std::string_view sv;
        if (auto ec = v.get_string().get(sv); ec)
            return ec;
        auto result = rfl::string_to_enum<E>(std::string{sv});
        if (!result)
            return simdjson::INCORRECT_TYPE;
        out = *result;
        return simdjson::SUCCESS;
    }

    static std::expected<E, std::string> from_toml(const toml::table &t, std::string_view fname) {
        auto sv = t[fname].value<std::string>();
        if (!sv)
            return std::unexpected{std::format("missing field '{}'", fname)};
        auto result = rfl::string_to_enum<E>(*sv);
        if (!result)
            return std::unexpected{std::format("invalid enum '{}' for field '{}'", *sv, fname)};
        return *result;
    }

    static E to_rfl(const E &v) { return v; }
    static E from_rfl(const E &v) { return v; }
};

// ─── uuids::uuid ──────────────────────────────────────────────────────────────

template <>
struct FieldConverter<uuids::uuid> {
    using rfl_type = std::string;

    static simdjson::error_code from_simdjson(simdjson::ondemand::value &v, uuids::uuid &out) {
        std::string_view sv;
        if (auto ec = v.get_string().get(sv); ec)
            return ec;
        auto id = uuids::uuid::from_string(sv);
        if (!id)
            return simdjson::INCORRECT_TYPE;
        out = *id;
        return simdjson::SUCCESS;
    }

    static std::expected<uuids::uuid, std::string> from_toml(const toml::table &t,
                                                             std::string_view fname) {
        auto sv = t[fname].value<std::string>();
        if (!sv)
            return std::unexpected{std::format("missing field '{}'", fname)};
        auto id = uuids::uuid::from_string(*sv);
        if (!id)
            return std::unexpected{std::format("invalid UUID for field '{}'", fname)};
        return *id;
    }

    static std::string to_rfl(const uuids::uuid &v) { return uuids::to_string(v); }
    static uuids::uuid from_rfl(const std::string &s) {
        return uuids::uuid::from_string(s).value_or(uuids::uuid{});
    }
};

// ─── std::optional<uuids::uuid> ───────────────────────────────────────────────

template <>
struct FieldConverter<std::optional<uuids::uuid>> {
    using rfl_type = std::optional<std::string>;

    static simdjson::error_code from_simdjson(simdjson::ondemand::value &v,
                                              std::optional<uuids::uuid> &out) {
        if (v.is_null()) {
            out = std::nullopt;
            return simdjson::SUCCESS;
        }
        std::string_view sv;
        if (auto ec = v.get_string().get(sv); ec)
            return ec;
        auto id = uuids::uuid::from_string(sv);
        if (!id)
            return simdjson::INCORRECT_TYPE;
        out = *id;
        return simdjson::SUCCESS;
    }

    static std::expected<std::optional<uuids::uuid>, std::string>
    from_toml(const toml::table &t, std::string_view fname) {
        auto sv = t[fname].value<std::string>();
        if (!sv)
            return std::nullopt;
        auto id = uuids::uuid::from_string(*sv);
        if (!id)
            return std::unexpected{std::format("invalid UUID for field '{}'", fname)};
        return *id;
    }

    static rfl_type to_rfl(const std::optional<uuids::uuid> &v) {
        if (!v)
            return std::nullopt;
        return uuids::to_string(*v);
    }
    static std::optional<uuids::uuid> from_rfl(const rfl_type &s) {
        if (!s)
            return std::nullopt;
        return uuids::uuid::from_string(*s).value_or(uuids::uuid{});
    }
};

// ─── std::chrono::system_clock::time_point ───────────────────────────────────

using TP = std::chrono::system_clock::time_point;

template <>
struct FieldConverter<TP> {
    using rfl_type = std::int64_t;

    static simdjson::error_code from_simdjson(simdjson::ondemand::value &v, TP &out) {
        std::int64_t ms{};
        if (auto ec = v.get_int64().get(ms); ec)
            return ec;
        out = TP{std::chrono::milliseconds{ms}};
        return simdjson::SUCCESS;
    }

    static std::expected<TP, std::string> from_toml(const toml::table &t, std::string_view fname) {
        auto ms = t[fname].value<std::int64_t>();
        if (!ms)
            return std::unexpected{std::format("missing field '{}'", fname)};
        return TP{std::chrono::milliseconds{*ms}};
    }

    static std::int64_t to_rfl(const TP &v) {
        return std::chrono::duration_cast<std::chrono::milliseconds>(v.time_since_epoch()).count();
    }
    static TP from_rfl(std::int64_t ms) { return TP{std::chrono::milliseconds{ms}}; }
};

// ─── std::optional<time_point> ────────────────────────────────────────────────

template <>
struct FieldConverter<std::optional<TP>> {
    using rfl_type = std::optional<std::int64_t>;

    static simdjson::error_code from_simdjson(simdjson::ondemand::value &v,
                                              std::optional<TP> &out) {
        if (v.is_null()) {
            out = std::nullopt;
            return simdjson::SUCCESS;
        }
        std::int64_t ms{};
        if (auto ec = v.get_int64().get(ms); ec)
            return ec;
        out = TP{std::chrono::milliseconds{ms}};
        return simdjson::SUCCESS;
    }

    static std::expected<std::optional<TP>, std::string> from_toml(const toml::table &t,
                                                                   std::string_view fname) {
        auto ms = t[fname].value<std::int64_t>();
        if (!ms)
            return std::nullopt;
        return TP{std::chrono::milliseconds{*ms}};
    }

    static rfl_type to_rfl(const std::optional<TP> &v) {
        if (!v)
            return std::nullopt;
        return std::chrono::duration_cast<std::chrono::milliseconds>(v->time_since_epoch()).count();
    }
    static std::optional<TP> from_rfl(const rfl_type &ms) {
        if (!ms)
            return std::nullopt;
        return TP{std::chrono::milliseconds{*ms}};
    }
};

// ─── std::vector<std::string> ────────────────────────────────────────────────

template <>
struct FieldConverter<std::vector<std::string>> {
    using rfl_type = std::vector<std::string>;

    static simdjson::error_code from_simdjson(simdjson::ondemand::value &v,
                                              std::vector<std::string> &out) {
        simdjson::ondemand::array arr;
        if (auto ec = v.get_array().get(arr); ec)
            return ec;
        for (auto elem : arr) {
            std::string_view sv;
            simdjson::ondemand::value elem_val;
            if (auto ec = elem.get(elem_val); ec)
                return ec;
            if (auto ec = elem_val.get_string().get(sv); ec)
                return ec;
            out.emplace_back(sv);
        }
        return simdjson::SUCCESS;
    }

    static std::expected<std::vector<std::string>, std::string> from_toml(const toml::table &t,
                                                                          std::string_view fname) {
        const auto *arr = t[fname].as_array();
        if (!arr)
            return std::vector<std::string>{};
        std::vector<std::string> result;
        result.reserve(arr->size());
        for (const auto &elem : *arr) {
            auto sv = elem.value<std::string>();
            if (!sv)
                return std::unexpected{std::format("element of '{}' must be a string", fname)};
            result.push_back(std::move(*sv));
        }
        return result;
    }

    static rfl_type to_rfl(const std::vector<std::string> &v) { return v; }
    static std::vector<std::string> from_rfl(const rfl_type &v) { return v; }
};

// ─── std::optional<std::string> ──────────────────────────────────────────────

template <>
struct FieldConverter<std::optional<std::string>> {
    using rfl_type = std::optional<std::string>;

    static simdjson::error_code from_simdjson(simdjson::ondemand::value &v,
                                              std::optional<std::string> &out) {
        if (v.is_null()) {
            out = std::nullopt;
            return simdjson::SUCCESS;
        }
        std::string_view sv;
        if (auto ec = v.get_string().get(sv); ec)
            return ec;
        out = std::string{sv};
        return simdjson::SUCCESS;
    }

    static std::expected<std::optional<std::string>, std::string>
    from_toml(const toml::table &t, std::string_view fname) {
        auto sv = t[fname].value<std::string>();
        if (!sv)
            return std::nullopt;
        return *sv;
    }

    static rfl_type to_rfl(const std::optional<std::string> &v) { return v; }
    static std::optional<std::string> from_rfl(const rfl_type &v) { return v; }
};

// ─── std::unordered_map<std::string, std::string> ────────────────────────────

template <>
struct FieldConverter<std::unordered_map<std::string, std::string>> {
    using rfl_type = std::map<std::string, std::string>;

    static simdjson::error_code from_simdjson(simdjson::ondemand::value &v,
                                              std::unordered_map<std::string, std::string> &out) {
        simdjson::ondemand::object obj;
        if (auto ec = v.get_object().get(obj); ec)
            return ec;
        for (auto f : obj) {
            std::string_view k, val;
            if (auto ec = f.unescaped_key().get(k); ec)
                return ec;
            if (auto ec = f.value().get_string().get(val); ec)
                return ec;
            out.emplace(k, val);
        }
        return simdjson::SUCCESS;
    }

    static std::expected<std::unordered_map<std::string, std::string>, std::string>
    from_toml(const toml::table &t, std::string_view fname) {
        const auto *sub = t[fname].as_table();
        if (!sub)
            return std::unordered_map<std::string, std::string>{};
        std::unordered_map<std::string, std::string> result;
        for (auto &&[k, v] : *sub) {
            auto sv = v.value<std::string>();
            if (!sv)
                return std::unexpected{std::format("value in '{}' must be a string", fname)};
            result.emplace(std::string{k.str()}, std::move(*sv));
        }
        return result;
    }

    static rfl_type to_rfl(const std::unordered_map<std::string, std::string> &v) {
        return rfl_type{v.begin(), v.end()};
    }
    static std::unordered_map<std::string, std::string> from_rfl(const rfl_type &m) {
        return std::unordered_map<std::string, std::string>{m.begin(), m.end()};
    }
};

} // namespace serde

// ─── NamedTuple builder (needs FieldConverter defined first) ──────────────────

export namespace serde {

template <typename T, typename... Fds>
auto build_named_tuple(const T &obj, std::tuple<Fds...>) {
    return std::apply(
        [&](auto... fds) {
            return (rfl::make_field<decltype(fds)::name>(
                        FieldConverter<typename decltype(fds)::ValueType>::to_rfl(
                            (obj.*decltype(fds)::getter)())) +
                    ...);
        },
        std::tuple<Fds...>{});
}

template <typename T, typename NT, typename... Fds>
void apply_named_tuple_to(T &obj, const NT &nt, std::tuple<Fds...>) {
    std::apply(
        [&](auto... fds) {
            ((obj.*
              decltype(fds)::setter)(FieldConverter<typename decltype(fds)::ValueType>::from_rfl(
                 rfl::get<decltype(fds)::name>(nt))),
             ...);
        },
        std::tuple<Fds...>{});
}

} // namespace serde

// ─── ISerializable FieldConverter specializations ────────────────────────────

export namespace serde {

template <typename VT>
    requires ISerializable<VT>
struct FieldConverter<VT> {
    using rfl_type =
        decltype(build_named_tuple(std::declval<const VT &>(), Serializable<VT>::fields()));

    static simdjson::error_code from_simdjson(simdjson::ondemand::value &v, VT &out) {
        return simdjson::tag_invoke(simdjson::deserialize_tag{}, v, out);
    }

    static std::expected<VT, std::string> from_toml(const toml::table &t, std::string_view fname) {
        const auto *sub = t[fname].as_table();
        if (!sub)
            return std::unexpected{std::format("field '{}' must be a TOML table", fname)};
        VT obj;
        if (auto r = from_toml_impl(*sub, obj); !r)
            return std::unexpected{r.error()};
        return obj;
    }

    static rfl_type to_rfl(const VT &v) { return build_named_tuple(v, Serializable<VT>::fields()); }
    static VT from_rfl(const rfl_type &nt) {
        VT obj;
        apply_named_tuple_to(obj, nt, Serializable<VT>::fields());
        return obj;
    }
};

template <typename VT>
    requires ISerializable<VT>
struct FieldConverter<std::optional<VT>> {
    using InnerRfl = typename FieldConverter<VT>::rfl_type;
    using rfl_type = std::optional<InnerRfl>;

    static simdjson::error_code from_simdjson(simdjson::ondemand::value &v,
                                              std::optional<VT> &out) {
        if (v.is_null()) {
            out = std::nullopt;
            return simdjson::SUCCESS;
        }
        VT obj;
        if (auto ec = FieldConverter<VT>::from_simdjson(v, obj); ec)
            return ec;
        out = std::move(obj);
        return simdjson::SUCCESS;
    }

    static std::expected<std::optional<VT>, std::string> from_toml(const toml::table &t,
                                                                   std::string_view fname) {
        const auto *sub = t[fname].as_table();
        if (!sub)
            return std::nullopt;
        VT obj;
        if (auto r = from_toml_impl(*sub, obj); !r)
            return std::unexpected{r.error()};
        return obj;
    }

    static rfl_type to_rfl(const std::optional<VT> &v) {
        if (!v)
            return std::nullopt;
        return FieldConverter<VT>::to_rfl(*v);
    }
    static std::optional<VT> from_rfl(const rfl_type &nt) {
        if (!nt)
            return std::nullopt;
        return FieldConverter<VT>::from_rfl(*nt);
    }
};

template <typename VT>
    requires ISerializable<VT>
struct FieldConverter<std::vector<VT>> {
    using InnerRfl = typename FieldConverter<VT>::rfl_type;
    using rfl_type = std::vector<InnerRfl>;

    static simdjson::error_code from_simdjson(simdjson::ondemand::value &v, std::vector<VT> &out) {
        simdjson::ondemand::array arr;
        if (auto ec = v.get_array().get(arr); ec)
            return ec;
        for (auto elem : arr) {
            simdjson::ondemand::value elem_val;
            if (auto ec = elem.get(elem_val); ec)
                return ec;
            VT obj;
            if (auto ec = FieldConverter<VT>::from_simdjson(elem_val, obj); ec)
                return ec;
            out.push_back(std::move(obj));
        }
        return simdjson::SUCCESS;
    }

    static std::expected<std::vector<VT>, std::string> from_toml(const toml::table &t,
                                                                 std::string_view fname) {
        const auto *arr = t[fname].as_array();
        if (!arr)
            return std::vector<VT>{};
        std::vector<VT> result;
        result.reserve(arr->size());
        for (const auto &elem : *arr) {
            const auto *sub = elem.as_table();
            if (!sub)
                return std::unexpected{std::format("element of '{}' must be a TOML table", fname)};
            VT obj;
            if (auto r = from_toml_impl(*sub, obj); !r)
                return std::unexpected{r.error()};
            result.push_back(std::move(obj));
        }
        return result;
    }

    static rfl_type to_rfl(const std::vector<VT> &v) {
        rfl_type result;
        result.reserve(v.size());
        for (const auto &e : v)
            result.push_back(FieldConverter<VT>::to_rfl(e));
        return result;
    }
    static std::vector<VT> from_rfl(const rfl_type &nt) {
        std::vector<VT> result;
        result.reserve(nt.size());
        for (const auto &e : nt)
            result.push_back(FieldConverter<VT>::from_rfl(e));
        return result;
    }
};

} // namespace serde

// ─── Per-field helpers ────────────────────────────────────────────────────────

namespace serde {

template <typename Fd>
simdjson::error_code extract_simdjson_field(simdjson::ondemand::object &obj,
                                            typename Fd::ClassType &out, Fd) {
    using VT = typename Fd::ValueType;
    simdjson::ondemand::value field_val;
    if (auto ec = obj.find_field_unordered(Fd::name.string_view()).get(field_val); ec)
        return ec;
    VT value{};
    if (auto ec = FieldConverter<VT>::from_simdjson(field_val, value); ec)
        return ec;
    (out.*Fd::setter)(std::move(value));
    return simdjson::SUCCESS;
}

template <typename Fd>
std::expected<void, std::string> extract_toml_field(const toml::table &t,
                                                    typename Fd::ClassType &out, Fd) {
    using VT = typename Fd::ValueType;
    auto result = FieldConverter<VT>::from_toml(t, Fd::name.string_view());
    if (!result)
        return std::unexpected{result.error()};
    (out.*Fd::setter)(std::move(*result));
    return {};
}

// from_toml_impl definition (declared above, used by FieldConverter<ISerializable>)
template <ISerializable T>
std::expected<void, std::string> from_toml_impl(const toml::table &t, T &obj) {
    std::expected<void, std::string> result{};
    std::apply([&](auto... fds) { ((result && (result = extract_toml_field(t, obj, fds))), ...); },
               Serializable<T>::fields());
    return result;
}

// Helper for rfl backend
template <ISerializable T>
auto rfl_build_from(const T &obj) {
    return build_named_tuple(obj, Serializable<T>::fields());
}

} // namespace serde

// ─── Generic rfl::Reflector ───────────────────────────────────────────────────

namespace rfl {

template <serde::ISerializable T>
struct Reflector<T> {
    using ReflType = decltype(serde::rfl_build_from(std::declval<const T &>()));

    static T to(const ReflType &nt) noexcept {
        T obj;
        serde::apply_named_tuple_to(obj, nt, serde::Serializable<T>::fields());
        return obj;
    }

    static ReflType from(const T &obj) { return serde::rfl_build_from(obj); }
};

} // namespace rfl

// ─── Generic simdjson::tag_invoke ────────────────────────────────────────────

namespace simdjson {

template <typename V, serde::ISerializable T>
auto tag_invoke(deserialize_tag, V &val, T &obj) {
    ondemand::object json_obj;
    if (auto ec = val.get_object().get(json_obj); ec)
        return ec;

    error_code result = SUCCESS;
    std::apply(
        [&](auto... fds) {
            ((result == SUCCESS ? (result = serde::extract_simdjson_field(json_obj, obj, fds))
                                : SUCCESS),
             ...);
        },
        serde::Serializable<T>::fields());
    return result;
}

} // namespace simdjson

// ─── Generic model::from_toml ────────────────────────────────────────────────

namespace model {

export template <serde::ISerializable T>
[[nodiscard]] std::expected<void, std::string> from_toml(const toml::table &t, T &obj) {
    return serde::from_toml_impl(t, obj);
}

} // namespace model
