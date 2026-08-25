export module engine:query;

import std;
import interfaces;
import shared;
import serde;
import :context;
#ifdef CONGELADO_TEST
import io_layer_http2;
import boost.ut;
#endif

export namespace engine {

// Routes:
//   POST /api/v1/query   → run_query
class QueryHandler {
  public:
    /**
     * @brief Builds a handler bound to the shared EngineContext — no state of its own beyond
     * that reference, run_query() leans on it purely to reach the configured database backend.
     * @param ctx the engine context to bind; caller keeps it alive for this handler's whole
     * lifetime.
     */
    explicit QueryHandler(EngineContext &ctx) noexcept : m_ctx{ctx} {}

    /**
     * @brief Handles `POST /api/v1/query` — runs the request body verbatim as SQL against the
     * configured database backend and returns the real result.
     * @warning This is a "viewer," not a general exec endpoint: `is_select()` rejects anything
     * that doesn't start with `SELECT` (case-insensitive, leading whitespace trimmed) with a 400
     * before it ever reaches the database — a simple keyword gate, not a real SQL parser, so it's
     * a speed bump against accidental DDL/DML from this box, not a hard security boundary.
     * @param req the inbound request; the raw body is the SQL text, Accept header picks the
     * response format.
     * @param res the response — 200 with the JSON row array on a successful SELECT, 400 if the
     * body isn't a SELECT, 503 if no database backend is configured, or 500 if the query itself
     * fails.
     */
    void run_query(interfaces::io::IRequest &req, interfaces::io::IResponse &res,
                   std::function<void()> send) noexcept {
        auto accept = req.find_header("accept");
        auto sql = flatten_body(req);

        // SECURITY: is_select() is a case-insensitive prefix check, not a SQL parser — PQexec
        // supports stacked statements, so a body like "SELECT 1; DROP TABLE x;" passes this gate
        // and executes the DROP. This route accepts the raw request body as SQL with no other
        // validation and (per routes.cppm) no authentication — effectively an unauthenticated SQL
        // console with a bypassable read-only fig-leaf. Needs a real parser/allow-list or removal.
        if (!is_select(sql)) {
            reply(res, serde::Ser::serialize_error(accept, "only SELECT statements are allowed"),
                  interfaces::io::types::Status::BAD_REQUEST);
            send();
            return;
        }

        auto *database = m_ctx.get().get_db();
        if (database == nullptr) {
            reply(res, serde::Ser::serialize_error(accept, "no database configured"),
                  interfaces::io::types::Status::SERVICE_UNAVAILABLE);
            send();
            return;
        }

        // Callback calls serde::Ser::serialize_raw()/serialize_error(), which can throw, so it's
        // wrapped instead of relying on this whole method staying noexcept through the callback.
        // `[&]` is safe here (unlike the Connector CRUD handlers): this is the *raw*
        // IDatabase::query(), which the postgres plugin invokes synchronously, inline, before
        // returning — the surrounding try/catch relies on exactly that — so the captured locals
        // are read while this frame is still alive.
        try {
            database->query(sql, [&](std::string_view result) {
                if (result.empty()) {
                    reply(res, serde::Ser::serialize_error(accept, "query failed"),
                          interfaces::io::types::Status::INTERNAL_SERVER_ERROR);
                    send();
                    return;
                }
                reply(res, serde::Ser::serialize_raw(accept, result));
                send();
            });
        } catch (...) {
            reply(res, serde::Ser::serialize_error(accept, "query failed"),
                  interfaces::io::types::Status::INTERNAL_SERVER_ERROR);
            send();
        }
    }

  private:
    std::reference_wrapper<EngineContext> m_ctx;

