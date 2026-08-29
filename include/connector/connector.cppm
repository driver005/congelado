module;
#define UUID_SYSTEM_GENERATOR
#include <uuid.h>

export module connector;

import core_contract;
import core_events;
import core_logger;
import core_otel;
import interfaces;
import shared;
import serde;
import std;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

// ─── SQL DDL/query codegen ────────────────────────────────────────────────────────────────────
//
// This is Connector's own per-T reflection step, not a wire-format concern in itself — it's
// driven by an arbitrary host-defined T reflected at compile time, so it can never cross a
// dlopen boundary into a plugin .so built before any host T existed. Connector is its only real
// caller, so this stays right here. What CAN cross the plugin boundary is everything downstream
// of that reflection: once T's fields are reduced into a plain SqlRequest (below — no T
// anywhere), Sql::build_*_sql<T>() dispatches it through the exact same serde::Ser/
// SerdeFormatRegistry machinery JSON/TOML already use (serde::Ser::serialize("application/sql+
// postgres", request)) — a dialect plugin (plugins/sql_postgres) does the actual text
// generation. serde::value_kind_of<VT>()/pk_column_name<T>() (the two genuinely dialect-agnostic
// reflection helpers this used to hand-roll locally) now live in serde::core, usable by anything.

// ─── Query options ────────────────────────────────────────────────────────────

export namespace connector {

class QueryOptions {
  public:
    /**
     * @brief Adds a raw JOIN clause fragment to the query — appended verbatim after the base
     * `FROM table`, so it needs to be a complete `JOIN ... ON ...` string.
     * @param join the raw SQL join fragment to add.
     * @warning No validation or escaping happens here — this gets spliced straight into the
     * generated SQL by Sql::build_inner_query. Only feed it trusted, pre-built fragments, never
     * raw user input, or it's an instant SQL-injection L.
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
     * @warning Same deal as `add_join` — no escaping, spliced in raw. Trusted fragments only,
     * bet.
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
     * @brief Sets a `LIMIT` on the query — only the last call before build wins, this isn't
     * cumulative like the adders.
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

} // namespace connector

// ─── SQL builders ─────────────────────────────────────────────────────────────
//
// Plain runtime-data types crossing into the active SQL dialect plugin via serde::Ser — no
// shared header with the plugin, just matching field names on both sides (same principle as
// this session's Document duplication for the client codegen tool): only the reduced
// rfl::Generic shape has to agree, not C++ type identity.

namespace connector {

struct SqlColumnDesc {
    std::string name;
    serde::ValueKind kind{serde::ValueKind::OTHER};
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

/// @brief The content-type the active SQL dialect plugin registers under — hardcoded, matches
/// Connector's existing single-backend-at-a-time design (see plugins/sql_postgres).
inline constexpr std::string_view SQL_DIALECT_CONTENT_TYPE = "application/sql+postgres";

} // namespace connector

export namespace connector {

class Sql {
  public:
    /**
     * @brief Generates a `CREATE TABLE IF NOT EXISTS` statement for T from its reflected
     * fields — column kinds come from `serde::value_kind_of`, constraints (PK/NOT NULL/UNIQUE/
     * FK) come straight off each field's FieldOptionsDb. The active SQL dialect plugin turns
     * this into actual DDL text.
     * @tparam T the connectable type to generate DDL for.
     * @return the generated `CREATE TABLE` SQL string.
     */
    template <serde::IConnectable T>
    [[nodiscard]] static std::string build_create_sql() {
        std::vector<SqlColumnDesc> columns;
        std::apply(
            [&](auto... fields) {
                (columns.push_back(SqlColumnDesc{
                     .name = std::string{fields.name.string_view()},
                     .kind = serde::value_kind_of<typename decltype(fields)::ValueType>(),
                     .primary_key = fields.options.m_db.m_primary_key,
                     .nullable = fields.options.m_db.m_nullable,
                     .unique = fields.options.m_db.m_unique,
                     .skip_update = fields.options.m_db.m_skip_update,
                     .ref_table = fields.options.m_db.m_ref_table != nullptr
                                      ? std::string{fields.options.m_db.m_ref_table}
                                      : std::string{},
                     .ref_column = fields.options.m_db.m_ref_column != nullptr
                                       ? std::string{fields.options.m_db.m_ref_column}
                                       : std::string{},
                 }),
                 ...);
            },
            serde::Serializable<T>::fields());
        return dispatch(SqlRequest{.op = "create_table",
                                   .table_name = std::string{serde::Serializable<T>::table_name()},
                                   .columns = std::move(columns)});
    }

