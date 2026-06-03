export module connector;

export import :local_cache;

import core_contract;
import interfaces;
import shared;
import serde;
import std;

export namespace connector {

template <serde::IConnectable T, typename CacheHelper = serde::Cache,
          typename SqlBuilder = serde::Sql, std::size_t MaxCapacity = 1024>
    requires serde::ICacheHelper<CacheHelper, T> && serde::ISqlBuilder<SqlBuilder, T>
class Connector : public shared::HandlerBase {
  public:
    Connector() = default;
    Connector(interfaces::ICache *cache, interfaces::IDatabase *database)
        : m_cache{cache}, m_database{database} {}

    void set_cache(interfaces::ICache *cache) noexcept { m_cache = cache; }
    void set_database(interfaces::IDatabase *database) noexcept { m_database = database; }

    void flush() noexcept {
        while (!m_pending.empty()) {
            auto operation = std::move(m_pending.front());
            m_pending.pop();
            operation();
        }
    }

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

            shared::this_handler::shedule();
        };
    }

    // ── Schema ────────────────────────────────────────────────────────────────

    void create_table(std::move_only_function<void(bool)> callback) noexcept {
        enqueue([this, callback = std::move(callback)]() mutable {
            active_database().query(
                SqlBuilder::template build_create_sql<T>(),
                [callback = std::move(callback)](std::string_view result) mutable {
                    callback(!result.empty());
                });
        });
    }

    // ── Read ──────────────────────────────────────────────────────────────────

    void find(std::string_view key,
              std::move_only_function<void(std::optional<T>)> callback) noexcept {
        enqueue([this, owned_key = std::string{key}, callback = std::move(callback)]() mutable {
            auto cache_key_string = CacheHelper::template cache_key<T>(owned_key);
            active_cache().get(cache_key_string, [this, owned_key, cache_key_string,
                                                  callback = std::move(callback)](
                                                     std::string_view cached_value) mutable {
                if (!cached_value.empty()) {
                    auto decoded = serde::Json::decode<T>(cached_value);
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
                active_database().query(SqlBuilder::template build_select_sql<T>(owned_key),
                                        [this, cache_key_string, callback = std::move(callback)](
                                            std::string_view db_result) mutable {
                                            if (db_result.empty()) {
                                                callback(std::nullopt);
                                                return;
                                            }
                                            auto decoded = serde::Json::decode<T>(db_result);
                                            if (!decoded) {
                                                callback(std::nullopt);
                                                return;
                                            }
                                            active_cache().set(cache_key_string,
                                                               CacheHelper::cache_value(*decoded),
                                                               [](std::string_view) {});
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
                SqlBuilder::template build_select_many_sql<T>(owned_keys),
                [callback = std::move(callback)](std::string_view db_result) mutable {
                    if (db_result.empty()) {
                        callback({});
                        return;
                    }
                    auto decoded = serde::Json::decode_array<T>(db_result);
                    callback(decoded ? std::move(*decoded) : std::vector<T>{});
                });
        });
    }

    void find_all(std::move_only_function<void(std::vector<T>)> callback) noexcept {
        enqueue([this, callback = std::move(callback)]() mutable {
            active_database().query(
                SqlBuilder::template build_select_all_sql<T>(),
                [callback = std::move(callback)](std::string_view db_result) mutable {
                    if (db_result.empty()) {
                        callback({});
                        return;
                    }
                    auto decoded = serde::Json::decode_array<T>(db_result);
                    callback(decoded ? std::move(*decoded) : std::vector<T>{});
                });
        });
    }

    // ── Write ─────────────────────────────────────────────────────────────────

    void insert(const T &value, std::move_only_function<void(bool)> callback) noexcept {
        enqueue([this, value, callback = std::move(callback)]() mutable {
            write_through(value, SqlBuilder::template build_insert_sql<T>(value),
                          std::move(callback));
        });
    }

    void insert_many(std::span<const T> values,
                     std::move_only_function<void(bool)> callback) noexcept {
        enqueue([this, owned_values = std::vector<T>{values.begin(), values.end()},
                 callback = std::move(callback)]() mutable {
            active_database().query(
                SqlBuilder::template build_insert_many_sql<T>(owned_values),
                [callback = std::move(callback)](std::string_view result) mutable {
                    callback(!result.empty());
                });
        });
    }

    void update(const T &value, std::move_only_function<void(bool)> callback) noexcept {
        enqueue([this, value, callback = std::move(callback)]() mutable {
            write_through(value, SqlBuilder::template build_update_sql<T>(value),
                          std::move(callback));
        });
    }

    void upsert(const T &value, std::move_only_function<void(bool)> callback) noexcept {
        enqueue([this, value, callback = std::move(callback)]() mutable {
            write_through(value, SqlBuilder::template build_upsert_sql<T>(value),
                          std::move(callback));
        });
    }

    void remove(std::string_view key, std::move_only_function<void(bool)> callback) noexcept {
        enqueue([this, owned_key = std::string{key}, callback = std::move(callback)]() mutable {
            auto cache_key_string = CacheHelper::template cache_key<T>(owned_key);
            active_cache().remove(cache_key_string, [](std::string_view) {});
            if (!m_database) {
                m_local_store.erase(owned_key);
                callback(true);
                return;
            }
            active_database().remove(
                SqlBuilder::template build_delete_sql<T>(owned_key),
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
                active_cache().remove(CacheHelper::template cache_key<T>(key),
                                      [](std::string_view) {});
            active_database().remove(
                SqlBuilder::template build_delete_many_sql<T>(owned_keys),
                [callback = std::move(callback)](std::string_view result) mutable {
                    callback(!result.empty());
                });
        });
    }

  private:
    void enqueue(std::move_only_function<void()> operation) noexcept {
        m_pending.push(std::move(operation));
    }

    interfaces::ICache &active_cache() noexcept {
        return m_cache ? *m_cache : static_cast<interfaces::ICache &>(m_local_cache);
    }

    interfaces::IDatabase &active_database() noexcept { return *m_database; }

    void write_through(const T &value, std::string sql,
                       std::move_only_function<void(bool)> callback) {
        auto cache_key_string = CacheHelper::cache_key(value);
        auto json_string = CacheHelper::cache_value(value);
        active_cache().set(cache_key_string, json_string, [](std::string_view) {});
        if (!m_database) {
            m_local_store.insert_or_assign(CacheHelper::pk_string(value), value);
            callback(true);
            return;
        }
        active_database().query(sql,
                                [callback = std::move(callback)](std::string_view result) mutable {
                                    callback(!result.empty());
                                });
    }


    std::queue<std::move_only_function<void()>> m_pending;
    interfaces::ICache *m_cache;
    interfaces::IDatabase *m_database;
    LocalCache m_local_cache;
    std::unordered_map<std::string, T> m_local_store;
};

} // namespace connector
