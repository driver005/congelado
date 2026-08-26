module;

#ifdef CONGELADO_TEST
#    include <rfl/Generic.hpp>
#    include <rfl/json.hpp>
#endif

export module client_worker;

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

export namespace worker_client {

using Leverager = io::base::leverage::Leverager<io::base::leverage::Context>;

/// @brief Typed input for the `client` worker, parsed from the task's dynamic input value via
/// `serde::Ser::from_value` — see the `Serializable<ClientInput>` specialization below.
/// `header.<Name>` keys don't fit a fixed field list, so they stay a manual scan over the raw
/// input object in `ClientWorker::run`. `method` defaults to "GET" via its in-class member
/// initializer; `path` is required (checked after parsing).
class ClientInput
{
public:
    void setPath(std::string value)
    {
        m_path = std::move(value);
    }

    void setMethod(std::string value)
    {
        m_method = std::move(value);
    }

    void setBody(std::optional<std::string> value)
    {
        m_body = std::move(value);
    }

    void setContentType(std::optional<std::string> value)
    {
        m_content_type = std::move(value);
    }

    [[nodiscard]] const std::string& getPath() const noexcept
    {
        return m_path;
    }

    [[nodiscard]] const std::string& getMethod() const noexcept
    {
        return m_method;
    }

    [[nodiscard]] const std::optional<std::string>& getBody() const noexcept
    {
        return m_body;
    }

    [[nodiscard]] const std::optional<std::string>& getContentType() const noexcept
    {
        return m_content_type;
    }

private:
    std::string m_path;
    // BUG: same reflect-cpp gotcha documented on HashInput::m_algo — this in-class default is
    // dead for task input parsed via `serde::Ser::from_value`, since `method` isn't
    // `std::optional`. A task input that omits "method" fails the WHOLE decode instead of
    // defaulting to GET. See the pinning test below.
    std::string m_method{"GET"};
    std::optional<std::string> m_body;
    std::optional<std::string> m_content_type;
};

} // namespace worker_client

template<>
struct serde::Serializable<worker_client::ClientInput>
{
    static constexpr auto fields()
    {
        using worker_client::ClientInput;
        return std::tuple{
            serde::FieldDesc<"path", &ClientInput::getPath, &ClientInput::setPath>{},
            serde::FieldDesc<"method", &ClientInput::getMethod, &ClientInput::setMethod>{},
            serde::FieldDesc<"body", &ClientInput::getBody, &ClientInput::setBody>{},
            serde::FieldDesc<
                "content_type", &ClientInput::getContentType, &ClientInput::setContentType>{},
        };
    }
};

export namespace worker_client {

/// @brief The `client` worker (worker_type `http`) — issues one request to the host-configured
/// downstream service using `core_client`: `core::client::Client` builds the request,
/// `core::client::Register` (owned by this worker) ships it and correlates the response. The
/// worker owns its own outbound HTTP/2 client, built via the host-injected
/// `interfaces::IProtocol` and connected once at load. Single fixed endpoint per deployment, so
/// input is `path` (not a full url). Input: `path` (required), `method` (default GET), `body`,
/// `content_type`, `header.<Name>` keys. Output: `status` (HTTP code), `body`, `http_status`
/// ("ok"/"error").
class ClientWorker final : public interfaces::IWorker
{
public:
    [[nodiscard]] std::string_view get_task_type() const noexcept override
    {
        return "http";
    }

    /// @brief Injects the host-owned protocol this worker builds its own client through.
    void set_protocol(interfaces::IProtocol<io::layer::http2::Server>* protocol) noexcept
    {
        m_protocol = protocol;
    }

    /// @brief Injects the host's io_uring leverager, needed to connect this worker's own
    /// client.
    void set_leverager(Leverager* leverager) noexcept
    {
        m_leverager = leverager;
    }

    /// @brief Injects the host's contract group, needed to connect this worker's own client —
    /// also binds this worker's own TaskQueue contract onto it (see
    /// IWorker::set_contract_group).
    void set_group(core::contract::ContractGroup<>* group)
    {
        m_group = group;
        if (group != nullptr) {
            set_contract_group(*group, core::contract::ContractState::IDLE);
        }
    }

