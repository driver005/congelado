module;

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <winsock2.h>
#else
#include <linux/time_types.h>
#endif

#ifdef CONGELADO_TEST
#include <rfl/Generic.hpp>
#include <rfl/json.hpp>
#endif

export module client_pool_worker;

import std;
import interfaces;
import core_client;
import io_layer_http2;
import io_base_socket;
import io_base_leverage;
import core_contract;
import core_logger;
import serde;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace worker_client_pool {

using Leverager = io::base::leverage::Leverager<io::base::leverage::Context>;

/// @brief Typed input for the `client_pool` worker, parsed from the task's dynamic input value via
/// `serde::Ser::from_value` — see the `Serializable<ClientPoolInput>` specialization below.
/// `header.<Name>` keys don't fit a fixed field list, so they stay a manual scan over the raw input
/// object in `ClientPoolWorker::run`. `method`/`expected_status`/`interval_ms`/`max_attempts` default
/// via in-class member initializers; `path` is required (checked after parsing).
class ClientPoolInput {
  public:
    void setPath(std::string value) { m_path = std::move(value); }
    void setMethod(std::string value) { m_method = std::move(value); }
    void setExpectedStatus(int value) noexcept { m_expected_status = value; }
    void setIntervalMs(long value) noexcept { m_interval_ms = value; }
    void setMaxAttempts(int value) noexcept { m_max_attempts = value; }

    [[nodiscard]] const std::string &getPath() const noexcept { return m_path; }
    [[nodiscard]] const std::string &getMethod() const noexcept { return m_method; }
    [[nodiscard]] int getExpectedStatus() const noexcept { return m_expected_status; }
    [[nodiscard]] long getIntervalMs() const noexcept { return m_interval_ms; }
    [[nodiscard]] int getMaxAttempts() const noexcept { return m_max_attempts; }

  private:
    std::string m_path;
    // BUG: same reflect-cpp gotcha documented on worker_hash::HashInput::m_algo — none of these
    // four fields is `std::optional`, so `serde::Ser::from_value` requires ALL of them present in
    // the task input or the WHOLE decode fails; these in-class defaults are only ever reached via
    // direct C++ construction, never via task input that omits the key. See the pinning tests
    // below.
    std::string m_method{"GET"};
    int m_expected_status{200};
    long m_interval_ms{1000};
    // SECURITY: `max_attempts`/`interval_ms` come straight from zero-auth task input with no
    // upper/lower bound check — a task can set max_attempts to a huge number and interval_ms to 0
    // to hammer the fixed downstream as fast as the event loop allows, indefinitely, or set
    // interval_ms negative (feeds directly into `__kernel_timespec::tv_sec/tv_nsec` in
    // handle_response, an unvalidated negative timeout). No cap exists anywhere on this path.
    int m_max_attempts{5};
};

} // namespace worker_client_pool

template <>
struct serde::Serializable<worker_client_pool::ClientPoolInput> {
    static constexpr auto fields() {
        using worker_client_pool::ClientPoolInput;
        return std::tuple{
            serde::FieldDesc<"path", &ClientPoolInput::getPath, &ClientPoolInput::setPath>{},
            serde::FieldDesc<"method", &ClientPoolInput::getMethod, &ClientPoolInput::setMethod>{},
            serde::FieldDesc<"expected_status", &ClientPoolInput::getExpectedStatus,
                             &ClientPoolInput::setExpectedStatus>{},
            serde::FieldDesc<"interval_ms", &ClientPoolInput::getIntervalMs,
                             &ClientPoolInput::setIntervalMs>{},
            serde::FieldDesc<"max_attempts", &ClientPoolInput::getMaxAttempts,
                             &ClientPoolInput::setMaxAttempts>{},
        };
    }
};

