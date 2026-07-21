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
    /**
     * @brief Default ctor — no cache, no database, nothing wired up yet. Every op runs fully
     * local until someone calls set_cache()/set_database().
     */
    Connector() = default;
    /**
     * @brief Builds a connector already pointed at a cache and a database backend.
     * @note Both pointers are non-owning — this class never deletes them, whoever owns the
     * backend instances keeps owning them.
     * @param cache the cache backend to read/write through, or nullptr for cache-less mode.
     * @param database the database backend to read/write through, or nullptr for local-only mode.
     */
    Connector(interfaces::ICache *cache, interfaces::IDatabase *database)
        : m_cache{cache}, m_database{database} {}

    /**
     * @brief Swaps in a new cache backend pointer.
     * @param cache the cache backend to use going forward, or nullptr to fall back to the
     * built-in LocalCache.
     */
    void set_cache(interfaces::ICache *cache) noexcept { m_cache = cache; }
    /**
     * @brief Swaps in a new database backend pointer.
     * @warning Flipping this from set to null (or vice versa) mid-flight changes whether
     * queued ops run sync or async — see enqueue(). Don't do it while ops are still in flight
     * unless you know exactly what you're doing.
     * @param database the database backend to use going forward, or nullptr for local-only mode.
     */
    void set_database(interfaces::IDatabase *database) noexcept { m_database = database; }

    /**
     * @brief Gets the currently configured cache backend.
     * @return the cache pointer, or nullptr if none's wired up.
     */
    [[nodiscard]] interfaces::ICache *get_cache() const noexcept { return m_cache; }
    /**
     * @brief Gets the currently configured database backend.
     * @return the database pointer, or nullptr if this connector's running local-only.
     */
    [[nodiscard]] interfaces::IDatabase *get_database() const noexcept { return m_database; }

    /**
     * @brief Identifies this handler for the controller's registry.
     * @return the fixed string "connector" — that's the whole identity, no cap.
     */
    [[nodiscard]] std::string_view get_name() const noexcept override { return "connector"; }

    /**
     * @brief The handler's per-tick work: drains one pending op off the queue, runs it, then
     * reschedules itself so the next one goes on the following tick. If the queue's empty this
     * just bails without rescheduling, so the handler quietly goes idle till enqueue() wakes it
     * back up.
     * @note Only ever has anything to drain when a database backend is configured — enqueue()
     * runs ops immediately, synchronously, whenever m_database is null.
     * @return the per-execution callable the controller invokes on schedule.
     */
    shared::WorkerFunction on_execute() override {
        return [this]() {
            // Nothing queued — quietly go idle, no rescheduling till enqueue() wakes us back up.
            if (m_pending.empty()) {
                return;
            }

            // Pull the next op off the front of the queue and run it, FIFO order.
            auto pending_operation = std::move(m_pending.front());
            m_pending.pop();

            pending_operation();

            // Reschedule so the following op (if any) gets drained on the next tick.
            shared::this_handler::shedule();
        };
    }

    /**
     * @brief Makes sure the backing table for `T` exists in the database. No-op success if
     * there's no database configured — a local-only connector doesn't need a table, bet.
     * @tparam T the connectable type whose table gets created, must satisfy serde::IConnectable.
     * @param callback gets `true` if the create succeeded (or there was no database to begin
     * with), `false` if the database came back empty-handed.
     */
    template <serde::IConnectable T>
    void create_table(std::move_only_function<void(bool)> callback) noexcept {
        enqueue([this, callback = std::move(callback)]() mutable {
            // No database wired up — nothing to create, just say it worked.
            if (!m_database) {
                callback(true);
                return;
            }
            // Otherwise fire the CREATE TABLE statement and report whether it landed.
            active_database().query(
                serde::Sql::template build_create_sql<T>(),
                [callback = std::move(callback)](std::string_view result) mutable {
                    callback(!result.empty());
                });
        });
    }

    /**
     * @brief Looks up one row by key — cache first, then falls through to whichever backing
     * store is actually configured. On a cache miss that resolves through the database, the
     * result gets written back into the cache before the callback fires (classic cache-aside).
     * @tparam T the connectable type being looked up, must satisfy serde::IConnectable.
     * @param key the primary-key value to look up.
     * @param callback gets the decoded value if found, std::nullopt if it's nowhere to be
     * found (cache miss, store miss, or a decode that fails).
     */
    template <serde::IConnectable T>
    void find(std::string_view key,
              std::move_only_function<void(std::optional<T>)> callback) noexcept {
        enqueue([this, owned_key = std::string{key}, callback = std::move(callback)]() mutable {
            // Build the cache key up front, then check the cache before anything slower.
            auto cache_key_string = serde::Cache::template cache_key<T>(owned_key);
            active_cache().get(cache_key_string, [this, owned_key, cache_key_string,
                                                  callback = std::move(callback)](
                                                     std::string_view cached_value) mutable {
                // Cache hit — decode it straight up, no need to go any further.
                if (!cached_value.empty()) {
                    auto decoded = serde::Json::decode<T>(cached_value);
                    callback(decoded ? std::optional<T>{std::move(*decoded)} : std::nullopt);
                    return;
                }
                // Cache miss and no database configured — fall through to the local store.
                if (!m_database) {
                    auto &store = get_local_store<T>();
                    auto local_iterator = store.find(owned_key);
                    callback(local_iterator != store.end()
                                 ? std::optional<T>{local_iterator->second}
                                 : std::nullopt);
                    return;
                }
                // Cache miss with a real database — go fetch it there.
                active_database().query(serde::Sql::template build_select_sql<T>(owned_key),
                                        [this, cache_key_string, callback = std::move(callback)](
                                            std::string_view db_result) mutable {
                                            // Nothing came back — treat it as a full miss.
                                            if (db_result.empty()) {
                                                callback(std::nullopt);
                                                return;
                                            }
                                            // Got a row but it failed to decode — same deal, a miss.
                                            auto decoded = serde::Json::decode<T>(db_result);
                                            if (!decoded) {
                                                callback(std::nullopt);
                                                return;
                                            }
                                            // Found and decoded — warm the cache before handing it
                                            // back, classic cache-aside, no cap.
                                            active_cache().set(cache_key_string,
                                                               serde::Cache::cache_value(*decoded),
                                                               [](std::string_view) {});
                                            callback(std::optional<T>{std::move(*decoded)});
                                        });
            });
        });
    }

    /**
     * @brief Batch version of find() — looks up several keys at once, skipping the cache
     * entirely and going straight to whichever store is configured.
     * @note No cache-aside motion here, unlike find(). Rows just come back from local storage
     * or the database, no cache read or write in the middle.
     * @tparam T the connectable type being looked up, must satisfy serde::IConnectable.
     * @param keys the primary-key values to look up.
     * @param callback gets every row that was actually found — missing keys just get skipped,
     * no placeholder, no error.
     */
    template <serde::IConnectable T>
    void find_many(std::span<const std::string_view> keys,
                   std::move_only_function<void(std::vector<T>)> callback) noexcept {
        enqueue([this, owned_keys = std::vector<std::string>{keys.begin(), keys.end()},
                 callback = std::move(callback)]() mutable {
            // No database — walk the local store and collect whatever keys actually hit, bet.
            if (!m_database) {
                auto &store = get_local_store<T>();
                std::vector<T> results;
                for (const auto &key : owned_keys) {
                    auto iterator = store.find(key);
                    if (iterator != store.end()) {
                        results.push_back(iterator->second);
                    }
                }
                callback(std::move(results));
                return;
            }
            // Database configured — one query for the whole batch instead of looping.
            active_database().query(
                serde::Sql::template build_select_many_sql<T>(owned_keys),
                [callback = std::move(callback)](std::string_view db_result) mutable {
                    // Empty result means nothing matched; otherwise decode the array and
                    // hand back whatever came through.
                    if (db_result.empty()) {
                        callback({});
                        return;
                    }
                    auto decoded = serde::Json::decode_array<T>(db_result);
                    callback(decoded ? std::move(*decoded) : std::vector<T>{});
                });
        });
    }

    /**
     * @brief Finds the first row matching `predicate`, per `sorter`'s ordering.
     * @warning `predicate`/`sorter` only actually run when there's no database — that branch
     * filters+sorts the in-memory store by hand. The database branch trusts `options` to
     * already encode the filter/order server-side and never touches `predicate` or `sorter`
     * at all. Keep both branches in sync yourself, this class won't do it for you.
     * @tparam T the connectable type being queried, must satisfy serde::IConnectable.
     * @param options query options handed straight to the SQL builder for the database branch.
     * @param predicate row filter used only in the local-store (no-database) branch.
     * @param sorter ordering comparator used only in the local-store (no-database) branch.
     * @param callback gets the first matching row, or std::nullopt if nothing matched.
     */
    template <serde::IConnectable T>
    void find_first(serde::QueryOptions options,
                    std::move_only_function<bool(const T &)> predicate,
                    std::move_only_function<bool(const T &, const T &)> sorter,
                    std::move_only_function<void(std::optional<T>)> callback) noexcept {
        enqueue([this, options = std::move(options), predicate = std::move(predicate),
                 sorter = std::move(sorter), callback = std::move(callback)]() mutable {
            // No database — lowkey just filter the local store by hand with the caller's predicate.
            if (!m_database) {
                auto &store = get_local_store<T>();
                std::vector<const T *> candidates;
                for (const auto &[key, value] : store) {
                    if (predicate(value)) {
                        candidates.push_back(&value);
                    }
                }
                // Nothing matched, nothing to sort or return.
                if (candidates.empty()) {
                    callback(std::nullopt);
                    return;
                }
                // Sort the matches with the caller's comparator and hand back the front one.
                std::sort(candidates.begin(), candidates.end(),
                          [&](const T *lhs, const T *rhs) { return sorter(*lhs, *rhs); });
                callback(std::optional<T>{*candidates.front()});
                return;
            }
            // Database configured — trust `options` already encodes filter/order server-side.
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

    /**
     * @brief Grabs every row of `T` from whichever store is configured. No filtering, no cache
     * involvement — straight dump of the whole table.
     * @tparam T the connectable type being fetched, must satisfy serde::IConnectable.
     * @param callback gets every row currently stored, empty vector if there's nothing there.
     */
    template <serde::IConnectable T>
    void find_all(std::move_only_function<void(std::vector<T>)> callback) noexcept {
        enqueue([this, callback = std::move(callback)]() mutable {
            // No database — just dump every row out of the local store.
            if (!m_database) {
                auto &store = get_local_store<T>();
                std::vector<T> results;
                results.reserve(store.size());
                for (const auto &[key, value] : store) {
                    results.push_back(value);
                }
                callback(std::move(results));
                return;
            }
            // Database configured — pull the whole table in one query and decode the array.
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

    /**
     * @brief Inserts `value` — writes through to the cache, then either the database or the
     * local in-memory store depending on what's configured. See write_through() for the actual
     * split.
     * @tparam T the connectable type being inserted, must satisfy serde::IConnectable.
     * @param value the row to insert.
     * @param callback gets the insert outcome, W or L.
     */
    template <serde::IConnectable T>
    void insert(const T &value, std::move_only_function<void(bool)> callback) noexcept {
        enqueue([this, value, callback = std::move(callback)]() mutable {
            write_through(value, serde::Sql::template build_insert_sql<T>(value),
                          std::move(callback));
        });
    }

    /**
     * @brief Batch insert for `values`.
     * @warning Unlike insert(), this one skips the cache entirely — no write-through here, only
     * the local store or the database gets touched. If you need these rows cache-warm, that's
     * on you.
     * @tparam T the connectable type being inserted, must satisfy serde::IConnectable.
     * @param values the rows to insert.
     * @param callback gets `true` on success (always true for the local-store branch), `false`
     * if the database came back empty-handed.
     */
    template <serde::IConnectable T>
    void insert_many(std::span<const T> values,
                     std::move_only_function<void(bool)> callback) noexcept {
        enqueue([this, owned_values = std::vector<T>{values.begin(), values.end()},
                 callback = std::move(callback)]() mutable {
            // No database — upsert every value straight into the local store, always a W.
            if (!m_database) {
                auto &store = get_local_store<T>();
                for (const auto &value : owned_values) {
                    store.insert_or_assign(serde::Cache::pk_string(value), value);
                }
                callback(true);
                return;
            }
            // Database configured — one batched INSERT for the whole set.
            active_database().query(
                serde::Sql::template build_insert_many_sql<T>(owned_values),
                [callback = std::move(callback)](std::string_view result) mutable {
                    callback(!result.empty());
                });
        });
    }

    /**
     * @brief Updates `value` in place — writes through to the cache, then either the database
     * or the local in-memory store. Same write_through() split as insert().
     * @tparam T the connectable type being updated, must satisfy serde::IConnectable.
     * @param value the row to update, keyed by its own primary key.
     * @param callback gets the update outcome.
     */
    template <serde::IConnectable T>
    void update(const T &value, std::move_only_function<void(bool)> callback) noexcept {
        enqueue([this, value, callback = std::move(callback)]() mutable {
            write_through(value, serde::Sql::template build_update_sql<T>(value),
                          std::move(callback));
        });
    }

    /**
     * @brief Upserts `value` — insert if it's new, update if it already exists. Same
     * write_through() cache-then-store split as insert()/update().
     * @tparam T the connectable type being upserted, must satisfy serde::IConnectable.
     * @param value the row to upsert.
     * @param callback gets the upsert outcome.
     */
    template <serde::IConnectable T>
    void upsert(const T &value, std::move_only_function<void(bool)> callback) noexcept {
        enqueue([this, value, callback = std::move(callback)]() mutable {
            write_through(value, serde::Sql::template build_upsert_sql<T>(value),
                          std::move(callback));
        });
    }

    /**
     * @brief Removes the row under `key` — cache entry gets yeeted unconditionally first
     * (fire-and-forget, result ignored), then the local store or the database gets cleaned up
     * too depending on what's configured.
     * @tparam T the connectable type being removed, must satisfy serde::IConnectable.
     * @param key the primary-key value to remove.
     * @param callback gets the removal outcome (always true for the local-store branch).
     */
    template <serde::IConnectable T>
    void remove(std::string_view key, std::move_only_function<void(bool)> callback) noexcept {
        enqueue([this, owned_key = std::string{key}, callback = std::move(callback)]() mutable {
            // Cache entry goes first, unconditionally, fire-and-forget.
            active_cache().remove(serde::Cache::template cache_key<T>(owned_key),
                                  [](std::string_view) {});
            // No database — erasing it from the local store is the whole delete.
            if (!m_database) {
                get_local_store<T>().erase(owned_key);
                callback(true);
                return;
            }
            // Database configured — fire the DELETE and report whether it hit.
            active_database().remove(
                serde::Sql::template build_delete_sql<T>(owned_key),
                [callback = std::move(callback)](std::string_view result) mutable {
                    callback(!result.empty());
                });
        });
    }

    /**
     * @brief Batch remove for `keys` — same cache-then-store cleanup as remove(), just looped
     * over every key first.
     * @tparam T the connectable type being removed, must satisfy serde::IConnectable.
     * @param keys the primary-key values to remove.
     * @param callback gets the removal outcome (always true for the local-store branch).
     */
    template <serde::IConnectable T>
    void remove_many(std::span<const std::string_view> keys,
                     std::move_only_function<void(bool)> callback) noexcept {
        enqueue([this, owned_keys = std::vector<std::string>{keys.begin(), keys.end()},
                 callback = std::move(callback)]() mutable {
            // Clear every key out of the cache first, same fire-and-forget deal as remove().
            for (const auto &key : owned_keys) {
                active_cache().remove(serde::Cache::template cache_key<T>(key),
                                      [](std::string_view) {});
            }
            // No database — erase the whole batch from the local store.
            if (!m_database) {
                auto &store = get_local_store<T>();
                for (const auto &key : owned_keys) {
                    store.erase(key);
                }
                callback(true);
                return;
            }
            // Database configured — one batched DELETE for the whole set.
            active_database().remove(
                serde::Sql::template build_delete_many_sql<T>(owned_keys),
                [callback = std::move(callback)](std::string_view result) mutable {
                    callback(!result.empty());
                });
        });
    }

  private:
    /**
     * @brief Runs `operation` right now if there's no database configured (local-only mode is
     * fully synchronous), otherwise queues it up to be drained one-at-a-time by on_execute().
     * @param operation the unit of work to run or queue.
     */
    void enqueue(std::move_only_function<void()> operation) noexcept {
        // No database means fully synchronous — run it now; otherwise queue it up for
        // on_execute() to drain one at a time.
        if (m_database == nullptr) {
            operation();
        } else {
            m_pending.push(std::move(operation));
        }
    }

    /**
     * @brief Resolves whichever cache is actually active right now.
     * @return `*m_cache` if a real backend's wired up, otherwise the built-in `m_local_cache` —
     * this always hands back something usable, never null.
     */
    interfaces::ICache &active_cache() noexcept {
        return m_cache != nullptr ? *m_cache : static_cast<interfaces::ICache &>(m_local_cache);
    }

    /**
     * @brief Resolves the active database backend.
     * @warning No null check here — dereferences `m_database` straight up. Every call site in
     * this class already guards on `!m_database` first, so don't call this one cold, that's an
     * instant UB L.
     * @return a reference to the configured database backend.
     */
    interfaces::IDatabase &active_database() noexcept { return *m_database; }

    /**
     * @brief Shared write path for insert()/update()/upsert(): always writes `value` into the
     * cache, then either upserts it into the local store or fires `sql` at the database,
     * whichever's configured.
     * @tparam T the connectable type being written.
     * @param value the row being written — used to derive the cache key and the local-store key.
     * @param sql the pre-built SQL statement to run when a database is configured.
     * @param callback gets the write outcome.
     */
    template <typename T>
    void write_through(const T &value, const std::string &sql,
                       std::move_only_function<void(bool)> callback) {
        // Cache gets the write unconditionally — insert/update/upsert all funnel through here.
        active_cache().set(serde::Cache::cache_key(value), serde::Cache::cache_value(value),
                           [](std::string_view) {});
        // No database — the local store is the source of truth, upsert it there.
        if (!m_database) {
            get_local_store<T>().insert_or_assign(serde::Cache::pk_string(value), value);
            callback(true);
            return;
        }
        // Database configured — run the pre-built SQL and report whether it landed.
        active_database().query(sql,
                                [callback = std::move(callback)](std::string_view result) mutable {
                                    callback(!result.empty());
                                });
    }

    /**
     * @brief Gets the type-erased in-memory store for `T`, lazily creating an empty one on
     * first touch. This is the whole local-only-mode persistence layer, no cap.
     * @tparam T the connectable type whose store gets resolved.
     * @return a reference to the `std::unordered_map<std::string, T>` backing `T`'s local rows.
     */
    // Not noexcept: m_local_stores[...] and the std::any assignment below can throw (bad_alloc).
    // Every call site is inside a lambda handed to enqueue(), which is itself the actual
    // noexcept boundary for the local-only (synchronous) path — dropping noexcept here doesn't
    // change behavior, an exception thrown from within still terminates at that boundary.
    template <typename T>
    std::unordered_map<std::string, T> &get_local_store() {
        auto &slot = m_local_stores[std::type_index(typeid(T))];
        // First touch for this type — lazily spin up an empty store.
        if (!slot.has_value()) {
            slot = std::unordered_map<std::string, T>{};
        }
        return std::any_cast<std::unordered_map<std::string, T> &>(slot);
    }

    std::queue<std::move_only_function<void()>> m_pending;
    interfaces::ICache *m_cache{nullptr};
    interfaces::IDatabase *m_database{nullptr};
    LocalCache m_local_cache;
    std::unordered_map<std::type_index, std::any> m_local_stores;
};

} // namespace connector
