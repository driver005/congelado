export module engine:search_handler;

import std;
import interfaces;
import serde;
import :context;
#ifdef CONGELADO_TEST
import io_layer_http2;
import boost.ut;
#endif

export namespace engine {

/// @brief Search collection names — the wire contract shared with the workflow_orchestrator
/// plugin's SummaryProjector (which writes these collections on every terminal transition).
/// Kept in sync with that plugin's copy; they must match for search to read what the projector
/// wrote.
inline constexpr std::string_view WORKFLOW_SUMMARY_COLLECTION = "workflow_summaries";
inline constexpr std::string_view TASK_SUMMARY_COLLECTION = "task_summaries";

/// @brief Request body for `POST /api/v1/{workflow,tasks}/search` — a plain getter/setter DTO
/// mirroring `interfaces::SearchQuery`'s fields (that struct itself has no serde spec of its
/// own; it's a cross-cutting interface type with plain public members, not this plugin's to
/// reflect over), converted to a real SearchQuery once deserialized.
class SearchRequestBody
{
public:
    SearchRequestBody() = default;

    void set_query(std::string value)
    {
        m_query = std::move(value);
    }

    void set_free_text(std::string value)
    {
        m_free_text = std::move(value);
    }

    void set_start(std::uint32_t value) noexcept
    {
        m_start = value;
    }

    void set_size(std::uint32_t value) noexcept
    {
        m_size = value;
    }

    void set_sort(std::string value)
    {
        m_sort = std::move(value);
    }

    [[nodiscard]] const std::string& get_query() const noexcept
    {
        return m_query;
    }

    [[nodiscard]] const std::string& get_free_text() const noexcept
    {
        return m_free_text;
    }

    [[nodiscard]] std::uint32_t get_start() const noexcept
    {
        return m_start;
    }

    [[nodiscard]] std::uint32_t get_size() const noexcept
    {
        return m_size;
    }

    [[nodiscard]] const std::string& get_sort() const noexcept
    {
        return m_sort;
    }

    [[nodiscard]] interfaces::SearchQuery to_search_query() const
    {
        return interfaces::SearchQuery{
            .query = m_query,
            .free_text = m_free_text,
            .start = m_start,
            .size = m_size,
            .sort = m_sort
        };
    }

private:
    std::string m_query;
    std::string m_free_text;
    std::uint32_t m_start{0};
    std::uint32_t m_size{100};
    std::string m_sort;
};

// Routes:
//   POST /api/v1/workflow/search   → search_workflows
//   POST /api/v1/tasks/search      → search_tasks
// POST-with-body, not GET-with-query-string — IRequest has no query-string parsing anywhere in
// this codebase to build on (same reasoning as every other Phase 6+ route that needed
// structured input, e.g. ScheduleHandler's next_few_runs or WorkflowHandler's bulk ops). Both
// routes degrade gracefully (200 with an empty array) rather than erroring when no search
// backend is configured — search is optional infra, same story as EngineContext's db/lua_bridge
// slots.
class SearchHandler
{
public:
    explicit SearchHandler(EngineContext& ctx) noexcept :
        m_ctx{ctx}
    {
    }

    /// @brief Handles `POST /api/v1/workflow/search` against the WorkflowSummary projection.
    void search_workflows(
        interfaces::io::IRequest& req, interfaces::io::IResponse& res, std::function<void()> send
    )
    {
        run_search(req, res, std::move(send), WORKFLOW_SUMMARY_COLLECTION);
    }

    /// @brief Handles `POST /api/v1/tasks/search` against the TaskSummary projection.
    void search_tasks(
        interfaces::io::IRequest& req, interfaces::io::IResponse& res, std::function<void()> send
    )
    {
        run_search(req, res, std::move(send), TASK_SUMMARY_COLLECTION);
    }

private:
    std::reference_wrapper<EngineContext> m_ctx;

    void run_search(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send,
        std::string_view collection
    )
    {
        auto accept = req.find_header("accept");
        auto content_type = req.find_header("content-type");
        auto body = flatten_body(req);
        auto parsed = serde::Ser::deserialize<SearchRequestBody>(content_type, body);
        interfaces::SearchQuery query =
            parsed ? parsed->to_search_query() : interfaces::SearchQuery{};

        auto* provider = m_ctx.get().get_search();
        if (provider == nullptr) {
            reply(res, serde::Ser::serialize_raw(accept, "[]"));
            send();
            return;
        }
        provider->search(
            collection, query, [&res, accept, send = std::move(send)](std::string_view result) {
                reply(res, serde::Ser::serialize_raw(accept, result.empty() ? "[]" : result));
                send();
            }
        );
    }