export namespace worker_client_pool {

/// @brief The `client_pool` worker (worker_type `client_pool`) — the polling counterpart to the
/// `client` worker: repeatedly requests the host-configured downstream service until it returns the
/// expected status (or attempts run out). Owns its own outbound HTTP/2 client, built via the
/// host-injected `interfaces::IProtocol` and connected once at load. Retries are driven by the
/// host's io_uring leverager timer — no thread ever blocks or sleeps. Input: `path` (required),
/// `method` (GET), `expected_status` (200), `interval_ms` (1000), `max_attempts` (5),
/// `header.<Name>` keys. Output: `status` (last HTTP code), `body`, `attempts`, `client_pool_status`
/// ("ok" when the expected status was hit, "timeout"/"error" otherwise).
class ClientPoolWorker final : public interfaces::IWorker {
  public:
    [[nodiscard]] std::string_view get_task_type() const noexcept override { return "client_pool"; }

    /// @brief Injects the host-owned protocol this worker builds its own client through.
    void set_protocol(interfaces::IProtocol<io::layer::http2::Server> *protocol) noexcept {
        m_protocol = protocol;
    }
    /// @brief Injects the host's io_uring leverager, needed to connect this worker's own client and
    /// to drive the retry timer.
    void set_leverager(Leverager *leverager) noexcept { m_leverager = leverager; }
    /// @brief Injects the host's contract group, needed to connect this worker's own client — also
    /// binds this worker's own TaskQueue contract onto it (see IWorker::set_contract_group).
    void set_group(core::contract::ContractGroup<> *group) {
        m_group = group;
        if (group != nullptr) {
            set_contract_group(*group, core::contract::ContractState::IDLE);
        }
    }

    /// @brief Builds this worker's own client via `IProtocol::get_client` and connects it to
    /// `endpoint`. Connect is async — a task arriving before it lands degrades to an error via
    /// `has_runtime()`, never blocks.
    void connect_downstream(io::base::socket::Endpoint endpoint, bool verify_peer) {
        if (m_protocol == nullptr || m_leverager == nullptr || m_group == nullptr) {
            return;
        }
        m_client = m_protocol->get_client(m_register.make_dispatch());
        auto *http2_client = dynamic_cast<io::layer::http2::Client *>(m_client.get());
        if (http2_client == nullptr) {
            return;
        }
        auto connect_result =
            http2_client->connect(std::move(endpoint), *m_leverager, *m_group, verify_peer,
                                  [this, http2_client] { m_register.set_runtime(*http2_client); });
        if (!connect_result) {
            core::logger::error("client_pool_worker", "downstream connect failed: {}",
                                connect_result.error());
        }
    }

    void run(const serde::Value &input,
            interfaces::WorkerCompletion on_complete) override {
        if (!m_register.has_runtime()) {
            on_complete(std::unexpected{interfaces::WorkerError{"no downstream client configured"}});
            return;
        }
        auto parsed = serde::Ser::from_value<ClientPoolInput>(input);
        if (!parsed) {
            on_complete(std::unexpected{interfaces::WorkerError{parsed.error()}});
            return;
        }
        if (parsed->getPath().empty()) {
            on_complete(std::unexpected{interfaces::WorkerError{"missing 'path'"}});
            return;
        }

        auto state = std::make_shared<RetryState>();
        state->set_method(parsed->getMethod());
        state->set_path(parsed->getPath());
        state->set_expected_status(parsed->getExpectedStatus());
        state->set_interval_ms(parsed->getIntervalMs());
        state->set_max_attempts(parsed->getMaxAttempts());
        std::vector<std::pair<std::string, std::string>> headers;
        // SECURITY: same unvalidated `header.<Name>` forwarding as worker_client::ClientWorker —
        // see the SECURITY note there. Host/port aren't attacker-controlled here either (fixed at
        // load), only header name/value and path are.
        if (auto object = input.to_object()) {
            for (const auto &[key, value] : *object) {
                if (key.starts_with("header.")) {
                    std::string header_value;
                    if (auto as_string = value.to_string()) {
                        header_value = *as_string;
                    } else {
                        header_value = serde::Ser::encode_json(value);
                    }
                    headers.emplace_back(key.substr(7), std::move(header_value));
                }
            }
        }
        state->set_headers(std::move(headers));
        state->set_completion(std::move(on_complete));
        issue_attempt(state);
    }

