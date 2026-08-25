module;

#define CONGELADO_GUEST
#include <congelado/plugin.h>
#ifdef CONGELADO_TEST
#include <rfl/Generic.hpp>
#include <rfl/json.hpp>
#endif

export module auth_db_query_worker_plugin;

import congelado_plugin;
import interfaces;
import auth_db_query_store;
import core_contract;
import serde;
import std;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

namespace {

/// @brief The auth app's "a database call is a worker" primitive. Persists a user and reads it back
/// through the connector the host injected — the connector pointer arrives via the plugin's on_load
/// and is handed to this worker. All connector/model contact lives in the auth_db_query_store module
/// so this stays free of the connector import.
class DbQueryWorker final : public interfaces::IWorker {
  public:
    [[nodiscard]] std::string_view get_task_type() const noexcept override { return "db_query"; }

    /// @brief Stashes the host-injected connector pointer. @param connector_ctx the connector.
    void set_connector_ctx(void *connector_ctx) noexcept { m_connector_ctx = connector_ctx; }

    void run(const serde::Value &input,
            interfaces::WorkerCompletion on_complete) override {
        auto parsed = serde::Ser::from_value<auth::DbQueryInput>(input);
        if (!parsed) {
            on_complete(std::unexpected{interfaces::WorkerError{parsed.error()}});
            return;
        }

        auto shared_complete = std::make_shared<interfaces::WorkerCompletion>(std::move(on_complete));
        auth::UserStore::store_and_read_async(
            m_connector_ctx, parsed->getUsername(), parsed->getPasswordHash(),
            [shared_complete](std::optional<std::pair<std::string, std::string>> stored) {
                if (!stored) {
                    (*shared_complete)(std::unexpected{interfaces::WorkerError{"store/read failed"}});
                    return;
                }
                // SECURITY: leaks the derived password hash back out through the task's own
                // output map. taskdefs/db_query.json declares "stored_hash" as a plain
                // output_key with an empty masked_fields list, so anything that can read this
                // task's result (workflow status API, logs, a downstream node's input mapping)
                // gets the raw credential hash verbatim — no redaction, no separate
                // "confirm write succeeded" signal that omits it. Should either drop this key
                // or have the taskdef mask it.
                (*shared_complete)(interfaces::WorkerOutput{{"db_status", "ok"},
                                                            {"stored_username", stored->first},
                                                            {"stored_hash", stored->second}});
            });
    }

  private:
    void *m_connector_ctx{nullptr};
};

/// @brief Code-built defs for the auth app — the IAppDefs (C++-builder) counterpart to the def
/// files. Builds a TaskDef JSON programmatically instead of shipping a file, proving the builder
/// registration path end to end. A distinct name (`auth_health`) so the registration is verifiable.
class AuthDefsBuilder final : public interfaces::IAppDefs {
  public:
    [[nodiscard]] std::vector<std::string> get_task_defs() const override {
        std::string task_type = "echo";
        return {std::format(
            R"({{"name":"auth_health","type":"SIMPLE","worker_type":"{}","input_keys":[],)"
            R"("output_keys":[],"retry":{{"max_attempts":1,"backoff":"FIXED","interval_ms":1000}},)"
            R"("timeout":{{"timeout_ms":30000,"action":"ALERT_ONLY"}},"enforce_schema":false,)"
            R"("masked_fields":[]}})",
            task_type)};
    }
    [[nodiscard]] std::vector<std::string> get_workflow_defs() const override { return {}; }
};

/// @brief The db-query worker plugin — exports the WORKER capability backed by DbQueryWorker (wiring
/// the host-injected connector into it on load) plus the APP_DEFS capability for the code-built def.
class DbQueryWorkerPlugin final : public congelado::Plugin {
  public:
    [[nodiscard]] std::string_view get_name() const noexcept override {
        return "auth_db_query_worker";
    }
    [[nodiscard]] std::string_view get_version() const noexcept override { return "1.0.0"; }
    [[nodiscard]] std::string_view get_unique_type() const noexcept override { return "worker"; }
    [[nodiscard]] std::uint32_t capabilities() const noexcept override {
        return CONGELADO_CAP_WORKER | CONGELADO_CAP_APP_DEFS;
    }