    /**
     * @brief Generates a statement fetching one row by primary key.
     * @tparam T the connectable type being queried.
     * @param key the primary-key value to match, e.g. from serde::Cache::pk_string.
     * @warning `key` rides through to the dialect plugin as plain text — only feed this
     * trusted, already-validated key material (e.g. a UUID or an id you generated), never raw
     * external input; escaping/parameterization is the dialect plugin's responsibility.
     * @return the generated `SELECT` SQL string.
     */
    template <serde::IConnectable T>
    [[nodiscard]] static std::string build_select_sql(std::string_view key) {
        return dispatch(SqlRequest{.op = "select",
                                   .table_name = std::string{serde::Serializable<T>::table_name()},
                                   .pk_column = std::string{serde::pk_column_name<T>()},
                                   .key = std::string{key}});
    }

    /**
     * @brief Generates a statement fetching every row whose primary key is in `keys`.
     * @tparam T the connectable type being queried.
     * @param keys the primary-key values to match against.
     * @return the generated `SELECT ... IN (...)` SQL string.
     */
    template <serde::IConnectable T>
    [[nodiscard]] static std::string build_select_many_sql(std::span<const std::string> keys) {
        return dispatch(SqlRequest{.op = "select_many",
                                   .table_name = std::string{serde::Serializable<T>::table_name()},
                                   .pk_column = std::string{serde::pk_column_name<T>()},
                                   .keys = {keys.begin(), keys.end()}});
    }

    /**
     * @brief Generates a statement fetching every row in T's table, no filtering.
     * @tparam T the connectable type being queried.
     * @return the generated `SELECT` SQL string.
     */
    template <serde::IConnectable T>
    [[nodiscard]] static std::string build_select_all_sql() {
        return dispatch(SqlRequest{
            .op = "select_all", .table_name = std::string{serde::Serializable<T>::table_name()}});
    }

    /**
     * @brief Generates an `INSERT` statement for a single row.
     * @tparam T the connectable type being inserted.
     * @param value the instance to insert.
     * @note `value` is JSON-encoded via `serde::Ser::encode_json` first, so this inherits
     * whatever type-coercion behavior that encoder has (e.g. how it renders optional/null
     * fields).
     * @return the generated `INSERT` SQL string.
     */
    template <serde::IConnectable T>
    [[nodiscard]] static std::string build_insert_sql(const T &value) {
        return dispatch(SqlRequest{.op = "insert",
                                   .table_name = std::string{serde::Serializable<T>::table_name()},
                                   .json_payload = serde::Ser::encode_json(value)});
    }

    /**
     * @brief Generates a bulk `INSERT` statement for multiple rows.
     * @tparam T the connectable type being inserted.
     * @param values the instances to insert, in order.
     * @return the generated bulk `INSERT` SQL string.
     */
    template <serde::IConnectable T>
    [[nodiscard]] static std::string build_insert_many_sql(std::span<const T> values) {
        std::string rows = "[";
        for (std::size_t index = 0; index < values.size(); ++index) {
            if (index > 0) {
                rows += ',';
            }
            rows += serde::Ser::encode_json(
                values[index]); // FIXME(clang-tidy): unchecked operator[], consider .at()
        }
        rows += ']';
        return dispatch(SqlRequest{.op = "insert_many",
                                   .table_name = std::string{serde::Serializable<T>::table_name()},
                                   .json_array_payload = std::move(rows)});
    }

    /**
     * @brief Generates an `UPDATE` statement matched by primary key, setting every column
     * except the PK and any marked `skip_update` in its FieldOptionsDb.
     * @tparam T the connectable type being updated.
     * @param value the instance whose fields (and PK, for the WHERE match) drive the update.
     * @return the generated `UPDATE` SQL string.
     */
    template <serde::IConnectable T>
    [[nodiscard]] static std::string build_update_sql(const T &value) {
        std::vector<std::string> set_columns;
        std::apply(
            [&](auto... fields) {
                ((fields.options.m_db.m_primary_key || fields.options.m_db.m_skip_update
                      ? void()
                      : void(set_columns.emplace_back(fields.name.string_view()))),
                 ...);
            },
            serde::Serializable<T>::fields());
        auto pk_column = std::string{serde::pk_column_name<T>()};
        return dispatch(SqlRequest{.op = "update",
                                   .table_name = std::string{serde::Serializable<T>::table_name()},
                                   .pk_column = pk_column,
                                   .json_payload = serde::Ser::encode_json(value),
                                   .set_columns = std::move(set_columns)});
    }

