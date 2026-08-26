module;
#include <memory>
#ifdef CONGELADO_TEST
#    include "core/manager/abi.h"
#endif

export module engine:context;

import connector;
import interfaces;
import model;
#ifdef CONGELADO_TEST
import std;
import boost.ut;
#endif

export namespace engine {

// Runtime dependency bundle injected once at startup before any request is dispatched.
class EngineContext
{
public:
    /**
     * @brief Default-constructs a local-only engine context with its own fallback Connector.
     * Tests use this path; production plugins replace it with the SDK-owned connector via
     * set_connector().
     */
    EngineContext() :
        m_owned_connector{std::make_unique<connector::Connector>()},
        m_connector{m_owned_connector.get()}
    {
    }

    /**
     * @brief Replaces the local fallback Connector with the SDK-owned shared one.
     * @param connector the connector to use for all DB/cache operations going forward.
     */
    void set_connector(connector::Connector* connector) noexcept
    {
        m_connector = connector;
    }

    /**
     * @brief Wires in the database backend, forwarding straight through to the underlying
     * Connector. Every handler sharing this context sees the new backend from here on out.
     * @param database the database backend to use going forward, or nullptr to drop back to
     * local-only mode.
     */
    void set_db(interfaces::IDatabase* database) noexcept
    {
        m_connector->set_database(database);
    }

    /**
     * @brief Wires in the cache backend, forwarding straight through to the underlying
     * Connector.
     * @param cache the cache backend to use going forward, or nullptr to drop back to the
     * Connector's built-in local cache.
     */
    void set_cache(interfaces::ICache* cache) noexcept
    {
        m_connector->set_cache(cache);
    }

    /**
     * @brief Gets the currently configured database backend.
     * @return the database pointer, or nullptr if this context is running local-only — check
     * before you dereference, no safety net here.
     */
    [[nodiscard]] interfaces::IDatabase* get_db() noexcept
    {
        return m_connector->get_database();
    }

    /**
     * @brief Gets the currently configured cache backend.
     * @return the cache pointer, or nullptr if none's wired up.
     */
    [[nodiscard]] interfaces::ICache* get_cache() noexcept
    {
        return m_connector->get_cache();
    }

    /**
     * @brief Wires in the resolved "lua" IBridge, if one was found — used by Orchestrator's
     * LuaEval to evaluate SWITCH edge conditions, DO_WHILE loop conditions, and (Phase 5)
     * EventHandler conditions.
     * @param bridge the resolved lua IBridge*, or nullptr if none was found.
     */
    void set_lua_bridge(interfaces::IBridge* bridge) noexcept
    {
        m_lua_bridge = bridge;
    }

    /**
     * @brief Gets the currently wired-in lua bridge.
     * @return the bridge pointer, or nullptr if none was resolved — check before dereferencing,
     * no safety net here.
     */
    [[nodiscard]] interfaces::IBridge* get_lua_bridge() noexcept
    {
        return m_lua_bridge;
    }

    /**
     * @brief Wires in the resolved search-capable backend, if one was found — used by
     * SummaryProjector to push WorkflowSummary/TaskSummary projections on every terminal
     * transition. No provider configured is a valid state, not an error — search routes just
     * degrade to empty results.
     * @param provider the resolved ISearchProvider*, or nullptr if none was found.
     */
    void set_search(interfaces::ISearchProvider* provider) noexcept
    {
        m_search = provider;
    }

    /**
     * @brief Gets the currently wired-in search provider.
     * @return the provider pointer, or nullptr if none was resolved — check before
     * dereferencing, no safety net here.
     */
    [[nodiscard]] interfaces::ISearchProvider* get_search() noexcept
    {
        return m_search;
    }

    /**
     * @brief Wires in the resolved cron backend, if one was found — the ScheduleHandler
     * validates and previews cron expressions through it, and EnginePlugin::on_load installs a
     * fire callback plus seeds existing schedules into it. No backend configured means
     * schedules never fire (a logged misconfiguration, not a silent degrade).
     * @param cron the resolved ICron*, or nullptr if none was found.
     */
    void set_cron(interfaces::ICron* cron) noexcept
    {
        m_cron = cron;
    }

    /**
     * @brief Gets the currently wired-in cron backend.
     * @return the cron pointer, or nullptr if none was resolved — check before dereferencing,
     * no safety net here.
     */
    [[nodiscard]] interfaces::ICron* get_cron() noexcept
    {
        return m_cron;
    }

