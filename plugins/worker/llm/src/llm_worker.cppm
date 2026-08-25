module;

#include <curl/curl.h>

#ifdef CONGELADO_TEST
#include <rfl/Generic.hpp>
#include <rfl/json.hpp>
#endif

export module llm_worker;

import std;
import interfaces;
import serde;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace worker_llm {

/// @brief Typed input for the `llm` worker, parsed from the task's dynamic input value via
/// `serde::Ser::from_value` — see the `Serializable<LlmInput>` specialization below. `api_url`/`model`
/// default via in-class member initializers (used whenever the object has no such key); `prompt` is
/// required (checked after parsing).
class LlmInput {
  public:
    void setApiUrl(std::string value) { m_api_url = std::move(value); }
    void setApiKey(std::string value) { m_api_key = std::move(value); }
    void setModel(std::string value) { m_model = std::move(value); }
    void setPrompt(std::string value) { m_prompt = std::move(value); }

    [[nodiscard]] const std::string &getApiUrl() const noexcept { return m_api_url; }
    [[nodiscard]] const std::string &getApiKey() const noexcept { return m_api_key; }
    [[nodiscard]] const std::string &getModel() const noexcept { return m_model; }
    [[nodiscard]] const std::string &getPrompt() const noexcept { return m_prompt; }

  private:
    // BUG: same reflect-cpp gotcha documented on worker_hash::HashInput::m_algo — neither field
    // is `std::optional`, so `serde::Ser::from_value` requires ALL FOUR fields present in the
    // task input or the WHOLE decode fails; these in-class defaults are dead on the task-input
    // path despite the doc comment above claiming api_url/model "default via in-class member
    // initializers". A caller relying on "omit api_url to get OpenAI's default" gets a hard parse
    // error instead.
    std::string m_api_url{"https://api.openai.com/v1/chat/completions"};
    std::string m_api_key;
    std::string m_model{"gpt-4o-mini"};
    std::string m_prompt;
};

} // namespace worker_llm

template <>
struct serde::Serializable<worker_llm::LlmInput> {
    static constexpr auto fields() {
        using worker_llm::LlmInput;
        return std::tuple{
            serde::FieldDesc<"api_url", &LlmInput::getApiUrl, &LlmInput::setApiUrl>{},
            serde::FieldDesc<"api_key", &LlmInput::getApiKey, &LlmInput::setApiKey>{},
            serde::FieldDesc<"model", &LlmInput::getModel, &LlmInput::setModel>{},
            serde::FieldDesc<"prompt", &LlmInput::getPrompt, &LlmInput::setPrompt>{},
        };
    }
};

export namespace worker_llm {

/// @brief The `llm` worker — calls an OpenAI-compatible chat-completions endpoint (Conductor's LLM
/// text/chat system tasks), as a reusable IWorker via libcurl. Input: `prompt` (required), `api_key`,
/// `model` (default gpt-4o-mini), `api_url` (default OpenAI's). Output: `response` (raw JSON the
/// caller parses), `status` (HTTP code), `llm_status`. Kept minimal on purpose — a single-turn chat
/// call; richer AI tasks (embeddings, vector search) are their own workers later.
class LlmWorker final : public interfaces::IWorker {
  public:
    [[nodiscard]] std::string_view get_task_type() const noexcept override { return "llm"; }

    /// @brief No async curl integration exists in this codebase — this runs on the worker's own
    /// dedicated TaskQueue contract (see IWorker::run), so the blocking `curl_easy_perform` call
    /// never holds up the caller or the shared contract pool beyond this one contract.
    // BUG: unlike worker_email::EmailWorker::run() (which explicitly rejects an empty
    // 'smtp_url' before ever touching curl), this run() has NO post-parse validation at all — an
    // empty (but present) 'prompt', or an api_url/api_key/model that are empty strings, all sail
    // straight through to send_blocking() and a real outbound call. The only thing that stops a
    // call from firing is from_value() itself failing (a required key missing outright).
    void run(const serde::Value &input,
            interfaces::WorkerCompletion on_complete) override {
        auto parsed = serde::Ser::from_value<LlmInput>(input);
        if (!parsed) {
            on_complete(std::unexpected{interfaces::WorkerError{parsed.error()}});
            return;
        }
        on_complete(send_blocking(parsed->getApiUrl(), parsed->getApiKey(), parsed->getModel(),
                                  parsed->getPrompt()));
    }

