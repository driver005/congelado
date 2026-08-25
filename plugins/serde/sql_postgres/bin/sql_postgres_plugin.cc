module;

#define CONGELADO_GUEST
#include <congelado/plugin.h>
#include <rfl/Generic.hpp>
#include <rfl/from_generic.hpp>

export module sql_postgres_plugin;

import congelado_plugin;
import interfaces;
import core_events;
import core_logger;
import std;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

// The Postgres SQL DDL/query-text dialect as a genuine plugin, registered under the *existing*
// interfaces::ISerdeFormat/CONGELADO_CAP_SERDE — no new interface or capability bit needed.
// include/connector/connector.cppm's class Sql reflects a host-defined T's fields at compile
// time (can never cross a dlopen boundary — a plugin .so is built before any host T exists),
// reduces that into a plain SqlRequest, and calls serde::Ser::serialize("application/sql+
// postgres", request) — landing here as an rfl::Generic. This plugin owns its own local copy of
// the SqlColumnDesc/SqlQueryOptions/SqlRequest shape (matching field names only — no shared
// header, same principle as this session's Document duplication for the client codegen tool)
// and turns the already-reduced runtime data into actual Postgres DDL/query text.

namespace {

enum class ValueKind : std::uint8_t {
    STRING, BOOLEAN, INT64, UINT64, INT32, UINT32, DOUBLE, FLOAT, TIMESTAMP, UUID, OTHER
};

struct SqlColumnDesc {
    std::string name;
    ValueKind kind{ValueKind::OTHER};
    bool primary_key = false;
    bool nullable = true;
    bool unique = false;
    bool skip_update = false;
    std::string ref_table;
    std::string ref_column;
};

struct SqlQueryOptions {
    std::vector<std::string> joins;
    std::vector<std::string> where_conditions;
    std::vector<std::pair<std::string, bool>> order_by_clauses;
    std::optional<std::size_t> limit;
};

struct SqlRequest {
    std::string op;
    std::string table_name;
    std::vector<SqlColumnDesc> columns;
    std::string pk_column;
    std::string key;
    std::vector<std::string> keys;
    std::string json_payload;
    std::string json_array_payload;
    std::vector<std::string> set_columns;
    std::vector<std::string> update_columns;
    SqlQueryOptions options;
};

/**
 * @brief Maps a generic value-kind tag to its Postgres column type — the one piece of this
 * whole plugin that's genuinely Postgres's own opinion (a different dialect plugin would map
 * these differently, or not need every kind at all).
 * @param kind the generic kind to map.
 * @return the Postgres column type as a string view (e.g. `"TEXT"`, `"BIGINT"`).
 */
[[nodiscard]] std::string_view postgres_type_of(ValueKind kind) noexcept {
    switch (kind) {
    case ValueKind::STRING: return "TEXT";
    case ValueKind::BOOLEAN: return "BOOLEAN";
    case ValueKind::INT64: return "BIGINT";
    case ValueKind::UINT64: return "BIGINT";
    case ValueKind::TIMESTAMP: return "BIGINT";
    case ValueKind::INT32: return "INTEGER";
    case ValueKind::UINT32: return "INTEGER";
    case ValueKind::DOUBLE: return "DOUBLE PRECISION";
    case ValueKind::FLOAT: return "REAL";
    case ValueKind::UUID: return "UUID";
    case ValueKind::OTHER: return "JSONB";
    }
    return "JSONB";
}

/**
 * @brief Comma-quotes every entry in `values` — the shared `WHERE col IN (...)` fragment
 * builder used by both select-many and delete-many.
 * @note Embedded single quotes are escaped by doubling them, standard SQL string-literal
 * escaping — safe against a value that itself contains a `'`. Still only feed trusted,
 * already-validated key material (same trust boundary the old connector-side code documented) —
 * this is quote-escaping, not general SQL sanitization.
 * @param values the values to quote-and-join.
 * @return the comma-joined, quoted list, ready to splice into an `IN (...)` clause.
 */
[[nodiscard]] std::string quoted_list(const std::vector<std::string> &values) {
    std::string list;
    for (std::size_t index = 0; index < values.size(); ++index) {
        if (index > 0) {
            list += ',';
        }
        // A value containing an embedded `'` used to break out of the intended string literal
        // (e.g. "O'Brien" produced `'O'Brien'`, a bare `Brien'` left dangling as SQL) — a real
        // SQL-injection-shaped gap since this feeds select_many/delete_many's `WHERE col IN
        // (...)` clauses, found while writing this file's tests. Standard SQL string-literal
        // escaping is doubling embedded single quotes.
        std::string escaped;
        escaped.reserve(values[index].size());
        for (char character : values[index]) {
            if (character == '\'') {
                escaped += '\'';
            }
            escaped += character;
        }
        list += std::format("'{}'", escaped);
    }
    return list;
}

/**
 * @brief Composes the shared `SELECT ... FROM ... [JOIN...] [WHERE...] [ORDER BY...]
 * [LIMIT...]` core used by both the "query" and "query_first" operations.
 * @param table_name the table being queried.
 * @param options the joins/conditions/ordering/limit to compose the query from.
 * @return the inner query string, without any outer JSON-aggregation wrapper.
 */
[[nodiscard]] std::string build_inner_query(std::string_view table_name, const SqlQueryOptions &options) {
    std::string inner = std::format("SELECT {}.* FROM {}", table_name, table_name);
    for (const auto &join : options.joins) {
        inner += " " + join;
    }
    bool first_where = true;
    for (const auto &condition : options.where_conditions) {
        inner += first_where ? " WHERE " : " AND ";
        inner += condition;
        first_where = false;
    }
    bool first_order = true;
    for (const auto &[column, ascending] : options.order_by_clauses) {
        inner += first_order ? " ORDER BY " : ", ";
        inner += column;
        inner += ascending ? " ASC" : " DESC";
        first_order = false;
    }
    if (options.limit) {
        inner += std::format(" LIMIT {}", *options.limit);
    }
    return inner;
}

class SqlPostgresPlugin : public congelado::Plugin, public interfaces::ISerdeFormat {
  public:
    /// @brief Plugin name reported to the host. @return `"sql_postgres"`.
    [[nodiscard]] std::string_view get_name() const noexcept override { return "sql_postgres"; }
    /// @brief Version string for this build. @return `"0.1.0"`.
    [[nodiscard]] std::string_view get_version() const noexcept override { return "0.1.0"; }
    /**
     * @brief Flags this as a serde-format-capable plugin, so the host wires `serde_get` into
     * the `_cap_dispatch` routing — the exact same capability JSON/TOML register under.
     * @return `CONGELADO_CAP_SERDE`.
     */
    [[nodiscard]] std::uint32_t capabilities() const noexcept override {
        return CONGELADO_CAP_SERDE;
    }