    /**
     * @brief Generates an upsert statement — every non-PK column gets updated on conflict.
     * @tparam T the connectable type being upserted.
     * @param value the instance to insert or update.
     * @return the generated upsert SQL string.
     */
    template <serde::IConnectable T>
    [[nodiscard]] static std::string build_upsert_sql(const T &value) {
        std::vector<std::string> update_columns;
        std::apply(
            [&](auto... fields) {
                ((fields.options.m_db.m_primary_key
                      ? void()
                      : void(update_columns.emplace_back(fields.name.string_view()))),
                 ...);
            },
            serde::Serializable<T>::fields());
        return dispatch(SqlRequest{.op = "upsert",
                                   .table_name = std::string{serde::Serializable<T>::table_name()},
                                   .pk_column = std::string{serde::pk_column_name<T>()},
                                   .json_payload = serde::Ser::encode_json(value),
                                   .update_columns = std::move(update_columns)});
    }

    /**
     * @brief Generates a `DELETE` statement matched by primary key.
     * @tparam T the connectable type being deleted from.
     * @param key the primary-key value to match.
     * @return the generated `DELETE` SQL string.
     */
    template <serde::IConnectable T>
    [[nodiscard]] static std::string build_delete_sql(std::string_view key) {
        return dispatch(SqlRequest{.op = "delete",
                                   .table_name = std::string{serde::Serializable<T>::table_name()},
                                   .pk_column = std::string{serde::pk_column_name<T>()},
                                   .key = std::string{key}});
    }

    /**
     * @brief Generates a `DELETE` statement matching any row whose primary key is in `keys`.
     * @tparam T the connectable type being deleted from.
     * @param keys the primary-key values to match against.
     * @return the generated `DELETE ... IN (...)` SQL string.
     */
    template <serde::IConnectable T>
    [[nodiscard]] static std::string build_delete_many_sql(std::span<const std::string> keys) {
        return dispatch(SqlRequest{.op = "delete_many",
                                   .table_name = std::string{serde::Serializable<T>::table_name()},
                                   .pk_column = std::string{serde::pk_column_name<T>()},
                                   .keys = {keys.begin(), keys.end()}});
    }

    /**
     * @brief Generates a statement over an arbitrary query built from `options` (joins, where,
     * order by, limit) — the general-purpose query path, versus the fixed-shape `build_select_*`
     * methods above.
     * @tparam T the connectable type being queried.
     * @param options the joins/conditions/ordering/limit to compose the query from.
     * @return the generated `SELECT` SQL string, rows aggregated.
     */
    template <serde::IConnectable T>
    [[nodiscard]] static std::string build_query_sql(const QueryOptions &options) {
        return dispatch(SqlRequest{.op = "query",
                                   .table_name = std::string{serde::Serializable<T>::table_name()},
                                   .options = to_sql_query_options(options)});
    }

    /**
     * @brief Same as `build_query_sql`, but requests only the first match.
     * @tparam T the connectable type being queried.
     * @param options the joins/conditions/ordering to compose the query from — any
     * `options.get_limit()` value is ignored by the dialect plugin, which always forces
     * `LIMIT 1` for this operation.
     * @return the generated `SELECT ... LIMIT 1` SQL string.
     */
    template <serde::IConnectable T>
    [[nodiscard]] static std::string build_query_first_sql(const QueryOptions &options) {
        return dispatch(SqlRequest{.op = "query_first",
                                   .table_name = std::string{serde::Serializable<T>::table_name()},
                                   .options = to_sql_query_options(options)});
    }

  private:
    /// @brief Snapshots a `QueryOptions` builder into the plain-data shape that crosses to the
    /// dialect plugin.
    /// @param options the builder to snapshot.
    /// @return the equivalent plain `SqlQueryOptions`.
    [[nodiscard]] static SqlQueryOptions to_sql_query_options(const QueryOptions &options) {
        return SqlQueryOptions{.joins = options.get_joins(),
                               .where_conditions = options.get_where_conditions(),
                               .order_by_clauses = options.get_order_by_clauses(),
                               .limit = options.get_limit()};
    }

