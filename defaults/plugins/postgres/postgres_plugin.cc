module;

#define CONGELADO_GUEST
#include <congelado/plugin.h>
#include <libpq-fe.h>

export module postgres_plugin;

import congelado_plugin;
import interfaces;
import shared;
import std;

class PostgresPlugin : public congelado::Plugin, public interfaces::IDatabase {
  public:
    /**
     * @brief Plugin name reported to the host.
     * @return `"postgres"`.
     */
    [[nodiscard]] std::string_view get_name() const noexcept override { return "postgres"; }
    /**
     * @brief Version string for this build of the postgres plugin.
     * @return `"0.1.0"`.
     */
    [[nodiscard]] std::string_view get_version() const noexcept override { return "0.1.0"; }
    /**
     * @brief Flags this as a storage-capable plugin, so the host wires `storage_get` into the
     * `_cap_dispatch` routing.
     * @return `CONGELADO_CAP_STORAGE`.
     */
    [[nodiscard]] std::uint32_t capabilities() const noexcept override {
        return CONGELADO_CAP_STORAGE;
    }

    /**
     * @brief Reads connection params out of config and opens the libpq connection.
     * @warning A failed connect doesn't take the process down — it just leaves `m_conn` null, so
     * `query`/`insert`/`update`/`remove` all silently degrade to reporting empty results instead
     * of a hard failure. Nothing downstream finds out the database is actually unreachable
     * unless it's watching the log line `host.log` writes here. Lowkey a quiet L for callers.
     * @param host the host callback table; used here only for `host.log` on connect
     * success/failure.
     * @param cfg this plugin's config view; reads `user` (default `postgres`), `password`
     * (default empty), `host` (default `localhost`), `dbname` (default `congelado`), and `port`
     * (default `5432`).
     */
    void on_load(CongeladoHostCallbacks const &host, CongeladoConfigView const &cfg) override {
        // Pull connection params out of config, falling back to sane local-dev defaults for
        // anything not set.
        const auto USER = std::string{congelado::config_get(cfg, "user").value_or("postgres")};
        const auto PASS = std::string{congelado::config_get(cfg, "password").value_or("")};
        const auto HOST_S = std::string{congelado::config_get(cfg, "host").value_or("localhost")};
        const auto DBNAME = std::string{congelado::config_get(cfg, "dbname").value_or("congelado")};
        const auto PORT_S = std::string{congelado::config_get(cfg, "port").value_or("5432")};

        // Build the libpq connstring and fire off the connect — synchronous, blocks on_load
        // until it resolves one way or the other.
        const auto CONNSTR = std::format("host='{}' port='{}' dbname='{}' user='{}' password='{}'",
                                         HOST_S, PORT_S, DBNAME, USER, PASS);
        m_conn = PQconnectdb(CONNSTR.c_str());
        // A bad connect doesn't take the process down — just log it, free the handle, and
        // leave m_conn null so every query later degrades to reporting empty instead of
        // crashing.
        if (PQstatus(m_conn) != CONNECTION_OK) {
            auto msg = std::format("postgres: {}", PQerrorMessage(m_conn));
            if (host.log != nullptr) {
                host.log(host.ctx, 3, msg.data(), msg.size());
            }
            PQfinish(m_conn);
            m_conn = nullptr;
        } else {
            if (host.log != nullptr) {
                host.log(host.ctx, 2, "postgres plugin loaded", 22);
            }
        }
    }

    /// @brief Closes the libpq connection if one's open — clean teardown, no leaked handle.
    void on_unload() noexcept override {
        if (m_conn != nullptr) {
            PQfinish(m_conn);
            m_conn = nullptr;
        }
    }

    /**
     * @brief Capability hook the host calls to get at this plugin's `IDatabase` surface.
     * @return this instance, upcast to `interfaces::IDatabase*`.
     */
    void *storage_get() noexcept { return static_cast<interfaces::IDatabase *>(this); }

    /**
     * @brief Identifies this db backend.
     * @return `"postgres"`.
     */
    [[nodiscard]] std::string_view backend_name() const noexcept override { return "postgres"; }
    /**
     * @brief Says whether this backend is load-bearing.
     * @return always `true` — postgres is a hard requirement here, no optional motion.
     */
    [[nodiscard]] bool required() const noexcept override { return true; }

    /**
     * @brief Runs `sql` through libpq and forwards the outcome.
     * @warning `callback` never sees real row data — `exec()` collapses every result down to `"ok"` or
     * `""`, so a SELECT's actual rows get pulled off the wire by libpq and then just thrown away.
     * Fine for existence/success checks, straight cooked for anything that needs the data back.
     * @param sql the query text to run.
     * @param callback gets `"ok"` on success, `""` on failure or if there's no live connection.
     */
    void query(std::string_view sql, shared::QueryReadFn &&callback) noexcept override {
        exec(sql, std::move(callback));
    }
    /**
     * @brief Runs `sql` through libpq and forwards the outcome.
     * @note Same `exec()` path as `query()` — `callback` only ever sees `"ok"`/`""`, never row data.
     * @param sql the insert statement to run.
     * @param callback gets `"ok"` on success, `""` on failure or if there's no live connection.
     */
    void insert(std::string_view sql, shared::QueryReadFn &&callback) noexcept override {
        exec(sql, std::move(callback));
    }
    /**
     * @brief Runs `sql` through libpq and forwards the outcome.
     * @note Same `exec()` path as `query()` — `callback` only ever sees `"ok"`/`""`, never row data.
     * @param sql the update statement to run.
     * @param callback gets `"ok"` on success, `""` on failure or if there's no live connection.
     */
    void update(std::string_view sql, shared::QueryReadFn &&callback) noexcept override {
        exec(sql, std::move(callback));
    }
    /**
     * @brief Runs `sql` through libpq and forwards the outcome.
     * @note Same `exec()` path as `query()` — `callback` only ever sees `"ok"`/`""`, never row data.
     * @param sql the delete statement to run.
     * @param callback gets `"ok"` on success, `""` on failure or if there's no live connection.
     */
    void remove(std::string_view sql, shared::QueryReadFn &&callback) noexcept override {
        exec(sql, std::move(callback));
    }

  private:
    PGconn *m_conn{nullptr};

    /**
     * @brief Executes raw SQL against the live connection and collapses the result down to a
     * bare ok/empty signal.
     * @param sql the statement text to execute.
     * @param callback receives `"ok"` if the command or tuple result came back clean, `""` for a dead
     * connection, a failed query, or a thrown exception.
     */
    void exec(std::string_view sql, shared::QueryReadFn callback) noexcept {
        // Dead connection means an instant empty result — no point even attempting the query.
        if (m_conn == nullptr) {
            callback("");
            return;
        }
        // Run the statement and collapse the result down to ok/empty — actual row data (for
        // SELECTs via PGRES_TUPLES_OK) gets pulled off the wire by PQexec and then discarded
        // right here, see the class-level @warning on query().
        try {
            std::string statement{sql};
            PGresult *result = PQexec(m_conn, statement.c_str());
            auto st = PQresultStatus(result);
            PQclear(result);
            callback((st == PGRES_COMMAND_OK || st == PGRES_TUPLES_OK) ? "ok" : "");
        } catch (...) {
            callback("");
        }
    }
};

CONGELADO_PLUGIN(PostgresPlugin);
