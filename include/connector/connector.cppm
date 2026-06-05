export module connector;

export import :local_cache;

import core_contract;
import interfaces;
import shared;
import serde;
import std;

export namespace connector {

class Connector : public shared::HandlerBase {
  public:
    Connector() = default;
    Connector(interfaces::ICache *cache, interfaces::IDatabase *database)
        : m_cache{cache}, m_database{database} {}

    void set_cache(interfaces::ICache *cache) noexcept { m_cache = cache; }
    void set_database(interfaces::IDatabase *database) noexcept { m_database = database; }

    [[nodiscard]] std::string_view get_name() const noexcept override { return "connector"; }

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

    template <serde::IConnectable T>
    void create_table(std::move_only_function<void(bool)> callback) noexcept {
        enqueue([this, callback = std::move(callback)]() mutable {
            if (!m_database) {
                callback(true);
                return;
            }
            active_database().query(
                serde::Sql::template build_create_sql<T>(),
                [callback = std::move(callback)](std::string_view result) mutable {
                    callback(!result.empty());
                });
        });
    }

    template <serde::IConnectable T>
    void find(std::string_view key,
              std::move_only_function<void(std::optional<T>)> callback) noexcept {
        enqueue([this, owned_key = std::string{key}, callback = std::move(callback)]() mutable {
            auto cache_key_string = serde::Cache::template cache_key<T>(owned_key);
            active_cache().get(cache_key_string, [this, owned_key, cache_key_string,
                                                  callback = std::move(callback)](
                                                     std::string_view cached_value) mutable {
                if (!cached_value.empty()) {
                    auto decoded = serde::Json::decode<T>(cached_value);
                    callback(decoded ? std::optional<T>{std::move(*decoded)} : std::nullopt);
                    return;
                }
                if (!m_database) {
                    auto &store = get_local_store<T>();
                    auto local_iterator = store.find(owned_key);
                    callback(local_iterator != store.end()
                                 ? std::optional<T>{local_iterator->second}
                                 : std::nullopt);
                    return;
                }
                active_database().query(serde::Sql::template build_select_sql<T>(owned_key),
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
                                                               serde::Cache::cache_value(*decoded),
                                                               [](std::string_view) {});
                                            callback(std::optional<T>{std::move(*decoded)});
                                        });
            });
        });
    }

    template <serde::IConnectable T>
    void find_many(std::span<const std::string_view> keys,
                   std::move_only_function<void(std::vector<T>)> callback) noexcept {
        enqueue([this, owned_keys = std::vector<std::string>{keys.begin(), keys.end()},
                 callback = std::move(callback)]() mutable {
            if (!m_database) {
                auto &store = get_local_store<T>();
                std::vector<T> results;
                for (const auto &key : owned_keys) {
                    auto iterator = store.find(key);
                    if (iterator != store.end())
                        results.push_back(iterator->second);
                }
                callback(std::move(results));
                return;
            }
            active_database().query(
                serde::Sql::template build_select_many_sql<T>(owned_keys),
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

    template <serde::IConnectable T>
    void find_first(serde::QueryOptions options,
                    std::move_only_function<bool(const T &)> predicate,
                    std::move_only_function<bool(const T &, const T &)> sorter,
                    std::move_only_function<void(std::optional<T>)> callback) noexcept {
        enqueue([this, options = std::move(options), predicate = std::move(predicate),
                 sorter = std::move(sorter), callback = std::move(callback)]() mutable {
            if (!m_database) {
                auto &store = get_local_store<T>();
                std::vector<const T *> candidates;
                for (const auto &[key, value] : store) {
                    if (predicate(value))
                        candidates.push_back(&value);
                }
                if (candidates.empty()) {
                    callback(std::nullopt);
                    return;
                }
                std::sort(candidates.begin(), candidates.end(),
                          [&](const T *lhs, const T *rhs) { return sorter(*lhs, *rhs); });
                callback(std::optional<T>{*candidates.front()});
                return;
            }
            active_database().query(
                serde::Sql::template build_query_first_sql<T>(options),
                [callback = std::move(callback)](std::string_view db_result) mutable {
                    if (db_result.empty()) {
                        callback(std::nullopt);
                        return;
                    }
                    auto decoded = serde::Json::decode<T>(db_result);
                    callback(decoded ? std::optional<T>{std::move(*decoded)} : std::nullopt);
                });
        });
    }

    template <serde::IConnectable T>
    void find_all(std::move_only_function<void(std::vector<T>)> callback) noexcept {
        enqueue([this, callback = std::move(callback)]() mutable {
            if (!m_database) {
                auto &store = get_local_store<T>();
                std::vector<T> results;
                results.reserve(store.size());
                for (const auto &[key, value] : store)
                    results.push_back(value);
                callback(std::move(results));
                return;
            }
            active_database().query(
                serde::Sql::template build_select_all_sql<T>(),
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

    template <serde::IConnectable T>
    void insert(const T &value, std::move_only_function<void(bool)> callback) noexcept {
        enqueue([this, value, callback = std::move(callback)]() mutable {
            write_through(value, serde::Sql::template build_insert_sql<T>(value),
                          std::move(callback));
        });
    }

    template <serde::IConnectable T>
    void insert_many(std::span<const T> values,
                     std::move_only_function<void(bool)> callback) noexcept {
        enqueue([this, owned_values = std::vector<T>{values.begin(), values.end()},
                 callback = std::move(callback)]() mutable {
            if (!m_database) {
                auto &store = get_local_store<T>();
                for (const auto &value : owned_values)
                    store.insert_or_assign(serde::Cache::pk_string(value), value);
                callback(true);
                return;
            }
            active_database().query(
                serde::Sql::template build_insert_many_sql<T>(owned_values),
                [callback = std::move(callback)](std::string_view result) mutable {
                    callback(!result.empty());
                });
        });
    }

    template <serde::IConnectable T>
    void update(const T &value, std::move_only_function<void(bool)> callback) noexcept {
        enqueue([this, value, callback = std::move(callback)]() mutable {
            write_through(value, serde::Sql::template build_update_sql<T>(value),
                          std::move(callback));
        });
    }

    template <serde::IConnectable T>
    void upsert(const T &value, std::move_only_function<void(bool)> callback) noexcept {
        enqueue([this, value, callback = std::move(callback)]() mutable {
            write_through(value, serde::Sql::template build_upsert_sql<T>(value),
                          std::move(callback));
        });
    }

    template <serde::IConnectable T>
    void remove(std::string_view key, std::move_only_function<void(bool)> callback) noexcept {
        enqueue([this, owned_key = std::string{key}, callback = std::move(callback)]() mutable {
            active_cache().remove(serde::Cache::template cache_key<T>(owned_key),
                                  [](std::string_view) {});
            if (!m_database) {
                get_local_store<T>().erase(owned_key);
                callback(true);
                return;
            }
            active_database().remove(
                serde::Sql::template build_delete_sql<T>(owned_key),
                [callback = std::move(callback)](std::string_view result) mutable {
                    callback(!result.empty());
                });
        });
    }

    template <serde::IConnectable T>
    void remove_many(std::span<const std::string_view> keys,
                     std::move_only_function<void(bool)> callback) noexcept {
        enqueue([this, owned_keys = std::vector<std::string>{keys.begin(), keys.end()},
                 callback = std::move(callback)]() mutable {
            for (const auto &key : owned_keys)
                active_cache().remove(serde::Cache::template cache_key<T>(key),
                                      [](std::string_view) {});
            if (!m_database) {
                auto &store = get_local_store<T>();
                for (const auto &key : owned_keys)
                    store.erase(key);
                callback(true);
                return;
            }
            active_database().remove(
                serde::Sql::template build_delete_many_sql<T>(owned_keys),
                [callback = std::move(callback)](std::string_view result) mutable {
                    callback(!result.empty());
                });
        });
    }

  private:
    void enqueue(std::move_only_function<void()> operation) noexcept {
        if (!m_database) {
            operation();
        } else {
            m_pending.push(std::move(operation));
        }
    }

    interfaces::ICache &active_cache() noexcept {
        return m_cache ? *m_cache : static_cast<interfaces::ICache &>(m_local_cache);
    }

    interfaces::IDatabase &active_database() noexcept { return *m_database; }

    template <typename T>
    void write_through(const T &value, std::string sql,
                       std::move_only_function<void(bool)> callback) {
        active_cache().set(serde::Cache::cache_key(value), serde::Cache::cache_value(value),
                           [](std::string_view) {});
        if (!m_database) {
            get_local_store<T>().insert_or_assign(serde::Cache::pk_string(value), value);
            callback(true);
            return;
        }
        active_database().query(sql,
                                [callback = std::move(callback)](std::string_view result) mutable {
                                    callback(!result.empty());
                                });
    }

    template <typename T>
    std::unordered_map<std::string, T> &get_local_store() noexcept {
        auto &slot = m_local_stores[std::type_index(typeid(T))];
        if (!slot.has_value())
            slot = std::unordered_map<std::string, T>{};
        return std::any_cast<std::unordered_map<std::string, T> &>(slot);
    }

    std::queue<std::move_only_function<void()>> m_pending;
    interfaces::ICache *m_cache{nullptr};
    interfaces::IDatabase *m_database{nullptr};
    LocalCache m_local_cache;
    std::unordered_map<std::type_index, std::any> m_local_stores;
};

} // namespace connector
