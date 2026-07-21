module;
#define UUID_SYSTEM_GENERATOR
#include <rfl.hpp>
#include <uuid.h>

export module serde:sql;

import :core;
import :json;
import std;

// ─── Internal helpers ─────────────────────────────────────────────────────────

namespace serde {

using TP = std::chrono::system_clock::time_point;

/**
 * @brief Maps a reflected field's C++ value type to its PostgreSQL column type — the
 * whole reason build_create_sql can generate a schema without anyone hand-writing DDL.
 * @tparam VT the field's declared value type.
 * @warning Anything that doesn't match a known type (string, bool, integer widths, floats,
 * enums, uuid) falls through to `"JSONB"` — silent, no compile error, no cap. A field of an
 * unexpected type quietly gets a JSONB column instead of failing loud.
 * @return the PostgreSQL column type as a string literal view (e.g. `"TEXT"`, `"BIGINT"`).
 */
template <typename VT>
constexpr std::string_view sql_type_of() {
    // Walk VT through every known C++-to-Postgres type mapping, most specific matches first...
    // (enum types fold into the same TEXT branch as string, since both map to the same column
    // type — keeping them separate would just be a repeated branch body.)
    if constexpr (std::same_as<VT, std::string> || std::same_as<VT, std::optional<std::string>> ||
                  std::is_enum_v<VT>) {
        return "TEXT";
    } else if constexpr (std::same_as<VT, bool>) {
        return "BOOLEAN";
    } else if constexpr (std::same_as<VT, std::int64_t> || std::same_as<VT, std::uint64_t> ||
                       std::same_as<VT, TP>            || std::same_as<VT, std::optional<TP>>) {
        return "BIGINT";
    } else if constexpr (std::same_as<VT, std::int32_t> || std::same_as<VT, std::uint32_t>) {
        return "INTEGER";
    } else if constexpr (std::same_as<VT, double>) {
        return "DOUBLE PRECISION";
    } else if constexpr (std::same_as<VT, float>) {
        return "REAL";
    } else if constexpr (std::same_as<VT, uuids::uuid> ||
                       std::same_as<VT, std::optional<uuids::uuid>>) {
        return "UUID";
    // ...and anything that matched none of the above quietly falls through to JSONB, no
    // compile error, no cap — an unexpected type just gets a JSONB column instead of failing.
    } else {
        return "JSONB";
    }
}

/**
 * @brief Finds the column name of `T`'s primary-key field by scanning every reflected
 * field for `options.m_db.m_primary_key` — the SQL-side counterpart to Cache::pk_string.
 * @tparam T the connectable type whose PK column is being looked up.
 * @warning If no field is marked `primary_key`, this quietly returns an empty
 * `string_view` instead of erroring — every caller (build_select_sql, build_update_sql,
 * etc.) then splices that empty string straight into generated SQL, producing a query like
 * `WHERE  = 'x'`. Lowkey a footgun for any T that forgot `.pk()` on its FieldOptionsDb.
 * @return the primary key's column name, or empty if no field is marked as one.
 */
template <IConnectable T>
std::string_view pk_column_name() {
    std::string_view result;
    // Fold over every reflected field looking for the one flagged primary_key — same linear
    // scan-via-fold pattern as Cache::pk_string, just returning the column name instead of a
    // stringified value.
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

} // namespace serde

// ─── Query options ────────────────────────────────────────────────────────────

export namespace serde {

class QueryOptions {
  public:
    /**
     * @brief Adds a raw JOIN clause fragment to the query — appended verbatim after the
     * base `FROM table`, so it needs to be a complete `JOIN ... ON ...` string.
     * @param join the raw SQL join fragment to add.
     * @warning No validation or escaping happens here — this gets spliced straight into the
     * generated SQL by Sql::build_inner_query. Only feed it trusted, pre-built fragments,
     * never raw user input, or it's an instant SQL-injection L.
     * @return `*this`, for chaining.
     */
    QueryOptions &add_join(std::string join) noexcept {
        m_joins.push_back(std::move(join));
        return *this;
    }
    /**
     * @brief Adds a raw WHERE condition — multiple conditions get joined with `AND` by
     * Sql::build_inner_query.
     * @param condition the raw SQL boolean condition to add.
     * @warning Same deal as `add_join` — no escaping, spliced in raw. Trusted fragments
     * only, bet.
     * @return `*this`, for chaining.
     */
    QueryOptions &add_where(std::string condition) noexcept {
        m_where_conditions.push_back(std::move(condition));
        return *this;
    }
    /**
     * @brief Adds an ORDER BY clause on `column`, defaulting to ascending — multiple calls
     * stack as comma-separated clauses in the order added.
     * @param column the column name to sort by.
     * @param ascending true for `ASC`, false for `DESC`.
     * @return `*this`, for chaining.
     */
    QueryOptions &add_order_by(std::string column, bool ascending = true) noexcept {
        m_order_by_clauses.emplace_back(std::move(column), ascending);
        return *this;
    }
    /**
     * @brief Sets a `LIMIT` on the query — only the last call before build wins, this
     * isn't cumulative like the adders.
     * @param limit the max row count to cap the query at.
     * @return `*this`, for chaining.
     */
    QueryOptions &set_limit(std::size_t limit) noexcept {
        m_limit = limit;
        return *this;
    }