  private:
    /// @brief One in-flight poll's retry bookkeeping — kept alive by shared ownership across every
    /// attempt so a concurrent second task on this same worker doesn't share state with it.
    class RetryState {
      public:
        void set_method(std::string method) { m_method = std::move(method); }
        void set_path(std::string path) { m_path = std::move(path); }
        void set_expected_status(int status) noexcept { m_expected_status = status; }
        void set_interval_ms(long interval_ms) noexcept { m_interval_ms = interval_ms; }
        void set_max_attempts(int max_attempts) noexcept { m_max_attempts = max_attempts; }
        void set_headers(std::vector<std::pair<std::string, std::string>> headers) {
            m_headers = std::move(headers);
        }
        void set_completion(interfaces::WorkerCompletion completion) {
            m_completion = std::move(completion);
        }

        [[nodiscard]] const std::string &get_method() const noexcept { return m_method; }
        [[nodiscard]] const std::string &get_path() const noexcept { return m_path; }
        [[nodiscard]] int get_expected_status() const noexcept { return m_expected_status; }
        [[nodiscard]] long get_interval_ms() const noexcept { return m_interval_ms; }
        [[nodiscard]] int get_attempt() const noexcept { return m_attempt; }
        [[nodiscard]] const std::vector<std::pair<std::string, std::string>> &
        get_headers() const noexcept {
            return m_headers;
        }
        [[nodiscard]] __kernel_timespec &get_timeout_spec() noexcept { return m_timeout_spec; }

        /// @brief Whether another attempt is allowed after this one.
        [[nodiscard]] bool has_more_attempts() const noexcept { return m_attempt < m_max_attempts; }
        /// @brief Counts the attempt that just finished.
        void increment_attempt() noexcept { ++m_attempt; }
        /// @brief Fires the stored completion once, with the final successful output map.
        void complete(std::unordered_map<std::string, std::string> output) {
            if (m_completion) {
                m_completion(interfaces::WorkerOutput{std::move(output)});
            }
        }
        /// @brief Fires the stored completion once, with a failure.
        void fail(interfaces::WorkerError error) {
            if (m_completion) {
                m_completion(std::unexpected{std::move(error)});
            }
        }

      private:
        std::string m_method;
        std::string m_path;
        int m_expected_status{200};
        long m_interval_ms{1000};
        int m_max_attempts{5};
        int m_attempt{0};
        std::vector<std::pair<std::string, std::string>> m_headers;
        __kernel_timespec m_timeout_spec{};
        interfaces::WorkerCompletion m_completion;
    };

    /// @brief Sends one attempt through the owned Register; the response resumes via
    /// `handle_response`.
    void issue_attempt(const std::shared_ptr<RetryState> &state) {
        auto builder = core::client::Client::custom(state->get_method(), state->get_path());
        for (const auto &[key, value] : state->get_headers()) {
            builder.add_header(key, value);
        }
        auto request = builder.build(m_register.runtime());
        m_register.send(std::move(request), [this, state](interfaces::io::IResponse &response) {
            handle_response(state, response);
        });
    }