    static void reply(
        interfaces::io::IResponse& res,
        std::vector<std::byte> bytes,
        interfaces::io::types::Status status = interfaces::io::types::Status::OK
    ) noexcept
    {
        res.set_body(std::move(bytes));
        res.set_status(status);
    }

    static std::string flatten_body(interfaces::io::IRequest& req) noexcept
    {
        std::string out;
        auto& view = req.get_body();
        out.reserve(view.size());
        for (std::byte byte: view) {
            out += static_cast<char>(byte);
        }
        return out;
    }
};

} // namespace engine

template<>
struct serde::Serializable<engine::SearchRequestBody>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "query", &engine::SearchRequestBody::get_query,
                &engine::SearchRequestBody::set_query>{},
            serde::FieldDesc<
                "free_text", &engine::SearchRequestBody::get_free_text,
                &engine::SearchRequestBody::set_free_text>{},
            serde::FieldDesc<
                "start", &engine::SearchRequestBody::get_start,
                &engine::SearchRequestBody::set_start>{},
            serde::FieldDesc<
                "size", &engine::SearchRequestBody::get_size,
                &engine::SearchRequestBody::set_size>{},
            serde::FieldDesc<
                "sort", &engine::SearchRequestBody::get_sort,
                &engine::SearchRequestBody::set_sort>{},
        };
    }
};

#ifdef CONGELADO_TEST
namespace engine::search_handler_tests {
using namespace boost::ut;

// Hands back whatever canned result was configured, and records which collection/query it was
// last called with — enough to drive both the "no provider" default and a "provider configured"
// success path for search_workflows()/search_tasks(). Signature spelled out with the raw
// std::move_only_function<void(std::string_view)> (rather than shared::QueryReadFn, its alias)
// since this file doesn't import `shared` — same convention EngineContext's own test block
// (context.cppm's FakeSearchProvider) already uses for the same reason.
class FakeSearchProvider final : public interfaces::ISearchProvider
{
public:
    explicit FakeSearchProvider(std::string result) :
        m_result{std::move(result)}
    {
    }

    [[nodiscard]] std::string_view backend_name() const noexcept override
    {
        return "fake_search";
    }

    void index(
        std::string_view,
        std::string_view,
        std::string_view,
        std::move_only_function<void(std::string_view)>&& callback
    ) noexcept override
    {
        callback("ok");
    }

    void remove(
        std::string_view,
        std::string_view,
        std::move_only_function<void(std::string_view)>&& callback
    ) noexcept override
    {
        callback("ok");
    }

    void search(
        std::string_view collection,
        const interfaces::SearchQuery& /*query*/,
        std::move_only_function<void(std::string_view)>&& callback
    ) noexcept override
    {
        m_last_collection = std::string{collection};
        callback(m_result);
    }