    /// @brief Gets the accumulated JOIN fragments, in add order.
    /// @return the JOIN clauses added so far.
    [[nodiscard]] const std::vector<std::string> &get_joins() const noexcept { return m_joins; }
    /// @brief Gets the accumulated WHERE conditions, in add order.
    /// @return the WHERE conditions added so far — joined with `AND` at build time.
    [[nodiscard]] const std::vector<std::string> &get_where_conditions() const noexcept {
        return m_where_conditions;
    }
    /// @brief Gets the accumulated ORDER BY clauses, in add order.
    /// @return each clause as a `(column, ascending)` pair.
    [[nodiscard]] const std::vector<std::pair<std::string, bool>> &
    get_order_by_clauses() const noexcept {
        return m_order_by_clauses;
    }
    /// @brief Gets the configured row limit, if any.
    /// @return the limit set via `set_limit`, or `nullopt` if never set.
    [[nodiscard]] const std::optional<std::size_t> &get_limit() const noexcept { return m_limit; }

  private:
    std::vector<std::string> m_joins;
    std::vector<std::string> m_where_conditions;
    std::vector<std::pair<std::string, bool>> m_order_by_clauses;
    std::optional<std::size_t> m_limit;
};

} // namespace serde

// ─── SQL builders ─────────────────────────────────────────────────────────────

export namespace serde {

class Sql {
  public:
    /**
     * @brief Generates a `CREATE TABLE IF NOT EXISTS` statement for T from its reflected
     * fields — column types come from sql_type_of, constraints (PK/NOT NULL/UNIQUE/FK) come
     * straight off each field's FieldOptionsDb. This is the whole codegen-your-schema
     * motion in one call.
     * @tparam T the connectable type to generate DDL for.
     * @return the generated `CREATE TABLE` SQL string.
     */
    template <IConnectable T>
    [[nodiscard]] static std::string build_create_sql() {
        std::string columns;
        std::size_t index = 0;
        // Fold over every reflected field, building up one column definition per field.
        std::apply(
            [&](auto... fields) {
                ([&] {
                    using VT = decltype(fields)::ValueType;
                    // Comma-separate every column after the first.
                    if (index++ > 0) {
                        columns += ", ";
                    }
                    columns += fields.name.string_view();
                    columns += ' ';
                    columns += sql_type_of<VT>();
                    // PK gets its own constraint; anything else non-nullable gets NOT NULL.
                    if (fields.options.m_db.m_primary_key) {
                        columns += " PRIMARY KEY";
                    } else if (!fields.options.m_db.m_nullable) {
                        columns += " NOT NULL";
                    }
                    // UNIQUE stacks on top, unless it's already the (implicitly unique) PK.
                    if (fields.options.m_db.m_unique && !fields.options.m_db.m_primary_key) {
                        columns += " UNIQUE";
                    }
                    // Tack on a REFERENCES clause if this field declared a foreign key.
                    if (fields.options.m_db.m_ref_table != nullptr) {
                        columns += std::format(" REFERENCES {}({})", fields.options.m_db.m_ref_table,
                                               fields.options.m_db.m_ref_column);
                    }
                }(),
                 ...);
            },
            Serializable<T>::fields());
        // Stitch the assembled column list into the full CREATE TABLE statement.
        return std::format("CREATE TABLE IF NOT EXISTS {} ({})", Serializable<T>::table_name(), columns);
    }