    /// @brief Grabs the shared connector the worker host resolved and injects it into the worker.
    /// @param host the host callbacks carrying `connector_ctx`. @param cfg unused.
    void on_load(CongeladoHostCallbacks const &host, CongeladoConfigView const & /*cfg*/) override {
        m_worker.set_connector_ctx(host.connector_ctx);
        if (auto *group = congelado::controller_ctx<core::contract::ContractGroup<>>(host);
            group != nullptr) {
            m_worker.set_contract_group(*group, core::contract::ContractState::IDLE);
        }
    }

    /// @brief Capability hook the host calls to get at this plugin's IWorker surface.
    /// @return this plugin's DbQueryWorker, upcast to interfaces::IWorker*.
    void *worker_get() noexcept { return static_cast<interfaces::IWorker *>(&m_worker); }
    /// @brief Capability hook the host calls to get at this plugin's IAppDefs surface.
    /// @return this plugin's AuthDefsBuilder, upcast to interfaces::IAppDefs*.
    void *app_defs_get() noexcept { return static_cast<interfaces::IAppDefs *>(&m_defs_builder); }

  private:
    DbQueryWorker m_worker;
    AuthDefsBuilder m_defs_builder;
};

} // namespace

CONGELADO_PLUGIN(DbQueryWorkerPlugin);

