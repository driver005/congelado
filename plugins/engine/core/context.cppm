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
     * @brief Gets the underlying Connector this context wraps — this class is just a thin
     * bundle around it, the connector's where the actual db/cache motion happens.
     * @return a reference to the connector instance.
     */
    [[nodiscard]] connector::Connector &get_connector() noexcept { return m_connector; }

  private:
    connector::Connector m_connector;
};

} // namespace engine