    /**
     * @brief Generates a `SELECT row_to_json(...)` statement fetching one row by primary
     * key, wrapped so the DB hands back JSON directly instead of a raw row.
     * @tparam T the connectable type being queried.
     * @param key the primary-key value to match, e.g. from Cache::pk_string.
     * @warning `key` is interpolated straight into the query between single quotes with no
     * escaping or parameterization — a key value containing a `'` breaks the query, and a
     * crafted one is a straight SQL-injection L. Only feed this trusted, already-validated
     * key material (e.g. a UUID or an id you generated), never raw external input.
     * @return the generated `SELECT` SQL string.
     */
    template <IConnectable T>
    [[nodiscard]] static std::string build_select_sql(std::string_view key) {
        return std::format(
            "SELECT row_to_json(row) FROM (SELECT * FROM {} WHERE {} = '{}') row",
            Serializable<T>::table_name(), pk_column_name<T>(), key);
    }

    /**
     * @brief Generates a `SELECT json_agg(...)` statement fetching every row whose primary
     * key is in `keys`.
     * @tparam T the connectable type being queried.
     * @param keys the primary-key values to match against.
     * @warning Same raw-interpolation footgun as `build_select_sql`, multiplied across every
     * element of `keys` — no escaping on any of them.
     * @return the generated `SELECT ... IN (...)` SQL string.
     */
    template <IConnectable T>
    [[nodiscard]] static std::string build_select_many_sql(std::span<const std::string> keys) {
        std::string list;
        // Comma-join every key, quoted, unescaped — same footgun as build_select_sql, just
        // repeated once per key.
        for (std::size_t index = 0; index < keys.size(); ++index) {
            if (index > 0) {
                list += ',';
            }
            list += std::format("'{}'", keys[index]);  // FIXME(clang-tidy): unchecked operator[], consider .at()
        }
        return std::format(
            "SELECT json_agg(row_to_json(row)) FROM (SELECT * FROM {} WHERE {} IN ({})) row",
            Serializable<T>::table_name(), pk_column_name<T>(), list);
    }

    /**
     * @brief Generates a `SELECT json_agg(...)` statement fetching every row in T's table,
     * no filtering.
     * @tparam T the connectable type being queried.
     * @return the generated `SELECT` SQL string.
     */
    template <IConnectable T>
    [[nodiscard]] static std::string build_select_all_sql() {
        return std::format("SELECT json_agg(row_to_json(row)) FROM {} row",
                           Serializable<T>::table_name());
    }

    /**
     * @brief Generates an `INSERT` statement for a single row, riding on Postgres's
     * `json_populate_record` to map JSON fields onto columns — no manual column list needed.
     * @tparam T the connectable type being inserted.
     * @param value the instance to insert.
     * @note `value` is JSON-encoded via Json::encode first, so this inherits whatever
     * type-coercion behavior that encoder has (e.g. how it renders optional/null fields).
     * @return the generated `INSERT` SQL string.
     */
    template <IConnectable T>
    [[nodiscard]] static std::string build_insert_sql(const T &value) {
        return std::format("INSERT INTO {} SELECT * FROM json_populate_record(NULL::{}, '{}')",
                           Serializable<T>::table_name(), Serializable<T>::table_name(),
                           Json::encode(value));
    }

    /**
     * @brief Generates a bulk `INSERT` statement for multiple rows via
     * `json_populate_recordset`, encoding all of `values` as one JSON array literal.
     * @tparam T the connectable type being inserted.
     * @param values the instances to insert, in order.
     * @return the generated bulk `INSERT` SQL string.
     */
    template <IConnectable T>
    [[nodiscard]] static std::string build_insert_many_sql(std::span<const T> values) {
        std::string rows;
        // Comma-join each value's JSON encoding into one big array literal for
        // json_populate_recordset to fan back out.
        for (std::size_t index = 0; index < values.size(); ++index) {
            if (index > 0) {
                rows += ',';
            }
            rows += std::format("'{}'", Json::encode(values[index]));
        }
        return std::format("INSERT INTO {} SELECT * FROM json_populate_recordset(NULL::{}, '[{}]')",
                           Serializable<T>::table_name(), Serializable<T>::table_name(), rows);
    }