    /// @brief Builds this worker's own client via `IProtocol::get_client` and connects it to
    /// `endpoint`. Connect is async — a task arriving before it lands degrades to an error via
    /// `has_runtime()`, never blocks.
    void connect_downstream(io::base::socket::Endpoint endpoint, bool verify_peer)
    {
        if (m_protocol == nullptr || m_leverager == nullptr || m_group == nullptr) {
            return;
        }
        m_client = m_protocol->get_client(m_register.make_dispatch());
        auto* http2_client = dynamic_cast<io::layer::http2::Client*>(m_client.get());
        if (http2_client == nullptr) {
            return;
        }
        auto connect_result = http2_client->connect(
            std::move(endpoint), *m_leverager, *m_group, verify_peer, [this, http2_client] {
                m_register.set_runtime(*http2_client);
            }
        );
        if (!connect_result) {
            core::logger::error(
                "client_worker", "downstream connect failed: {}", connect_result.error()
            );
        }
    }

    void run(const serde::Value& input, interfaces::WorkerCompletion on_complete) override
    {
        if (!m_register.has_runtime()) {
            on_complete(
                std::unexpected{interfaces::WorkerError{"no downstream client configured"}}
            );
            return;
        }
        auto parsed = serde::Ser::from_value<ClientInput>(input);
        if (!parsed) {
            on_complete(std::unexpected{interfaces::WorkerError{parsed.error()}});
            return;
        }
        if (parsed->getPath().empty()) {
            on_complete(std::unexpected{interfaces::WorkerError{"missing 'path'"}});
            return;
        }

        auto builder = core::client::Client::custom(parsed->getMethod(), parsed->getPath());
        if (parsed->getContentType().has_value()) {
            builder.add_header(
                interfaces::io::types::Token::CONTENT_TYPE, *parsed->getContentType()
            );
        }
        // SECURITY: every `header.<Name>` key in the zero-auth task input is forwarded verbatim
        // as an outgoing header to the fixed downstream (name AND value, no allowlist/denylist)
        // — a task can overwrite/inject headers the downstream may trust (e.g. Authorization,
        // Host-equivalent pseudo-headers) with no validation here. Host/port themselves are NOT
        // attacker-controlled (fixed at load from this plugin's own config), so this isn't
        // SSRF, but it is an unvalidated-header-injection-shaped gap onto whatever the operator
        // pointed this worker at.
        if (auto object = input.to_object()) {
            for (const auto& [key, value]: *object) {
                if (key.starts_with("header.")) {
                    std::string header_value;
                    if (auto as_string = value.to_string()) {
                        header_value = *as_string;
                    } else {
                        header_value = serde::Ser::encode_json(value);
                    }
                    builder.add_header(std::string_view{key}.substr(7), header_value);
                }
            }
        }
        if (parsed->getBody().has_value()) {
            builder.add_body(*parsed->getBody());
        }

        auto request = builder.build(m_register.runtime());
        m_register.send(
            std::move(request),
            [on_complete = std::move(on_complete)](interfaces::io::IResponse& response) mutable {
                auto& view = response.get_body();
                std::string body;
                body.reserve(view.size());
                for (auto byte: view) {
                    body.push_back(static_cast<char>(byte));
                }
                on_complete(
                    interfaces::WorkerOutput{
                        {"http_status", "ok"},
                        {"status", std::to_string(
                                       static_cast<int>(
                                           interfaces::io::types::status_code(response.get_status())
                                       )
                                   )},
                        {"body", std::move(body)}
                    }
                );
            }
        );
    }

private:
    interfaces::IProtocol<io::layer::http2::Server>* m_protocol{nullptr};
    Leverager* m_leverager{nullptr};
    core::contract::ContractGroup<>* m_group{nullptr};
    core::client::Register m_register;
    std::unique_ptr<interfaces::IClient> m_client;
};

} // namespace worker_client

