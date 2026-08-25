module;
#ifdef CONGELADO_TEST
#include <rfl/Generic.hpp>
#include <rfl/json.hpp>
#endif

export module engine:metadata;

import std;
import interfaces;
import model;
import shared;
import serde;
import core_events;
import core_logger;
import :context;
#ifdef CONGELADO_TEST
import io_layer_http2;
import boost.ut;
#endif

export namespace engine {

// Routes:
//   GET /api/v1/metadata/tasks       → list_task_definitions
//   GET /api/v1/metadata/workflows   → list_workflow_definitions
//   GET /api/v1/metadata/health      → health_check
class MetadataHandler {
  public:
    /**
     * @brief Builds a handler bound to the shared EngineContext — no state of its own beyond
     * that reference, every route below just leans on `m_ctx` to reach the connector.
     * @note The find_all() callbacks below capture their locals explicitly by copy/move (not
     * `[&]`) since Connector::find_all() defers its callback to a later tick with a real db
     * backend. The health-check cache/db-ping callbacks stay `[&]` on purpose: those go through
     * the *raw* ICache::get()/IDatabase::query() (not the Connector's async queue), which the
     * redis/postgres plugins invoke synchronously, inline, before returning — so their captures
     * are read while this frame is still alive.
     * @param ctx the engine context to bind; caller keeps it alive for this handler's whole
     * lifetime.
     */
    explicit MetadataHandler(EngineContext &ctx) noexcept : m_ctx{ctx} {}

    /**
     * @brief Handles `GET /api/v1/metadata/tasks` — dumps every stored TaskDef, no filtering,
     * straight off Connector::find_all().
     * @param req the inbound request; only its Accept header gets read here.
     * @param res the response this writes the serialized task-definition list into.
     */
    void list_task_definitions(interfaces::io::IRequest &req, interfaces::io::IResponse &res,
                               std::function<void()> send) noexcept {
        // grab the Accept header up front so we know which format to serialize into
        auto accept = req.find_header("accept");

        // no filtering here — just dump every stored TaskDef straight to the client. Callback
        // is not noexcept: serde::Ser::serialize() may throw (alloc/format errors), and
        // Connector::find_all()'s callback parameter (std::move_only_function<void(...)>)
        // doesn't require a noexcept target.
        m_ctx.get().get_connector().find_all<model::TaskDef>(
            [&res, accept, send = std::move(send)](const std::vector<model::TaskDef> &tasks) {
                reply(res, serde::Ser::serialize(accept, tasks));
                send();
            });
    }

    /**
     * @brief Handles `GET /api/v1/metadata/workflows` — dumps every stored WorkflowDef, same
     * no-filter find_all() motion as list_task_definitions().
     * @param req the inbound request; only its Accept header gets read here.
     * @param res the response this writes the serialized workflow-definition list into.
     */
    void list_workflow_definitions(interfaces::io::IRequest &req, interfaces::io::IResponse &res,
                                   std::function<void()> send) noexcept {
        // same deal as list_task_definitions — just the Accept header, nothing else needed
        auto accept = req.find_header("accept");

        // no filtering, dump every stored WorkflowDef. Not noexcept — same reasoning as
        // list_task_definitions() above.
        m_ctx.get().get_connector().find_all<model::WorkflowDef>(
            [&res, accept, send = std::move(send)](const std::vector<model::WorkflowDef> &defs) {
                reply(res, serde::Ser::serialize(accept, defs));
                send();
            });
    }

