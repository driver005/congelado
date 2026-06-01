export module connector;

export import :local_cache;

import core_contract;
import interfaces;
import shared;
import serde;
import std;

export namespace connector {

template <typename T>
concept IConnectable = serde::ISerializable<T> && requires {
    { serde::Serializable<T>::table_name() } -> std::convertible_to<std::string_view>;
};

template <IConnectable T, std::size_t MaxCapacity = 1024>
class Connector : public shared::HandlerBase {
  public:
    Connector(core::contract::ContractGroup<MaxCapacity> &group, interfaces::ICache *cache,
              interfaces::IDatabase *database)
        : m_cache{cache}, m_database{database},
          m_contract{this->create(group, core::contract::ContractState::IDLE)} {}

    [[nodiscard]] std::string_view get_name() const noexcept override {
        return serde::Serializable<T>::table_name();
    }

    shared::WorkerFunction on_execute() override {
        return [this]() {
            if (m_pending.empty())
                return;
            auto pending_operation = std::move(m_pending.front());
            m_pending.pop();
            pending_operation();
            if (!m_pending.empty())
                shared::this_handler::shedule();
        };
    }

    // ── Schema ────────────────────────────────────────────────────────────────

    void create_table(std::move_only_function<void(bool)> callback) noexcept {
        enqueue([this, callback = std::move(callback)]() mutable {
            active_database().query(build_create_sql(), [callback = std::move(callback)](
                                                            std::string_view result) mutable {
                callback(!result.empty());
            });
        });
    }

    // ── Read ──────────────────────────────────────────────────────────────────

    void find(std::string_view key,
              std::move_only_function<void(std::optional<T>)> callback) noexcept {
        enqueue([this, owned_key = std::string{key}, callback = std::move(callback)]() mutable {
            auto cache_key_string = make_cache_key(owned_key);
            active_cache().get(cache_key_string, [this, owned_key, cache_key_string,
                                                  callback = std::move(callback)](
                                                     std::string_view cached_value) mutable {
                if (!cached_value.empty()) {
                    auto decoded = serde::from_json<T>(cached_value);
                    callback(decoded ? std::optional<T>{std::move(*decoded)} : std::nullopt);
                    return;
                }
                if (!m_database) {
                    auto local_iterator = m_local_store.find(owned_key);
                    callback(local_iterator != m_local_store.end()
                                 ? std::optional<T>{local_iterator->second}
                                 : std::nullopt);
                    return;
                }
                active_database().query(
                    build_select_sql(owned_key),
                    [this, cache_key_string,
                     callback = std::move(callback)](std::string_view db_result) mutable {
                        if (db_result.empty()) {
                            callback(std::nullopt);
                            return;
                        }
                        auto decoded = serde::from_json<T>(db_result);
                        if (!decoded) {
                            callback(std::nullopt);
                            return;
                        }
                        auto encoded = serde::to_json(*decoded);
                        active_cache().set(cache_key_string, encoded, [](std::string_view) {});
                        callback(std::optional<T>{std::move(*decoded)});
                    });
            });
        });
    }

    void find_many(std::span<const std::string_view> keys,
                   std::move_only_function<void(std::vector<T>)> callback) noexcept {
        enqueue([this, owned_keys = std::vector<std::string>{keys.begin(), keys.end()},
                 callback = std::move(callback)]() mutable {
            active_database().query(
                build_select_many_sql(owned_keys),
                [callback = std::move(callback)](std::string_view db_result) mutable {
                    if (db_result.empty()) {
                        callback({});
                        return;
                    }
                    auto decoded = serde::from_json<std::vector<T>>(db_result);
                    callback(decoded ? std::move(*decoded) : std::vector<T>{});
                });
        });
    }

    void find_all(std::move_only_function<void(std::vector<T>)> callback) noexcept {
        enqueue([this, callback = std::move(callback)]() mutable {
            active_database().query(
                build_select_all_sql(),
                [callback = std::move(callback)](std::string_view db_result) mutable {
                    if (db_result.empty()) {
                        callback({});
                        return;
                    }
                    auto decoded = serde::from_json<std::vector<T>>(db_result);
                    callback(decoded ? std::move(*decoded) : std::vector<T>{});
                });
        });
    }

    // ── Write ─────────────────────────────────────────────────────────────────

    void insert(const T &value, std::move_only_function<void(bool)> callback) noexcept {
        enqueue([this, value, callback = std::move(callback)]() mutable {
            write_through(value, build_insert_sql(value), std::move(callback));
        });
    }

    void insert_many(std::span<const T> values,
                     std::move_only_function<void(bool)> callback) noexcept {
        enqueue([this, owned_values = std::vector<T>{values.begin(), values.end()},
                 callback = std::move(callback)]() mutable {
            active_database().query(
                build_insert_many_sql(owned_values),
                [callback = std::move(callback)](std::string_view result) mutable {
                    callback(!result.empty());
                });
        });
    }

    void update(const T &value, std::move_only_function<void(bool)> callback) noexcept {
        enqueue([this, value, callback = std::move(callback)]() mutable {
            write_through(value, build_update_sql(value), std::move(callback));
        });
    }

    void upsert(const T &value, std::move_only_function<void(bool)> callback) noexcept {
        enqueue([this, value, callback = std::move(callback)]() mutable {
            write_through(value, build_upsert_sql(value), std::move(callback));
        });
    }

