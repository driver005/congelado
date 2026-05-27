export module engine:context;

import interfaces;

export namespace engine {

// Runtime dependency bundle injected once at startup before any request is dispatched.
class EngineContext {
  public:
    void set_db(interfaces::IDatabase *database) noexcept { m_db = database; }
    void set_cache(interfaces::ICache *cache) noexcept { m_cache = cache; }

    [[nodiscard]] interfaces::IDatabase *get_db() const noexcept { return m_db; }
    [[nodiscard]] interfaces::ICache *get_cache() const noexcept { return m_cache; }

  private:
    interfaces::IDatabase *m_db{nullptr};
    interfaces::ICache *m_cache{nullptr};
};

} // namespace engine
