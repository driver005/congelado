export module engine:context;

import connector;
import interfaces;
import model;

export namespace engine {

// Runtime dependency bundle injected once at startup before any request is dispatched.
class EngineContext {
  public:
    void set_db(interfaces::IDatabase *database) noexcept {
        m_db = database;
        m_connector.set_database(database);
    }

    void set_cache(interfaces::ICache *cache) noexcept {
        m_cache = cache;
        m_connector.set_cache(cache);
    }

    [[nodiscard]] interfaces::IDatabase *get_db() const noexcept { return m_db; }
    [[nodiscard]] interfaces::ICache *get_cache() const noexcept { return m_cache; }

    [[nodiscard]] connector::Connector &get_connector() noexcept { return m_connector; }

  private:
    interfaces::IDatabase *m_db{nullptr};
    interfaces::ICache *m_cache{nullptr};

    connector::Connector m_connector;
};

} // namespace engine