    /**
     * @brief Wires in the dispatch/orchestration backend the queue endpoints route through —
     * the built-in engine Orchestrator by default, or a resolved worker_orchestrator plugin
     * that overrides it. EnginePlugin::on_load sets this after resolving the capability.
     * @param orchestrator the IWorkerOrchestrator* to use going forward.
     */
    void set_orchestrator(interfaces::IWorkerOrchestrator* orchestrator) noexcept
    {
        m_orchestrator = orchestrator;
    }

    /**
     * @brief Gets the currently wired-in dispatch/orchestration backend.
     * @return the orchestrator pointer, or nullptr if none was set — check before
     * dereferencing.
     */
    [[nodiscard]] interfaces::IWorkerOrchestrator* get_orchestrator() noexcept
    {
        return m_orchestrator;
    }

    /**
     * @brief Wires in the workflow-lifecycle backend (the DAG-side counterpart to the dispatch
     * orchestrator) — a resolved workflow_orchestrator plugin. EnginePlugin::on_load sets it
     * after resolving the capability. NULL means none was resolved.
     * @param workflow the IWorkflowOrchestrator* to use.
     */
    void set_workflow_orchestrator(interfaces::IWorkflowOrchestrator* workflow) noexcept
    {
        m_workflow = workflow;
    }

    /**
     * @brief Gets the currently wired-in workflow-lifecycle backend.
     * @return the pointer, or nullptr if none was resolved — check before dereferencing.
     */
    [[nodiscard]] interfaces::IWorkflowOrchestrator* get_workflow_orchestrator() noexcept
    {
        return m_workflow;
    }

    /**
     * @brief Wires in the external payload storage backend — unlike db/lua_bridge/search, this
     * isn't a resolved plugin capability, just a plain object EnginePlugin::on_load constructs
     * directly (see LocalPayloadStorage's own docs on why no capability-plugin resolution is
     * needed for the local-disk default).
     * @param storage the storage instance to use going forward; this context does not take
     * ownership — caller keeps it alive for this context's whole lifetime.
     */
    void set_payload_storage(interfaces::IExternalPayloadStorage* storage) noexcept
    {
        m_payload_storage = storage;
    }

    /**
     * @brief Gets the currently wired-in payload storage backend.
     * @return the storage pointer, or nullptr if none was set — check before dereferencing, no
     * safety net here.
     */
    [[nodiscard]] interfaces::IExternalPayloadStorage* get_payload_storage() noexcept
    {
        return m_payload_storage;
    }

    /**
     * @brief Gets the underlying Connector this context wraps — this class is just a thin
     * bundle around it, the connector's where the actual db/cache motion happens.
     * @return a reference to the connector instance.
     */
    [[nodiscard]] connector::Connector& get_connector() noexcept
    {
        return *m_connector;
    }

private:
    // Fallback connector for tests or local-only use; production receives the SDK-owned one.
    std::unique_ptr<connector::Connector> m_owned_connector;
    connector::Connector* m_connector;
    interfaces::IBridge* m_lua_bridge{nullptr};
    interfaces::ISearchProvider* m_search{nullptr};
    interfaces::ICron* m_cron{nullptr};
    interfaces::IWorkerOrchestrator* m_orchestrator{nullptr};
    interfaces::IWorkflowOrchestrator* m_workflow{nullptr};
    interfaces::IExternalPayloadStorage* m_payload_storage{nullptr};
};

} // namespace engine

#ifdef CONGELADO_TEST
namespace engine::context_tests {
using namespace boost::ut;

// Minimal fakes — just enough to instantiate a real, distinguishable pointer of each interface
// EngineContext holds. Behavior is irrelevant here; only pointer identity round-trips are under
// test.

class FakeDatabase final : public interfaces::IDatabase
{
public:
    [[nodiscard]] std::string_view backend_name() const noexcept override
    {
        return "fake_db";
    }

    void query(
        std::string_view, std::move_only_function<void(std::string_view)>&& result
    ) noexcept override
    {
        result("");
    }

    void insert(
        std::string_view, std::move_only_function<void(std::string_view)>&& result
    ) noexcept override
    {
        result("");
    }

    void update(
        std::string_view, std::move_only_function<void(std::string_view)>&& result
    ) noexcept override
    {
        result("");
    }

    void remove(
        std::string_view, std::move_only_function<void(std::string_view)>&& result
    ) noexcept override
    {
        result("");
    }
};

class FakeCache final : public interfaces::ICache
{
public:
    [[nodiscard]] std::string_view backend_name() const noexcept override
    {
        return "fake_cache";
    }

    void get(
        std::string_view, std::move_only_function<void(std::string_view)>&& result
    ) noexcept override
    {
        result("");
    }

    void set(
        std::string_view, std::string_view, std::move_only_function<void(std::string_view)>&& result
    ) noexcept override
    {
        result("");
    }

