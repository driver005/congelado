module;

#define CONGELADO_GUEST
#include <congelado/plugin.h>
#include <curl/curl.h>

export module elasticsearch_plugin;

import congelado_plugin;
import interfaces;
import shared;
import core_events;
import core_logger;
import std;

namespace {

/// @brief Accumulates a libcurl response body — the `CURLOPT_WRITEFUNCTION` callback appends
/// every chunk libcurl hands it into `*static_cast<std::string*>(userdata)`.
std::size_t write_callback(char *ptr, std::size_t size, std::size_t nmemb, void *userdata) {
    auto *out = static_cast<std::string *>(userdata);
    out->append(ptr, size * nmemb);
    return size * nmemb;
}

} // namespace

/**
 * @brief `ISearchProvider` backed by Elasticsearch's REST API — talks HTTP directly via libcurl
 * rather than this codebase's own `IClient`/router stack, matching `postgres_plugin`'s own
 * precedent of linking a client library straight and managing its connection itself: no
 * ready-made "connect to an arbitrary host:port, send one HTTP request" abstraction exists
 * anywhere in this codebase to build on (`IClient` implementations are all router/protocol-layer
 * integrated), so this is the established escape hatch, not a one-off shortcut.
 */
class ElasticsearchPlugin : public congelado::Plugin, public interfaces::ISearchProvider {
  public:
    [[nodiscard]] std::string_view get_name() const noexcept override { return "elasticsearch"; }
    [[nodiscard]] std::string_view get_version() const noexcept override { return "0.1.0"; }
    [[nodiscard]] std::uint32_t capabilities() const noexcept override {
        return CONGELADO_CAP_SEARCH;
    }

    /**
     * @brief Reads connection params out of config and brings up libcurl's global state.
     * @warning A single ES node, not a cluster host list — a genuine simplification, not a
     * silent gap: multi-host failover would need its own retry/round-robin logic this pass
     * doesn't build. `hosts` names exactly one base URL.
     * @param host unused — this plugin doesn't read any host callback fields.
     * @param cfg this plugin's config view; reads `hosts` (default `http://localhost:9200`),
     * `index_prefix` (default `congelado_`), and optional `username`/`password` for HTTP basic
     * auth.
     */
    void on_load(CongeladoHostCallbacks const & /*host*/,
                CongeladoConfigView const &cfg) override {
        m_base_url = congelado::config_get(cfg, "hosts").value_or("http://localhost:9200");
        m_index_prefix = congelado::config_get(cfg, "index_prefix").value_or("congelado_");
        m_username = congelado::config_get(cfg, "username").value_or("");
        m_password = congelado::config_get(cfg, "password").value_or("");
        curl_global_init(CURL_GLOBAL_DEFAULT);
        core::logger::debug("elasticsearch", "configured against {}", m_base_url);
    }

    /// @brief Tears down libcurl's global state.
    void on_unload() noexcept override { curl_global_cleanup(); }

    /**
     * @brief Capability hook the host calls to get at this plugin's `ISearchProvider` surface.
     * @return this instance, upcast to `interfaces::ISearchProvider*`.
     */
    void *search_get() noexcept { return static_cast<interfaces::ISearchProvider *>(this); }

    [[nodiscard]] std::string_view backend_name() const noexcept override {
        return "elasticsearch";
    }
    [[nodiscard]] bool required() const noexcept override { return false; }

    /**
     * @brief Upserts a document via `PUT {index}/_doc/{id}`.
     * @param collection maps to the ES index `{index_prefix}{collection}`.
     * @param id the document id.
     * @param document_json the document body, already JSON-encoded by the caller.
     * @param callback gets `"ok"` on a 2xx response, `""` otherwise.
     */
    void index(std::string_view collection, std::string_view id, std::string_view document_json,
              shared::QueryReadFn &&callback) noexcept override {
        auto url = std::format("{}/{}{}/_doc/{}", m_base_url, m_index_prefix, collection, id);
        auto [status, body] = request("PUT", url, document_json);
        static_cast<void>(body);
        callback(status >= 200 && status < 300 ? "ok" : "");
    }

    /**
     * @brief Removes a document via `DELETE {index}/_doc/{id}`.
     * @param collection maps to the ES index `{index_prefix}{collection}`.
     * @param id the document id.
     * @param callback gets `"ok"` on a 2xx or 404 (already gone counts as removed) response,
     * `""` otherwise.
     */
    void remove(std::string_view collection, std::string_view id,
               shared::QueryReadFn &&callback) noexcept override {
        auto url = std::format("{}/{}{}/_doc/{}", m_base_url, m_index_prefix, collection, id);
        auto [status, body] = request("DELETE", url, "");
        static_cast<void>(body);
        callback((status >= 200 && status < 300) || status == 404 ? "ok" : "");
    }