// Testing notes: the success path of connect_downstream()/run() needs a real
// io::layer::http2::Client — Register::has_runtime() only flips true off a live dynamic_cast onto
// that concrete type (see connect_downstream), which in turn needs a live socket + io_uring
// leverager to connect. No injectable seam exists short of that, so the actual request-send path
// is a legitimate, documented skip here (same reasoning core_client's own Register/Client suites
// already document for their success paths). What IS fully testable without any of that: every
// ClientInput parse/validation path, and every early-return guard in run()/connect_downstream that
// fires before a runtime is ever needed — a freshly constructed ClientWorker starts with no runtime
// bound, so those guards are reachable with zero mocking.
#ifdef CONGELADO_TEST
namespace worker_client::client_worker_tests {
using namespace boost::ut;

/// @brief Builds a `serde::Value` straight from a JSON literal — same recipe used by every
/// other worker's inline tests in this repo (see hash_worker.cppm).
[[nodiscard]] serde::Value make_value(std::string_view json)
{
    return rfl::json::read<rfl::Generic>(std::string{json}).value();
}

suite<"ClientInput"> client_input_suite = [] {
    "setPath/getPath round-trips"_test = [] {
        ClientInput input;
        input.setPath("/foo");
        expect(input.getPath() == "/foo");
    };

    "setMethod/getMethod round-trips"_test = [] {
        ClientInput input;
        input.setMethod("POST");
        expect(input.getMethod() == "POST");
    };

    "setBody/getBody round-trips"_test = [] {
        ClientInput input;
        input.setBody("payload");
        expect(input.getBody().has_value()) << fatal;
        expect(*input.getBody() == "payload");
    };

    "setContentType/getContentType round-trips"_test = [] {
        ClientInput input;
        input.setContentType("application/json");
        expect(input.getContentType().has_value()) << fatal;
        expect(*input.getContentType() == "application/json");
    };

    "default-constructed method is GET, body/content_type are unset"_test = [] {
        ClientInput input;
        expect(input.getMethod() == "GET");
        expect(!input.getBody().has_value());
        expect(!input.getContentType().has_value());
    };

    "from_value fails entirely when 'path' is omitted"_test = [] {
        auto value = make_value(R"({"method":"GET"})");
        auto parsed = serde::Ser::from_value<ClientInput>(value);
        expect(!parsed.has_value()) << fatal;
        expect(parsed.error().contains("path")) << parsed.error();
    };

    // BUG: pins the finding documented above ClientInput::m_method — omitting "method" fails
    // the whole decode despite the doc comment claiming it defaults to GET.
    "BUG: from_value fails entirely when 'method' is omitted, despite its documented default"_test =
        [] {
            auto value = make_value(R"({"path":"/foo"})");
            auto parsed = serde::Ser::from_value<ClientInput>(value);
            expect(!parsed.has_value()) << fatal;
            expect(parsed.error().contains("method")) << parsed.error();
        };

    "from_value succeeds when 'body'/'content_type' are omitted — they're std::optional"_test = [] {
        auto value = make_value(R"({"path":"/foo","method":"GET"})");
        auto parsed = serde::Ser::from_value<ClientInput>(value);
        expect(parsed.has_value()) << fatal;
        expect(parsed->getPath() == "/foo");
        expect(!parsed->getBody().has_value());
        expect(!parsed->getContentType().has_value());
    };

    "from_value succeeds when every field is present"_test = [] {
        auto value =
            make_value(R"({"path":"/foo","method":"POST","body":"b","content_type":"text/plain"})");
        auto parsed = serde::Ser::from_value<ClientInput>(value);
        expect(parsed.has_value()) << fatal;
        expect(parsed->getPath() == "/foo");
        expect(parsed->getMethod() == "POST");
        expect(*parsed->getBody() == "b");
        expect(*parsed->getContentType() == "text/plain");
    };
};

suite<"ClientWorker"> client_worker_suite = [] {
    "get_task_type reports 'http'"_test = [] {
        ClientWorker worker;
        expect(worker.get_task_type() == "http");
    };

    "run() with no downstream configured fails without touching input parsing"_test = [] {
        ClientWorker worker;
        // Deliberately malformed (missing every ClientInput field) — if run() reached parsing
        // this would surface a from_value error message instead of the runtime-missing one.
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

    "set_protocol(nullptr)/set_leverager(nullptr)/set_group(nullptr) are all safe no-ops"_test =
        [] {
            ClientWorker worker;
            expect(nothrow([&] {
                worker.set_protocol(nullptr);
                worker.set_leverager(nullptr);
                worker.set_group(nullptr);
            }));
        };

    "set_group with a real ContractGroup binds the worker's TaskQueue without crashing"_test = [] {
        ClientWorker worker;
        core::contract::ContractGroup<> group;
        expect(nothrow([&] {
            worker.set_group(&group);
        }));
    };

    "connect_downstream is a no-op when no dependency was ever injected"_test = [] {
        ClientWorker worker;
        expect(nothrow([&] {
            worker.connect_downstream(io::base::socket::Endpoint{"127.0.0.1", 443}, false);
        }));
        // Still no runtime — run() takes the same early-exit path as with nothing configured.
        auto value = make_value(R"({})");
        interfaces::WorkerResult observed = interfaces::WorkerOutput{};
        worker.run(value, [&](interfaces::WorkerResult result) {
            observed = std::move(result);
        });
        expect(!observed.has_value()) << fatal;
        expect(observed.error().getMessage() == "no downstream client configured");
    };

    "connect_downstream is a no-op when only the contract group was injected (protocol/leverager still null)"_test =
        [] {
            ClientWorker worker;
            core::contract::ContractGroup<> group;
            worker.set_group(&group);
            expect(nothrow([&] {
                worker.connect_downstream(io::base::socket::Endpoint{"127.0.0.1", 443}, true);
            }));
        };
};

} // namespace worker_client::client_worker_tests
#endif
