module engine.context;

@nogc nothrow:

import interfaces.interfaces;
import connector.connector;

// Runtime dependency bundle injected once at startup before any request is dispatched.
class EngineContext {
    void set_db(IDatabase database) {
        m_db = database;
        m_connector.set_database(database);
    }

    void set_cache(ICache cache) {
        m_cache = cache;
        m_connector.set_cache(cache);
    }

    IDatabase get_db() const { return m_db; }
    ICache get_cache() const { return m_cache; }

    ref Connector get_connector() { return m_connector; }

  private:
    IDatabase m_db;
    ICache m_cache;

    Connector m_connector;
}