    /**
     * @brief Searches via `POST {index}/_search` — `free_text` becomes a `query_string` clause
     * over every field, `query`, if set, is spliced directly into the request body's `query`
     * object as a raw ES Query-DSL fragment (same trust boundary `postgres_plugin`'s own
     * `query`-splicing takes — the caller, not an end user, owns what it contains). `sort` is
     * ignored for now, same documented simplification `postgres_plugin` takes.
     * @param collection maps to the ES index `{index_prefix}{collection}`.
     * @param query the search request.
     * @param callback gets a JSON array of matched `_source` documents (`"[]"` for zero hits),
     * or `""` on failure.
     */
    void search(std::string_view collection, const interfaces::SearchQuery &query,
               shared::QueryReadFn &&callback) noexcept override {
        auto url = std::format("{}/{}{}/_search", m_base_url, m_index_prefix, collection);
        std::string clause = query.query.empty()
                                 ? R"({"match_all":{}})"
                                 : query.query;
        if (!query.free_text.empty()) {
            clause = std::format(
                R"({{"bool":{{"must":[{},{{"query_string":{{"query":"{}"}}}}]}}}})", clause,
                escape_json(query.free_text));
        }
        auto body = std::format(R"({{"from":{},"size":{},"query":{}}})", query.start, query.size,
                                clause);
        auto [status, response] = request("POST", url, body);
        if (status < 200 || status >= 300) {
            callback("");
            return;
        }
        callback(extract_sources(response));
    }

  private:
    std::string m_base_url;
    std::string m_index_prefix;
    std::string m_username;
    std::string m_password;

    /**
     * @brief Fires one blocking HTTP request via libcurl.
     * @param method the HTTP verb (`GET`/`PUT`/`POST`/`DELETE`).
     * @param url the full request URL.
     * @param body the request body, empty for a bodyless request.
     * @return the response's HTTP status code (0 on a transport-level failure) and body text.
     */
    [[nodiscard]] std::pair<long, std::string> request(const char *method, std::string_view url,
                                                       std::string_view body) const noexcept {
        CURL *curl = curl_easy_init();
        if (curl == nullptr) {
            return {0, ""};
        }
        std::string response;
        std::string url_owned{url};
        std::string body_owned{body};
        struct curl_slist *headers = nullptr;
        headers = curl_slist_append(headers, "Content-Type: application/json");

        curl_easy_setopt(curl, CURLOPT_URL, url_owned.c_str());
        curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, method);
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10L);
        if (!body_owned.empty()) {
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, body_owned.c_str());
            curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, static_cast<long>(body_owned.size()));
        }
        if (!m_username.empty()) {
            curl_easy_setopt(curl, CURLOPT_USERNAME, m_username.c_str());
            curl_easy_setopt(curl, CURLOPT_PASSWORD, m_password.c_str());
        }

        auto result = curl_easy_perform(curl);
        long status = 0;
        if (result == CURLE_OK) {
            curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &status);
        } else {
            core::logger::warning("elasticsearch", "{} {} failed: {}", method, url_owned,
                                  curl_easy_strerror(result));
            core::events::publish("elasticsearch.request_failed",
                                  {{"method", std::string{method}},
                                   {"url", url_owned},
                                   {"error", curl_easy_strerror(result)}});
        }
        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
        return {status, std::move(response)};
    }

    /**
     * @brief Escapes a value for embedding inside a JSON string literal — same minimal
     * quote/backslash/control-character set `postgres_plugin`'s own `escape_json()` handles, for
     * the same reason (no JSON library linked here, just libcurl and `std`).
     * @param value the raw text to escape.
     * @return `value`, JSON-string-literal-safe, still missing the surrounding quotes.
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
            default:
                out += character;
            }
        }
        return out;
    }

    /**
     * @brief Pulls every `"_source":{...}` object out of a raw `_search` response body and
     * rebuilds them into one flat JSON array — the shape `ISearchProvider::search()` promises,
     * matching what `postgres_plugin`'s own backend returns for the same call. Hand-rolled
     * brace/string-aware scanning (no JSON library linked here) rather than a regex, since a
     * `_source` object can itself contain arbitrarily nested braces and escaped quotes.
     * @param response the raw ES `_search` response body.
     * @return a JSON array of the matched documents' `_source` objects, `"[]"` if none were
     * found.
     */
    [[nodiscard]] static std::string extract_sources(std::string_view response) {
        static constexpr std::string_view MARKER = "\"_source\":";
        std::string out = "[";
        bool first = true;
        std::size_t pos = 0;
        while (true) {
            auto marker_pos = response.find(MARKER, pos);
            if (marker_pos == std::string_view::npos) {
                break;
            }
            auto brace_start = response.find('{', marker_pos + MARKER.size());
            if (brace_start == std::string_view::npos) {
                break;
            }
            auto brace_end = matching_brace(response, brace_start);
            if (brace_end == std::string_view::npos) {
                break;
            }
            if (!first) {
                out += ",";
            }
            first = false;
            out += response.substr(brace_start, brace_end - brace_start + 1);
            pos = brace_end + 1;
        }
        out += "]";
        return out;
    }

    /**
     * @brief Finds the index of the `}` matching the `{` at `open`, tracking nesting depth and
     * skipping over quoted-string contents (including escaped characters) so a brace inside a
     * string value doesn't throw off the count.
     * @param text the text to scan.
     * @param open the index of the opening `{`.
     * @return the matching `}`'s index, or `std::string_view::npos` if the text ends unbalanced.
     */
    [[nodiscard]] static std::size_t matching_brace(std::string_view text, std::size_t open) {
        int depth = 0;
        bool in_string = false;
        for (std::size_t index = open; index < text.size(); ++index) {
            char character = text[index];
            if (in_string) {
                if (character == '\\') {
                    ++index;
                } else if (character == '"') {
                    in_string = false;
                }
                continue;
            }
            if (character == '"') {
                in_string = true;
            } else if (character == '{') {
                ++depth;
            } else if (character == '}') {
                --depth;
                if (depth == 0) {
                    return index;
                }
            }
        }
        return std::string_view::npos;
    }
};

CONGELADO_PLUGIN(ElasticsearchPlugin);