    /**
     * @brief Generates an `UPDATE` statement matched by primary key, setting every column
     * except the PK and any marked `skip_update` in its FieldOptionsDb.
     * @tparam T the connectable type being updated.
     * @param value the instance whose fields (and PK, for the WHERE match) drive the update.
     * @note If every non-PK field is marked `skip_update`, `set_list` comes out empty and
     * the generated SQL has a bare `SET FROM ...` with nothing to set — that's on the schema
     * design, not this function, but worth knowing before you hit it.
     * @return the generated `UPDATE` SQL string.
     */
    template <IConnectable T>
    [[nodiscard]] static std::string build_update_sql(const T &value) {
        std::string set_list;
        std::size_t index = 0;
        // Fold over every field, skipping the PK (matched in WHERE, not SET) and anything
        // flagged skip_update, comma-separating whatever's left.
        std::apply(
            [&](auto... fields) {
                ([&] {
                    if (fields.options.m_db.m_primary_key || fields.options.m_db.m_skip_update) {
                        return;
                    }
                    if (index++ > 0) {
                        set_list += ", ";
                    }
                    auto column = fields.name.string_view();
                    set_list += std::format("{} = src.{}", column, column);
                }(),
                 ...);
            },
            Serializable<T>::fields());
        return std::format(
            "UPDATE {} AS target SET {} FROM json_populate_record(NULL::{}, '{}') AS src "
            "WHERE target.{} = src.{}",
            Serializable<T>::table_name(), set_list, Serializable<T>::table_name(),
            Json::encode(value), pk_column_name<T>(), pk_column_name<T>());
    }

    /**
     * @brief Generates an `INSERT ... ON CONFLICT (pk) DO UPDATE` statement — every non-PK
     * column gets an `EXCLUDED.column` update clause, classic upsert motion.
     * @tparam T the connectable type being upserted.
     * @param value the instance to insert or update.
     * @return the generated `INSERT ... ON CONFLICT` SQL string.
     */
    template <IConnectable T>
    [[nodiscard]] static std::string build_upsert_sql(const T &value) {
        std::string update_list;
        std::size_t index = 0;
        // Fold over every non-PK field, building the ON CONFLICT DO UPDATE SET list — the PK
        // itself is the conflict target, not something to overwrite.
        std::apply(
            [&](auto... fields) {
                ([&] {
                    if (fields.options.m_db.m_primary_key) {
                        return;
                    }
                    if (index++ > 0) {
                        update_list += ", ";
                    }
                    auto column = fields.name.string_view();
                    update_list += std::format("{} = EXCLUDED.{}", column, column);
                }(),
                 ...);
            },
            Serializable<T>::fields());
        return std::format(
            "INSERT INTO {} SELECT * FROM json_populate_record(NULL::{}, '{}') "
            "ON CONFLICT ({}) DO UPDATE SET {}",
            Serializable<T>::table_name(), Serializable<T>::table_name(), Json::encode(value),
            pk_column_name<T>(), update_list);
    }

    /**
     * @brief Generates a `DELETE` statement matched by primary key.
     * @tparam T the connectable type being deleted from.
     * @param key the primary-key value to match.
     * @warning Same raw-interpolation footgun as `build_select_sql` — `key` goes in
     * unescaped. Only pass trusted key material; this is a delete, so the blast radius on a
     * bad key is worse than a read.
     * @return the generated `DELETE` SQL string.
     */
    template <IConnectable T>
    [[nodiscard]] static std::string build_delete_sql(std::string_view key) {
        return std::format("DELETE FROM {} WHERE {} = '{}'", Serializable<T>::table_name(),
                           pk_column_name<T>(), key);
    }

