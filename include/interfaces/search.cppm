export module interfaces:search;

import std;
import shared;

export namespace interfaces {

/// @brief A read-model search request — free_text and/or a backend-specific structured query
/// string, paginated. Shape mirrors Conductor's own search query params
/// (query/freeText/start/size/sort) — `query`'s grammar is left up to whichever backend is
/// actually active (SQL `WHERE`-style for postgres, Lucene-style for Elasticsearch).
struct SearchQuery
{
    std::string query;
    std::string free_text;
    std::uint32_t start{0};
    std::uint32_t size{100};
    std::string sort;
};

/// @brief Pluggable, domain-agnostic document index/search. Modeled on `IDatabase`'s
/// single-active-backend shape, not a registry of many — a deployment picks exactly one search
/// backend at a time. Every payload crossing this interface is an opaque, backend-agnostic
/// string (a JSON-encoded document for index calls, a JSON array of matched documents for
/// search calls) — same "opaque string in, opaque string out" idiom `IDatabase` already uses
/// for SQL text. `collection` names WHICH set of documents an operation applies to (e.g. an
/// engine plugin might use "workflow_summaries"/"task_summaries") — this interface has no
/// concept of workflows or tasks at all, so any plugin can reuse it for any kind of document.
class ISearchProvider
{
public:
    /**
     * @brief Virtual dtor, default's good — polymorphic search backends clean up fine through
     * the base pointer, no extra motion needed.
     */
    virtual ~ISearchProvider() = default;
    ISearchProvider() = default;
    ISearchProvider(const ISearchProvider&) = delete;
    ISearchProvider& operator=(const ISearchProvider&) = delete;
    ISearchProvider(ISearchProvider&&) = delete;
    ISearchProvider& operator=(ISearchProvider&&) = delete;

    /**
     * @brief Tells you which search backend is actually running the show behind this interface
     * (postgres, elasticsearch, whatever got plugged in).
     * @return the backend's name.
     */
    [[nodiscard]] virtual std::string_view backend_name() const noexcept = 0;

    /**
     * @brief Upserts a document into the index.
     * @param collection which set of documents this belongs to (caller-defined namespace).
     * @param id the document's id within `collection` — index() is an upsert keyed on
     * (collection, id).
     * @param document_json the document, already JSON-encoded by the caller.
     * @param callback gets `"ok"` on success, `""` on failure.
     */
    virtual void index(
        std::string_view collection,
        std::string_view id,
        std::string_view document_json,
        shared::QueryReadFn&& callback
    ) noexcept = 0;
    /**
     * @brief Removes a document from the index.
     * @param collection which set of documents this belongs to.
     * @param id the document's id within `collection`.
     * @param callback gets `"ok"` on success, `""` on failure.
     */
    virtual void remove(
        std::string_view collection, std::string_view id, shared::QueryReadFn&& callback
    ) noexcept = 0;
    /**
     * @brief Searches documents within one collection.
     * @param collection which set of documents to search.
     * @param query the search request.
     * @param callback gets a JSON array of matched documents (`"[]"` for zero hits), or `""` on
     * failure.
     */
    virtual void search(
        std::string_view collection, const SearchQuery& query, shared::QueryReadFn&& callback
    ) noexcept = 0;
};

} // namespace interfaces