    /**
     * @brief Handles `GET /api/v1/metadata/health` — cache-first health probe: serves the
     * cached `{"status":"ok"}` if one's sitting there, else pings the db (warming the cache
     * with the OK payload on the way out) if one's configured, else falls back to reporting no
     * backend wired up at all and logs a warning.
     * @warning The db ping's actual result is thrown away — the query callback's parameter is
     * deliberately left unnamed — so reaching the callback at all gets treated as "ok"
     * regardless of what the db returned. A db
     * that answers with an error body still reports healthy here. That's a real gap for a
     * health endpoint specifically, not just vibes — don't trust this one to catch a db that's
     * up but unhappy.
     * @param req the inbound request; only its Accept header gets read here.
     * @param res the response this writes the health payload into.
     */
    void health_check(interfaces::io::IRequest &req, interfaces::io::IResponse &res,
                      std::function<void()> send) noexcept {
        static constexpr std::string_view CACHE_KEY = "engine:health";
        static constexpr std::string_view OK_PAYLOAD = R"({"status":"ok"})";
        static constexpr std::string_view BARE = R"({"status":"ok","db":false,"cache":false})";

        auto accept = req.find_header("accept");

        // cache-first — if a cached "ok" is already sitting there, ship it and skip the db entirely
        if (m_ctx.get().get_cache() != nullptr) {
            bool done = false;
            m_ctx.get().get_cache()->get(CACHE_KEY, [&](std::string_view cached) noexcept {
                if (!cached.empty()) {
                    reply(res, serde::Ser::serialize_raw(accept, cached));
                    send();
                    done = true;
                }
            });
            if (done) {
                return;
            }
        }

        // no cache hit — fall back to pinging the db, warming the cache with the OK payload on the
        // way out
        if (m_ctx.get().get_db() != nullptr) {
            m_ctx.get().get_db()->query(
                R"({"op":"ping"})", [&](std::string_view /*result*/) noexcept {
                    if (m_ctx.get().get_cache()) {
                        m_ctx.get().get_cache()->set(CACHE_KEY, OK_PAYLOAD,
                                                     [](std::string_view) noexcept {});
                    }
                    reply(res, serde::Ser::serialize_raw(accept, OK_PAYLOAD));
                    send();
                });
            return;
        }

        // neither backend configured — log it and report the bare-bones status
        core::logger::warning("engine", "health: no db or cache");
        core::events::publish("engine.health.no_backend");
        reply(res, serde::Ser::serialize_raw(accept, BARE));
        send();
    }

  private:
    std::reference_wrapper<EngineContext> m_ctx;

