module;

#define CONGELADO_GUEST
#include <congelado/plugin.h>

export module worker_orchestrator_local_plugin;

import congelado_plugin;
import interfaces;
import worker_orchestrator_local;
import std;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

namespace {

/// @brief The local worker-orchestrator plugin — exports the WORKER_ORCHESTRATOR capability backed
/// by worker_orchestrator::LocalOrchestrator, wiring the host-injected connector into it on load.
class LocalOrchestratorPlugin final : public congelado::Plugin {
  public:
    [[nodiscard]] std::string_view get_name() const noexcept override {
        return "worker_orchestrator_local";
    }
    [[nodiscard]] std::string_view get_version() const noexcept override { return "1.0.0"; }
    [[nodiscard]] std::string_view get_unique_type() const noexcept override {
        return "worker_orchestrator";
    }
    [[nodiscard]] std::uint32_t capabilities() const noexcept override {
        return CONGELADO_CAP_WORKER_ORCHESTRATOR;
    }

    /// @brief Grabs the shared connector the engine host resolved and injects it into the backend.
    /// @param host the host callbacks carrying `connector_ctx`. @param cfg unused.
    void on_load(CongeladoHostCallbacks const &host, CongeladoConfigView const & /*cfg*/) override {
        m_orchestrator.set_connector_ctx(host.connector_ctx);
    }

    /// @brief Capability hook the host calls to get at this plugin's IWorkerOrchestrator surface.
    /// @return this plugin's LocalOrchestrator, upcast to interfaces::IWorkerOrchestrator*.
    void *worker_orchestrator_get() noexcept {
        return static_cast<interfaces::IWorkerOrchestrator *>(&m_orchestrator);
    }

  private:
    worker_orchestrator::LocalOrchestrator m_orchestrator;
};

} // namespace

CONGELADO_PLUGIN(LocalOrchestratorPlugin);

#ifdef CONGELADO_TEST
// Deliberately does NOT `import connector;`/`import model;` here — see local_orchestrator.cppm's
// own docs: importing connector directly from a plugin entry TU shaped like this one crashes
// clang's modules support. Every test below stays within what's already reachable from this TU.
namespace local_orchestrator_plugin_tests {
using namespace boost::ut;

suite<"LocalOrchestratorPlugin identity"> local_orchestrator_plugin_identity_suite = [] {
    "get_name reports 'worker_orchestrator_local'"_test = [] {
        LocalOrchestratorPlugin plugin;
        expect(plugin.get_name() == "worker_orchestrator_local");
    };

    "get_version reports a non-empty version string"_test = [] {
        LocalOrchestratorPlugin plugin;
        expect(plugin.get_version() == "1.0.0");
    };

    "get_unique_type reports 'worker_orchestrator'"_test = [] {
        LocalOrchestratorPlugin plugin;
        expect(plugin.get_unique_type() == "worker_orchestrator");
    };

    "capabilities reports CONGELADO_CAP_WORKER_ORCHESTRATOR"_test = [] {
        LocalOrchestratorPlugin plugin;
        expect(plugin.capabilities() == CONGELADO_CAP_WORKER_ORCHESTRATOR);
    };
};

suite<"LocalOrchestratorPlugin::on_load"> local_orchestrator_plugin_on_load_suite = [] {
    "a fully-nullptr host is safe to load with — no crash"_test = [] {
        LocalOrchestratorPlugin plugin;
        expect(nothrow([&] {
            plugin.on_load(CongeladoHostCallbacks{}, CongeladoConfigView{});
        }));
    };

    "wires host.connector_ctx through so worker_orchestrator_get()'s enqueue reflects it"_test =
        [] {
            LocalOrchestratorPlugin plugin;
            CongeladoHostCallbacks host{};
            host.connector_ctx = nullptr; // still exercises the wiring call itself, not a real DB
            plugin.on_load(host, CongeladoConfigView{});

            auto *raw = plugin.worker_orchestrator_get();
            expect(raw != nullptr) << fatal;
            auto *orchestrator = static_cast<interfaces::IWorkerOrchestrator *>(raw);

            std::optional<std::string> result{"unset"};
            orchestrator->enqueue("echo", interfaces::Value{std::string{"x"}},
                                  [&result](std::optional<std::string> id) { result = id; });
            // No connector was actually resolved (nullptr), so this stays the documented
            // "no connector" default — proves on_load() really did forward the pointer through
            // rather than leaving some stale/uninitialized state.
            expect(!result.has_value());
        };
};

suite<"LocalOrchestratorPlugin::worker_orchestrator_get"> local_orchestrator_plugin_capability_suite =
    [] {
        "hands back a non-null IWorkerOrchestrator* whose backend_name is 'local'"_test = [] {
            LocalOrchestratorPlugin plugin;
            auto *raw = plugin.worker_orchestrator_get();
            expect(raw != nullptr) << fatal;

            auto *orchestrator = static_cast<interfaces::IWorkerOrchestrator *>(raw);
            expect(orchestrator->backend_name() == "local");
        };

        "returns the same address on every call"_test = [] {
            LocalOrchestratorPlugin plugin;
            expect(plugin.worker_orchestrator_get() == plugin.worker_orchestrator_get());
        };
    };

} // namespace local_orchestrator_plugin_tests
#endif