    /**
     * @brief Capability hook the host calls to get at this plugin's `ISerdeFormat` surface.
     * @return this instance, upcast to `interfaces::ISerdeFormat*`.
     */
    void *serde_get() noexcept { return static_cast<interfaces::ISerdeFormat *>(this); }

    /// @brief The content-type this "format" registers under. @return `"application/sql+postgres"`.
    [[nodiscard]] std::string_view content_type() const noexcept override {
        return "application/sql+postgres";
    }
    /// @brief Short human-readable format name. @return `"sql_postgres"`.
    [[nodiscard]] std::string_view format_name() const noexcept override { return "sql_postgres"; }

    /**
     * @brief Generates Postgres SQL text for whatever operation `value` (an `rfl::Generic`
     * reduced from `connector::SqlRequest`) describes.
     * @param value the request describing which SQL operation to generate text for.
     * @return the generated SQL text, or an error message if `value` doesn't match the expected
     * request shape or names an unrecognized `op`.
     */
    [[nodiscard]] std::expected<std::string, std::string>
    encode(const rfl::Generic &value) const override {
        auto request = rfl::from_generic<SqlRequest>(value);
        if (!request) {
            core::logger::warning("sql_postgres", "encode failed: {}", request.error().what());
            core::events::publish("serde.sql_postgres.encode_failed", {{"error", request.error().what()}});
            return std::unexpected{std::string{request.error().what()}};
        }

        std::string sql;
        if (request->op == "create_table") { sql = build_create_sql(*request); }
        else if (request->op == "select") { sql = build_select_sql(*request); }
        else if (request->op == "select_many") { sql = build_select_many_sql(*request); }
        else if (request->op == "select_all") { sql = build_select_all_sql(*request); }
        else if (request->op == "insert") { sql = build_insert_sql(*request); }
        else if (request->op == "insert_many") { sql = build_insert_many_sql(*request); }
        else if (request->op == "update") { sql = build_update_sql(*request); }
        else if (request->op == "upsert") { sql = build_upsert_sql(*request); }
        else if (request->op == "delete") { sql = build_delete_sql(*request); }
        else if (request->op == "delete_many") { sql = build_delete_many_sql(*request); }
        else if (request->op == "query") { sql = build_query_sql(*request); }
        else if (request->op == "query_first") { sql = build_query_first_sql(*request); }
        else {
            core::logger::warning("sql_postgres", "unrecognized op '{}'", request->op);
            core::events::publish("serde.sql_postgres.unrecognized_op", {{"op", request->op}});
            return std::unexpected{std::format("sql_postgres: unrecognized op '{}'", request->op)};
        }

        // Genuinely useful at debug level, unlike json/toml's encode() — this *is* the SQL text
        // the engine is about to run through postgres_plugin, not just an intermediate encoding.
        core::logger::debug("sql_postgres", "generated '{}' SQL: {}", request->op, sql);
        return sql;
    }