    /**
     * @brief Shared reply helper — writes `bytes` into the response body and sets the status,
     * defaulting to 200 OK when the caller doesn't hand over anything else. Every route above
     * funnels its response through this one spot.
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
};

} // namespace engine

#ifdef CONGELADO_TEST
namespace engine::metadata_handler_tests {
using namespace boost::ut;

// Trivial synchronous in-memory ICache — Connector's write_through() path (used when seeding
// data directly through upsert() below) abort()s via active_cache() if no cache is wired in.
class FakeCache final : public interfaces::ICache {
  public:
    [[nodiscard]] std::string_view backend_name() const noexcept override { return "fake_cache"; }
    void get(std::string_view key, shared::QueryReadFn &&result) noexcept override {
        auto found = m_store.find(std::string{key});
        result(found != m_store.end() ? std::string_view{found->second} : std::string_view{});
    }
    void set(std::string_view key, std::string_view value,
             shared::QueryReadFn &&result) noexcept override {
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

// Always answers get() with the same canned value, regardless of key — enough to simulate a
// warm "engine:health" cache entry for health_check()'s cache-hit branch.
class CachedValueCache final : public interfaces::ICache {
  public:
    explicit CachedValueCache(std::string value) : m_value{std::move(value)} {}
    [[nodiscard]] std::string_view backend_name() const noexcept override {
        return "cached_value_cache";
    }
    void get(std::string_view, shared::QueryReadFn &&result) noexcept override {
        result(m_value);
    }
    void set(std::string_view, std::string_view, shared::QueryReadFn &&result) noexcept override {
        result("ok");
    }
    void remove(std::string_view, shared::QueryReadFn &&result) noexcept override {
        result("ok");
    }

  private:
    std::string m_value;
};

// Records whether query() ever got called — lets the cache-hit health_check() test prove the
// database branch was never reached.
class RecordingDatabase final : public interfaces::IDatabase {
  public:
    [[nodiscard]] std::string_view backend_name() const noexcept override {
        return "recording_db";
    }
    void query(std::string_view, shared::QueryReadFn &&result) noexcept override {
        m_query_called = true;
        result(R"({"status":"ok"})");
    }
    void insert(std::string_view, shared::QueryReadFn &&result) noexcept override { result(""); }
    void update(std::string_view, shared::QueryReadFn &&result) noexcept override { result(""); }
    void remove(std::string_view, shared::QueryReadFn &&result) noexcept override { result(""); }
    [[nodiscard]] bool was_queried() const noexcept { return m_query_called; }

  private:
    bool m_query_called{false};
};

// Minimal real ISerdeFormat, JSON via rfl::json directly (same recipe as json_plugin.cc's own
// JsonPlugin) — needed so list_task_definitions()/list_workflow_definitions() below can actually
// go through serde::Ser::serialize() and produce a real, countable JSON array instead of the
// "no format plugin loaded" error payload serialize() falls back to with nothing registered.
class MockJsonFormat final : public interfaces::ISerdeFormat {
  public:
    [[nodiscard]] std::string_view content_type() const noexcept override {
        return "application/json";
    }
    [[nodiscard]] std::string_view format_name() const noexcept override { return "mock-json"; }
    [[nodiscard]] std::expected<std::string, std::string>
    encode(const rfl::Generic &value) const override {
        return rfl::json::write(value);
    }
    [[nodiscard]] std::expected<rfl::Generic, std::string>
    decode(std::string_view data) const override {
        auto result = rfl::json::read<rfl::Generic>(data);
        if (!result) {
            return std::unexpected{result.error().what()};
        }
        return *result;
    }
};

// Reports every db ping as failed/error, but — same as RecordingDatabase — still invokes the
// query() callback, since health_check()'s callback parameter is unnamed/discarded. Pins that a
// db actively erroring on its ping still gets reported healthy.
class FailingDatabase final : public interfaces::IDatabase {
  public:
    [[nodiscard]] std::string_view backend_name() const noexcept override {
        return "failing_db";
    }
    void query(std::string_view, shared::QueryReadFn &&result) noexcept override {
        m_query_called = true;
        result(R"({"error":"connection refused"})");
    }
    void insert(std::string_view, shared::QueryReadFn &&result) noexcept override { result(""); }
    void update(std::string_view, shared::QueryReadFn &&result) noexcept override { result(""); }
    void remove(std::string_view, shared::QueryReadFn &&result) noexcept override { result(""); }
    [[nodiscard]] bool was_queried() const noexcept { return m_query_called; }

  private:
    bool m_query_called{false};
};

/// @brief Flattens a response's body bytes back into a plain string for content assertions —
/// safe here because health_check() replies via serde::Ser::serialize_raw(), which (unlike
/// serialize()) never depends on a registered format plugin, so the body is real, comparable
/// text even with no format plugin loaded in this test binary.
[[nodiscard]] std::string body_to_string(interfaces::io::IResponse &res) {
    std::string out;
    auto &view = res.get_body();
    out.reserve(view.size());
    for (std::byte byte : view) {
        out += static_cast<char>(byte);
    }
    return out;
}

suite<"MetadataHandler"> metadata_handler_suite = [] {
    "list_task_definitions replies 200 with an empty list on a freshly-constructed context"_test =
        [] {
            engine::EngineContext ctx;
            engine::MetadataHandler handler{ctx};
            io::layer::http2::HttpRequest req{1};
            io::layer::http2::HttpResponse res{1};
            bool sent = false;

            handler.list_task_definitions(req, res, [&sent] { sent = true; });

            expect(sent);
            expect(res.get_status() == interfaces::io::types::Status::OK);
        };

    "list_task_definitions replies 200 after a definition was upserted directly through the connector"_test =
        [] {
            engine::EngineContext ctx;
            FakeCache cache;
            ctx.set_cache(&cache);
            model::TaskDef seeded;
            seeded.set_name("echo");
            bool upserted = false;
            ctx.get_connector().upsert<model::TaskDef>(seeded,
                                                        [&upserted](bool oke) { upserted = oke; });
            expect(upserted) << fatal;

            engine::MetadataHandler handler{ctx};
            io::layer::http2::HttpRequest req{1};
            io::layer::http2::HttpResponse res{1};
            bool sent = false;

            handler.list_task_definitions(req, res, [&sent] { sent = true; });

            expect(sent);
            expect(res.get_status() == interfaces::io::types::Status::OK);
        };

    "list_workflow_definitions replies 200 with an empty list on a freshly-constructed context"_test =
        [] {
            engine::EngineContext ctx;
            engine::MetadataHandler handler{ctx};
            io::layer::http2::HttpRequest req{1};
            io::layer::http2::HttpResponse res{1};
            bool sent = false;

            handler.list_workflow_definitions(req, res, [&sent] { sent = true; });

            expect(sent);
            expect(res.get_status() == interfaces::io::types::Status::OK);
        };

    "list_workflow_definitions replies 200 after a definition was upserted directly through the connector"_test =
        [] {
            engine::EngineContext ctx;
            FakeCache cache;
            ctx.set_cache(&cache);
            model::WorkflowDef seeded;
            seeded.set_name("order_flow");
            bool upserted = false;
            ctx.get_connector().upsert<model::WorkflowDef>(
                seeded, [&upserted](bool oke) { upserted = oke; });
            expect(upserted) << fatal;

            engine::MetadataHandler handler{ctx};
            io::layer::http2::HttpRequest req{1};
            io::layer::http2::HttpResponse res{1};
            bool sent = false;

            handler.list_workflow_definitions(req, res, [&sent] { sent = true; });

            expect(sent);
            expect(res.get_status() == interfaces::io::types::Status::OK);
        };

    "health_check reports the bare status when neither db nor cache is configured"_test = [] {
        engine::EngineContext ctx;
        engine::MetadataHandler handler{ctx};
        io::layer::http2::HttpRequest req{1};
        io::layer::http2::HttpResponse res{1};
        bool sent = false;

        handler.health_check(req, res, [&sent] { sent = true; });

        expect(sent);
        expect(res.get_status() == interfaces::io::types::Status::OK);
        expect(body_to_string(res) == R"({"status":"ok","db":false,"cache":false})");
    };

    "health_check serves the cached payload straight away, without ever touching the database"_test =
        [] {
            engine::EngineContext ctx;
            CachedValueCache cache{R"({"status":"ok","cached":true})"};
            RecordingDatabase db;
            ctx.set_cache(&cache);
            ctx.set_db(&db);
            engine::MetadataHandler handler{ctx};
            io::layer::http2::HttpRequest req{1};
            io::layer::http2::HttpResponse res{1};
            bool sent = false;

            handler.health_check(req, res, [&sent] { sent = true; });

            expect(sent);
            expect(res.get_status() == interfaces::io::types::Status::OK);
            expect(body_to_string(res) == R"({"status":"ok","cached":true})");
            expect(!db.was_queried());
        };

    "health_check pings the database and reports ok when no cache is configured"_test = [] {
        engine::EngineContext ctx;
        RecordingDatabase db;
        ctx.set_db(&db);
        engine::MetadataHandler handler{ctx};
        io::layer::http2::HttpRequest req{1};
        io::layer::http2::HttpResponse res{1};
        bool sent = false;

        handler.health_check(req, res, [&sent] { sent = true; });

        expect(sent);
        expect(res.get_status() == interfaces::io::types::Status::OK);
        expect(body_to_string(res) == R"({"status":"ok"})");
        expect(db.was_queried());
    };

    "health_check reports healthy even when the database ping itself errors out, because the ping result is discarded"_test =
        [] {
            engine::EngineContext ctx;
            FailingDatabase db;
            ctx.set_db(&db);
            engine::MetadataHandler handler{ctx};
            io::layer::http2::HttpRequest req{1};
            io::layer::http2::HttpResponse res{1};
            bool sent = false;

            handler.health_check(req, res, [&sent] { sent = true; });

            expect(sent);
            expect(db.was_queried());
            // The db answered its ping with an error body — health_check() still reports the
            // same "ok" it would for a genuinely healthy db, because query()'s callback
            // parameter is unnamed/ignored. This pins the monitoring-bypass gap: nothing about
            // this response distinguishes a healthy db from one actively erroring.
            expect(res.get_status() == interfaces::io::types::Status::OK);
            expect(body_to_string(res) == R"({"status":"ok"})");
        };

    "list_task_definitions returns every seeded record with no pagination limit applied"_test =
        [] {
            engine::EngineContext ctx;
            FakeCache cache;
            ctx.set_cache(&cache);
            serde::SerdeFormatRegistry registry;
            registry.add_format(std::make_shared<MockJsonFormat>());
            serde::SerdeFormatRegistry::set_active(&registry);

            constexpr int seeded_count = 50;
            for (int i = 0; i < seeded_count; ++i) {
                model::TaskDef seeded;
                seeded.set_name(std::format("task_{}", i));
                bool upserted = false;
                ctx.get_connector().upsert<model::TaskDef>(
                    seeded, [&upserted](bool oke) { upserted = oke; });
                expect(upserted) << fatal;
            }

            engine::MetadataHandler handler{ctx};
            io::layer::http2::HttpRequest req{1};
            io::layer::http2::HttpResponse res{1};
            // list_task_definitions() never reads req.get_path() at all (only the Accept
            // header) — a page/limit/offset query param here would be silently inert even if
            // IRequest parsed query strings, which it doesn't anywhere in this codebase. Set
            // anyway to document that intent explicitly.
            req.set_header(interfaces::io::types::Token::PATH,
                           "/api/v1/metadata/tasks?page=1&limit=10");
            req.set_header("accept", "application/json");
            bool sent = false;

            handler.list_task_definitions(req, res, [&sent] { sent = true; });

            expect(sent);
            expect(res.get_status() == interfaces::io::types::Status::OK);
            auto parsed = rfl::json::read<rfl::Generic>(body_to_string(res));
            expect(parsed.has_value()) << fatal;
            auto array = parsed->to_array();
            expect(array.has_value()) << fatal;
            // Every one of the 50 seeded rows comes back, unbounded — pins the "no
            // pagination/limit exists" gap for GET /api/v1/metadata/tasks.
            expect(array->size() == seeded_count);

            serde::SerdeFormatRegistry::set_active(nullptr);
        };

    "list_workflow_definitions returns every seeded record with no pagination limit applied"_test =
        [] {
            engine::EngineContext ctx;
            FakeCache cache;
            ctx.set_cache(&cache);
            serde::SerdeFormatRegistry registry;
            registry.add_format(std::make_shared<MockJsonFormat>());
            serde::SerdeFormatRegistry::set_active(&registry);

            constexpr int seeded_count = 50;
            for (int i = 0; i < seeded_count; ++i) {
                model::WorkflowDef seeded;
                seeded.set_name(std::format("workflow_{}", i));
                bool upserted = false;
                ctx.get_connector().upsert<model::WorkflowDef>(
                    seeded, [&upserted](bool oke) { upserted = oke; });
                expect(upserted) << fatal;
            }

            engine::MetadataHandler handler{ctx};
            io::layer::http2::HttpRequest req{1};
            io::layer::http2::HttpResponse res{1};
            req.set_header(interfaces::io::types::Token::PATH,
                           "/api/v1/metadata/workflows?page=1&limit=10");
            req.set_header("accept", "application/json");
            bool sent = false;

            handler.list_workflow_definitions(req, res, [&sent] { sent = true; });

            expect(sent);
            expect(res.get_status() == interfaces::io::types::Status::OK);
            auto parsed = rfl::json::read<rfl::Generic>(body_to_string(res));
            expect(parsed.has_value()) << fatal;
            auto array = parsed->to_array();
            expect(array.has_value()) << fatal;
            // Every one of the 50 seeded rows comes back, unbounded — pins the "no
            // pagination/limit exists" gap for GET /api/v1/metadata/workflows.
            expect(array->size() == seeded_count);

            serde::SerdeFormatRegistry::set_active(nullptr);
        };
};

} // namespace engine::metadata_handler_tests
#endif
