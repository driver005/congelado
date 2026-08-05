export module engine:search_handler;

import std;
import interfaces;
import serde;
import :context;
import :search_projector;

export namespace engine {

/// @brief Request body for `POST /api/v1/{workflow,tasks}/search` — a plain getter/setter DTO
/// mirroring `interfaces::SearchQuery`'s fields (that struct itself has no serde spec of its
/// own; it's a cross-cutting interface type with plain public members, not this plugin's to
/// reflect over), converted to a real SearchQuery once deserialized.
class SearchRequestBody {
  public:
    SearchRequestBody() = default;

    void set_query(std::string value) { m_query = std::move(value); }
    void set_free_text(std::string value) { m_free_text = std::move(value); }
    void set_start(std::uint32_t value) noexcept { m_start = value; }
    void set_size(std::uint32_t value) noexcept { m_size = value; }
    void set_sort(std::string value) { m_sort = std::move(value); }

    [[nodiscard]] const std::string &get_query() const noexcept { return m_query; }
    [[nodiscard]] const std::string &get_free_text() const noexcept { return m_free_text; }
    [[nodiscard]] std::uint32_t get_start() const noexcept { return m_start; }
    [[nodiscard]] std::uint32_t get_size() const noexcept { return m_size; }
    [[nodiscard]] const std::string &get_sort() const noexcept { return m_sort; }

    [[nodiscard]] interfaces::SearchQuery to_search_query() const {
        return interfaces::SearchQuery{
            .query = m_query, .free_text = m_free_text, .start = m_start, .size = m_size,
            .sort = m_sort};
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
// this codebase to build on (same reasoning as every other Phase 6+ route that needed structured
// input, e.g. ScheduleHandler's next_few_runs or WorkflowHandler's bulk ops). Both routes degrade
// gracefully (200 with an empty array) rather than erroring when no search backend is configured
// — search is optional infra, same story as EngineContext's db/lua_bridge slots.
class SearchHandler {
  public:
    explicit SearchHandler(EngineContext &ctx) noexcept : m_ctx{ctx} {}

    /// @brief Handles `POST /api/v1/workflow/search` against the WorkflowSummary projection.
    void search_workflows(interfaces::io::IRequest &req, interfaces::io::IResponse &res) {
        run_search(req, res, WORKFLOW_SUMMARY_COLLECTION);
    }

    /// @brief Handles `POST /api/v1/tasks/search` against the TaskSummary projection.
    void search_tasks(interfaces::io::IRequest &req, interfaces::io::IResponse &res) {
        run_search(req, res, TASK_SUMMARY_COLLECTION);
    }

  private:
    std::reference_wrapper<EngineContext> m_ctx;

    void run_search(interfaces::io::IRequest &req, interfaces::io::IResponse &res,
                    std::string_view collection) {
        auto accept = req.find_header("accept");
        auto content_type = req.find_header("content-type");
        auto body = flatten_body(req);
        auto parsed = serde::Ser::deserialize<SearchRequestBody>(content_type, body);
        interfaces::SearchQuery query =
            parsed ? parsed->to_search_query() : interfaces::SearchQuery{};

        auto *provider = m_ctx.get().get_search();
        if (provider == nullptr) {
            reply(res, serde::Ser::serialize_raw(accept, "[]"));
            return;
        }
        provider->search(collection, query, [&res, accept](std::string_view result) {
            reply(res, serde::Ser::serialize_raw(accept, result.empty() ? "[]" : result));
        });
    }

    static void
    reply(interfaces::io::IResponse &res, std::vector<std::byte> bytes,
          interfaces::io::types::Status status = interfaces::io::types::Status::OK) noexcept {
        res.set_body(std::move(bytes));
        res.set_status(status);
    }

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

template <>
struct serde::Serializable<engine::SearchRequestBody> {
    static constexpr auto fields() {
        return std::tuple{
            serde::FieldDesc<"query", &engine::SearchRequestBody::get_query,
                             &engine::SearchRequestBody::set_query>{},
            serde::FieldDesc<"free_text", &engine::SearchRequestBody::get_free_text,
                             &engine::SearchRequestBody::set_free_text>{},
            serde::FieldDesc<"start", &engine::SearchRequestBody::get_start,
                             &engine::SearchRequestBody::set_start>{},
            serde::FieldDesc<"size", &engine::SearchRequestBody::get_size,
                             &engine::SearchRequestBody::set_size>{},
            serde::FieldDesc<"sort", &engine::SearchRequestBody::get_sort,
                             &engine::SearchRequestBody::set_sort>{},
        };
    }
};