    /**
     * @brief Not supported — this format is write-only, there's nothing to decode SQL text
     * back into.
     * @return always an error.
     */
    [[nodiscard]] std::expected<rfl::Generic, std::string>
    decode(std::string_view /*data*/) const override {
        core::logger::warning("sql_postgres", "decode() called — this format is write-only");
        return std::unexpected{"sql_postgres format is write-only, decode is not supported"};
    }

  private:
    [[nodiscard]] static std::string build_create_sql(const SqlRequest &request) {
        std::string columns;
        for (std::size_t index = 0; index < request.columns.size(); ++index) {
            const auto &column = request.columns[index];
            if (index > 0) {
                columns += ", ";
            }
            columns += column.name;
            columns += ' ';
            columns += postgres_type_of(column.kind);
            if (column.primary_key) {
                columns += " PRIMARY KEY";
            } else if (!column.nullable) {
                columns += " NOT NULL";
            }
            if (column.unique && !column.primary_key) {
                columns += " UNIQUE";
            }
            if (!column.ref_table.empty()) {
                columns += std::format(" REFERENCES {}({})", column.ref_table, column.ref_column);
            }
        }
        return std::format("CREATE TABLE IF NOT EXISTS {} ({})", request.table_name, columns);
    }

    [[nodiscard]] static std::string build_select_sql(const SqlRequest &request) {
        return std::format("SELECT row_to_json(row) FROM (SELECT * FROM {} WHERE {} = '{}') row",
                           request.table_name, request.pk_column, request.key);
    }

    [[nodiscard]] static std::string build_select_many_sql(const SqlRequest &request) {
        return std::format(
            "SELECT json_agg(row_to_json(row)) FROM (SELECT * FROM {} WHERE {} IN ({})) row",
            request.table_name, request.pk_column, quoted_list(request.keys));
    }

    [[nodiscard]] static std::string build_select_all_sql(const SqlRequest &request) {
        return std::format("SELECT json_agg(row_to_json(row)) FROM {} row", request.table_name);
    }

    [[nodiscard]] static std::string build_insert_sql(const SqlRequest &request) {
        return std::format("INSERT INTO {} SELECT * FROM json_populate_record(NULL::{}, '{}')",
                           request.table_name, request.table_name, request.json_payload);
    }

