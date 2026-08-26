module;
#ifdef CONGELADO_TEST
#    include "core/manager/abi.h"
#endif

export module workflow_engine:context;

import connector;
import interfaces;
#ifdef CONGELADO_TEST
import shared;
import boost.ut;
#endif

export namespace engine {

/// @brief Runtime dependency bundle for the moved DAG orchestrator — the subset of the engine's
/// old `EngineContext` the orchestrator actually uses: the shared `Connector`, the resolved
/// "lua" `IBridge` (edge/loop/event conditions via LuaEval + SystemTaskExecutor), and the
/// search provider (SummaryProjector's terminal-transition projections). Built once in the
/// plugin's on_load from the host callbacks. Named `WorkflowContext` so the moved
/// `Orchestrator`/`SummaryProjector` code reads the same as before (the sed rename from
/// `EngineContext`).
class WorkflowContext
{
public:
    WorkflowContext() = default;

    /// @brief Wires in the shared connector. @param connector the connector for all DB/cache
    /// ops.
    void set_connector(connector::Connector* connector) noexcept
    {
        m_connector = connector;
    }

    /// @brief Wires in the resolved "lua" bridge. @param bridge the lua IBridge*, or nullptr.
    void set_lua_bridge(interfaces::IBridge* bridge) noexcept
    {
        m_lua_bridge = bridge;
    }

    /// @brief Wires in the resolved search provider. @param provider the ISearchProvider*, or
    /// nullptr.
    void set_search(interfaces::ISearchProvider* provider) noexcept
    {
        m_search = provider;
    }

    /// @brief Gets the connector. @return a reference to the connector instance.
    [[nodiscard]] connector::Connector& get_connector() noexcept
    {
        return *m_connector;
    }

    /// @brief Gets the lua bridge. @return the bridge pointer, or nullptr if none was resolved.
    [[nodiscard]] interfaces::IBridge* get_lua_bridge() noexcept
    {
        return m_lua_bridge;
    }

    /// @brief Gets the search provider. @return the provider pointer, or nullptr if none.
    [[nodiscard]] interfaces::ISearchProvider* get_search() noexcept
    {
        return m_search;
    }

private:
    connector::Connector* m_connector{nullptr};
    interfaces::IBridge* m_lua_bridge{nullptr};
    interfaces::ISearchProvider* m_search{nullptr};
};

} // namespace engine

#ifdef CONGELADO_TEST
namespace engine::workflow_context_tests {
using namespace boost::ut;

/// @brief Minimal ISearchProvider test double — every op is a no-op, just enough to exist as a
/// non-null pointer for WorkflowContext's search slot round-trip.
class FakeSearchProvider final : public interfaces::ISearchProvider
{
public:
    [[nodiscard]] std::string_view backend_name() const noexcept override
    {
        return "fake_search";
    }

    void index(
        std::string_view /*collection*/,
        std::string_view /*id*/,
        std::string_view /*document_json*/,
        shared::QueryReadFn&& callback
    ) noexcept override
    {
        callback("ok");
    }

    void remove(
        std::string_view /*collection*/, std::string_view /*id*/, shared::QueryReadFn&& callback
    ) noexcept override
    {
        callback("ok");
    }

    void search(
        std::string_view /*collection*/,
        const interfaces::SearchQuery& /*query*/,
        shared::QueryReadFn&& callback
    ) noexcept override
    {
        callback("[]");
    }
};

/// @brief Minimal IBridge test double — enough to exist as a non-null pointer for
/// WorkflowContext's lua_bridge slot round-trip.
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
        return ".lua";
    }

    [[nodiscard]] int run_script(std::string_view /*path*/) override
    {
        return 0;
    }
};

suite<"WorkflowContext"> workflow_context_suite = [] {
    "defaults every slot to nullptr"_test = [] {
        WorkflowContext ctx;
        expect(ctx.get_lua_bridge() == nullptr);
        expect(ctx.get_search() == nullptr);
    };

    "set_connector/get_connector round-trips"_test = [] {
        WorkflowContext ctx;
        connector::Connector connector;
        ctx.set_connector(&connector);
        expect(&ctx.get_connector() == &connector);
    };

    "set_lua_bridge/get_lua_bridge round-trips, including clearing back to nullptr"_test = [] {
        WorkflowContext ctx;
        FakeBridge bridge;
        ctx.set_lua_bridge(&bridge);
        expect(ctx.get_lua_bridge() == &bridge);

        ctx.set_lua_bridge(nullptr);
        expect(ctx.get_lua_bridge() == nullptr);
    };

    "set_search/get_search round-trips, including clearing back to nullptr"_test = [] {
        WorkflowContext ctx;
        FakeSearchProvider provider;
        ctx.set_search(&provider);
        expect(ctx.get_search() == &provider);

        ctx.set_search(nullptr);
        expect(ctx.get_search() == nullptr);
    };
};

} // namespace engine::workflow_context_tests
#endif