  private:
    /// @brief The actual blocking chat-completions call — runs on this worker's own dedicated
    /// contract.
    static interfaces::WorkerResult
    send_blocking(const std::string &api_url, const std::string &api_key, const std::string &model,
                 const std::string &prompt) {
        static const int global_init = [] {
            curl_global_init(CURL_GLOBAL_DEFAULT);
            return 0;
        }();
        (void)global_init;

        // BUG: `model` is spliced into the JSON body raw — only `prompt` goes through
        // json_escape(). A model value containing a `"` breaks out of the JSON string and lets a
        // task submitter inject arbitrary extra JSON fields/messages into the request body sent
        // upstream (e.g. `model` = `x","messages":[{"role":"system","content":"..."}],"x":"`).
        std::string request_body = R"({"model":")" + model +
                                   R"(","messages":[{"role":"user","content":")" +
                                   json_escape(prompt) + R"("}]})";

        CURL *curl = curl_easy_init();
        if (curl == nullptr) {
            return std::unexpected{interfaces::WorkerError{"curl init failed"}};
        }
        std::string response;
        curl_slist *headers = curl_slist_append(nullptr, "Content-Type: application/json");
        if (!api_key.empty()) {
            headers = curl_slist_append(headers, ("Authorization: Bearer " + api_key).c_str());
        }
        // SECURITY (SSRF): `api_url` is task input handed straight to CURLOPT_URL with no
        // allowlist/validation — worse than client/client_pool's fixed-at-load host, this one is
        // fully attacker-chosen PER TASK (defaults to OpenAI's endpoint but nothing stops a task
        // from overriding it). A task can point this worker at an internal service, a cloud
        // metadata endpoint (e.g. 169.254.169.254), or anywhere else reachable from the process —
        // and it'll even carry the configured `api_key`/Authorization header there if one is set.
        // This is the clearest SSRF vector across all five plugins covered in this pass.
        curl_easy_setopt(curl, CURLOPT_URL, api_url.c_str());
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, static_cast<long>(request_body.size()));
        curl_easy_setopt(curl, CURLOPT_COPYPOSTFIELDS, request_body.data());
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 120L);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, &write_callback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

        CURLcode result = curl_easy_perform(curl);
        long status_code = 0;
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &status_code);
        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);

        if (result != CURLE_OK) {
            return std::unexpected{interfaces::WorkerError{curl_easy_strerror(result)}};
        }
        return interfaces::WorkerOutput{{"llm_status", "ok"},
                                        {"status", std::to_string(status_code)},
                                        {"response", std::move(response)}};
    }

    /// @brief Minimal JSON string escaping for the prompt content.
    static std::string json_escape(std::string_view text) {
        std::string out;
        out.reserve(text.size() + 8);
        for (char character : text) {
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
                out += character;
            }
        }
        return out;
    }

    // SECURITY: no cap on how much a downstream response can grow `response` to — a task can
    // point `api_url` (see the SSRF note above) at any endpoint willing to stream back an
    // unbounded body, and this callback appends every byte with no size limit. CURLOPT_TIMEOUT
    // bounds wall-clock time (120s) but not response size, so a slow-but-huge response can still
    // exhaust memory within that window.
    static std::size_t write_callback(char *pointer, std::size_t size, std::size_t count,
                                      void *userdata) {
        auto *out = static_cast<std::string *>(userdata);
        out->append(pointer, size * count);
        return size * count;
    }
};

} // namespace worker_llm