    [[nodiscard]] static std::string build_insert_many_sql(const SqlRequest &request) {
        return std::format("INSERT INTO {} SELECT * FROM json_populate_recordset(NULL::{}, '{}')",
                           request.table_name, request.table_name, request.json_array_payload);
    }

    [[nodiscard]] static std::string build_update_sql(const SqlRequest &request) {
        std::string set_list;
        for (std::size_t index = 0; index < request.set_columns.size(); ++index) {
            if (index > 0) {
                set_list += ", ";
            }
            const auto &column = request.set_columns[index];
            set_list += std::format("{} = src.{}", column, column);
        }
        return std::format(
            "UPDATE {} AS target SET {} FROM json_populate_record(NULL::{}, '{}') AS src "
            "WHERE target.{} = src.{}",
            request.table_name, set_list, request.table_name, request.json_payload,
            request.pk_column, request.pk_column);
    }

    [[nodiscard]] static std::string build_upsert_sql(const SqlRequest &request) {
        std::string update_list;
        for (std::size_t index = 0; index < request.update_columns.size(); ++index) {
            if (index > 0) {
                update_list += ", ";
            }
            const auto &column = request.update_columns[index];
            update_list += std::format("{} = EXCLUDED.{}", column, column);
        }
        return std::format(
            "INSERT INTO {} SELECT * FROM json_populate_record(NULL::{}, '{}') "
            "ON CONFLICT ({}) DO UPDATE SET {}",
            request.table_name, request.table_name, request.json_payload, request.pk_column,
            update_list);
    }

    [[nodiscard]] static std::string build_delete_sql(const SqlRequest &request) {
        return std::format("DELETE FROM {} WHERE {} = '{}'", request.table_name,
                           request.pk_column, request.key);
    }

    [[nodiscard]] static std::string build_delete_many_sql(const SqlRequest &request) {
        return std::format("DELETE FROM {} WHERE {} IN ({})", request.table_name,
                           request.pk_column, quoted_list(request.keys));
    }

    [[nodiscard]] static std::string build_query_sql(const SqlRequest &request) {
        return std::format("SELECT json_agg(row_to_json(row)) FROM ({}) row",
                           build_inner_query(request.table_name, request.options));
    }

    [[nodiscard]] static std::string build_query_first_sql(const SqlRequest &request) {
        return std::format("SELECT row_to_json(row) FROM ({} LIMIT 1) row",
                           build_inner_query(request.table_name, request.options));
    }
};

} // namespace

CONGELADO_PLUGIN(SqlPostgresPlugin);

