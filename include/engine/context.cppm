export module engine:context;

import connector;
import interfaces;
import model;

export namespace engine {

// Runtime dependency bundle injected once at startup before any request is dispatched.
class EngineContext {
  public:
    void set_db(interfaces::IDatabase *database) noexcept { m_connector.set_database(database); }

    void set_cache(interfaces::ICache *cache) noexcept { m_connector.set_cache(cache); }

    [[nodiscard]] interfaces::IDatabase *get_db() noexcept { return m_connector.get_database(); }

    [[nodiscard]] interfaces::ICache *get_cache() noexcept { return m_connector.get_cache(); }

    [[nodiscard]] connector::Connector &get_connector() noexcept { return m_connector; }

  private:
    connector::Connector m_connector;
};

} // namespace engine