    /**
     * @brief Generates a `DELETE` statement matching any row whose primary key is in `keys`.
     * @tparam T the connectable type being deleted from.
     * @param keys the primary-key values to match against.
     * @warning Same raw-interpolation footgun, multiplied across `keys` — no escaping on
     * any element, and this deletes every match, no cap.
     * @return the generated `DELETE ... IN (...)` SQL string.
     */
    template <IConnectable T>
    [[nodiscard]] static std::string build_delete_many_sql(std::span<const std::string> keys) {
        std::string list;
        // Same comma-join-unescaped motion as build_select_many_sql — every match gets deleted.
        for (std::size_t index = 0; index < keys.size(); ++index) {
            if (index > 0) {
                list += ',';
            }
            list += std::format("'{}'", keys[index]);  // FIXME(clang-tidy): unchecked operator[], consider .at()
        }
        return std::format("DELETE FROM {} WHERE {} IN ({})", Serializable<T>::table_name(),
                           pk_column_name<T>(), list);
    }

    /**
     * @brief Generates a `SELECT json_agg(...)` statement over an arbitrary query built from
     * `options` (joins, where, order by, limit) — the general-purpose query path, versus the
     * fixed-shape `build_select_*` methods above.
     * @tparam T the connectable type being queried.
     * @param options the joins/conditions/ordering/limit to compose the query from.
     * @return the generated `SELECT` SQL string, rows aggregated as a JSON array.
     */
    template <IConnectable T>
    [[nodiscard]] static std::string build_query_sql(const QueryOptions &options) {
        return std::format("SELECT json_agg(row_to_json(row)) FROM ({}) row",
                           build_inner_query<T>(options));
    }

    /**
     * @brief Same as `build_query_sql`, but wraps the inner query with `LIMIT 1` and returns
     * a single `row_to_json` instead of an aggregated array — for callers that only want the
     * first match.
     * @tparam T the connectable type being queried.
     * @param options the joins/conditions/ordering to compose the query from — any
     * `options.get_limit()` value is ignored, this always forces `LIMIT 1`.
     * @return the generated `SELECT ... LIMIT 1` SQL string.
     */
    template <IConnectable T>
    [[nodiscard]] static std::string build_query_first_sql(const QueryOptions &options) {
        return std::format("SELECT row_to_json(row) FROM ({} LIMIT 1) row",
                           build_inner_query<T>(options));
    }

  private:
    /**
     * @brief Composes the shared `SELECT ... FROM ... [JOIN...] [WHERE...] [ORDER BY...]
     * [LIMIT...]` core used by both `build_query_sql` and `build_query_first_sql`.
     * @tparam T the connectable type being queried.
     * @param options the joins/conditions/ordering/limit to compose the query from.
     * @warning Every join and where-condition fragment in `options` gets appended verbatim —
     * this function trusts them completely, it's on the QueryOptions adders (and their
     * callers) to keep that safe.
     * @return the inner query string, without any outer JSON-aggregation wrapper.
     */
    template <IConnectable T>
    [[nodiscard]] static std::string build_inner_query(const QueryOptions &options) {
        auto table = Serializable<T>::table_name();
        std::string inner = std::format("SELECT {}.* FROM {}", table, table);
        // Every JOIN fragment gets appended verbatim, in add order.
        for (const auto &join : options.get_joins()) {
            inner += " " + join;
        }
        // First WHERE condition gets the keyword; every one after that is AND-chained.
        bool first_where = true;
        for (const auto &condition : options.get_where_conditions()) {
            inner += first_where ? " WHERE " : " AND ";
            inner += condition;
            first_where = false;
        }
        // Same first-vs-rest pattern for ORDER BY, with each clause carrying its own
        // direction.
        bool first_order = true;
        for (const auto &[column, ascending] : options.get_order_by_clauses()) {
            inner += first_order ? " ORDER BY " : ", ";
            inner += column;
            inner += ascending ? " ASC" : " DESC";
            first_order = false;
        }
        // LIMIT is optional and tacked on last, if set at all.
        if (options.get_limit()) {
            inner += std::format(" LIMIT {}", *options.get_limit());
        }
        return inner;
    }
};

} // namespace serde
