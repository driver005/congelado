export module engine:query;

import std;
import interfaces;
import shared;
import serde;
import :context;

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