    void remove(std::string_view key, std::move_only_function<void(bool)> callback) noexcept {
        enqueue([this, owned_key = std::string{key}, callback = std::move(callback)]() mutable {
            auto cache_key_string = make_cache_key(owned_key);
            active_cache().remove(cache_key_string, [](std::string_view) {});
            if (!m_database) {
                m_local_store.erase(owned_key);
                callback(true);
                return;
            }
            active_database().remove(
                build_delete_sql(owned_key),
                [callback = std::move(callback)](std::string_view result) mutable {
                    callback(!result.empty());
                });
        });
    }

    void remove_many(std::span<const std::string_view> keys,
                     std::move_only_function<void(bool)> callback) noexcept {
        enqueue([this, owned_keys = std::vector<std::string>{keys.begin(), keys.end()},
                 callback = std::move(callback)]() mutable {
            for (const auto &key : owned_keys)
                active_cache().remove(make_cache_key(key), [](std::string_view) {});
            active_database().remove(
                build_delete_many_sql(owned_keys),
                [callback = std::move(callback)](std::string_view result) mutable {
                    callback(!result.empty());
                });
        });
    }

  private:
    void enqueue(std::move_only_function<void()> operation) noexcept {
        m_pending.push(std::move(operation));
        m_contract.schedule();
    }

    interfaces::ICache &active_cache() noexcept {
        return m_cache ? *m_cache : static_cast<interfaces::ICache &>(m_local_cache);
    }

    interfaces::IDatabase &active_database() noexcept { return *m_database; }

    std::string make_cache_key(std::string_view key) const {
        return std::format("{}:{}", serde::Serializable<T>::table_name(), key);
    }

    void write_through(const T &value, std::string sql,
                       std::move_only_function<void(bool)> callback) {
        auto cache_key_string = make_cache_key(pk_value(value));
        auto json_string = serde::to_json(value);
        active_cache().set(cache_key_string, json_string, [](std::string_view) {});
        if (!m_database) {
            m_local_store.insert_or_assign(pk_value(value), value);
            callback(true);
            return;
        }
        active_database().query(sql,
                                [callback = std::move(callback)](std::string_view result) mutable {
                                    callback(!result.empty());
                                });
    }

    // ── SQL builders ──────────────────────────────────────────────────────────

    static std::string build_create_sql() {
        return std::format("CREATE TABLE IF NOT EXISTS {} (id TEXT PRIMARY KEY)",
                           serde::Serializable<T>::table_name());
    }

    static std::string build_select_sql(std::string_view key) {
        return std::format("SELECT row_to_json(row) FROM (SELECT * FROM {} WHERE id = '{}') row",
                           serde::Serializable<T>::table_name(), key);
    }

    static std::string build_select_many_sql(const std::vector<std::string> &keys) {
        std::string list;
        for (std::size_t index = 0; index < keys.size(); ++index) {
            if (index > 0)
                list += ',';
            list += std::format("'{}'", keys[index]);
        }
        return std::format(
            "SELECT json_agg(row_to_json(row)) FROM (SELECT * FROM {} WHERE id IN ({})) row",
            serde::Serializable<T>::table_name(), list);
    }

    static std::string build_select_all_sql() {
        return std::format("SELECT json_agg(row_to_json(row)) FROM {} row",
                           serde::Serializable<T>::table_name());
    }

    static std::string build_insert_sql(const T &value) {
        return std::format("INSERT INTO {} SELECT * FROM json_populate_record(NULL::{}, '{}')",
                           serde::Serializable<T>::table_name(),
                           serde::Serializable<T>::table_name(), serde::to_json(value));
    }

    static std::string build_insert_many_sql(const std::vector<T> &values) {
        std::string rows;
        for (std::size_t index = 0; index < values.size(); ++index) {
            if (index > 0)
                rows += ',';
            rows += std::format("'{}'", serde::to_json(values[index]));
        }
        return std::format("INSERT INTO {} SELECT * FROM json_populate_recordset(NULL::{}, '[{}]')",
                           serde::Serializable<T>::table_name(),
                           serde::Serializable<T>::table_name(), rows);
    }

    static std::string build_update_sql(const T &value) {
        return std::format("UPDATE {} SET data = row FROM json_populate_record(NULL::{}, '{}') row "
                           "WHERE {}.id = row.id",
                           serde::Serializable<T>::table_name(),
                           serde::Serializable<T>::table_name(), serde::to_json(value),
                           serde::Serializable<T>::table_name());
    }

    static std::string build_upsert_sql(const T &value) {
        return std::format("INSERT INTO {} SELECT * FROM json_populate_record(NULL::{}, '{}') "
                           "ON CONFLICT (id) DO UPDATE SET id = EXCLUDED.id",
                           serde::Serializable<T>::table_name(),
                           serde::Serializable<T>::table_name(), serde::to_json(value));
    }

    static std::string build_delete_sql(std::string_view key) {
        return std::format("DELETE FROM {} WHERE id = '{}'", serde::Serializable<T>::table_name(),
                           key);
    }

    static std::string build_delete_many_sql(const std::vector<std::string> &keys) {
        std::string list;
        for (std::size_t index = 0; index < keys.size(); ++index) {
            if (index > 0)
                list += ',';
            list += std::format("'{}'", keys[index]);
        }
        return std::format("DELETE FROM {} WHERE id IN ({})", serde::Serializable<T>::table_name(),
                           list);
    }

    static std::string pk_value(const T &value) {
        return serde::to_json(value); // placeholder — FieldOptions.db.primary_key drives this
    }

    // ── Members ───────────────────────────────────────────────────────────────

    std::queue<std::move_only_function<void()>> m_pending;
    interfaces::ICache *m_cache;
    interfaces::IDatabase *m_database;
    LocalCache m_local_cache;
    std::unordered_map<std::string, T> m_local_store;
    core::contract::Contract<MaxCapacity> m_contract;
};

} // namespace connector