    /// @brief Decides whether `state` is done (matched or out of attempts) or needs another attempt
    /// after `interval_ms`, driven by the leverager timer — never blocks a thread.
    void handle_response(const std::shared_ptr<RetryState> &state,
                         interfaces::io::IResponse &response) {
        auto &view = response.get_body();
        std::string body;
        body.reserve(view.size());
        for (auto byte : view) {
            body.push_back(static_cast<char>(byte));
        }
        auto status = static_cast<int>(interfaces::io::types::status_code(response.get_status()));
        state->increment_attempt();

        if (status == state->get_expected_status()) {
            state->complete({{"client_pool_status", "ok"},
                            {"status", std::to_string(status)},
                            {"attempts", std::to_string(state->get_attempt())},
                            {"body", std::move(body)}});
            return;
        }
        if (!state->has_more_attempts()) {
            state->complete({{"client_pool_status", "timeout"},
                            {"status", std::to_string(status)},
                            {"attempts", std::to_string(state->get_attempt())},
                            {"body", std::move(body)}});
            return;
        }
        if (m_leverager == nullptr) {
            state->fail(interfaces::WorkerError{"no leverager configured"});
            return;
        }
        auto &timeout_spec = state->get_timeout_spec();
        timeout_spec.tv_sec = state->get_interval_ms() / 1000;
        timeout_spec.tv_nsec = (state->get_interval_ms() % 1000) * 1000000;
        m_leverager->timeout(&timeout_spec, [this, state](int /*result*/) { issue_attempt(state); });
    }

    interfaces::IProtocol<io::layer::http2::Server> *m_protocol{nullptr};
    Leverager *m_leverager{nullptr};
    core::contract::ContractGroup<> *m_group{nullptr};
    core::client::Register m_register;
    std::unique_ptr<interfaces::IClient> m_client;
};

} // namespace worker_client_pool

