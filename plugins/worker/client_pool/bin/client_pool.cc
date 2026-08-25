module;

#define CONGELADO_GUEST
#include <congelado/plugin.h>

export module client_pool_worker_plugin;

import congelado_plugin;
import interfaces;
import client_pool_worker;
import io_layer_http2;
import io_base_socket;
import io_base_leverage;
import core_contract;
import std;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

namespace {

/// @brief The client_pool worker plugin — exports the WORKER capability backed by
/// worker_client_pool::ClientPoolWorker (own core-client transport, polling). on_load hands the
/// worker the host's protocol/leverager/contract group so it can build and connect its own client,
/// plus the downstream endpoint read from this plugin's own config section.
class ClientPoolWorkerPlugin final : public congelado::Plugin {
  public:
    [[nodiscard]] std::string_view get_name() const noexcept override {
        return "client_pool_worker";
    }
    [[nodiscard]] std::string_view get_version() const noexcept override { return "1.0.0"; }
    [[nodiscard]] std::string_view get_unique_type() const noexcept override { return "worker"; }
    [[nodiscard]] std::uint32_t capabilities() const noexcept override {
        return CONGELADO_CAP_WORKER;
    }

    void on_load(CongeladoHostCallbacks const &host, CongeladoConfigView const &cfg) override {
        m_worker.set_protocol(
            static_cast<interfaces::IProtocol<io::layer::http2::Server> *>(host.client_protocol_ctx));
        m_worker.set_leverager(
            congelado::leverager_ctx<worker_client_pool::Leverager>(host));
        m_worker.set_group(
            congelado::controller_ctx<core::contract::ContractGroup<>>(host));

        auto host_field = congelado::config_get(cfg, "host");
        if (!host_field.has_value()) {
            return;
        }
        std::uint16_t port = 443;
        if (auto port_field = congelado::config_get(cfg, "port"); port_field.has_value()) {
            std::from_chars(port_field->data(), port_field->data() + port_field->size(), port);
        }
        bool verify_peer = congelado::config_get(cfg, "cert").has_value() ||
                           congelado::config_get(cfg, "key").has_value();
        m_worker.connect_downstream(io::base::socket::Endpoint{*host_field, port}, verify_peer);
    }

    /// @brief Capability hook the host calls to get at this plugin's IWorker surface.
    /// @return this plugin's ClientPoolWorker, upcast to interfaces::IWorker*.
    void *worker_get() noexcept { return static_cast<interfaces::IWorker *>(&m_worker); }

  private:
    worker_client_pool::ClientPoolWorker m_worker;
};

} // namespace

CONGELADO_PLUGIN(ClientPoolWorkerPlugin);

#ifdef CONGELADO_TEST
namespace client_pool_worker_plugin_tests {
using namespace boost::ut;

suite<"ClientPoolWorkerPlugin"> client_pool_worker_plugin_suite = [] {
    "identity/capabilities are the declared client-pool-worker surface"_test = [] {
        ClientPoolWorkerPlugin plugin;

        expect(plugin.get_name() == "client_pool_worker");
        expect(plugin.get_version() == "1.0.0");
        expect(plugin.get_unique_type() == "worker");
        expect(plugin.capabilities() == CONGELADO_CAP_WORKER);
    };

    "worker_get returns a non-null pointer castable to IWorker, task type 'client_pool'"_test = [] {
        ClientPoolWorkerPlugin plugin;
        void *raw = plugin.worker_get();
        expect(raw != nullptr) << fatal;
        auto *worker = static_cast<interfaces::IWorker *>(raw);
        expect(worker->get_task_type() == "client_pool");
    };

    "on_load with an empty host/config view does not crash (no 'host' key, nothing connects)"_test =
        [] {
            ClientPoolWorkerPlugin plugin;
            expect(nothrow(
                [&] { plugin.on_load(CongeladoHostCallbacks{}, CongeladoConfigView{}); }));
        };

    "on_load with a 'host' key but every host ctx pointer null does not crash"_test = [] {
        ClientPoolWorkerPlugin plugin;
        congelado::ConfigViewBuilder builder;
        builder.add("host", "example.internal");
        builder.add("port", "8443");
        builder.add("cert", "/path/to/cert");
        CongeladoHostCallbacks host{};
        expect(nothrow([&] { plugin.on_load(host, builder.view()); }));
    };

    "on_load with a real contract group binds the worker's TaskQueue without crashing"_test = [] {
        ClientPoolWorkerPlugin plugin;
        core::contract::ContractGroup<> group;
        CongeladoHostCallbacks host{};
        host.controller_ctx = &group;
        expect(nothrow([&] { plugin.on_load(host, CongeladoConfigView{}); }));
    };
};

} // namespace client_pool_worker_plugin_tests
#endif
