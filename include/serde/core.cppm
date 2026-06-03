module;
#include <rfl.hpp>

export module serde:core;

import std;

export namespace serde {

// ─── StringLiteral ────────────────────────────────────────────────────────────
// Structural NTTP wrapper so string literals can be template parameters.
// Usage: template <StringLiteral S> — caller writes MyTemplate<"application/json">.

template <std::size_t N>
struct StringLiteral {
    consteval StringLiteral(const char (&str)[N]) noexcept {
        std::copy_n(str, N, value);
    }
    char value[N];

    [[nodiscard]] constexpr std::string_view string_view() const noexcept {
        return {value, N - 1};
    }

    template <std::size_t M>
    [[nodiscard]] constexpr bool operator==(const StringLiteral<M> &other) const noexcept {
        if constexpr (N != M) return false;
        for (std::size_t i = 0; i < N; ++i)
            if (value[i] != other.value[i]) return false;
        return true;
    }
};

// ─── MFPTraits ────────────────────────────────────────────────────────────────

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

// ─── FieldOptionsDb ───────────────────────────────────────────────────────────

class FieldOptionsDb {
  public:
    bool             primary_key = false;
    bool             unique      = false;
    bool             nullable    = true;
    bool             skip_insert = false;
    bool             skip_update = false;
    const char *ref_table  = nullptr;
    const char *ref_column = nullptr;

    static constexpr FieldOptionsDb init() { return {}; }

    constexpr FieldOptionsDb pk() const {
        auto opt = *this;
        opt.primary_key = true;
        return opt;
    }
    constexpr FieldOptionsDb not_null() const {
        auto opt = *this;
        opt.nullable = false;
        return opt;
    }
    constexpr FieldOptionsDb no_insert() const {
        auto opt = *this;
        opt.skip_insert = true;
        return opt;
    }
    constexpr FieldOptionsDb no_update() const {
        auto opt = *this;
        opt.skip_update = true;
        return opt;
    }
    constexpr FieldOptionsDb references(const char *tbl, const char *col) const {
        auto opt = *this;
        opt.ref_table = tbl;
        opt.ref_column = col;
        return opt;
    }
};

// ─── FieldOptions ─────────────────────────────────────────────────────────────

class FieldOptions {
  public:
    FieldOptionsDb db{};

    static constexpr FieldOptions init() { return {}; }

    constexpr FieldOptions with_db(FieldOptionsDb dbo) const {
        auto opt = *this;
        opt.db = dbo;
        return opt;
    }
};

// ─── FieldDesc ────────────────────────────────────────────────────────────────

template <rfl::internal::StringLiteral Name, auto Getter, auto Setter,
          FieldOptions Opts = FieldOptions{}>
struct FieldDesc {
    static constexpr auto         name    = Name;
    static constexpr auto         getter  = Getter;
    static constexpr auto         setter  = Setter;
    static constexpr FieldOptions options = Opts;
    using ClassType = typename MFPTraits<decltype(Getter)>::class_t;
    using ValueType = typename MFPTraits<decltype(Getter)>::value_t;
};

// ─── Serializable<T> + concepts ───────────────────────────────────────────────

template <typename T>
struct Serializable;

template <typename T>
concept ISerializable = requires {
    { Serializable<T>::fields() };
};

template <typename T>
concept IConnectable = ISerializable<T> && requires {
    { Serializable<T>::table_name() } -> std::convertible_to<std::string_view>;
};

// ─── Format concepts ──────────────────────────────────────────────────────────
// IAnyFormat: a format class declares a content_type (usable in template packs).
// IFormat<F,T>: full wire-format contract — encode T → string, decode string → T.

template <typename F>
concept IAnyFormat = requires {
    { F::content_type } -> std::convertible_to<std::string_view>;
};

template <typename F, typename T>
concept IFormat =
    IAnyFormat<F> && ISerializable<T> &&
    requires(const T &value, std::string_view data) {
        { F::encode(value) } -> std::same_as<std::string>;
        { F::decode(data) } -> std::same_as<std::expected<T, std::string>>;
    };

// ─── Cache / SQL concepts ─────────────────────────────────────────────────────

template <typename F, typename T>
concept ICacheHelper =
    IConnectable<T> &&
    requires(const T &value, std::string_view pk) {
        { F::pk_string(value)          } -> std::same_as<std::string>;
        { F::cache_key(value)          } -> std::same_as<std::string>;
        { F::template cache_key<T>(pk) } -> std::same_as<std::string>;
        { F::cache_value(value)        } -> std::same_as<std::string>;
    };

template <typename F, typename T>
concept ISqlBuilder =
    IConnectable<T> &&
    requires(const T &value, std::string_view key,
             std::span<const std::string> keys, std::span<const T> values) {
        { F::template build_create_sql<T>()            } -> std::same_as<std::string>;
        { F::template build_select_sql<T>(key)         } -> std::same_as<std::string>;
        { F::template build_select_many_sql<T>(keys)   } -> std::same_as<std::string>;
        { F::template build_select_all_sql<T>()        } -> std::same_as<std::string>;
        { F::template build_insert_sql<T>(value)       } -> std::same_as<std::string>;
        { F::template build_insert_many_sql<T>(values) } -> std::same_as<std::string>;
        { F::template build_update_sql<T>(value)       } -> std::same_as<std::string>;
        { F::template build_upsert_sql<T>(value)       } -> std::same_as<std::string>;
        { F::template build_delete_sql<T>(key)         } -> std::same_as<std::string>;
        { F::template build_delete_many_sql<T>(keys)   } -> std::same_as<std::string>;
    };

} // namespace serde