#ifdef CONGELADO_TEST
namespace auth_db_query_worker_plugin_tests {
using namespace boost::ut;

/// @brief Builds a `serde::Value` straight from a JSON literal — same recipe as
/// engine's handler tests for a minimal `ISerdeFormat`-free `Value` fixture.
[[nodiscard]] serde::Value make_value(std::string_view json) {
    return rfl::json::read<rfl::Generic>(std::string{json}).value();
}

suite<"DbQueryWorker"> db_query_worker_suite = [] {
    "get_task_type reports 'db_query'"_test = [] {
        DbQueryWorker worker;
        expect(worker.get_task_type() == "db_query");
    };

    "run() with no connector wired (nullptr default) fails with 'store/read failed'"_test = [] {
        DbQueryWorker worker;
        auto value = make_value(R"({"username":"alice","password_hash":"hash"})");
        interfaces::WorkerResult observed = std::unexpected{interfaces::WorkerError{"unset"}};
        bool called = false;

        worker.run(value, [&](interfaces::WorkerResult result) {
            called = true;
            observed = std::move(result);
        });

        expect(called) << fatal;
        expect(!observed.has_value()) << fatal;
        expect(observed.error().getMessage() == "store/read failed");
    };

    "run() with a connector explicitly set to null behaves identically to never wiring one"_test = [] {
        DbQueryWorker worker;
        worker.set_connector_ctx(nullptr);
        auto value = make_value(R"({"username":"bob","password_hash":"hash"})");
        interfaces::WorkerResult observed = std::unexpected{interfaces::WorkerError{"unset"}};

        worker.run(value, [&](interfaces::WorkerResult result) { observed = std::move(result); });

        expect(!observed.has_value()) << fatal;
        expect(observed.error().getMessage() == "store/read failed");
    };

    "run() with input that doesn't decode into DbQueryInput reports the parse error instead of touching the connector"_test =
        [] {
            DbQueryWorker worker;
            auto value = make_value("[1,2,3]"); // an array, not an object
            interfaces::WorkerResult observed = interfaces::WorkerOutput{};
            bool called = false;

            worker.run(value, [&](interfaces::WorkerResult result) {
                called = true;
                observed = std::move(result);
            });

            expect(called) << fatal;
            expect(!observed.has_value());
        };

    "run() with an object missing password_hash fails to parse — from_value (rfl::from_generic) requires every reflected field present, unlike from_map's lenient defaulting"_test =
        [] {
            DbQueryWorker worker;
            auto value = make_value(R"({"username":"nopass"})");
            interfaces::WorkerResult observed = interfaces::WorkerOutput{};

            worker.run(value, [&](interfaces::WorkerResult result) { observed = std::move(result); });

            expect(!observed.has_value()) << fatal;
            // DbQueryInput::m_password_hash defaults empty in-class, but run() decodes via
            // serde::Ser::from_value, which goes through rfl::from_generic — reflect-cpp's
            // NamedTuple decode from a generic tree requires every reflected field to be
            // present in the source object, so a missing key is a hard parse error here, not
            // a silent default. That's a different (stricter) leniency than from_map's, which
            // does default absent fields (see store.cppm's "from_map leaves username at its
            // 'default_user' default when the key is absent"). Never reaches the connector.
            expect(observed.error().getMessage().contains("password_hash"));
            expect(observed.error().getMessage().contains("not found"));
        };
};

suite<"AuthDefsBuilder"> auth_defs_builder_suite = [] {
    "get_task_defs returns exactly one code-built 'auth_health' def wired to the 'echo' worker"_test =
        [] {
            AuthDefsBuilder builder;
            auto defs = builder.get_task_defs();

            expect(defs.size() == 1) << fatal;
            expect(defs[0] ==
                   R"({"name":"auth_health","type":"SIMPLE","worker_type":"echo","input_keys":[],)"
                   R"("output_keys":[],"retry":{"max_attempts":1,"backoff":"FIXED","interval_ms":1000},)"
                   R"("timeout":{"timeout_ms":30000,"action":"ALERT_ONLY"},"enforce_schema":false,)"
                   R"("masked_fields":[]})");
        };

    "get_workflow_defs is always empty — this app registers no code-built workflows"_test = [] {
        AuthDefsBuilder builder;
        expect(builder.get_workflow_defs().empty());
    };
};

suite<"DbQueryWorkerPlugin"> db_query_worker_plugin_suite = [] {
    "identity/capabilities are the declared db-query-worker + app-defs surface"_test = [] {
        DbQueryWorkerPlugin plugin;

        expect(plugin.get_name() == "auth_db_query_worker");
        expect(plugin.get_version() == "1.0.0");
        expect(plugin.get_unique_type() == "worker");
        expect(plugin.capabilities() == (CONGELADO_CAP_WORKER | CONGELADO_CAP_APP_DEFS));
    };

    "worker_get() exposes the same DbQueryWorker instance via IWorker*"_test = [] {
        DbQueryWorkerPlugin plugin;
        auto *worker = static_cast<interfaces::IWorker *>(plugin.worker_get());

        expect(worker != nullptr) << fatal;
        expect(worker->get_task_type() == "db_query");
    };

    "app_defs_get() exposes the same AuthDefsBuilder instance via IAppDefs*"_test = [] {
        DbQueryWorkerPlugin plugin;
        auto *defs = static_cast<interfaces::IAppDefs *>(plugin.app_defs_get());

        expect(defs != nullptr) << fatal;
        expect(defs->get_task_defs().size() == 1);
    };

    "on_load with no controller_ctx (host wiring absent) doesn't crash, and forwards a null connector_ctx through to the worker"_test =
        [] {
            DbQueryWorkerPlugin plugin;
            CongeladoHostCallbacks host{};
            CongeladoConfigView cfg{};

            plugin.on_load(host, cfg);

            // DbQueryWorker::run() overrides IWorker's protected run() as public, but access
            // control follows the pointer's static type — use the concrete type here so the
            // public override is what's actually reachable.
            auto *worker = static_cast<DbQueryWorker *>(plugin.worker_get());
            auto value = make_value(R"({"username":"x","password_hash":"y"})");
            interfaces::WorkerResult observed = interfaces::WorkerOutput{};
            worker->run(value, [&](interfaces::WorkerResult result) { observed = std::move(result); });

            expect(!observed.has_value()) << fatal;
            expect(observed.error().getMessage() == "store/read failed");
        };
};

} // namespace auth_db_query_worker_plugin_tests
#endif
