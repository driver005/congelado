export module worker_orchestrator_local;

import std;
import interfaces;
import worker_orchestrator_local_store;
#ifdef CONGELADO_TEST
import connector;
import model;
import serde;
import shared;
import boost.ut;
#endif

export namespace worker_orchestrator {

/// @brief The local (in-process, connector-backed) worker_orchestrator backend — the
/// engine-side dispatch/orchestration capability, symmetric to the external worker manager.
/// Implements the async IWorkerOrchestrator by delegating to the
/// worker_orchestrator_local_store module (which owns all connector/model contact, keeping this
/// free of direct connector plumbing). Shares the engine's task store via the host-injected
/// connector, so it claims the same tasks the engine enqueues.
class LocalOrchestrator final : public interfaces::IWorkerOrchestrator
{
public:
    /// @brief Stashes the host-injected connector pointer. @param connector_ctx the connector.
    void set_connector_ctx(void* connector_ctx) noexcept
    {
        m_connector_ctx = connector_ctx;
    }

    [[nodiscard]] std::string_view backend_name() const noexcept override
    {
        return "local";
    }

    void enqueue(
        std::string_view task_type,
        const interfaces::Value& input,
        std::move_only_function<void(std::optional<std::string>)> callback
    ) override
    {
        OrchestratorStore::enqueue(
            m_connector_ctx, std::string{task_type}, input, std::move(callback)
        );
    }

    void claim(
        std::string_view worker_type,
        std::optional<std::string_view> domain,
        std::move_only_function<void(std::optional<std::string>)> callback
    ) override
    {
        auto domain_owned =
            domain ? std::optional<std::string>{std::string{*domain}} : std::nullopt;
        OrchestratorStore::claim(
            m_connector_ctx, std::string{worker_type}, std::move(domain_owned), std::move(callback)
        );
    }

    void submit_result(
        std::string_view task_id,
        bool success,
        const std::unordered_map<std::string, std::string>& output,
        std::move_only_function<void(bool)> callback
    ) override
    {
        OrchestratorStore::submit_result(
            m_connector_ctx, std::string{task_id}, success, output, std::move(callback)
        );
    }

    void requeue(
        std::string_view worker_type, std::move_only_function<void(std::size_t)> callback
    ) override
    {
        OrchestratorStore::requeue(m_connector_ctx, std::string{worker_type}, std::move(callback));
    }

    void queue_size(
        std::string_view worker_type, std::move_only_function<void(std::size_t)> callback
    ) override
    {
        OrchestratorStore::queue_size(
            m_connector_ctx, std::string{worker_type}, std::move(callback)
        );
    }

    void start_workflow(
        std::string_view def_name,
        const std::unordered_map<std::string, std::string>& variables,
        std::move_only_function<void(std::optional<std::string>)> callback
    ) override
    {
        OrchestratorStore::start_workflow(
            m_connector_ctx, std::string{def_name}, variables, std::move(callback)
        );
    }

    void start_server() override {}

    void shutdown_all() override {}

    void set_health_callback(
        std::move_only_function<void(const interfaces::OrchestratorInfo&)> callback
    ) override
    {
        m_health = std::move(callback);
    }

private:
    void* m_connector_ctx{nullptr};
    std::move_only_function<void(const interfaces::OrchestratorInfo&)> m_health;
};

} // namespace worker_orchestrator

#ifdef CONGELADO_TEST
namespace worker_orchestrator::local_orchestrator_tests {
using namespace boost::ut;

/// @brief Trivial synchronous in-memory ICache — Connector aborts via active_cache() if none is
/// wired in.
class FakeCache final : public interfaces::ICache
{
public:
    [[nodiscard]] std::string_view backend_name() const noexcept override
    {
        return "fake_cache";
    }

    void get(std::string_view key, shared::QueryReadFn&& result) noexcept override
    {
        auto found = m_store.find(std::string{key});
        result(found != m_store.end() ? std::string_view{found->second} : std::string_view{});
    }

    void set(
        std::string_view key, std::string_view value, shared::QueryReadFn&& result
    ) noexcept override
    {
        m_store[std::string{key}] = std::string{value};
        result("ok");
    }

