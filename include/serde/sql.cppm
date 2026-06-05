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

template <typename VT>
constexpr std::string_view sql_type_of() {
    if constexpr (std::same_as<VT, std::string> || std::same_as<VT, std::optional<std::string>>)
        return "TEXT";
    else if constexpr (std::same_as<VT, bool>)
        return "BOOLEAN";
    else if constexpr (std::same_as<VT, std::int64_t> || std::same_as<VT, std::uint64_t> ||
                       std::same_as<VT, TP>            || std::same_as<VT, std::optional<TP>>)
        return "BIGINT";
    else if constexpr (std::same_as<VT, std::int32_t> || std::same_as<VT, std::uint32_t>)
        return "INTEGER";
    else if constexpr (std::same_as<VT, double>)
        return "DOUBLE PRECISION";
    else if constexpr (std::same_as<VT, float>)
        return "REAL";
    else if constexpr (std::is_enum_v<VT>)
        return "TEXT";
    else if constexpr (std::same_as<VT, uuids::uuid> ||
                       std::same_as<VT, std::optional<uuids::uuid>>)
        return "UUID";
    else
        return "JSONB";
}

template <IConnectable T>
std::string_view pk_column_name() {
    std::string_view result;
    std::apply(
        [&](auto... fds) {
            ((fds.options.db.primary_key ? (result = fds.name.string_view()) : std::string_view{}),
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
    QueryOptions &add_join(std::string join) noexcept {
        m_joins.push_back(std::move(join));
        return *this;
    }
    QueryOptions &add_where(std::string condition) noexcept {
        m_where_conditions.push_back(std::move(condition));
        return *this;
    }
    QueryOptions &add_order_by(std::string column, bool ascending = true) noexcept {
        m_order_by_clauses.emplace_back(std::move(column), ascending);
        return *this;
    }
    QueryOptions &set_limit(std::size_t limit) noexcept {
        m_limit = limit;
        return *this;
    }

    [[nodiscard]] const std::vector<std::string> &get_joins() const noexcept { return m_joins; }
    [[nodiscard]] const std::vector<std::string> &get_where_conditions() const noexcept {
        return m_where_conditions;
    }
    [[nodiscard]] const std::vector<std::pair<std::string, bool>> &
    get_order_by_clauses() const noexcept {
        return m_order_by_clauses;
    }
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
    template <IConnectable T>
    [[nodiscard]] static std::string build_create_sql() {
        std::string columns;
        std::size_t index = 0;
        std::apply(
            [&](auto... fds) {
                ([&] {
                    using VT = typename decltype(fds)::ValueType;
                    if (index++ > 0)
                        columns += ", ";
                    columns += fds.name.string_view();
                    columns += ' ';
                    columns += sql_type_of<VT>();
                    if (fds.options.db.primary_key)
                        columns += " PRIMARY KEY";
                    else if (!fds.options.db.nullable)
                        columns += " NOT NULL";
                    if (fds.options.db.unique && !fds.options.db.primary_key)
                        columns += " UNIQUE";
                    if (fds.options.db.ref_table != nullptr)
                        columns += std::format(" REFERENCES {}({})", fds.options.db.ref_table,
                                               fds.options.db.ref_column);
                }(),
                 ...);
            },
            Serializable<T>::fields());
        return std::format("CREATE TABLE IF NOT EXISTS {} ({})", Serializable<T>::table_name(), columns);
    }

    template <IConnectable T>
    [[nodiscard]] static std::string build_select_sql(std::string_view key) {
        return std::format(
            "SELECT row_to_json(row) FROM (SELECT * FROM {} WHERE {} = '{}') row",
            Serializable<T>::table_name(), pk_column_name<T>(), key);
    }

    template <IConnectable T>
    [[nodiscard]] static std::string build_select_many_sql(std::span<const std::string> keys) {
        std::string list;
        for (std::size_t index = 0; index < keys.size(); ++index) {
            if (index > 0)
                list += ',';
            list += std::format("'{}'", keys[index]);
        }
        return std::format(
            "SELECT json_agg(row_to_json(row)) FROM (SELECT * FROM {} WHERE {} IN ({})) row",
            Serializable<T>::table_name(), pk_column_name<T>(), list);
    }

    template <IConnectable T>
    [[nodiscard]] static std::string build_select_all_sql() {
        return std::format("SELECT json_agg(row_to_json(row)) FROM {} row",
                           Serializable<T>::table_name());
    }

    template <IConnectable T>
    [[nodiscard]] static std::string build_insert_sql(const T &value) {
        return std::format("INSERT INTO {} SELECT * FROM json_populate_record(NULL::{}, '{}')",
                           Serializable<T>::table_name(), Serializable<T>::table_name(),
                           Json::encode(value));
    }

    template <IConnectable T>
    [[nodiscard]] static std::string build_insert_many_sql(std::span<const T> values) {
        std::string rows;
        for (std::size_t index = 0; index < values.size(); ++index) {
            if (index > 0)
                rows += ',';
            rows += std::format("'{}'", Json::encode(values[index]));
        }
        return std::format("INSERT INTO {} SELECT * FROM json_populate_recordset(NULL::{}, '[{}]')",
                           Serializable<T>::table_name(), Serializable<T>::table_name(), rows);
    }

    template <IConnectable T>
    [[nodiscard]] static std::string build_update_sql(const T &value) {
        std::string set_list;
        std::size_t index = 0;
        std::apply(
            [&](auto... fds) {
                ([&] {
                    if (fds.options.db.primary_key || fds.options.db.skip_update)
                        return;
                    if (index++ > 0)
                        set_list += ", ";
                    auto column = fds.name.string_view();
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

    template <IConnectable T>
    [[nodiscard]] static std::string build_upsert_sql(const T &value) {
        std::string update_list;
        std::size_t index = 0;
        std::apply(
            [&](auto... fds) {
                ([&] {
                    if (fds.options.db.primary_key)
                        return;
                    if (index++ > 0)
                        update_list += ", ";
                    auto column = fds.name.string_view();
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

    template <IConnectable T>
    [[nodiscard]] static std::string build_delete_sql(std::string_view key) {
        return std::format("DELETE FROM {} WHERE {} = '{}'", Serializable<T>::table_name(),
                           pk_column_name<T>(), key);
    }

    template <IConnectable T>
    [[nodiscard]] static std::string build_delete_many_sql(std::span<const std::string> keys) {
        std::string list;
        for (std::size_t index = 0; index < keys.size(); ++index) {
            if (index > 0)
                list += ',';
            list += std::format("'{}'", keys[index]);
        }
        return std::format("DELETE FROM {} WHERE {} IN ({})", Serializable<T>::table_name(),
                           pk_column_name<T>(), list);
    }

    template <IConnectable T>
    [[nodiscard]] static std::string build_query_sql(const QueryOptions &options) {
        return std::format("SELECT json_agg(row_to_json(row)) FROM ({}) row",
                           build_inner_query<T>(options));
    }

    template <IConnectable T>
    [[nodiscard]] static std::string build_query_first_sql(const QueryOptions &options) {
        return std::format("SELECT row_to_json(row) FROM ({} LIMIT 1) row",
                           build_inner_query<T>(options));
    }

  private:
    template <IConnectable T>
    [[nodiscard]] static std::string build_inner_query(const QueryOptions &options) {
        auto table = Serializable<T>::table_name();
        std::string inner = std::format("SELECT {}.* FROM {}", table, table);
        for (const auto &join : options.get_joins())
            inner += " " + join;
        bool first_where = true;
        for (const auto &condition : options.get_where_conditions()) {
            inner += first_where ? " WHERE " : " AND ";
            inner += condition;
            first_where = false;
        }
        bool first_order = true;
        for (const auto &[column, ascending] : options.get_order_by_clauses()) {
            inner += first_order ? " ORDER BY " : ", ";
            inner += column;
            inner += ascending ? " ASC" : " DESC";
            first_order = false;
        }
        if (options.get_limit())
            inner += std::format(" LIMIT {}", *options.get_limit());
        return inner;
    }
};

} // namespace serde