    void remove(
        std::string_view, std::move_only_function<void(std::string_view)>&& result
    ) noexcept override
    {
        result("");
    }
};

class FakeBridge final : public interfaces::IBridge
{
public:
    [[nodiscard]] CongeladoAny from_native(void* /*native_obj*/) override
    {
        return CongeladoAny{};
    }

    void* to_native(const CongeladoAny& /*value*/) override
    {
        return nullptr;
    }

    void install_method(
        std::unique_ptr<FnContext> /*ctx*/, const std::string& /*lang_name*/
    ) override
    {
    }

    [[nodiscard]] std::string_view runtime_name() const noexcept override
    {
        return "fake_lua";
    }

    [[nodiscard]] std::string_view script_extension() const noexcept override
    {
        return ".fake";
    }

    [[nodiscard]] int run_script(std::string_view /*path*/) override
    {
        return 0;
    }
};

class FakeSearchProvider final : public interfaces::ISearchProvider
{
public:
    [[nodiscard]] std::string_view backend_name() const noexcept override
    {
        return "fake_search";
    }

    void index(
        std::string_view,
        std::string_view,
        std::string_view,
        std::move_only_function<void(std::string_view)>&& callback
    ) noexcept override
    {
        callback("ok");
    }

    void remove(
        std::string_view,
        std::string_view,
        std::move_only_function<void(std::string_view)>&& callback
    ) noexcept override
    {
        callback("ok");
    }

    void search(
        std::string_view,
        const interfaces::SearchQuery&,
        std::move_only_function<void(std::string_view)>&& callback
    ) noexcept override
    {
        callback("[]");
    }
};

class FakeCron final : public interfaces::ICron
{
public:
    [[nodiscard]] std::string_view backend_name() const noexcept override
    {
        return "fake_cron";
    }

    [[nodiscard]] bool validate(std::string_view) const noexcept override
    {
        return true;
    }

    [[nodiscard]] std::optional<std::chrono::system_clock::time_point>
    next_after(std::string_view, std::chrono::system_clock::time_point) const noexcept override
    {
        return std::nullopt;
    }

    void set_fire_callback(std::move_only_function<void(std::string_view)>) override {}

    void upsert_job(std::string_view, std::string_view) override {}

    void remove_job(std::string_view) override {}
};

class FakeWorkerOrchestrator final : public interfaces::IWorkerOrchestrator
{
public:
    [[nodiscard]] std::string_view backend_name() const noexcept override
    {
        return "fake_worker_orch";
    }

    void enqueue(
        std::string_view,
        const interfaces::Value&,
        std::move_only_function<void(std::optional<std::string>)> callback
    ) override
    {
        callback(std::nullopt);
    }

    void claim(
        std::string_view,
        std::optional<std::string_view>,
        std::move_only_function<void(std::optional<std::string>)> callback
    ) override
    {
        callback(std::nullopt);
    }

    void submit_result(
        std::string_view,
        bool,
        const std::unordered_map<std::string, std::string>&,
        std::move_only_function<void(bool)> callback
    ) override
    {
        callback(true);
    }

    void requeue(std::string_view, std::move_only_function<void(std::size_t)> callback) override
    {
        callback(0);
    }

    void queue_size(std::string_view, std::move_only_function<void(std::size_t)> callback) override
    {
        callback(0);
    }

    void start_workflow(
        std::string_view,
        const std::unordered_map<std::string, std::string>&,
        std::move_only_function<void(std::optional<std::string>)> callback
    ) override
    {
        callback(std::nullopt);
    }

    void start_server() override {}

    void shutdown_all() override {}

    void set_health_callback(
        std::move_only_function<void(const interfaces::OrchestratorInfo&)>
    ) override
    {
    }
};

class FakeWorkflowOrchestrator final : public interfaces::IWorkflowOrchestrator
{
public:
    [[nodiscard]] std::string_view backend_name() const noexcept override
    {
        return "fake_wf_orch";
    }

    void start_workflow(
        std::string_view,
        const std::unordered_map<std::string, std::string>&,
        std::move_only_function<void(std::optional<std::string>)> callback
    ) override
    {
        callback(std::nullopt);
    }

    void on_task_terminal(std::string_view, std::move_only_function<void(bool)> callback) override
    {
        callback(true);
    }

    void on_execution_terminal(
        std::string_view, std::move_only_function<void(bool)> callback
    ) override
    {
        callback(true);
    }

    void pause(std::string_view, std::move_only_function<void(bool)> callback) override
    {
        callback(true);
    }

