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
    /**
     * @brief Copies a string literal's characters (including the trailing NUL) into `value`
     * at compile time — this is the whole trick that lets a literal ride along as a
     * template parameter, no cap.
     * @param source the string literal being wrapped, e.g. `"application/json"`.
     */
    consteval StringLiteral(const char (&source)[N]) noexcept {
        std::copy_n(source, N, m_value);
    }
    char m_value[N]{};

    /**
     * @brief Gets a view over the literal, trailing NUL excluded.
     * @return a `string_view` of length `N - 1` over `m_value`.
     */
    [[nodiscard]] constexpr std::string_view string_view() const noexcept {
        return {m_value, N - 1};
    }

    /**
     * @brief Compares two StringLiterals for equality, char by char — different lengths are
     * an instant false, same length falls through to a straight scan.
     * @tparam M the other StringLiteral's length (including its trailing NUL).
     * @param other the StringLiteral being compared against.
     * @return true if both literals hold the same characters.
     */
    template <std::size_t M>
    [[nodiscard]] constexpr bool operator==(const StringLiteral<M> &other) const noexcept {
        // Different lengths can never be equal — bail before touching a single char.
        if constexpr (N != M) {
            return false;
        }
        // Same length, so straight scan char by char, first mismatch is an instant false.
        for (std::size_t index = 0; index < N; ++index) {
            if (m_value[index] != other.m_value[index]) {
                return false;
            }
        }
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
    bool             m_primary_key = false;
    bool             m_unique      = false;
    bool             m_nullable    = true;
    bool             m_skip_insert = false;
    bool             m_skip_update = false;
    const char *m_ref_table  = nullptr;
    const char *m_ref_column = nullptr;

    /// @brief Builds a default-initialized FieldOptionsDb — the fluent-chain starting point.
    /// @return a fresh FieldOptionsDb with every flag at its default (nullable, no PK/unique).
    [[nodiscard]] static constexpr FieldOptionsDb init() { return {}; }

    /**
     * @brief Returns a copy with `m_primary_key` flipped on — chain this on `init()` to mark
     * a column as the table's PK, bet.
     * @return the modified copy; the original is untouched.
     */
    [[nodiscard]] constexpr FieldOptionsDb pk() const {
        auto opt = *this;
        opt.m_primary_key = true;
        return opt;
    }
    /**
     * @brief Returns a copy with `m_nullable` flipped off.
     * @return the modified copy; the original is untouched.
     */
    [[nodiscard]] constexpr FieldOptionsDb not_null() const {
        auto opt = *this;
        opt.m_nullable = false;
        return opt;
    }
    /**
     * @brief Returns a copy with `m_skip_insert` flipped on, so Sql::build_insert_sql /
     * build_insert_many_sql leave this column out of generated INSERTs.
     * @return the modified copy; the original is untouched.
     */
    [[nodiscard]] constexpr FieldOptionsDb no_insert() const {
        auto opt = *this;
        opt.m_skip_insert = true;
        return opt;
    }
    /**
     * @brief Returns a copy with `m_skip_update` flipped on, so Sql::build_update_sql leaves
     * this column out of generated UPDATE SET clauses.
     * @return the modified copy; the original is untouched.
     */
    [[nodiscard]] constexpr FieldOptionsDb no_update() const {
        auto opt = *this;
        opt.m_skip_update = true;
        return opt;
    }
    /**
     * @brief Returns a copy with a foreign-key reference attached, rendered as
     * `REFERENCES table(column)` by Sql::build_create_sql.
     * @param table_name the referenced table's name.
     * @param column_name the referenced column's name.
     * @warning Both pointers are stashed raw, no copy made — pass string literals (as every
     * call site does) or `table_name`/`column_name` must outlive every FieldOptionsDb copied
     * from this one. Pass a temporary `std::string::c_str()` and it's a dangling-pointer L
     * waiting to happen.
     * @return the modified copy; the original is untouched.
     */
    [[nodiscard]] constexpr FieldOptionsDb references(const char *table_name, const char *column_name) const {
        auto opt = *this;
        opt.m_ref_table = table_name;
        opt.m_ref_column = column_name;
        return opt;
    }
};

// ─── FieldOptions ─────────────────────────────────────────────────────────────

class FieldOptions {
  public:
    FieldOptionsDb m_db{};

    /// @brief Builds a default-initialized FieldOptions — the fluent-chain starting point.
    /// @return a fresh FieldOptions with a default-constructed `m_db` block.
    [[nodiscard]] static constexpr FieldOptions init() { return {}; }

    /**
     * @brief Returns a copy with `m_db` swapped in — this is how FieldDesc's `Opts` template
     * param picks up its DB-column metadata (PK, nullability, FK, etc).
     * @param db_options the FieldOptionsDb to attach.
     * @return the modified copy; the original is untouched.
     */
    [[nodiscard]] constexpr FieldOptions with_db(FieldOptionsDb db_options) const {
        auto opt = *this;
        opt.m_db = db_options;
        return opt;
    }
};

// ─── FieldDesc ────────────────────────────────────────────────────────────────

template <rfl::internal::StringLiteral Name, auto Getter, auto Setter,
          FieldOptions Opts = FieldOptions{}>
struct FieldDesc {
    // FIXME(clang-tidy): readability-identifier-naming — name/getter/setter/options have
    // external callers across 20+ files (e.g. include/model/**, include/serde/sql.cppm,
    // include/serde/cache.cppm, sdk/client/**, plugins/engine/handler/**); rename needs a
    // repo-wide sweep.
    static constexpr auto         name    = Name;  // NOLINT(readability-identifier-naming) — shared field name with 20+ external call sites across model/serde/sdk/plugins, rename out of scope
    static constexpr auto         getter  = Getter;  // NOLINT(readability-identifier-naming) — shared field name with 20+ external call sites across model/serde/sdk/plugins, rename out of scope
    static constexpr auto         setter  = Setter;  // NOLINT(readability-identifier-naming) — shared field name with 20+ external call sites across model/serde/sdk/plugins, rename out of scope
    static constexpr FieldOptions options = Opts;  // NOLINT(readability-identifier-naming) — shared field name with 20+ external call sites across model/serde/sdk/plugins, rename out of scope
    using ClassType = MFPTraits<decltype(Getter)>::class_t;
    using ValueType = MFPTraits<decltype(Getter)>::value_t;
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
    requires(const T &value, std::string_view pk_value) {
        { F::pk_string(value)                } -> std::same_as<std::string>;
        { F::cache_key(value)                } -> std::same_as<std::string>;
        { F::template cache_key<T>(pk_value) } -> std::same_as<std::string>;
        { F::cache_value(value)              } -> std::same_as<std::string>;
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
