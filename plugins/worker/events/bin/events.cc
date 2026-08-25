module;

#define CONGELADO_GUEST
#include <congelado/plugin.h>

export module events_worker_plugin;

import congelado_plugin;
import interfaces;
import events_worker;
import core_contract;
import std;

namespace {

/// @brief The events worker plugin — exports the WORKER capability backed by
/// worker_events::EventsWorker (publishes through the process event bus). No linked broker library;
/// the host's injected IEventSink plugins are the actual producers.
class EventsWorkerPlugin final : public congelado::Plugin {
  public:
    [[nodiscard]] std::string_view get_name() const noexcept override { return "events_worker"; }
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
    /// @return this plugin's EventsWorker, upcast to interfaces::IWorker*.
    void *worker_get() noexcept { return static_cast<interfaces::IWorker *>(&m_worker); }

  private:
    worker_events::EventsWorker m_worker;
};

} // namespace

CONGELADO_PLUGIN(EventsWorkerPlugin);