    [[nodiscard]] const std::string& last_collection() const noexcept
    {
        return m_last_collection;
    }

private:
    std::string m_result;
    std::string m_last_collection;
};

/// @brief Flattens a response's body back into a plain string — safe here because run_search()
/// always replies via serde::Ser::serialize_raw(), which never depends on a registered format
/// plugin, so the body is real, comparable text even with no format plugin loaded.
[[nodiscard]] std::string body_to_string(interfaces::io::IResponse& res)
{
    std::string out;
    auto& view = res.get_body();
    out.reserve(view.size());
    for (std::byte byte: view) {
        out += static_cast<char>(byte);
    }
    return out;
}

suite<"SearchRequestBody"> search_request_body_suite = [] {
    "default-constructs with empty query/free_text/sort, start 0, size 100"_test = [] {
        engine::SearchRequestBody body;
        expect(body.get_query().empty());
        expect(body.get_free_text().empty());
        expect(body.get_start() == 0);
        expect(body.get_size() == 100);
        expect(body.get_sort().empty());
    };

    "set_query/get_query round-trip"_test = [] {
        engine::SearchRequestBody body;
        body.set_query("status:RUNNING");
        expect(body.get_query() == "status:RUNNING");
    };

    "set_free_text/get_free_text round-trip"_test = [] {
        engine::SearchRequestBody body;
        body.set_free_text("hello world");
        expect(body.get_free_text() == "hello world");
    };

    "set_start/get_start round-trip"_test = [] {
        engine::SearchRequestBody body;
        body.set_start(20);
        expect(body.get_start() == 20);
    };

    "set_size/get_size round-trip"_test = [] {
        engine::SearchRequestBody body;
        body.set_size(10);
        expect(body.get_size() == 10);
    };

    "set_sort/get_sort round-trip"_test = [] {
        engine::SearchRequestBody body;
        body.set_sort("created_at:desc");
        expect(body.get_sort() == "created_at:desc");
    };

    "to_search_query copies every field across into a real SearchQuery"_test = [] {
        engine::SearchRequestBody body;
        body.set_query("q");
        body.set_free_text("ft");
        body.set_start(5);
        body.set_size(15);
        body.set_sort("s");

        auto query = body.to_search_query();

        expect(query.query == "q");
        expect(query.free_text == "ft");
        expect(query.start == 5);
        expect(query.size == 15);
        expect(query.sort == "s");
    };
};

suite<"SearchHandler"> search_handler_suite = [] {
    "search_workflows replies 200 with an empty array when no search provider is configured"_test =
        [] {
            engine::EngineContext ctx;
            engine::SearchHandler handler{ctx};
            io::layer::http2::HttpRequest req{1};
            io::layer::http2::HttpResponse res{1};
            bool sent = false;

            handler.search_workflows(req, res, [&sent] {
                sent = true;
            });

            expect(sent);
            expect(res.get_status() == interfaces::io::types::Status::OK);
            expect(body_to_string(res) == "[]");
        };

    "search_tasks replies 200 with an empty array when no search provider is configured"_test = [] {
        engine::EngineContext ctx;
        engine::SearchHandler handler{ctx};
        io::layer::http2::HttpRequest req{1};
        io::layer::http2::HttpResponse res{1};
        bool sent = false;

        handler.search_tasks(req, res, [&sent] {
            sent = true;
        });

        expect(sent);
        expect(res.get_status() == interfaces::io::types::Status::OK);
        expect(body_to_string(res) == "[]");
    };

    "search_workflows replies 200 with the provider's result, searching the workflow_summaries collection"_test =
        [] {
            engine::EngineContext ctx;
            FakeSearchProvider provider{R"([{"exec_id":"e1"}])"};
            ctx.set_search(&provider);
            engine::SearchHandler handler{ctx};
            io::layer::http2::HttpRequest req{1};
            io::layer::http2::HttpResponse res{1};
            bool sent = false;

            handler.search_workflows(req, res, [&sent] {
                sent = true;
            });

            expect(sent);
            expect(res.get_status() == interfaces::io::types::Status::OK);
            expect(body_to_string(res) == R"([{"exec_id":"e1"}])");
            expect(provider.last_collection() == engine::WORKFLOW_SUMMARY_COLLECTION);
        };

    "search_tasks replies 200 with the provider's result, searching the task_summaries collection"_test =
        [] {
            engine::EngineContext ctx;
            FakeSearchProvider provider{R"([{"task_id":"t1"}])"};
            ctx.set_search(&provider);
            engine::SearchHandler handler{ctx};
            io::layer::http2::HttpRequest req{1};
            io::layer::http2::HttpResponse res{1};
            bool sent = false;

            handler.search_tasks(req, res, [&sent] {
                sent = true;
            });

            expect(sent);
            expect(res.get_status() == interfaces::io::types::Status::OK);
            expect(body_to_string(res) == R"([{"task_id":"t1"}])");
            expect(provider.last_collection() == engine::TASK_SUMMARY_COLLECTION);
        };

    "search_workflows falls back to an empty array when the provider reports failure (empty result)"_test =
        [] {
            engine::EngineContext ctx;
            FakeSearchProvider provider{""};
            ctx.set_search(&provider);
            engine::SearchHandler handler{ctx};
            io::layer::http2::HttpRequest req{1};
            io::layer::http2::HttpResponse res{1};
            bool sent = false;

            handler.search_workflows(req, res, [&sent] {
                sent = true;
            });

            expect(sent);
            expect(res.get_status() == interfaces::io::types::Status::OK);
            expect(body_to_string(res) == "[]");
        };
};

} // namespace engine::search_handler_tests
#endif