    /**
     * @brief Sends `request` through the active SQL dialect format plugin and returns the
     * generated SQL text.
     * @param request the fully-built request describing which operation to generate SQL for.
     * @warning No format plugin registered for `SQL_DIALECT_CONTENT_TYPE` means
     * `serde::Ser::serialize` already returns a clean JSON error payload (its existing
     * no-silent-fallback behavior) instead of SQL text — that payload would then fail loud when
     * a caller tries to execute it as SQL, rather than corrupting a query silently.
     * @return the generated SQL text, or an error-shaped JSON payload if no dialect is loaded.
     */
    [[nodiscard]] static std::string dispatch(const SqlRequest &request) {
        auto encoded = serde::Ser::serialize(SQL_DIALECT_CONTENT_TYPE, request);
        return {reinterpret_cast<const char *>(encoded.data()),
                encoded.size()}; // NOLINT(cppcoreguidelines-pro-type-reinterpret-cast) —
                                 // byte-vector-to-string is the standard shape Ser::serialize's
                                 // callers use to get text back out
    }
};

} // namespace connector

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
     * @param cache the cache backend to use going forward. Required before any cache-touching
     * operation runs — there's no built-in fallback anymore (the `local` cache plugin is the
     * default backend the host resolves); passing nullptr leaves cache ops with nothing to call.
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
     * @brief Installs a callback enqueue() fires after pushing an async operation, so the
     * owner can schedule this handler when it was idle. No-op if left unset.
     * @param wake the callback to invoke under the pending-queue mutex.
     */
    void set_wake(std::move_only_function<void()> wake) noexcept { m_wake = std::move(wake); }

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
            std::move_only_function<void()> pending_operation;
            {
                std::lock_guard lock{m_pending_mutex};
                // Nothing queued — quietly go idle, no rescheduling till enqueue() wakes us back
                // up.
                if (m_pending.empty()) {
                    return;
                }

                // Pull the next op off the front of the queue and run it, FIFO order. Mark
                // ourselves executing so a reentrant enqueue() from within `operation()` (e.g. a
                // baseline migration chaining straight into its next create_table() from inside
                // this op's own completion callback, since a synchronous backend like Postgres
                // fires that callback inline) knows not to wake us — see enqueue()'s own comment.
                pending_operation = std::move(m_pending.front());
                m_pending.pop();
                m_executing = true;
            }

            pending_operation();

            bool has_more = false;
            {
                std::lock_guard lock{m_pending_mutex};
                m_executing = false;
                has_more = !m_pending.empty();
            }
            // Only reschedule if there's still work — either left over from before this tick, or
            // pushed reentrantly by the operation just run. Rescheduling unconditionally would
            // double-schedule the contract in the reentrant case (enqueue() already deferred its
            // own wake to us while m_executing was true) and trip core/contract's "already
            // scheduled" fatal guard.
            if (has_more) {
                shared::this_handler::shedule();
            }
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
        enqueue("create_table", [this, callback = std::move(callback)]() mutable {
            // No database wired up — nothing to create, just say it worked.
            if (!m_database) {
                callback(true);
                return;
            }
            // Otherwise fire the CREATE TABLE statement and report whether it landed.
            active_database().query(
                Sql::template build_create_sql<T>(),
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
        enqueue("find", [this, owned_key = std::string{key},
                         callback = std::move(callback)]() mutable {
            // Build the cache key up front, then check the cache before anything slower.
            auto cache_key_string = serde::Cache::template cache_key<T>(owned_key);
            active_cache().get(cache_key_string, [this, owned_key, cache_key_string,
                                                  callback = std::move(callback)](
                                                     std::string_view cached_value) mutable {
                // Cache hit — decode it straight up, no need to go any further. A decode failure
                // (e.g. no "application/json" format plugin registered, the normal case for a
                // local-only/test connector that never loaded one) falls through exactly like a
                // cache miss instead of reporting a false negative — write_through() caches
                // unconditionally even with no database, so without this fallthrough every
                // find() after an insert()/update() would wrongly report nullopt.
                if (!cached_value.empty()) {
                    auto decoded = serde::Ser::deserialize<T>("application/json", cached_value);
                    if (decoded) {
                        callback(std::optional<T>{std::move(*decoded)});
                        return;
                    }
                }
                // Cache miss (or an undecodable cache hit) and no database configured — fall
                // through to the local store.
                if (!m_database) {
                    auto &store = get_local_store<T>();
                    auto local_iterator = store.find(owned_key);
                    callback(local_iterator != store.end()
                                 ? std::optional<T>{local_iterator->second}
                                 : std::nullopt);
                    return;
                }
                // Cache miss with a real database — go fetch it there.
                active_database().query(
                    Sql::template build_select_sql<T>(owned_key),
                    [this, cache_key_string,
                     callback = std::move(callback)](std::string_view db_result) mutable {
                        // Nothing came back — treat it as a full miss.
                        if (db_result.empty()) {
                            callback(std::nullopt);
                            return;
                        }
                        // Got a row but it failed to decode — same deal, a miss.
                        auto decoded = serde::Ser::deserialize<T>("application/json", db_result);
                        if (!decoded) {
                            callback(std::nullopt);
                            return;
                        }
                        // Found and decoded — warm the cache before handing it
                        // back, classic cache-aside, no cap.
                        active_cache().set(cache_key_string, serde::Cache::cache_value(*decoded),
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
        enqueue("find_many", [this, owned_keys = std::vector<std::string>{keys.begin(), keys.end()},
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
                Sql::template build_select_many_sql<T>(owned_keys),
                [callback = std::move(callback)](std::string_view db_result) mutable {
                    // Empty result means nothing matched; otherwise decode the array and
                    // hand back whatever came through.
                    if (db_result.empty()) {
                        callback({});
                        return;
                    }
                    auto decoded =
                        serde::Ser::deserialize<std::vector<T>>("application/json", db_result);
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
    void find_first(QueryOptions options, std::move_only_function<bool(const T &)> predicate,
                    std::move_only_function<bool(const T &, const T &)> sorter,
                    std::move_only_function<void(std::optional<T>)> callback) noexcept {
        enqueue("find_first", [this, options = std::move(options), predicate = std::move(predicate),
                               sorter = std::move(sorter),
                               callback = std::move(callback)]() mutable {
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
                Sql::template build_query_first_sql<T>(options),
                [callback = std::move(callback)](std::string_view db_result) mutable {
                    if (db_result.empty()) {
                        callback(std::nullopt);
                        return;
                    }
                    auto decoded = serde::Ser::deserialize<T>("application/json", db_result);
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
        enqueue("find_all", [this, callback = std::move(callback)]() mutable {
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
                Sql::template build_select_all_sql<T>(),
                [callback = std::move(callback)](std::string_view db_result) mutable {
                    if (db_result.empty()) {
                        callback({});
                        return;
                    }
                    auto decoded =
                        serde::Ser::deserialize<std::vector<T>>("application/json", db_result);
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
        enqueue("insert", [this, value, callback = std::move(callback)]() mutable {
            write_through("insert", value, Sql::template build_insert_sql<T>(value),
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
        enqueue("insert_many", [this, owned_values = std::vector<T>{values.begin(), values.end()},
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
                Sql::template build_insert_many_sql<T>(owned_values),
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
        enqueue("update", [this, value, callback = std::move(callback)]() mutable {
            write_through("update", value, Sql::template build_update_sql<T>(value),
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
        enqueue("upsert", [this, value, callback = std::move(callback)]() mutable {
            write_through("upsert", value, Sql::template build_upsert_sql<T>(value),
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
        enqueue("remove",
                [this, owned_key = std::string{key}, callback = std::move(callback)]() mutable {
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
                        Sql::template build_delete_sql<T>(owned_key),
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
        enqueue("remove_many",
                [this, owned_keys = std::vector<std::string>{keys.begin(), keys.end()},
                 callback = std::move(callback)]() mutable {
                    // Clear every key out of the cache first, same fire-and-forget deal as
                    // remove().
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
                        Sql::template build_delete_many_sql<T>(owned_keys),
                        [callback = std::move(callback)](std::string_view result) mutable {
                            callback(!result.empty());
                        });
                });
    }

  private:
    /**
     * @brief Runs `operation` right now if there's no database configured (local-only mode is
     * fully synchronous), otherwise queues it up to be drained one-at-a-time by on_execute().
     * Wraps it in a `db.{operation_name}` span either way.
     * @note The span's recorded duration covers queueing + dispatch-to-backend (the window this
     * class actually controls) — if the configured `IDatabase` backend's own callback fires
     * later, fully asynchronously, past `operation()`'s own synchronous return, the true
     * round-trip runs a bit longer than what gets reported here. Still real, useful signal
     * (queue latency is itself worth seeing), just not a claim of exact end-to-end coverage for
     * every possible backend.
     * @param operation_name the operation's name for the span (e.g. `"find"`, `"insert"`).
     * @param operation the unit of work to run or queue.
     */
    void enqueue(std::string_view operation_name,
                 std::move_only_function<void()> operation) noexcept {
        auto span_name = std::format("db.{}", operation_name);
        // No database means fully synchronous — the calling thread is still right here when
        // `operation()` returns, so an ordinary ambient ScopedSpan is accurate and simplest.
        if (m_database == nullptr) {
            auto span = core::otel::start_span(span_name, interfaces::SpanKind::INTERNAL);
            operation();
            return;
        }
        // Queued for a later tick, possibly drained by a different pool thread — same
        // genuine-async-gap situation as ClientRuntime's send()/dispatch(), so this needs a
        // DetachedSpan (no ambient stack involvement) rather than a ScopedSpan.
        auto span = core::otel::start_detached_span(span_name, interfaces::SpanKind::CLIENT);
        {
            std::lock_guard lock{m_pending_mutex};
            // Only wake the connector handler when transitioning from idle (empty queue) to
            // having work. If the queue already had items, the handler is either scheduled or
            // currently executing and will reschedule itself, so calling wake again would
            // double-schedule the contract and trip `core/contract`'s "already scheduled" guard.
            const bool was_idle = m_pending.empty();
            m_pending.push([operation = std::move(operation), span = std::move(span)]() mutable {
                operation();
                span.end();
            });
            // Skip the wake if this push landed while on_execute() is mid-operation (m_executing)
            // — that happens when the operation just run reentrantly enqueues its own next step
            // (e.g. a migration's create_table<T> chaining into create_table<T+1> from inside a
            // synchronous backend's inline completion callback). on_execute()'s own trailing
            // reschedule check picks this item up once the operation returns; waking here too
            // would double-schedule the contract and trip core/contract's "already scheduled"
            // fatal guard.
            if (was_idle && !m_executing && m_wake) {
                m_wake();
            }
        }
    }

    /**
     * @brief Resolves the active cache backend.
     * @warning Requires `set_cache()` to already have been called with a non-null pointer — the
     * connector requires a cache backend to be wired via `set_cache()` (the `local` cache plugin
     * is the default one the host resolves); there's no built-in in-process fallback anymore. A
     * null `m_cache` here means the host's wiring is broken (e.g. the cache plugin failed to
     * load), not a normal degraded mode — logs and aborts instead of an unguarded deref, so this
     * fails loud with a reason instead of silently corrupting memory.
     * @return a reference to the configured cache backend.
     */
    interfaces::ICache &active_cache() noexcept {
        if (m_cache == nullptr) {
            core::logger::error("connector", "active_cache() called with no cache backend configured");
            std::abort();
        }
        return *m_cache;
    }

    /**
     * @brief Resolves the active database backend.
     * @warning Every call site in this class already guards on `!m_database` first — this is a
     * second line of defense, not the primary check. A null `m_database` here means a call site
     * skipped that guard; logs and aborts instead of an unguarded deref.
     * @return a reference to the configured database backend.
     */
    interfaces::IDatabase &active_database() noexcept {
        if (m_database == nullptr) {
            core::logger::error("connector",
                                "active_database() called with no database backend configured");
            std::abort();
        }
        return *m_database;
    }

    /**
     * @brief Shared write path for insert()/update()/upsert(): always writes `value` into the
     * cache, then either upserts it into the local store or fires `sql` at the database,
     * whichever's configured.
     * @tparam T the connectable type being written.
     * @param operation_name the write operation's name (`"insert"`/`"update"`/`"upsert"`), used
     * to tag the `connector.write.*` event published once the outcome is known.
     * @param value the row being written — used to derive the cache key and the local-store key.
     * @param sql the pre-built SQL statement to run when a database is configured.
     * @param callback gets the write outcome.
     */
    template <typename T>
    void write_through(std::string_view operation_name, const T &value, const std::string &sql,
                       std::move_only_function<void(bool)> callback) {
        // Cache gets the write unconditionally — insert/update/upsert all funnel through here.
        active_cache().set(serde::Cache::cache_key(value), serde::Cache::cache_value(value),
                           [](std::string_view) {});
        // No database — the local store is the source of truth, upsert it there.
        if (!m_database) {
            get_local_store<T>().insert_or_assign(serde::Cache::pk_string(value), value);
            core::events::publish(
                "connector.write.completed",
                {{"operation", std::string{operation_name}}, {"backend", "local"}});
            callback(true);
            return;
        }
        // Database configured — run the pre-built SQL and report whether it landed.
        active_database().query(
            sql, [callback = std::move(callback),
                  operation_name = std::string{operation_name}](std::string_view result) mutable {
                bool const ok = !result.empty();
                core::events::publish(ok ? "connector.write.completed" : "connector.write.failed",
                                      {{"operation", operation_name}, {"backend", "database"}});
                callback(ok);
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
    std::mutex m_pending_mutex;
    std::move_only_function<void()> m_wake;
    /// @brief True only while on_execute() is running a popped op — lets enqueue() tell a
    /// reentrant push (from within that op's own completion callback) apart from an external one,
    /// so it can defer to on_execute()'s trailing reschedule instead of waking twice.
    bool m_executing{false};
    interfaces::ICache *m_cache{nullptr};
    interfaces::IDatabase *m_database{nullptr};
    std::unordered_map<std::type_index, std::any> m_local_stores;
};

} // namespace connector

#ifdef CONGELADO_TEST
namespace connector::tests {

// Minimal serde::IConnectable fixture — one PK field, one plain field — just enough to drive
// Sql's reflection walk and Connector's cache/local-store paths without a real database.
class SqlTestRecord {
  public:
    SqlTestRecord() = default;

    void set_id(std::string id) { m_id = std::move(id); }
    void set_label(std::string label) { m_label = std::move(label); }

    [[nodiscard]] const std::string &get_id() const noexcept { return m_id; }
    [[nodiscard]] const std::string &get_label() const noexcept { return m_label; }

  private:
    std::string m_id;
    std::string m_label;
};

// Trivial synchronous in-memory ICache — every call resolves its callback immediately, no
// network/socket involved, just enough for Connector's write-through/cache-aside paths.
class InMemoryCache : public interfaces::ICache {
  public:
    [[nodiscard]] std::string_view backend_name() const noexcept override { return "test_cache"; }

    void get(std::string_view key, shared::QueryReadFn &&result) noexcept override {
        auto found = m_store.find(std::string{key});
        result(found != m_store.end() ? std::string_view{found->second} : std::string_view{});
    }
    void set(std::string_view key, std::string_view value, shared::QueryReadFn &&result) noexcept override {
        m_store[std::string{key}] = std::string{value};
        result("ok");
    }
    void remove(std::string_view key, shared::QueryReadFn &&result) noexcept override {
        m_store.erase(std::string{key});
        result("ok");
    }

  private:
    std::unordered_map<std::string, std::string> m_store;
};

// Deferred/cancellable-callback mock: get() STASHES the completion callback instead of invoking
// it, simulating a slow backend whose async op is still in flight when the caller moves on. Used
// to demonstrate — structurally, via this mock's own state — that Connector has no way to cancel
// or invalidate a pending callback when it's destroyed mid-flight.
class DeferredCache : public interfaces::ICache {
  public:
    [[nodiscard]] std::string_view backend_name() const noexcept override { return "deferred_cache"; }

    void get(std::string_view /*key*/, shared::QueryReadFn &&result) noexcept override {
        m_pending_get = std::move(result);
    }
    void set(std::string_view /*key*/, std::string_view /*value*/,
             shared::QueryReadFn &&result) noexcept override {
        result("ok");
    }
    void remove(std::string_view /*key*/, shared::QueryReadFn &&result) noexcept override {
        result("ok");
    }

    /** @brief Whether an async get() completion is still sitting here, unresolved. */
    [[nodiscard]] bool has_pending_get() const noexcept {
        return static_cast<bool>(m_pending_get);
    }

  private:
    shared::QueryReadFn m_pending_get;
};

} // namespace connector::tests

template <>
struct serde::Serializable<connector::tests::SqlTestRecord> {
    static constexpr auto fields() {
        return std::tuple{
            serde::FieldDesc<"id", &connector::tests::SqlTestRecord::get_id,
                             &connector::tests::SqlTestRecord::set_id,
                             serde::FieldOptions::init().with_db(serde::FieldOptionsDb::init().pk())>{},
            serde::FieldDesc<"label", &connector::tests::SqlTestRecord::get_label,
                             &connector::tests::SqlTestRecord::set_label>{},
        };
    }
    static constexpr std::string_view table_name() { return "sql_test_records"; }
};

namespace connector::tests {
using namespace boost::ut;

suite<"QueryOptions"> query_options_suite = [] {
    "starts empty"_test = [] {
        QueryOptions options;

        expect(options.get_joins().empty());
        expect(options.get_where_conditions().empty());
        expect(options.get_order_by_clauses().empty());
        expect(not options.get_limit().has_value());
    };

    "add_join/add_where/add_order_by accumulate in call order"_test = [] {
        QueryOptions options;
        options.add_join("JOIN b ON a.id = b.a_id");
        options.add_where("a.active = true");
        options.add_where("b.deleted = false");
        options.add_order_by("a.created_at");
        options.add_order_by("b.name", false);

        expect(options.get_joins().size() == 1);
        expect(options.get_where_conditions().size() == 2);
        expect(options.get_where_conditions()[0] == "a.active = true");
        expect(options.get_where_conditions()[1] == "b.deleted = false");
        expect(options.get_order_by_clauses().size() == 2);
        expect(options.get_order_by_clauses()[0].first == "a.created_at");
        expect(options.get_order_by_clauses()[0].second == true);
        expect(options.get_order_by_clauses()[1].second == false);
    };

    "set_limit is not cumulative — only the last call wins"_test = [] {
        QueryOptions options;
        options.set_limit(10);
        options.set_limit(5);

        expect(options.get_limit().value() == 5);
    };

    "every adder returns *this for chaining"_test = [] {
        QueryOptions options;
        options.add_join("JOIN x").add_where("y = 1").add_order_by("z").set_limit(3);

        expect(options.get_joins().size() == 1);
        expect(options.get_where_conditions().size() == 1);
        expect(options.get_order_by_clauses().size() == 1);
        expect(options.get_limit().value() == 3);
    };
};

suite<"Sql"> sql_suite = [] {
    "build_*_sql reports a clean error when no dialect plugin is registered"_test = [] {
        auto *previous = serde::SerdeFormatRegistry::get_active();
        serde::SerdeFormatRegistry::set_active(nullptr);

        auto create_sql = Sql::build_create_sql<SqlTestRecord>();
        auto select_sql = Sql::build_select_sql<SqlTestRecord>("some-id");

        serde::SerdeFormatRegistry::set_active(previous);

        expect(create_sql.contains("no format plugin loaded"));
        expect(create_sql.contains(std::string{connector::SQL_DIALECT_CONTENT_TYPE}));
        expect(select_sql.contains("no format plugin loaded"));
    };
};

suite<"Connector"> connector_suite = [] {
    "insert then find round-trips through the local store and cache"_test = [] {
        InMemoryCache cache;
        Connector connector{&cache, nullptr};

        SqlTestRecord record;
        record.set_id("abc-1");
        record.set_label("hello");

        bool insert_ok = false;
        connector.insert(record, [&insert_ok](bool ok) { insert_ok = ok; });

        std::optional<SqlTestRecord> found;
        connector.find<SqlTestRecord>(
            "abc-1", [&found](std::optional<SqlTestRecord> result) { found = std::move(result); });

        expect(insert_ok);
        expect(found.has_value()) << fatal;
        expect(found->get_label() == "hello");
    };

    "remove clears a previously inserted row from the local store"_test = [] {
        InMemoryCache cache;
        Connector connector{&cache, nullptr};

        SqlTestRecord record;
        record.set_id("abc-2");
        record.set_label("bye");
        connector.insert(record, [](bool) {});

        bool remove_ok = false;
        connector.remove<SqlTestRecord>("abc-2", [&remove_ok](bool ok) { remove_ok = ok; });

        std::optional<SqlTestRecord> found;
        connector.find<SqlTestRecord>(
            "abc-2", [&found](std::optional<SqlTestRecord> result) { found = std::move(result); });

        expect(remove_ok);
        expect(not found.has_value());
    };

    "find_all reflects every inserted row when there is no database"_test = [] {
        InMemoryCache cache;
        Connector connector{&cache, nullptr};

        SqlTestRecord first;
        first.set_id("all-1");
        first.set_label("one");
        SqlTestRecord second;
        second.set_id("all-2");
        second.set_label("two");
        connector.insert(first, [](bool) {});
        connector.insert(second, [](bool) {});

        std::vector<SqlTestRecord> all;
        connector.find_all<SqlTestRecord>(
            [&all](std::vector<SqlTestRecord> results) { all = std::move(results); });

        expect(all.size() == 2);
    };

    "get_name identifies this handler as 'connector'"_test = [] {
        Connector connector;
        expect(connector.get_name() == "connector");
    };

    // Regression/design-gap marker, NOT a fix: find()'s cache-aside callback chain captures a
    // raw `this` (see find()'s active_cache().get(cache_key_string, [this, ...]{ ... }) above).
    // Connector has no cancel()/invalidate() hook, and ICache::get() takes no cancellation
    // token, so a still-pending callback held by a slower backend simply outlives a destroyed
    // Connector with nothing to neuter it. This test proves that gap structurally, through the
    // mock's own state — it deliberately NEVER invokes the stored callback after the Connector
    // is destroyed, since actually doing so would be a real use-after-free.
    "destroying a Connector leaves an in-flight cache callback dangling with no cancellation hook"_test =
        [] {
            DeferredCache cache;
            {
                Connector connector{&cache, nullptr};
                connector.find<SqlTestRecord>("some-id", [](std::optional<SqlTestRecord>) {
                    // Never reached in this test — the mock never resolves the callback.
                });
                // No database configured, so enqueue() ran find()'s body synchronously; by now
                // active_cache().get() has already handed its `this`-capturing callback to the
                // mock, which stashed it instead of calling it.
                expect(cache.has_pending_get()) << fatal;
            } // `connector` destroyed here — nothing reaches into `cache` to cancel anything.

            // The callback is still sitting there, fully intact, capturing a `this` that now
            // points at a destroyed Connector. Nothing in Connector's or ICache's API could have
            // invalidated it even if it wanted to.
            expect(cache.has_pending_get());
        };
};

} // namespace connector::tests
#endif