#ifdef CONGELADO_TEST
namespace sql_postgres_plugin_tests {
using namespace boost::ut;

/**
 * @brief Builds a `SqlQueryOptions`-shaped `rfl::Generic::Object` for feeding into
 * `rfl::from_generic<SqlRequest>` — every field of `SqlQueryOptions` is required in the
 * generic tree except `limit` (it's `std::optional`, so omitting the key entirely stands in
 * for "no limit").
 */
rfl::Generic::Object make_options_object(const std::vector<std::string> &joins,
                                          const std::vector<std::string> &where_conditions,
                                          const std::vector<std::pair<std::string, bool>> &order_by,
                                          std::optional<std::size_t> limit) {
    rfl::Generic::Object object;
    rfl::Generic::Array joins_array;
    for (const auto &join : joins) {
        joins_array.push_back(rfl::Generic{join});
    }
    object.insert(std::string{"joins"}, rfl::Generic{joins_array});

    rfl::Generic::Array where_array;
    for (const auto &condition : where_conditions) {
        where_array.push_back(rfl::Generic{condition});
    }
    object.insert(std::string{"where_conditions"}, rfl::Generic{where_array});

    rfl::Generic::Array order_array;
    for (const auto &clause : order_by) {
        rfl::Generic::Array pair_array{rfl::Generic{clause.first}, rfl::Generic{clause.second}};
        order_array.push_back(rfl::Generic{pair_array});
    }
    object.insert(std::string{"order_by_clauses"}, rfl::Generic{order_array});

    if (limit) {
        object.insert(std::string{"limit"}, rfl::Generic{static_cast<int>(*limit)});
    }
    return object;
}

/**
 * @brief Builds a full `SqlRequest`-shaped `rfl::Generic`, matching the exact field names
 * `SqlRequest` reflects on — every field is required (none are `rfl::DefaultVal`-wrapped), so
 * every one must be present even when empty. `columns` is always an empty array here since none
 * of the ops this test file exercises (`select`/`query`/unrecognized) read it.
 */
rfl::Generic make_request_generic(const std::string &op, const std::string &table_name,
                                   const std::string &pk_column, const std::string &key,
                                   const rfl::Generic::Object &options_object) {
    rfl::Generic::Object object;
    object.insert(std::string{"op"}, rfl::Generic{op});
    object.insert(std::string{"table_name"}, rfl::Generic{table_name});
    object.insert(std::string{"columns"}, rfl::Generic{rfl::Generic::Array{}});
    object.insert(std::string{"pk_column"}, rfl::Generic{pk_column});
    object.insert(std::string{"key"}, rfl::Generic{key});
    object.insert(std::string{"keys"}, rfl::Generic{rfl::Generic::Array{}});
    object.insert(std::string{"json_payload"}, rfl::Generic{std::string{}});
    object.insert(std::string{"json_array_payload"}, rfl::Generic{std::string{}});
    object.insert(std::string{"set_columns"}, rfl::Generic{rfl::Generic::Array{}});
    object.insert(std::string{"update_columns"}, rfl::Generic{rfl::Generic::Array{}});
    object.insert(std::string{"options"}, rfl::Generic{options_object});
    return rfl::Generic{object};
}

suite<"postgres_type_of"> postgres_type_of_suite = [] {
    "maps every ValueKind to its Postgres column type"_test = [] {
        expect(postgres_type_of(ValueKind::STRING) == "TEXT");
        expect(postgres_type_of(ValueKind::BOOLEAN) == "BOOLEAN");
        expect(postgres_type_of(ValueKind::INT64) == "BIGINT");
        expect(postgres_type_of(ValueKind::UINT64) == "BIGINT");
        expect(postgres_type_of(ValueKind::INT32) == "INTEGER");
        expect(postgres_type_of(ValueKind::UINT32) == "INTEGER");
        expect(postgres_type_of(ValueKind::DOUBLE) == "DOUBLE PRECISION");
        expect(postgres_type_of(ValueKind::FLOAT) == "REAL");
        expect(postgres_type_of(ValueKind::TIMESTAMP) == "BIGINT");
        expect(postgres_type_of(ValueKind::UUID) == "UUID");
        expect(postgres_type_of(ValueKind::OTHER) == "JSONB");
    };
};

suite<"quoted_list"> quoted_list_suite = [] {
    "empty list produces an empty string"_test = [] {
        expect(quoted_list({}) == "");
    };

    "single value is wrapped in single quotes"_test = [] {
        expect(quoted_list({"abc"}) == "'abc'");
    };

    "multiple values are comma-joined, each individually quoted"_test = [] {
        expect(quoted_list({"a", "b", "c"}) == "'a','b','c'");
    };

    // Regression test: quoted_list() previously did no escaping at all — a value containing a
    // single quote spliced straight through and broke out of the surrounding quotes, a real
    // SQL-injection-shaped gap since this feeds select_many/delete_many's `WHERE col IN (...)`
    // clauses (found while writing this file's tests). FIXED by doubling embedded single quotes,
    // standard SQL string-literal escaping.
    "a value containing a quote character is escaped by doubling the quote"_test = [] {
        expect(quoted_list({"O'Brien"}) == "'O''Brien'");
    };

    "a value with multiple embedded quotes is escaped throughout"_test = [] {
        expect(quoted_list({"'; DROP TABLE users; --"}) == "'''; DROP TABLE users; --'");
    };
};

suite<"build_inner_query"> build_inner_query_suite = [] {
    "a bare table with no joins/conditions/ordering/limit"_test = [] {
        SqlQueryOptions options;
        expect(build_inner_query("users", options) == "SELECT users.* FROM users");
    };

    "joins, where conditions, ordering, and a limit all compose together in order"_test = [] {
        SqlQueryOptions options;
        options.joins = {"JOIN orders ON orders.user_id = users.id"};
        options.where_conditions = {"active = true", "age > 18"};
        options.order_by_clauses = {{"name", true}, {"age", false}};
        options.limit = std::size_t{10};

        expect(build_inner_query("users", options) ==
               "SELECT users.* FROM users JOIN orders ON orders.user_id = users.id "
               "WHERE active = true AND age > 18 ORDER BY name ASC, age DESC LIMIT 10");
    };
};

suite<"SqlPostgresPlugin"> sql_postgres_plugin_suite = [] {
    "get_name returns sql_postgres"_test = [] {
        SqlPostgresPlugin plugin;
        expect(plugin.get_name() == "sql_postgres");
    };

    "get_version returns 0.1.0"_test = [] {
        SqlPostgresPlugin plugin;
        expect(plugin.get_version() == "0.1.0");
    };

    "capabilities reports CONGELADO_CAP_SERDE"_test = [] {
        SqlPostgresPlugin plugin;
        expect(plugin.capabilities() == CONGELADO_CAP_SERDE);
    };

    "serde_get returns a non-null pointer castable to ISerdeFormat"_test = [] {
        SqlPostgresPlugin plugin;
        void *raw = plugin.serde_get();
        expect(raw != nullptr) << fatal;
        auto *format = static_cast<interfaces::ISerdeFormat *>(raw);
        expect(format->format_name() == "sql_postgres");
    };

    "content_type returns application/sql+postgres"_test = [] {
        SqlPostgresPlugin plugin;
        expect(plugin.content_type() == "application/sql+postgres");
    };

    "format_name returns sql_postgres"_test = [] {
        SqlPostgresPlugin plugin;
        expect(plugin.format_name() == "sql_postgres");
    };

    "encode generates a SELECT-by-key query for the select op"_test = [] {
        SqlPostgresPlugin plugin;
        auto request = make_request_generic("select", "users", "id", "42",
                                             make_options_object({}, {}, {}, std::nullopt));
        auto result = plugin.encode(request);
        expect(result.has_value()) << fatal;
        expect(*result ==
               "SELECT row_to_json(row) FROM (SELECT * FROM users WHERE id = '42') row");
    };

    "encode generates a filtered/ordered/limited query for the query op"_test = [] {
        SqlPostgresPlugin plugin;
        auto options = make_options_object({}, {"active = true"}, {{"name", true}}, std::size_t{5});
        auto request = make_request_generic("query", "users", "", "", options);
        auto result = plugin.encode(request);
        expect(result.has_value()) << fatal;
        expect(*result ==
               "SELECT json_agg(row_to_json(row)) FROM (SELECT users.* FROM users "
               "WHERE active = true ORDER BY name ASC LIMIT 5) row");
    };

    "encode fails on an unrecognized op"_test = [] {
        SqlPostgresPlugin plugin;
        auto request = make_request_generic("bogus_op", "users", "", "",
                                             make_options_object({}, {}, {}, std::nullopt));
        auto result = plugin.encode(request);
        expect(!result.has_value()) << fatal;
        expect(result.error() == "sql_postgres: unrecognized op 'bogus_op'");
    };

    "decode always fails — this format is write-only"_test = [] {
        SqlPostgresPlugin plugin;
        auto result = plugin.decode("SELECT 1");
        expect(!result.has_value()) << fatal;
        expect(result.error() == "sql_postgres format is write-only, decode is not supported");
    };
};

} // namespace sql_postgres_plugin_tests
#endif
