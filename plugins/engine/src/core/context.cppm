export module engine:context;

import connector;
import interfaces;
import model;

export namespace engine {

// Runtime dependency bundle injected once at startup before any request is dispatched.
class EngineContext {
  public:
    /**
     * @brief Wires in the database backend, forwarding straight through to the underlying
     * Connector. Every handler sharing this context sees the new backend from here on out.
     * @param database the database backend to use going forward, or nullptr to drop back to
     * local-only mode.
     */
    void set_db(interfaces::IDatabase *database) noexcept { m_connector.set_database(database); }

    /**
     * @brief Wires in the cache backend, forwarding straight through to the underlying
     * Connector.
     * @param cache the cache backend to use going forward, or nullptr to drop back to the
     * Connector's built-in local cache.
     */
    void set_cache(interfaces::ICache *cache) noexcept { m_connector.set_cache(cache); }

    /**
     * @brief Gets the currently configured database backend.
     * @return the database pointer, or nullptr if this context is running local-only — check
     * before you dereference, no safety net here.
     */
    [[nodiscard]] interfaces::IDatabase *get_db() noexcept { return m_connector.get_database(); }

    /**
     * @brief Gets the currently configured cache backend.
     * @return the cache pointer, or nullptr if none's wired up.
     */
    [[nodiscard]] interfaces::ICache *get_cache() noexcept { return m_connector.get_cache(); }

    /**
     * @brief Wires in the resolved "lua" IBridge, if one was found — used by Orchestrator's
     * LuaEval to evaluate SWITCH edge conditions, DO_WHILE loop conditions, and (Phase 5)
     * EventHandler conditions.
     * @param bridge the resolved lua IBridge*, or nullptr if none was found.
     */
    void set_lua_bridge(interfaces::IBridge *bridge) noexcept { m_lua_bridge = bridge; }

    /**
     * @brief Gets the currently wired-in lua bridge.
     * @return the bridge pointer, or nullptr if none was resolved — check before dereferencing,
     * no safety net here.
     */
    [[nodiscard]] interfaces::IBridge *get_lua_bridge() noexcept { return m_lua_bridge; }

    /**
     * @brief Wires in the resolved search-capable backend, if one was found — used by
     * SummaryProjector to push WorkflowSummary/TaskSummary projections on every terminal
     * transition. No provider configured is a valid state, not an error — search routes just
     * degrade to empty results.
     * @param provider the resolved ISearchProvider*, or nullptr if none was found.
     */
    void set_search(interfaces::ISearchProvider *provider) noexcept { m_search = provider; }

    /**
     * @brief Gets the currently wired-in search provider.
     * @return the provider pointer, or nullptr if none was resolved — check before
     * dereferencing, no safety net here.
     */
    [[nodiscard]] interfaces::ISearchProvider *get_search() noexcept { return m_search; }

    /**
     * @brief Wires in the external payload storage backend — unlike db/lua_bridge/search, this
     * isn't a resolved plugin capability, just a plain object EnginePlugin::on_load constructs
     * directly (see LocalPayloadStorage's own docs on why no capability-plugin resolution is
     * needed for the local-disk default).
     * @param storage the storage instance to use going forward; this context does not take
     * ownership — caller keeps it alive for this context's whole lifetime.
     */
    void set_payload_storage(interfaces::IExternalPayloadStorage *storage) noexcept {
        m_payload_storage = storage;
    }

    /**
     * @brief Gets the currently wired-in payload storage backend.
     * @return the storage pointer, or nullptr if none was set — check before dereferencing, no
     * safety net here.
     */
    [[nodiscard]] interfaces::IExternalPayloadStorage *get_payload_storage() noexcept {
        return m_payload_storage;
    }

    /**
     * @brief Gets the underlying Connector this context wraps — this class is just a thin
     * bundle around it, the connector's where the actual db/cache motion happens.
     * @return a reference to the connector instance.
     */
    [[nodiscard]] connector::Connector &get_connector() noexcept { return m_connector; }

  private:
    connector::Connector m_connector;
    interfaces::IBridge *m_lua_bridge{nullptr};
    interfaces::ISearchProvider *m_search{nullptr};
    interfaces::IExternalPayloadStorage *m_payload_storage{nullptr};
};

} // namespace engine
