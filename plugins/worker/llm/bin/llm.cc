module;

#define CONGELADO_GUEST
#include <congelado/plugin.h>

export module llm_worker_plugin;

import congelado_plugin;
import interfaces;
import llm_worker;
import core_contract;
import std;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

namespace {

/// @brief The llm worker plugin — exports the WORKER capability backed by worker_llm::LlmWorker
/// (OpenAI-compatible chat via libcurl). on_load binds the worker's TaskQueue onto the host's shared
/// contract group.
class LlmWorkerPlugin final : public congelado::Plugin {
  public:
    [[nodiscard]] std::string_view get_name() const noexcept override { return "llm_worker"; }
    [[nodiscard]] std::string_view get_version() const noexcept override { return "1.0.0"; }
    [[nodiscard]] std::string_view get_unique_type() const noexcept override { return "worker"; }
    [[nodiscard]] std::uint32_t capabilities() const noexcept override {
        return CONGELADO_CAP_WORKER;
    }

    void on_load(CongeladoHostCallbacks const &host, CongeladoConfigView const & /*cfg*/) override {
        if (auto *group = congelado::controller_ctx<core::contract::ContractGroup<>>(host);
            group != nullptr) {
            m_worker.set_contract_group(*group, core::contract::ContractState::IDLE);
        }
    }

    /// @brief Capability hook the host calls to get at this plugin's IWorker surface.
    /// @return this plugin's LlmWorker, upcast to interfaces::IWorker*.
    void *worker_get() noexcept { return static_cast<interfaces::IWorker *>(&m_worker); }

  private:
    worker_llm::LlmWorker m_worker;
};

} // namespace

CONGELADO_PLUGIN(LlmWorkerPlugin);

#ifdef CONGELADO_TEST
namespace llm_worker_plugin_tests {
using namespace boost::ut;

suite<"LlmWorkerPlugin"> llm_worker_plugin_suite = [] {
    "identity/capabilities are the declared llm-worker surface"_test = [] {
        LlmWorkerPlugin plugin;

        expect(plugin.get_name() == "llm_worker");
        expect(plugin.get_version() == "1.0.0");
        expect(plugin.get_unique_type() == "worker");
        expect(plugin.capabilities() == CONGELADO_CAP_WORKER);
    };

    "worker_get returns a non-null pointer castable to IWorker, task type 'llm'"_test = [] {
        LlmWorkerPlugin plugin;
        void *raw = plugin.worker_get();
        expect(raw != nullptr) << fatal;
        auto *worker = static_cast<interfaces::IWorker *>(raw);
        expect(worker->get_task_type() == "llm");
    };

    "on_load with an empty host/config view does not crash"_test = [] {
        LlmWorkerPlugin plugin;
        expect(nothrow(
            [&] { plugin.on_load(CongeladoHostCallbacks{}, CongeladoConfigView{}); }));
    };

    "on_load with a real contract group binds the worker's TaskQueue without crashing"_test = [] {
        LlmWorkerPlugin plugin;
        core::contract::ContractGroup<> group;
        CongeladoHostCallbacks host{};
        host.controller_ctx = &group;
        expect(nothrow([&] { plugin.on_load(host, CongeladoConfigView{}); }));
    };
};

} // namespace llm_worker_plugin_tests
#endif
