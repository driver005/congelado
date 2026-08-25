module;

#define CONGELADO_GUEST
#include <congelado/plugin.h>

export module hash_worker_plugin;

import congelado_plugin;
import interfaces;
import hash_worker;
import core_contract;
import std;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

namespace {

/// @brief The hash worker plugin — exports the WORKER capability backed by worker_hash::HashWorker
/// (OpenSSL EVP digests). Pure compute, no injected resources.
class HashWorkerPlugin final : public congelado::Plugin {
  public:
    [[nodiscard]] std::string_view get_name() const noexcept override { return "hash_worker"; }
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
    /// @return this plugin's HashWorker, upcast to interfaces::IWorker*.
    void *worker_get() noexcept { return static_cast<interfaces::IWorker *>(&m_worker); }

  private:
    worker_hash::HashWorker m_worker;
};

} // namespace

CONGELADO_PLUGIN(HashWorkerPlugin);

#ifdef CONGELADO_TEST
namespace hash_worker_plugin_tests {
using namespace boost::ut;

suite<"HashWorkerPlugin"> hash_worker_plugin_suite = [] {
    "get_name reports 'hash_worker'"_test = [] {
        HashWorkerPlugin plugin;
        expect(plugin.get_name() == "hash_worker");
    };

    "get_version reports '1.0.0'"_test = [] {
        HashWorkerPlugin plugin;
        expect(plugin.get_version() == "1.0.0");
    };

    "get_unique_type reports 'worker'"_test = [] {
        HashWorkerPlugin plugin;
        expect(plugin.get_unique_type() == "worker");
    };

    "capabilities reports CONGELADO_CAP_WORKER"_test = [] {
        HashWorkerPlugin plugin;
        expect(plugin.capabilities() == CONGELADO_CAP_WORKER);
    };

    "worker_get returns a non-null pointer castable to IWorker, task type 'hash'"_test = [] {
        HashWorkerPlugin plugin;
        void *raw = plugin.worker_get();
        expect(raw != nullptr) << fatal;
        auto *worker = static_cast<interfaces::IWorker *>(raw);
        expect(worker->get_task_type() == "hash");
    };

    "on_load with an empty host/config view does not crash (no contract group resolved)"_test = [] {
        HashWorkerPlugin plugin;
        expect(nothrow(
            [&] { plugin.on_load(CongeladoHostCallbacks{}, CongeladoConfigView{}); }));
    };

    "on_load with a real contract group binds the worker's TaskQueue without crashing"_test = [] {
        HashWorkerPlugin plugin;
        core::contract::ContractGroup<> group;
        CongeladoHostCallbacks host{};
        host.controller_ctx = &group;
        expect(nothrow([&] { plugin.on_load(host, CongeladoConfigView{}); }));
    };
};

} // namespace hash_worker_plugin_tests
#endif
