module;

#define CONGELADO_GUEST
#include <congelado/plugin.h>

export module email_worker_plugin;

import congelado_plugin;
import interfaces;
import email_worker;
import core_contract;
import std;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

namespace {

/// @brief The email worker plugin — exports the WORKER capability backed by worker_email::EmailWorker
/// (SMTP send via libcurl). on_load binds the worker's TaskQueue onto the host's shared contract
/// group.
class EmailWorkerPlugin final : public congelado::Plugin {
  public:
    [[nodiscard]] std::string_view get_name() const noexcept override { return "email_worker"; }
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
    /// @return this plugin's EmailWorker, upcast to interfaces::IWorker*.
    void *worker_get() noexcept { return static_cast<interfaces::IWorker *>(&m_worker); }

  private:
    worker_email::EmailWorker m_worker;
};

} // namespace

CONGELADO_PLUGIN(EmailWorkerPlugin);

#ifdef CONGELADO_TEST
namespace email_worker_plugin_tests {
using namespace boost::ut;

suite<"EmailWorkerPlugin"> email_worker_plugin_suite = [] {
    "identity/capabilities are the declared email-worker surface"_test = [] {
        EmailWorkerPlugin plugin;

        expect(plugin.get_name() == "email_worker");
        expect(plugin.get_version() == "1.0.0");
        expect(plugin.get_unique_type() == "worker");
        expect(plugin.capabilities() == CONGELADO_CAP_WORKER);
    };

    "worker_get returns a non-null pointer castable to IWorker, task type 'email'"_test = [] {
        EmailWorkerPlugin plugin;
        void *raw = plugin.worker_get();
        expect(raw != nullptr) << fatal;
        auto *worker = static_cast<interfaces::IWorker *>(raw);
        expect(worker->get_task_type() == "email");
    };

    "on_load with an empty host/config view does not crash"_test = [] {
        EmailWorkerPlugin plugin;
        expect(nothrow(
            [&] { plugin.on_load(CongeladoHostCallbacks{}, CongeladoConfigView{}); }));
    };

    "on_load with a real contract group binds the worker's TaskQueue without crashing"_test = [] {
        EmailWorkerPlugin plugin;
        core::contract::ContractGroup<> group;
        CongeladoHostCallbacks host{};
        host.controller_ctx = &group;
        expect(nothrow([&] { plugin.on_load(host, CongeladoConfigView{}); }));
    };
};

} // namespace email_worker_plugin_tests
#endif
