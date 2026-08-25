module;
#define UUID_SYSTEM_GENERATOR
#include <rfl.hpp>
#include <uuid.h>

export module serde:core;

import std;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

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

// ─── Generic value-kind classification / field-reflection helpers ────────────
//
// Deliberately not SQL-specific: value_kind_of<VT>()/pk_column_name<T>() just classify a
// reflected field's C++ type into a generic tag and find a type's PK field name — any
// consumer needing that (SQL codegen, or anything else) reaches for these, not a
// dialect-flavored string. Mapping a ValueKind to an actual column-type string (e.g.
// Postgres's "TEXT"/"BIGINT"/"JSONB") is a dialect's own opinion, not serde's.

enum class ValueKind : std::uint8_t {
    STRING, BOOLEAN, INT64, UINT64, INT32, UINT32, DOUBLE, FLOAT, TIMESTAMP, UUID, OTHER
};

/**
 * @brief Classifies a reflected field's C++ value type into a generic kind tag.
 * @tparam VT the field's declared value type.
 * @warning Anything that doesn't match a known kind falls through to `ValueKind::OTHER` — no
 * compile error, no cap. A dialect plugin decides what that fallback actually means (e.g.
 * Postgres treats it as `JSONB`).
 * @return the value's generic kind.
 */
template <typename VT>
constexpr ValueKind value_kind_of() {
    if constexpr (std::same_as<VT, std::string> || std::same_as<VT, std::optional<std::string>> ||
                  std::is_enum_v<VT>) {
        return ValueKind::STRING;
    } else if constexpr (std::same_as<VT, bool>) {
        return ValueKind::BOOLEAN;
    } else if constexpr (std::same_as<VT, std::chrono::system_clock::time_point> ||
                       std::same_as<VT, std::optional<std::chrono::system_clock::time_point>>) {
        return ValueKind::TIMESTAMP;
    } else if constexpr (std::same_as<VT, std::int64_t>) {
        return ValueKind::INT64;
    } else if constexpr (std::same_as<VT, std::uint64_t>) {
        return ValueKind::UINT64;
    } else if constexpr (std::same_as<VT, std::int32_t>) {
        return ValueKind::INT32;
    } else if constexpr (std::same_as<VT, std::uint32_t>) {
        return ValueKind::UINT32;
    } else if constexpr (std::same_as<VT, double>) {
        return ValueKind::DOUBLE;
    } else if constexpr (std::same_as<VT, float>) {
        return ValueKind::FLOAT;
    } else if constexpr (std::same_as<VT, uuids::uuid> || std::same_as<VT, std::optional<uuids::uuid>>) {
        return ValueKind::UUID;
    } else {
        return ValueKind::OTHER;
    }
}

/**
 * @brief Finds the column name of `T`'s primary-key field by scanning every reflected field for
 * `options.m_db.m_primary_key`.
 * @tparam T the connectable type whose PK field is being looked up.
 * @warning If no field is marked `primary_key`, this quietly returns an empty `string_view`
 * instead of erroring — every caller then splices that empty string into whatever it's building,
 * a footgun for any `T` that forgot `.pk()` on its `FieldOptionsDb`.
 * @return the primary key's field name, or empty if no field is marked as one.
 */
template <IConnectable T>
std::string_view pk_column_name() {
    std::string_view result;
    // Fold over every reflected field looking for the one flagged primary_key — linear
    // scan wearing a fold-expression trenchcoat.
    std::apply(
        [&](auto... fields) {
            ((fields.options.m_db.m_primary_key
                  ? (result = fields.name.string_view())
                  : std::string_view{}),
             ...);
        },
        Serializable<T>::fields());
    return result;
}

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

#ifdef CONGELADO_TEST
namespace serde::tests {

// Minimal IConnectable fixture for pk_column_name<T>() — one field, flagged as the PK.
class CoreTestRecord {
  public:
    CoreTestRecord() = default;

    void set_id(std::string id) { m_id = std::move(id); }
    [[nodiscard]] const std::string &get_id() const noexcept { return m_id; }

  private:
    std::string m_id;
};

} // namespace serde::tests

