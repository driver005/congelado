module;

#define CONGELADO_GUEST
#include <congelado/plugin.h>
#include <libpq-fe.h>

export module postgres_plugin;

import congelado_plugin;
import interfaces;
import shared;
import core_logger;
import core_events;
import std;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

class PostgresPlugin : public congelado::Plugin,
                       public interfaces::IDatabase,
                       public interfaces::ISearchProvider {
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
     * @brief Flags this as both a storage-capable AND search-capable plugin, so the host wires
     * both `storage_get`/`search_get` into the `_cap_dispatch` routing — this plugin reuses its
     * own `IDatabase` connection for the search read-model tables rather than opening a second
     * one, so both capabilities live on the same instance.
     * @return `CONGELADO_CAP_STORAGE | CONGELADO_CAP_SEARCH`.
     */
    [[nodiscard]] std::uint32_t capabilities() const noexcept override {
        return CONGELADO_CAP_STORAGE | CONGELADO_CAP_SEARCH;
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
            // core::logger reaches this project's own LoggerRegistry fan-out (e.g. file_logger)
            // — a separate sink from host.log above (which only reaches whatever plugin-provided
            // logger is wired in via the ABI), not a redundant duplicate of it.
            core::logger::warning("postgres", "connect to {}:{}/{} failed: {}", HOST_S, PORT_S,
                                  DBNAME, PQerrorMessage(m_conn));
            core::events::publish("postgres.connection.failed",
                                 {{"host", HOST_S}, {"port", PORT_S}, {"dbname", DBNAME}});
            PQfinish(m_conn);
            m_conn = nullptr;
        } else {
            if (host.log != nullptr) {
                host.log(host.ctx, 2, "postgres plugin loaded", 22);
            }
            core::logger::debug("postgres", "connected to {}:{}/{}", HOST_S, PORT_S, DBNAME);
            core::events::publish("postgres.connection.established",
                                 {{"host", HOST_S}, {"port", PORT_S}, {"dbname", DBNAME}});
            ensure_search_tables();
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
     * @brief Capability hook the host calls to get at this plugin's `ISearchProvider` surface —
     * reuses the same libpq connection `storage_get()` exposes, just a different interface view
     * over it.
     * @return this instance, upcast to `interfaces::ISearchProvider*`.
     */
    void *search_get() noexcept { return static_cast<interfaces::ISearchProvider *>(this); }

    /**
     * @brief Identifies this db backend.
     * @return `"postgres"`.
     */
    [[nodiscard]] std::string_view backend_name() const noexcept override { return "postgres"; }
    /**
     * @brief Says whether this backend is load-bearing.
     * @return always `true` — postgres is a hard requirement here, no optional motion.
     */
    [[nodiscard]] bool required() const noexcept { return true; }
    /**
     * @brief Says whether libpq actually has a live connection right now.
     * @return true if `on_load`'s `PQconnectdb` succeeded and the connection hasn't been torn
     * down; false if it failed (`m_conn` left null — see `on_load`'s comment) or `on_unload` ran.
     */
    [[nodiscard]] bool is_connected() const noexcept override { return m_conn != nullptr; }

    /**
     * @brief Runs `sql` through libpq and forwards the outcome.
     * @note For a SELECT (`PGRES_TUPLES_OK`), `callback` gets the real result set — every row
     * marshaled into a JSON array of column-name → cell-text objects (nulls as JSON `null`) —
     * since `shared::QueryReadFn` only carries a single string, that JSON text *is* the whole
     * result set. For a command with no rows (`PGRES_COMMAND_OK` — INSERT/UPDATE/DELETE/DDL),
     * `callback` still just gets `"ok"`, unchanged from before.
     * @param sql the query text to run.
     * @param callback gets the JSON row array on a successful SELECT, `"ok"` on a successful
     * command, `""` on failure or if there's no live connection.
     */
    void query(std::string_view sql, shared::QueryReadFn &&callback) noexcept override {
        exec(sql, std::move(callback));
    }
    /**
     * @brief Runs `sql` through libpq and forwards the outcome.
     * @note Same `exec()` path as `query()` — this only ever runs command-style SQL (no rows to
     * return), so `callback` only ever sees `"ok"`/`""` in practice, same as before.
     * @param sql the insert statement to run.
     * @param callback gets `"ok"` on success, `""` on failure or if there's no live connection.
     */
    void insert(std::string_view sql, shared::QueryReadFn &&callback) noexcept override {
        exec(sql, std::move(callback));
    }
    /**
     * @brief Runs `sql` through libpq and forwards the outcome.
     * @note Same `exec()` path as `query()` — this only ever runs command-style SQL (no rows to
     * return), so `callback` only ever sees `"ok"`/`""` in practice, same as before.
     * @param sql the update statement to run.
     * @param callback gets `"ok"` on success, `""` on failure or if there's no live connection.
     */
    void update(std::string_view sql, shared::QueryReadFn &&callback) noexcept override {
        exec(sql, std::move(callback));
    }
    /**
     * @brief Runs `sql` through libpq and forwards the outcome.
     * @note Same `exec()` path as `query()` — this only ever runs command-style SQL (no rows to
     * return), so `callback` only ever sees `"ok"`/`""` in practice, same as before.
     * @param sql the delete statement to run.
     * @param callback gets `"ok"` on success, `""` on failure or if there's no live connection.
     */
    void remove(std::string_view sql, shared::QueryReadFn &&callback) noexcept override {
        exec(sql, std::move(callback));
    }

    /**
     * @brief Upserts a document into `search_documents`, keyed by (collection, id).
     * @param collection which set of documents this belongs to — a plain value in this table,
     * never spliced into SQL as an identifier, so it needs no sanitization.
     * @param id the document's id within `collection`.
     * @param document_json the document, already JSON-encoded by the caller — stored as-is in
     * a `JSONB` column, never destructured into individual SQL columns.
     * @param callback gets `"ok"` on success, `""` on failure or if there's no live connection.
     */
    void index(std::string_view collection, std::string_view id, std::string_view document_json,
              shared::QueryReadFn &&callback) noexcept override {
        auto sql = std::format(
            "INSERT INTO search_documents (collection, id, data, updated_at) "
            "VALUES ('{}', '{}', '{}'::jsonb, now()) "
            "ON CONFLICT (collection, id) DO UPDATE SET data = excluded.data, "
            "updated_at = excluded.updated_at",
            escape_sql_literal(collection), escape_sql_literal(id), escape_sql_literal(document_json));
        exec(sql, std::move(callback));
    }
    /**
     * @brief Deletes a document from the index.
     * @param collection which set of documents this belongs to.
     * @param id the document's id within `collection`.
     * @param callback gets `"ok"` on success, `""` on failure or if there's no live connection.
     */
    void remove(std::string_view collection, std::string_view id,
               shared::QueryReadFn &&callback) noexcept override {
        auto sql = std::format("DELETE FROM search_documents WHERE collection = '{}' AND id = '{}'",
                               escape_sql_literal(collection), escape_sql_literal(id));
        exec(sql, std::move(callback));
    }
    /**
     * @brief Searches documents within one collection.
     * @param collection which set of documents to search.
     * @param query the search request — `free_text` scans the JSONB column as text via `ILIKE`;
     * `query`, if set, is spliced directly into the `WHERE` clause as a raw SQL boolean
     * expression (same trust boundary as every other raw-SQL payload `IDatabase` already
     * accepts on this plugin — the caller, not an end user, is responsible for what it
     * contains). `sort` is ignored for now — results always come back newest-`updated_at`-first;
     * flagged as a simplification, not silently dropped.
     * @param callback gets a JSON array of matched documents (`"[]"` for zero hits), or `""` on
     * failure or if there's no live connection.
     */
    void search(std::string_view collection, const interfaces::SearchQuery &query,
               shared::QueryReadFn &&callback) noexcept override {
        if (m_conn == nullptr) {
            core::logger::warning("postgres", "search skipped, no live connection: {}", collection);
            callback("");
            return;
        }
        std::string where = std::format("collection = '{}'", escape_sql_literal(collection));
        if (!query.free_text.empty()) {
            where += std::format(" AND data::text ILIKE '%{}%'", escape_sql_literal(query.free_text));
        }
        // SECURITY: SQL injection. Unlike free_text just above, query.query is spliced into the
        // WHERE clause with zero escaping — it's taken verbatim from the request body
        // (SearchRequestBody.query, see plugins/manager/engine/src/handler/search.cppm) and
        // reaches PQexec() unparameterized. PQexec supports stacked statements, so this is full
        // arbitrary-SQL execution (not just boolean-blind), reachable unauthenticated via
        // POST /api/v1/tasks/search and POST /api/v1/workflow/search. Needs parameterization or
        // an allow-listed query grammar before this is safe to expose.
        if (!query.query.empty()) {
            where += std::format(" AND ({})", query.query);
        }
        auto sql = std::format(
            "SELECT data FROM search_documents WHERE {} ORDER BY updated_at DESC LIMIT {} OFFSET {}",
            where, query.size, query.start);
        select_documents(sql, std::move(callback));
    }

  private:
    PGconn *m_conn{nullptr};

    /**
     * @brief Executes raw SQL against the live connection and reports the outcome — real row
     * data for a SELECT, a bare ok/empty signal for everything else.
     * @param sql the statement text to execute.
     * @param callback receives: the result set as JSON (see rows_to_json()) for
     * `PGRES_TUPLES_OK`; `"ok"` for `PGRES_COMMAND_OK`; `""` for a dead connection, a failed
     * query, or a thrown exception.
     */
    void exec(std::string_view sql, shared::QueryReadFn callback) noexcept {
        // Dead connection means an instant empty result — no point even attempting the query.
        if (m_conn == nullptr) {
            core::logger::warning("postgres", "exec skipped, no live connection: {}", sql);
            callback("");
            return;
        }
        core::logger::debug("postgres", "executing: {}", sql);
        try {
            std::string statement{sql};
            PGresult *result = PQexec(m_conn, statement.c_str());
            auto st = PQresultStatus(result);
            if (st == PGRES_TUPLES_OK) {
                // SELECT — hand the real rows back instead of collapsing them away.
                auto row_count = PQntuples(result);
                core::logger::debug("postgres", "returned {} row(s)", row_count);
                callback(rows_to_json(result));
            } else if (st == PGRES_COMMAND_OK) {
                core::logger::debug("postgres", "command ok, {} row(s) affected",
                                    PQcmdTuples(result));
                callback("ok");
            } else {
                core::logger::warning("postgres", "exec failed: {}", PQresultErrorMessage(result));
                core::events::publish("postgres.exec.failed",
                                     {{"sql", std::string{sql}},
                                      {"error", PQresultErrorMessage(result)}});
                callback("");
            }
            PQclear(result);
        } catch (...) {
            core::logger::warning("postgres", "exec threw for: {}", sql);
            core::events::publish("postgres.exec.exception", {{"sql", std::string{sql}}});
            callback("");
        }
    }

    /**
     * @brief Creates the one generic search-document table if it doesn't already exist — called
     * once right after a successful connect. Every collection this backend ever serves shares
     * this single table, discriminated by the `collection` column — a document's `data` column
     * holds the whole JSON blob verbatim, so a caller-side document shape change needs no
     * matching migration here.
     */
    void ensure_search_tables() noexcept {
        exec("CREATE TABLE IF NOT EXISTS search_documents ("
             "collection TEXT NOT NULL, id TEXT NOT NULL, data JSONB NOT NULL, "
             "updated_at TIMESTAMPTZ NOT NULL DEFAULT now(), "
             "PRIMARY KEY (collection, id))",
             [](std::string_view) {});
    }

    /**
     * @brief Escapes a value for embedding inside a single-quoted SQL string literal — doubles
     * embedded single quotes, standard SQL escaping (assumes `standard_conforming_strings`, the
     * postgres default since 9.1, so backslashes need no special handling).
     * @param value the raw text to escape.
     * @return `value` with every `'` doubled, still missing the surrounding quotes.
     */
    [[nodiscard]] static std::string escape_sql_literal(std::string_view value) {
        std::string out;
        out.reserve(value.size());
        for (char character : value) {
            if (character == '\'') {
                out += "''";
            } else {
                out += character;
            }
        }
        return out;
    }

    /**
     * @brief Runs `sql` (expected to `SELECT data FROM search_documents ...`) and hands back the
     * matched rows as a flat JSON array — bypasses `exec()`/`rows_to_json()` on purpose: those
     * wrap each cell in a JSON *string* (double-encoding an already-JSON `data` value), where
     * this needs the raw JSONB text spliced straight into the array unescaped, since postgres
     * already emits it as canonical JSON text.
     * @param sql the SELECT statement to run.
     * @param callback gets the matched rows as a JSON array (`"[]"` for zero hits), or `""` on
     * failure.
     */
    void select_documents(std::string_view sql, shared::QueryReadFn &&callback) noexcept {
        core::logger::debug("postgres", "search executing: {}", sql);
        try {
            std::string statement{sql};
            PGresult *result = PQexec(m_conn, statement.c_str());
            if (PQresultStatus(result) != PGRES_TUPLES_OK) {
                core::logger::warning("postgres", "search failed: {}", PQresultErrorMessage(result));
                core::events::publish("postgres.search.failed",
                                     {{"sql", std::string{sql}},
                                      {"error", PQresultErrorMessage(result)}});
                PQclear(result);
                callback("");
                return;
            }
            auto row_count = PQntuples(result);
            std::string out = "[";
            for (int row = 0; row < row_count; ++row) {
                if (row > 0) {
                    out += ",";
                }
                out += PQgetisnull(result, row, 0) != 0 ? "null" : PQgetvalue(result, row, 0);
            }
            out += "]";
            PQclear(result);
            callback(out);
        } catch (...) {
            core::logger::warning("postgres", "search threw for: {}", sql);
            core::events::publish("postgres.search.exception", {{"sql", std::string{sql}}});
            callback("");
        }
    }

    /**
     * @brief Escapes a single cell value for embedding in a JSON string literal — quotes,
     * backslashes, and control characters, the same minimal set `std::format`-based JSON
     * elsewhere in this codebase (e.g. serde's own writers) also has to handle by hand at this
     * level, since this plugin has no JSON library import of its own (just `libpq-fe.h` and
     * `std`) and pulling one in for a single call site isn't worth it.
     * @param value the raw cell text to escape.
     * @return `value`, JSON-string-literal-safe (still missing the surrounding quotes).
     */
    [[nodiscard]] static std::string escape_json(std::string_view value) {
        std::string out;
        out.reserve(value.size());
        for (char character : value) {
            switch (character) {
            case '"':
                out += "\\\"";
                break;
            case '\\':
                out += "\\\\";
                break;
            case '\n':
                out += "\\n";
                break;
            case '\r':
                out += "\\r";
                break;
            case '\t':
                out += "\\t";
                break;
            default:
                if (static_cast<unsigned char>(character) < 0x20) {
                    out += std::format("\\u{:04x}", static_cast<unsigned char>(character));
                } else {
                    out += character;
                }
            }
        }
        return out;
    }

    /**
     * @brief Marshals a `PGRES_TUPLES_OK` result into a JSON array of row objects, column name
     * to cell text, so the single string `shared::QueryReadFn` allows can carry the whole result
     * set back to the caller.
     * @param result a `PQexec()` result already confirmed to be `PGRES_TUPLES_OK` — column names
     * are read once via `PQfname()`, then every row's cells via `PQgetvalue()`/`PQgetisnull()`.
     * @return the result set as a JSON array of `{"column": "value", ...}` objects; a SQL `NULL`
     * cell becomes JSON `null`, unquoted.
     */
    [[nodiscard]] static std::string rows_to_json(PGresult *result) {
        const int ROW_COUNT = PQntuples(result);
        const int COLUMN_COUNT = PQnfields(result);

        std::vector<std::string> column_names;
        column_names.reserve(static_cast<std::size_t>(COLUMN_COUNT));
        for (int column = 0; column < COLUMN_COUNT; ++column) {
            column_names.emplace_back(PQfname(result, column));
        }

        std::string out = "[";
        for (int row = 0; row < ROW_COUNT; ++row) {
            if (row > 0) {
                out += ",";
            }
            out += "{";
            for (int column = 0; column < COLUMN_COUNT; ++column) {
                if (column > 0) {
                    out += ",";
                }
                out += std::format("\"{}\":", escape_json(column_names[static_cast<std::size_t>(column)]));
                if (PQgetisnull(result, row, column) != 0) {
                    out += "null";
                } else {
                    out += std::format("\"{}\"", escape_json(PQgetvalue(result, row, column)));
                }
            }
            out += "}";
        }
        out += "]";
        return out;
    }
};

CONGELADO_PLUGIN(PostgresPlugin);

#ifdef CONGELADO_TEST
namespace postgres_plugin_tests {
using namespace boost::ut;

/// @brief Findings #1 (`search()`'s `query.query` SQL injection, ~L240-ish, see the SECURITY
/// comment on it above) and #2 (`search()`'s unbounded LIMIT/OFFSET, same method) both live
/// entirely inside `search()`, gated behind `if (m_conn == nullptr)` at the very top — the
/// `where`/final `sql` strings are only ever built AFTER that check passes, and `m_conn` is a
/// private `PGconn*` only ever set by a real `PQconnectdb()` call inside `on_load()`. There's no
/// separable SQL-building helper to call directly (unlike e.g. `escape_sql_literal()`, which is
/// pure and testable on its own but isn't where either finding lives), and no logging hook fires
/// before that connection check either — `select_documents()`'s "search executing: {}" debug log
/// only runs once `m_conn` is already non-null, i.e. strictly after the vulnerable string is
/// already built and about to be handed to `PQexec()`. Faking `m_conn` non-null through some
/// test-only seam would let `search()` reach `PQexec()` on a garbage `PGconn*`, which is
/// undefined behavior, not a real test — so per this task's scope, the actual injected
/// `query.query`/`query.size`/`query.start` values are documented-and-skipped here rather than
/// forced through a live Postgres connection. What IS observable without a live connection — the
/// fail-safe early-return path `search()` takes for ANY query shape, injection-shaped or not —
/// is covered below instead, just to confirm that path doesn't itself misbehave (e.g. crash) on
/// attacker-shaped input before a connection even exists.
suite<"PostgresPlugin::search fail-safe path (findings #1/#2 — documented-skip, see comment above)">
    postgres_search_failsafe_suite = [] {
    "search with no live connection short-circuits to an empty result before ever building SQL, even with an injection-shaped query.query"_test = [] {
        PostgresPlugin plugin;
        interfaces::SearchQuery query{.query = "'; DROP TABLE workflow_definitions; --",
                                      .free_text = "",
                                      .start = 0,
                                      .size = 100,
                                      .sort = ""};
        std::optional<std::string> observed;
        plugin.search("workflow_summaries", query,
                      [&](std::string_view result) { observed = std::string{result}; });
        expect(observed.has_value()) << fatal;
        expect(observed->empty());
    };

    "search with no live connection short-circuits to an empty result even with an unbounded query.size/query.start"_test = [] {
        PostgresPlugin plugin;
        interfaces::SearchQuery query{.query = "",
                                      .free_text = "",
                                      .start = std::numeric_limits<std::uint32_t>::max(),
                                      .size = std::numeric_limits<std::uint32_t>::max(),
                                      .sort = ""};
        std::optional<std::string> observed;
        plugin.search("workflow_summaries", query,
                      [&](std::string_view result) { observed = std::string{result}; });
        expect(observed.has_value()) << fatal;
        expect(observed->empty());
    };
};

/// @brief Finding #3: `on_load()`'s `CONNSTR` is built via unescaped `std::format` interpolation
/// of config values into libpq's `keyword='value'` connstring grammar. Unlike findings #1/#2
/// above, this one IS observable without a live Postgres connection — it needs no separable
/// helper because `on_load()` already has a real seam for it: `host.log`, the host-provided
/// logging callback, which `on_load()` calls with `std::format("postgres: {}", ...)` wrapping
/// `PQerrorMessage(m_conn)` on any connect failure. A `password` value containing an unescaped
/// `'` breaks the `keyword='value'` structure itself (the value's closing quote lands mid-string,
/// leaving trailing characters libpq's connstring grammar doesn't expect) — libpq's own parser
/// rejects this as a syntax error before ever attempting a socket, so this is a fast,
/// deterministic, network-free failure, not a real connection attempt against a live server.
suite<"PostgresPlugin::on_load connstring interpolation (finding #3)"> postgres_connstr_suite = [] {
    "on_load's unescaped connstring interpolation breaks on a single quote embedded in the password config value, observed via host.log"_test = [] {
        PostgresPlugin plugin;

        const char *keys[] = {"user", "password", "host", "dbname", "port"};    // NOLINT(cppcoreguidelines-avoid-c-arrays)
        const char *values[] = {"postgres", "abc'req", "localhost", "congelado", "5432"};    // NOLINT(cppcoreguidelines-avoid-c-arrays)
        CongeladoConfigView cfg{.keys = keys, .values = values, .count = 5};

        std::string captured_log;
        CongeladoHostCallbacks host{};
        host.ctx = &captured_log;
        host.log = [](void *ctx, int /*level*/, const char *msg, std::size_t len) {
            *static_cast<std::string *>(ctx) = std::string{msg, len};
        };

        plugin.on_load(host, cfg);

        // A correctly-escaped connstring would have either connected fine or failed with a
        // *connection*-refused-style error (no server listening) — here the embedded quote
        // breaks the keyword='value' grammar itself, a *syntax* error libpq catches during
        // parsing, before any socket is ever opened.
        expect(!plugin.is_connected());
        expect(!captured_log.empty()) << fatal;
        expect(captured_log.starts_with("postgres: "));
    };
};

} // namespace postgres_plugin_tests
#endif