    /**
     * @brief Checks whether `sql` (leading whitespace trimmed) starts with `SELECT`,
     * case-insensitively — the whole of this handler's read-only enforcement.
     * @param sql the statement text to check.
     * @return true if `sql` looks like a SELECT, false otherwise (including an empty body).
     */
    [[nodiscard]] static bool is_select(std::string_view sql) noexcept {
        static constexpr std::string_view SELECT = "select";

        auto start = sql.find_first_not_of(" \t\r\n");
        if (start == std::string_view::npos || sql.size() - start < SELECT.size()) {
            return false;
        }
        auto head = sql.substr(start, SELECT.size());
        return std::ranges::equal(head, SELECT, [](char lhs, char rhs) {
            return std::tolower(static_cast<unsigned char>(lhs)) == rhs;
        });
    }

    /**
     * @brief Shared reply helper — writes `bytes` into the response body and sets the status,
     * defaulting to 200 OK when the caller doesn't hand over anything else.
     * @param res the response to fill in.
     * @param bytes the body bytes to write.
     * @param status the status code to set, defaults to OK.
     */
    static void
    reply(interfaces::io::IResponse &res, std::vector<std::byte> bytes,
          interfaces::io::types::Status status = interfaces::io::types::Status::OK) noexcept {
        res.set_body(std::move(bytes));
        res.set_status(status);
    }

    /**
     * @brief Copies the request body's byte view out into a plain `std::string` — same
     * flattening every other handler in this plugin needs before treating the body as text.
     * @param req the request whose body gets flattened.
     * @return the body bytes reinterpreted as a string, same length, same order.
     */
    static std::string flatten_body(interfaces::io::IRequest &req) noexcept {
        std::string out;
        auto &view = req.get_body();
        out.reserve(view.size());
        for (std::byte byte : view) {
            out += static_cast<char>(byte);
        }
        return out;
    }
};

} // namespace engine

#ifdef CONGELADO_TEST
namespace engine::query_handler_tests {
using namespace boost::ut;

// Hands back whatever canned result was configured — enough to drive both run_query()'s
// success path (non-empty result) and its "query failed" 500 path (empty result).
class FakeDatabase final : public interfaces::IDatabase {
  public:
    explicit FakeDatabase(std::string result) : m_result{std::move(result)} {}
    [[nodiscard]] std::string_view backend_name() const noexcept override { return "fake_db"; }
    void query(std::string_view, shared::QueryReadFn &&result) noexcept override {
        result(m_result);
    }
    void insert(std::string_view, shared::QueryReadFn &&result) noexcept override { result(""); }
    void update(std::string_view, shared::QueryReadFn &&result) noexcept override { result(""); }
    void remove(std::string_view, shared::QueryReadFn &&result) noexcept override { result(""); }