// Testing notes: run() has NO gate that stops a successfully-parsed input from reaching
// send_blocking() (see the BUG comment above run() — unlike email's explicit empty-smtp_url
// check), and send_blocking()/json_escape()/write_callback() are private statics reachable only
// through a real curl_easy_perform() call with no injectable transport seam. That means NO test
// may ever call LlmWorker::run() with all four of api_url/api_key/model/prompt present — doing so
// would fire a real outbound network attempt. Every test below either stays entirely at the
// LlmInput level (no run() involved) or calls run() with a JSON object missing at least one
// required key, which from_value() rejects before send_blocking() is ever reached.
#ifdef CONGELADO_TEST
namespace worker_llm::llm_worker_tests {
using namespace boost::ut;

/// @brief Builds a `serde::Value` straight from a JSON literal.
[[nodiscard]] serde::Value make_value(std::string_view json) {
    return rfl::json::read<rfl::Generic>(std::string{json}).value();
}

suite<"LlmInput"> llm_input_suite = [] {
    "setApiUrl/getApiUrl round-trips"_test = [] {
        LlmInput input;
        input.setApiUrl("https://internal.example/v1/chat");
        expect(input.getApiUrl() == "https://internal.example/v1/chat");
    };

    "setApiKey/getApiKey round-trips"_test = [] {
        LlmInput input;
        input.setApiKey("sk-test");
        expect(input.getApiKey() == "sk-test");
    };

    "setModel/getModel round-trips"_test = [] {
        LlmInput input;
        input.setModel("gpt-4o");
        expect(input.getModel() == "gpt-4o");
    };

    "setPrompt/getPrompt round-trips"_test = [] {
        LlmInput input;
        input.setPrompt("hello");
        expect(input.getPrompt() == "hello");
    };

    "default-constructed api_url/model match the documented OpenAI defaults, prompt/api_key are empty"_test =
        [] {
            LlmInput input;
            expect(input.getApiUrl() == "https://api.openai.com/v1/chat/completions");
            expect(input.getModel() == "gpt-4o-mini");
            expect(input.getApiKey().empty());
            expect(input.getPrompt().empty());
        };

    "from_value fails entirely when 'prompt' is omitted"_test = [] {
        auto value = make_value(R"({"api_url":"https://x","api_key":"k","model":"m"})");
        auto parsed = serde::Ser::from_value<LlmInput>(value);
        expect(!parsed.has_value()) << fatal;
        expect(parsed.error().contains("prompt")) << parsed.error();
    };

    "from_value fails entirely when 'api_key' is omitted"_test = [] {
        auto value = make_value(R"({"api_url":"https://x","model":"m","prompt":"p"})");
        auto parsed = serde::Ser::from_value<LlmInput>(value);
        expect(!parsed.has_value()) << fatal;
        expect(parsed.error().contains("api_key")) << parsed.error();
    };

    // BUG: pins the finding documented above LlmInput's m_api_url/m_model — omitting either fails
    // the whole decode despite the doc comment claiming they default via in-class initializers.
    "BUG: from_value fails entirely when 'api_url' is omitted, despite its documented default"_test =
        [] {
            auto value = make_value(R"({"api_key":"k","model":"m","prompt":"p"})");
            auto parsed = serde::Ser::from_value<LlmInput>(value);
            expect(!parsed.has_value()) << fatal;
            expect(parsed.error().contains("api_url")) << parsed.error();
        };

    "BUG: from_value fails entirely when 'model' is omitted, despite its documented default"_test =
        [] {
            auto value = make_value(R"({"api_url":"https://x","api_key":"k","prompt":"p"})");
            auto parsed = serde::Ser::from_value<LlmInput>(value);
            expect(!parsed.has_value()) << fatal;
            expect(parsed.error().contains("model")) << parsed.error();
        };

    // Parsing alone never performs I/O — safe to exercise the fully-populated success path here
    // (as opposed to through LlmWorker::run(), which would go on to call send_blocking()).
    "from_value succeeds when every declared field is present"_test = [] {
        auto value =
            make_value(R"({"api_url":"https://x","api_key":"k","model":"m","prompt":"p"})");
        auto parsed = serde::Ser::from_value<LlmInput>(value);
        expect(parsed.has_value()) << fatal;
        expect(parsed->getApiUrl() == "https://x");
        expect(parsed->getApiKey() == "k");
        expect(parsed->getModel() == "m");
        expect(parsed->getPrompt() == "p");
    };

    // SECURITY pin: api_url accepts literally anything, including a cloud metadata address —
    // pins the SSRF finding documented above send_blocking()'s CURLOPT_URL call.
    "SECURITY: api_url accepts an internal/metadata-shaped address with no validation"_test = [] {
        LlmInput input;
        input.setApiUrl("http://169.254.169.254/latest/meta-data/");
        expect(input.getApiUrl() == "http://169.254.169.254/latest/meta-data/");
    };

    // BUG pin: 'model' isn't run through json_escape() in send_blocking() (only 'prompt' is) —
    // the DTO itself imposes no restriction on characters that would break the JSON body.
    "BUG: model accepts an embedded double-quote that would break the hand-built JSON body"_test =
        [] {
            LlmInput input;
            input.setModel(R"(x","messages":[{"role":"system","content":"pwned")");
            expect(input.getModel().contains('"'));
        };
};

suite<"LlmWorker"> llm_worker_suite = [] {
    "get_task_type reports 'llm'"_test = [] {
        LlmWorker worker;
        expect(worker.get_task_type() == "llm");
    };

    "run() propagates the from_value parse error when 'prompt' is missing, never touches curl"_test =
        [] {
            LlmWorker worker;
            auto value = make_value(R"({"api_url":"https://x","api_key":"k","model":"m"})");
            interfaces::WorkerResult observed = interfaces::WorkerOutput{};
            bool called = false;

            worker.run(value, [&](interfaces::WorkerResult result) {
                called = true;
                observed = std::move(result);
            });

            expect(called) << fatal;
            expect(!observed.has_value()) << fatal;
            expect(observed.error().getMessage().contains("prompt"));
        };

    "run() propagates the from_value parse error when 'api_key' is missing"_test = [] {
        LlmWorker worker;
        auto value = make_value(R"({"api_url":"https://x","model":"m","prompt":"p"})");
        interfaces::WorkerResult observed = interfaces::WorkerOutput{};

        worker.run(value, [&](interfaces::WorkerResult result) { observed = std::move(result); });

        expect(!observed.has_value()) << fatal;
        expect(observed.error().getMessage().contains("api_key"));
    };

    "run() propagates the from_value parse error on a completely empty object"_test = [] {
        LlmWorker worker;
        auto value = make_value(R"({})");
        interfaces::WorkerResult observed = interfaces::WorkerOutput{};

        worker.run(value, [&](interfaces::WorkerResult result) { observed = std::move(result); });

        expect(!observed.has_value());
    };
};

} // namespace worker_llm::llm_worker_tests
#endif