// Testing notes: same story as worker_client::ClientWorker — issue_attempt()/handle_response()
// need a live io::layer::http2::Client + leverager timer, unreachable without real socket/io_uring
// machinery, so the actual retry loop is a legitimate, documented skip. RetryState is a private
// nested type, not independently testable either. What's fully testable: ClientPoolInput
// parse/validation, get_task_type, and every early-return guard in run() that fires before a
// runtime is needed.
#ifdef CONGELADO_TEST
namespace worker_client_pool::client_pool_worker_tests {
using namespace boost::ut;

/// @brief Builds a `serde::Value` straight from a JSON literal.
[[nodiscard]] serde::Value make_value(std::string_view json) {
    return rfl::json::read<rfl::Generic>(std::string{json}).value();
}

suite<"ClientPoolInput"> client_pool_input_suite = [] {
    "setPath/getPath round-trips"_test = [] {
        ClientPoolInput input;
        input.setPath("/health");
        expect(input.getPath() == "/health");
    };

    "setMethod/getMethod round-trips"_test = [] {
        ClientPoolInput input;
        input.setMethod("HEAD");
        expect(input.getMethod() == "HEAD");
    };

    "setExpectedStatus/getExpectedStatus round-trips"_test = [] {
        ClientPoolInput input;
        input.setExpectedStatus(204);
        expect(input.getExpectedStatus() == 204);
    };

    "setIntervalMs/getIntervalMs round-trips"_test = [] {
        ClientPoolInput input;
        input.setIntervalMs(2500L);
        expect(input.getIntervalMs() == 2500L);
    };

    "setMaxAttempts/getMaxAttempts round-trips"_test = [] {
        ClientPoolInput input;
        input.setMaxAttempts(10);
        expect(input.getMaxAttempts() == 10);
    };

    "default-constructed fields match the documented defaults"_test = [] {
        ClientPoolInput input;
        expect(input.getMethod() == "GET");
        expect(input.getExpectedStatus() == 200);
        expect(input.getIntervalMs() == 1000L);
        expect(input.getMaxAttempts() == 5);
    };

    // SECURITY pin: no clamping anywhere on these setters — negative/huge values pass straight
    // through, matching the SECURITY note on ClientPoolInput::m_max_attempts above.
    "setMaxAttempts/setIntervalMs accept negative and absurdly large values with no clamping"_test =
        [] {
            ClientPoolInput input;
            input.setMaxAttempts(-1);
            expect(input.getMaxAttempts() == -1);
            input.setMaxAttempts(2000000000);
            expect(input.getMaxAttempts() == 2000000000);
            input.setIntervalMs(-500L);
            expect(input.getIntervalMs() == -500L);
        };

    "from_value fails entirely when 'path' is omitted"_test = [] {
        auto value = make_value(R"({"method":"GET"})");
        auto parsed = serde::Ser::from_value<ClientPoolInput>(value);
        expect(!parsed.has_value()) << fatal;
        expect(parsed.error().contains("path")) << parsed.error();
    };

    "BUG: from_value fails entirely when 'method' is omitted, despite its documented default"_test =
        [] {
            auto value =
                make_value(R"({"path":"/x","expected_status":200,"interval_ms":1000,"max_attempts":5})");
            auto parsed = serde::Ser::from_value<ClientPoolInput>(value);
            expect(!parsed.has_value()) << fatal;
            expect(parsed.error().contains("method")) << parsed.error();
        };

    "BUG: from_value fails entirely when 'max_attempts' is omitted, despite its documented default"_test =
        [] {
            auto value =
                make_value(R"({"path":"/x","method":"GET","expected_status":200,"interval_ms":1000})");
            auto parsed = serde::Ser::from_value<ClientPoolInput>(value);
            expect(!parsed.has_value()) << fatal;
            expect(parsed.error().contains("max_attempts")) << parsed.error();
        };

    "from_value succeeds when every declared field is present"_test = [] {
        auto value = make_value(
            R"({"path":"/x","method":"GET","expected_status":204,"interval_ms":250,"max_attempts":3})");
        auto parsed = serde::Ser::from_value<ClientPoolInput>(value);
        expect(parsed.has_value()) << fatal;
        expect(parsed->getPath() == "/x");
        expect(parsed->getExpectedStatus() == 204);
        expect(parsed->getIntervalMs() == 250L);
        expect(parsed->getMaxAttempts() == 3);
    };
};

suite<"ClientPoolWorker"> client_pool_worker_suite = [] {
    "get_task_type reports 'client_pool'"_test = [] {
        ClientPoolWorker worker;
        expect(worker.get_task_type() == "client_pool");
    };

    "run() with no downstream configured fails without touching input parsing"_test = [] {
        ClientPoolWorker worker;
        auto value = make_value(R"({})");
        interfaces::WorkerResult observed = interfaces::WorkerOutput{};
        bool called = false;

        worker.run(value, [&](interfaces::WorkerResult result) {
            called = true;
            observed = std::move(result);
        });

        expect(called) << fatal;
        expect(!observed.has_value()) << fatal;
        expect(observed.error().getMessage() == "no downstream client configured");
    };

    "set_protocol(nullptr)/set_leverager(nullptr)/set_group(nullptr) are all safe no-ops"_test = [] {
        ClientPoolWorker worker;
        expect(nothrow([&] {
            worker.set_protocol(nullptr);
            worker.set_leverager(nullptr);
            worker.set_group(nullptr);
        }));
    };

    "set_group with a real ContractGroup binds the worker's TaskQueue without crashing"_test = [] {
        ClientPoolWorker worker;
        core::contract::ContractGroup<> group;
        expect(nothrow([&] { worker.set_group(&group); }));
    };

    "connect_downstream is a no-op when no dependency was ever injected"_test = [] {
        ClientPoolWorker worker;
        expect(nothrow([&] {
            worker.connect_downstream(io::base::socket::Endpoint{"127.0.0.1", 443}, false);
        }));
        auto value = make_value(R"({})");
        interfaces::WorkerResult observed = interfaces::WorkerOutput{};
        worker.run(value, [&](interfaces::WorkerResult result) { observed = std::move(result); });
        expect(!observed.has_value()) << fatal;
        expect(observed.error().getMessage() == "no downstream client configured");
    };

    "connect_downstream is a no-op when only the contract group was injected"_test = [] {
        ClientPoolWorker worker;
        core::contract::ContractGroup<> group;
        worker.set_group(&group);
        expect(nothrow([&] {
            worker.connect_downstream(io::base::socket::Endpoint{"127.0.0.1", 443}, true);
        }));
    };
};

} // namespace worker_client_pool::client_pool_worker_tests
#endif