    void remove(std::string_view key, shared::QueryReadFn&& result) noexcept override
    {
        m_store.erase(std::string{key});
        result("ok");
    }

private:
    std::unordered_map<std::string, std::string> m_store;
};

suite<"LocalOrchestrator identity"> local_orchestrator_identity_suite = [] {
    "backend_name reports 'local'"_test = [] {
        LocalOrchestrator orchestrator;
        expect(orchestrator.backend_name() == "local");
    };

    "start_server/shutdown_all are harmless no-ops"_test = [] {
        LocalOrchestrator orchestrator;
        expect(nothrow([&] {
            orchestrator.start_server();
            orchestrator.shutdown_all();
        }));
    };

    "set_health_callback stores the callback without invoking it — this backend never "
    "publishes a health snapshot"_test = [] {
        LocalOrchestrator orchestrator;
        bool invoked = false;
        expect(nothrow([&] {
            orchestrator.set_health_callback([&invoked](const interfaces::OrchestratorInfo&) {
                invoked = true;
            });
        }));
        expect(!invoked);
    };
};

suite<"LocalOrchestrator with no connector wired"> local_orchestrator_no_connector_suite = [] {
    "enqueue reports std::nullopt before set_connector_ctx is ever called"_test = [] {
        LocalOrchestrator orchestrator;
        std::optional<std::string> result{"unset"};
        orchestrator.enqueue(
            "echo", interfaces::Value{std::string{"x"}}, [&result](std::optional<std::string> id) {
                result = id;
            }
        );
        expect(!result.has_value());
    };

    "claim reports std::nullopt"_test = [] {
        LocalOrchestrator orchestrator;
        std::optional<std::string> result{"unset"};
        orchestrator.claim("echo", std::nullopt, [&result](std::optional<std::string> id) {
            result = id;
        });
        expect(!result.has_value());
    };

    "submit_result reports false"_test = [] {
        LocalOrchestrator orchestrator;
        bool result = true;
        orchestrator.submit_result("some-id", true, {}, [&result](bool ok) {
            result = ok;
        });
        expect(!result);
    };

    "requeue/queue_size report 0"_test = [] {
        LocalOrchestrator orchestrator;
        std::size_t requeue_result = 99;
        std::size_t queue_size_result = 99;
        orchestrator.requeue("echo", [&requeue_result](std::size_t n) {
            requeue_result = n;
        });
        orchestrator.queue_size("echo", [&queue_size_result](std::size_t n) {
            queue_size_result = n;
        });
        expect(requeue_result == std::size_t{0});
        expect(queue_size_result == std::size_t{0});
    };

    "start_workflow reports std::nullopt"_test = [] {
        LocalOrchestrator orchestrator;
        std::optional<std::string> result{"unset"};
        orchestrator.start_workflow("order_pipeline", {}, [&result](std::optional<std::string> id) {
            result = id;
        });
        expect(!result.has_value());
    };
};

suite<"LocalOrchestrator delegates to a wired connector"> local_orchestrator_wired_suite = [] {
    "set_connector_ctx wires a real connector through to enqueue/claim/submit_result"_test = [] {
        FakeCache cache;
        connector::Connector real_connector;
        real_connector.set_cache(&cache);

        model::TaskDef def;
        def.set_name("send_email");
        def.set_worker_type("echo");
        real_connector.insert<model::TaskDef>(def, [](bool) {});

        LocalOrchestrator orchestrator;
        orchestrator.set_connector_ctx(&real_connector);

        std::optional<std::string> enqueued_id;
        orchestrator.enqueue(
            "send_email", interfaces::Value{serde::Value::Object{}},
            [&enqueued_id](std::optional<std::string> id) {
                enqueued_id = id;
            }
        );
        expect(enqueued_id.has_value()) << fatal;

        std::optional<std::string> claimed;
        orchestrator.claim("echo", std::nullopt, [&claimed](std::optional<std::string> value) {
            claimed = value;
        });
        expect(claimed.has_value()) << fatal;

        bool submit_ok = false;
        orchestrator.submit_result(*enqueued_id, true, {{"k", "v"}}, [&submit_ok](bool ok) {
            submit_ok = ok;
        });
        expect(submit_ok);
    };

    "set_connector_ctx wires start_workflow through to a real WorkflowDef lookup"_test = [] {
        FakeCache cache;
        connector::Connector real_connector;
        real_connector.set_cache(&cache);
        model::WorkflowDef def;
        def.set_name("order_pipeline");
        real_connector.insert<model::WorkflowDef>(def, [](bool) {});

        LocalOrchestrator orchestrator;
        orchestrator.set_connector_ctx(&real_connector);

        std::optional<std::string> result;
        orchestrator.start_workflow("order_pipeline", {}, [&result](std::optional<std::string> id) {
            result = id;
        });
        expect(result.has_value());
    };
};

} // namespace worker_orchestrator::local_orchestrator_tests
#endif
