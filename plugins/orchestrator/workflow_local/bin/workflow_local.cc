module;

#define CONGELADO_GUEST
#include <congelado/plugin.h>

export module workflow_local_plugin;

import congelado_plugin;
import interfaces;
import workflow_local;
import std;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

namespace {

/// @brief The local workflow-orchestrator plugin — exports the WORKFLOW_ORCHESTRATOR capability
/// backed by workflow_orchestrator::WorkflowLocal, wiring the host-injected connector into it on
/// load.
class WorkflowLocalPlugin final : public congelado::Plugin {
  public:
    [[nodiscard]] std::string_view get_name() const noexcept override {
        return "workflow_orchestrator_local";
    }
    [[nodiscard]] std::string_view get_version() const noexcept override { return "1.0.0"; }
    [[nodiscard]] std::string_view get_unique_type() const noexcept override {
        return "workflow_orchestrator";
    }
    [[nodiscard]] std::uint32_t capabilities() const noexcept override {
        return CONGELADO_CAP_WORKFLOW_ORCHESTRATOR;
    }

    /// @brief Wires the DAG backend from the host callbacks (connector + lua + search) and starts its
    /// sweep contract. All the real construction lives in the workflow_local module so this entry TU
    /// stays free of connector/contract imports.
    /// @param host the host callbacks. @param cfg unused.
    void on_load(CongeladoHostCallbacks const &host, CongeladoConfigView const & /*cfg*/) override {
        m_workflow.on_load(host.connector_ctx, host.lua_bridge_ctx, host.search_ctx,
                           host.controller_ctx, host.registry_ctx);
    }

    /// @brief Capability hook the host calls to get at this plugin's IWorkflowOrchestrator surface.
    /// @return this plugin's WorkflowLocal, upcast to interfaces::IWorkflowOrchestrator*.
    void *workflow_orchestrator_get() noexcept {
        return static_cast<interfaces::IWorkflowOrchestrator *>(&m_workflow);
    }

  private:
    workflow_orchestrator::WorkflowLocal m_workflow;
};

} // namespace

CONGELADO_PLUGIN(WorkflowLocalPlugin);

#ifdef CONGELADO_TEST
// Deliberately does NOT `import connector;`/`import model;` here — see store.cppm's/this file's
// own docs: importing connector directly from a plugin entry TU shaped like this one (module
// implementation unit, `#define CONGELADO_GUEST` + `<congelado/plugin.h>` in the global module
// fragment) crashes clang's modules support. Every test below stays within what's already
// reachable from this TU (interfaces/congelado_plugin), same boundary the production code itself
// respects.
namespace workflow_local_plugin_tests {
using namespace boost::ut;

suite<"WorkflowLocalPlugin identity"> workflow_local_plugin_identity_suite = [] {
    "get_name reports 'workflow_orchestrator_local'"_test = [] {
        WorkflowLocalPlugin plugin;
        expect(plugin.get_name() == "workflow_orchestrator_local");
    };

    "get_version reports a non-empty version string"_test = [] {
        WorkflowLocalPlugin plugin;
        expect(plugin.get_version() == "1.0.0");
    };

    "get_unique_type reports 'workflow_orchestrator'"_test = [] {
        WorkflowLocalPlugin plugin;
        expect(plugin.get_unique_type() == "workflow_orchestrator");
    };

    "capabilities reports CONGELADO_CAP_WORKFLOW_ORCHESTRATOR"_test = [] {
        WorkflowLocalPlugin plugin;
        expect(plugin.capabilities() == CONGELADO_CAP_WORKFLOW_ORCHESTRATOR);
    };

    "get_type/get_requires/get_load_before_types fall back to Plugin's inert defaults"_test = [] {
        WorkflowLocalPlugin plugin;
        expect(plugin.get_type() == "plugin");
        expect(plugin.get_requires().empty());
        expect(plugin.get_load_before_types().empty());
    };
};

suite<"WorkflowLocalPlugin::on_load"> workflow_local_plugin_on_load_suite = [] {
    "a fully-nullptr host is safe to load with — no crash"_test = [] {
        WorkflowLocalPlugin plugin;
        expect(nothrow([&] {
            plugin.on_load(CongeladoHostCallbacks{}, CongeladoConfigView{});
        }));
    };
};

suite<"WorkflowLocalPlugin::workflow_orchestrator_get"> workflow_local_plugin_capability_suite = [] {
    "hands back a non-null IWorkflowOrchestrator* whose backend_name is 'local'"_test = [] {
        WorkflowLocalPlugin plugin;
        auto *raw = plugin.workflow_orchestrator_get();
        expect(raw != nullptr) << fatal;

        auto *orchestrator = static_cast<interfaces::IWorkflowOrchestrator *>(raw);
        expect(orchestrator->backend_name() == "local");
    };

    "returns the same address on every call — stable identity, not a fresh instance"_test = [] {
        WorkflowLocalPlugin plugin;
        expect(plugin.workflow_orchestrator_get() == plugin.workflow_orchestrator_get());
    };
};

} // namespace workflow_local_plugin_tests
#endif