template <>
struct serde::Serializable<serde::tests::CoreTestRecord> {
    static constexpr auto fields() {
        return std::tuple{
            serde::FieldDesc<"id", &serde::tests::CoreTestRecord::get_id,
                             &serde::tests::CoreTestRecord::set_id,
                             serde::FieldOptions::init().with_db(serde::FieldOptionsDb::init().pk())>{},
        };
    }
    static constexpr std::string_view table_name() { return "core_test_records"; }
};

namespace serde::tests {
using namespace boost::ut;

suite<"StringLiteral"> string_literal_suite = [] {
    "string_view excludes the trailing NUL"_test = [] {
        constexpr StringLiteral literal{"hello"};

        expect(literal.string_view() == "hello");
        expect(literal.string_view().size() == 5);
    };

    "equal literals of the same length compare equal"_test = [] {
        constexpr StringLiteral first{"abc"};
        constexpr StringLiteral second{"abc"};

        expect(first == second);
    };

    "same-length literals with different content compare unequal"_test = [] {
        constexpr StringLiteral first{"abc"};
        constexpr StringLiteral second{"abd"};

        expect(not (first == second));
    };

    "different-length literals compare unequal"_test = [] {
        constexpr StringLiteral first{"abc"};
        constexpr StringLiteral second{"abcd"};

        expect(not (first == second));
    };
};

suite<"FieldOptionsDb"> field_options_db_suite = [] {
    "init defaults to nullable, no PK, no unique, no skip flags"_test = [] {
        constexpr auto options = FieldOptionsDb::init();

        expect(not options.m_primary_key);
        expect(not options.m_unique);
        expect(options.m_nullable);
        expect(not options.m_skip_insert);
        expect(not options.m_skip_update);
    };

    "pk/not_null/no_insert/no_update chain without mutating the original"_test = [] {
        constexpr auto base = FieldOptionsDb::init();
        constexpr auto derived = base.pk().not_null().no_insert().no_update();

        expect(derived.m_primary_key);
        expect(not derived.m_nullable);
        expect(derived.m_skip_insert);
        expect(derived.m_skip_update);
        // Original stays untouched — every mutator on FieldOptionsDb returns a modified copy.
        expect(not base.m_primary_key);
        expect(base.m_nullable);
    };

    "references stores the referenced table/column pointers"_test = [] {
        constexpr auto derived = FieldOptionsDb::init().references("users", "id");

        expect(std::string_view{derived.m_ref_table} == "users");
        expect(std::string_view{derived.m_ref_column} == "id");
    };
};

suite<"FieldOptions"> field_options_suite = [] {
    "with_db attaches the db options block without mutating the original"_test = [] {
        constexpr auto db_options = FieldOptionsDb::init().pk();
        constexpr auto options = FieldOptions::init().with_db(db_options);

        expect(options.m_db.m_primary_key);
    };
};

suite<"value_kind_of"> value_kind_of_suite = [] {
    "classifies every known C++ type into its ValueKind tag"_test = [] {
        expect(value_kind_of<std::string>() == ValueKind::STRING);
        expect(value_kind_of<bool>() == ValueKind::BOOLEAN);
        expect(value_kind_of<std::int64_t>() == ValueKind::INT64);
        expect(value_kind_of<std::uint64_t>() == ValueKind::UINT64);
        expect(value_kind_of<std::int32_t>() == ValueKind::INT32);
        expect(value_kind_of<std::uint32_t>() == ValueKind::UINT32);
        expect(value_kind_of<double>() == ValueKind::DOUBLE);
        expect(value_kind_of<float>() == ValueKind::FLOAT);
        expect(value_kind_of<uuids::uuid>() == ValueKind::UUID);
        expect(value_kind_of<std::chrono::system_clock::time_point>() == ValueKind::TIMESTAMP);
    };

    "an unrecognized type falls back to OTHER"_test = [] {
        expect(value_kind_of<std::vector<int>>() == ValueKind::OTHER);
    };
};

suite<"pk_column_name"> pk_column_name_suite = [] {
    "finds the field flagged primary_key in FieldOptionsDb"_test = [] {
        expect(pk_column_name<CoreTestRecord>() == "id");
    };
};

} // namespace serde::tests
#endif