  private:
    std::string m_result;
};

/// @brief Copies a plain string into the byte vector run_query() reads as the SQL body — mirrors
/// Ser::to_bytes, just local to this test block.
[[nodiscard]] std::vector<std::byte> to_bytes(std::string_view text) {
    std::vector<std::byte> bytes;
    bytes.reserve(text.size());
    for (char character : text) {
        bytes.push_back(static_cast<std::byte>(character));
    }
    return bytes;
}

/// @brief Flattens a response's body back into a plain string — safe here because run_query()'s
/// success/failure replies go through serde::Ser::serialize_raw()/serialize_error(), neither of
/// which needs a registered format plugin, so the body is real, comparable text.
[[nodiscard]] std::string body_to_string(interfaces::io::IResponse &res) {
    std::string out;
    auto &view = res.get_body();
    out.reserve(view.size());
    for (std::byte byte : view) {
        out += static_cast<char>(byte);
    }
    return out;
}

suite<"QueryHandler"> query_handler_suite = [] {
    "run_query replies 400 when the body isn't a SELECT statement"_test = [] {
        engine::EngineContext ctx;
        engine::QueryHandler handler{ctx};
        io::layer::http2::HttpRequest req{1};
        io::layer::http2::HttpResponse res{1};
        req.set_body(to_bytes("DELETE FROM tasks"));
        bool sent = false;

        handler.run_query(req, res, [&sent] { sent = true; });

        expect(sent);
        expect(res.get_status() == interfaces::io::types::Status::BAD_REQUEST);
    };

    "run_query replies 400 for an empty body"_test = [] {
        engine::EngineContext ctx;
        engine::QueryHandler handler{ctx};
        io::layer::http2::HttpRequest req{1};
        io::layer::http2::HttpResponse res{1};
        bool sent = false;

        handler.run_query(req, res, [&sent] { sent = true; });

        expect(sent);
        expect(res.get_status() == interfaces::io::types::Status::BAD_REQUEST);
    };

    "run_query accepts a lowercase, leading-whitespace SELECT — reaches the backend check instead of BAD_REQUEST"_test =
        [] {
            engine::EngineContext ctx;
            engine::QueryHandler handler{ctx};
            io::layer::http2::HttpRequest req{1};
            io::layer::http2::HttpResponse res{1};
            req.set_body(to_bytes("  select 1"));
            bool sent = false;

            handler.run_query(req, res, [&sent] { sent = true; });

            expect(sent);
            // No db configured — is_select() let it through, so it fails past the gate on the
            // missing-backend check instead of BAD_REQUEST.
            expect(res.get_status() == interfaces::io::types::Status::SERVICE_UNAVAILABLE);
        };

    // SECURITY: pins the finding in the SECURITY comment above run_query() — is_select() is a
    // case-insensitive prefix check, not a parser, so a stacked-statement payload that merely
    // starts with SELECT sails past it. is_select() itself is private, so the bypass is driven
    // observably through run_query(): with no db configured, a payload the gate should have
    // rejected instead reaches the missing-backend check (503), same as a real SELECT would,
    // instead of BAD_REQUEST.
    "run_query's is_select() gate is bypassable via a stacked statement — SELECT-prefixed payload with a trailing DROP TABLE reaches the backend check instead of BAD_REQUEST"_test =
        [] {
            engine::EngineContext ctx;
            engine::QueryHandler handler{ctx};
            io::layer::http2::HttpRequest req{1};
            io::layer::http2::HttpResponse res{1};
            req.set_body(to_bytes("SELECT 1; DROP TABLE workflow_definitions;"));
            bool sent = false;

            handler.run_query(req, res, [&sent] { sent = true; });

            expect(sent);
            expect(res.get_status() == interfaces::io::types::Status::SERVICE_UNAVAILABLE);
        };

    "run_query replies 503 when no database backend is configured"_test = [] {
        engine::EngineContext ctx;
        engine::QueryHandler handler{ctx};
        io::layer::http2::HttpRequest req{1};
        io::layer::http2::HttpResponse res{1};
        req.set_body(to_bytes("SELECT * FROM tasks"));
        bool sent = false;

        handler.run_query(req, res, [&sent] { sent = true; });

        expect(sent);
        expect(res.get_status() == interfaces::io::types::Status::SERVICE_UNAVAILABLE);
    };

    "run_query replies 200 with the raw query result for a successful SELECT"_test = [] {
        engine::EngineContext ctx;
        FakeDatabase db{R"([{"id":"1"}])"};
        ctx.set_db(&db);
        engine::QueryHandler handler{ctx};
        io::layer::http2::HttpRequest req{1};
        io::layer::http2::HttpResponse res{1};
        req.set_body(to_bytes("SELECT * FROM tasks"));
        bool sent = false;

        handler.run_query(req, res, [&sent] { sent = true; });

        expect(sent);
        expect(res.get_status() == interfaces::io::types::Status::OK);
        expect(body_to_string(res) == R"([{"id":"1"}])");
    };

    "run_query replies 500 when the database comes back empty-handed"_test = [] {
        engine::EngineContext ctx;
        FakeDatabase db{""};
        ctx.set_db(&db);
        engine::QueryHandler handler{ctx};
        io::layer::http2::HttpRequest req{1};
        io::layer::http2::HttpResponse res{1};
        req.set_body(to_bytes("SELECT * FROM tasks"));
        bool sent = false;

        handler.run_query(req, res, [&sent] { sent = true; });

        expect(sent);
        expect(res.get_status() == interfaces::io::types::Status::INTERNAL_SERVER_ERROR);
    };
};

} // namespace engine::query_handler_tests
#endif