    void resume(std::string_view, std::move_only_function<void(bool)> callback) override
    {
        callback(true);
    }

    void retry(std::string_view, std::move_only_function<void(bool)> callback) override
    {
        callback(true);
    }

    void restart(std::string_view, std::move_only_function<void(bool)> callback) override
    {
        callback(true);
    }

    void terminate(std::string_view, std::move_only_function<void(bool)> callback) override
    {
        callback(true);
    }

    void reconcile(std::string_view, std::move_only_function<void(bool)> callback) override
    {
        callback(true);
    }

    void rerun(
        std::string_view,
        std::string_view,
        const interfaces::Value&,
        std::move_only_function<void(bool)> callback
    ) override
    {
        callback(true);
    }

    void signal(
        std::string_view,
        std::string_view,
        std::optional<std::string_view>,
        std::move_only_function<void(bool)> callback
    ) override
    {
        callback(true);
    }

    void complete_task(
        std::string_view,
        std::string_view,
        bool,
        const std::unordered_map<std::string, std::string>&,
        std::move_only_function<void(bool)> callback
    ) override
    {
        callback(true);
    }

    void start_server() override {}

    void shutdown_all() override {}
};

class FakePayloadStorage final : public interfaces::IExternalPayloadStorage
{
public:
    void write(
        interfaces::PayloadType,
        std::string_view,
        std::move_only_function<void(std::string_view)>&& callback
    ) noexcept override
    {
        callback("ref");
    }

    void read(
        std::string_view, std::move_only_function<void(std::string_view)>&& callback
    ) noexcept override
    {
        callback("");
    }
};

suite<"EngineContext"> engine_context_suite = [] {
    "default-constructs local-only, with db and cache unwired"_test = [] {
        engine::EngineContext ctx;
        expect(ctx.get_db() == nullptr);
        expect(ctx.get_cache() == nullptr);
    };

    "set_connector replaces the owned fallback, get_connector reflects it"_test = [] {
        engine::EngineContext ctx;
        connector::Connector external;
        ctx.set_connector(&external);
        expect(&ctx.get_connector() == &external);
    };

    "set_db/get_db round-trip, including dropping back to nullptr"_test = [] {
        engine::EngineContext ctx;
        FakeDatabase db;
        ctx.set_db(&db);
        expect(ctx.get_db() == &db);
        ctx.set_db(nullptr);
        expect(ctx.get_db() == nullptr);
    };

    "set_cache/get_cache round-trip, including dropping back to nullptr"_test = [] {
        engine::EngineContext ctx;
        FakeCache cache;
        ctx.set_cache(&cache);
        expect(ctx.get_cache() == &cache);
        ctx.set_cache(nullptr);
        expect(ctx.get_cache() == nullptr);
    };

    "set_lua_bridge/get_lua_bridge round-trip"_test = [] {
        engine::EngineContext ctx;
        expect(ctx.get_lua_bridge() == nullptr);
        FakeBridge bridge;
        ctx.set_lua_bridge(&bridge);
        expect(ctx.get_lua_bridge() == &bridge);
    };

    "set_search/get_search round-trip"_test = [] {
        engine::EngineContext ctx;
        expect(ctx.get_search() == nullptr);
        FakeSearchProvider search;
        ctx.set_search(&search);
        expect(ctx.get_search() == &search);
    };

    "set_cron/get_cron round-trip"_test = [] {
        engine::EngineContext ctx;
        expect(ctx.get_cron() == nullptr);
        FakeCron cron;
        ctx.set_cron(&cron);
        expect(ctx.get_cron() == &cron);
    };

    "set_orchestrator/get_orchestrator round-trip"_test = [] {
        engine::EngineContext ctx;
        expect(ctx.get_orchestrator() == nullptr);
        FakeWorkerOrchestrator orchestrator;
        ctx.set_orchestrator(&orchestrator);
        expect(ctx.get_orchestrator() == &orchestrator);
    };

    "set_workflow_orchestrator/get_workflow_orchestrator round-trip"_test = [] {
        engine::EngineContext ctx;
        expect(ctx.get_workflow_orchestrator() == nullptr);
        FakeWorkflowOrchestrator workflow;
        ctx.set_workflow_orchestrator(&workflow);
        expect(ctx.get_workflow_orchestrator() == &workflow);
    };

    "set_payload_storage/get_payload_storage round-trip"_test = [] {
        engine::EngineContext ctx;
        expect(ctx.get_payload_storage() == nullptr);
        FakePayloadStorage storage;
        ctx.set_payload_storage(&storage);
        expect(ctx.get_payload_storage() == &storage);
    };
};

} // namespace engine::context_tests
#endif
