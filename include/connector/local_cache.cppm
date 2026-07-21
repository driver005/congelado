export module connector:local_cache;

import interfaces;
import shared;
import std;

export namespace connector {

class LocalCache : public interfaces::ICache {
  public:
    /**
     * @brief Identifies this cache backend.
     * @return the fixed string "local" — this is the in-process fallback, no external service
     * behind it.
     */
    [[nodiscard]] std::string_view backend_name() const noexcept override { return "local"; }

    /**
     * @brief Says whether this backend is load-bearing.
     * @return always `false` — this is Connector's fallback cache, it's optional motion by
     * design, never the hard requirement.
     */
    [[nodiscard]] bool required() const noexcept override { return false; }

    /**
     * @brief Looks up `key` in the in-memory store.
     * @note Fully synchronous under the hood despite the async-shaped signature — `result`
     * fires before this call even returns.
     * @param key the key to look up.
     * @param result gets the stored value, or an empty string if `key` isn't in the store.
     */
    void get(std::string_view key, shared::QueryReadFn &&result) noexcept override {
        auto iterator = m_store.find(std::string{key});
        // Miss reports back an empty string; hit hands back the stored value — both fire
        // synchronously despite the async-shaped callback signature.
        if (iterator == m_store.end()) {
            std::move(result)("");
        } else {
            std::move(result)(iterator->second);
        }
    }

    /**
     * @brief Writes `value` under `key`, overwriting whatever was there before.
     * @param key the key to write to.
     * @param value the value getting stashed.
     * @param result always gets an empty string — this backend never fails a write, so there's
     * no outcome payload worth reporting.
     */
    void set(std::string_view key, std::string_view value,
             shared::QueryReadFn &&result) noexcept override {
        m_store.insert_or_assign(std::string{key}, std::string{value});
        std::move(result)("");
    }

    /**
     * @brief Yeets whatever's stored under `key` out of the map. No-op, no error, if the key
     * was never there to begin with — bet either way.
     * @param key the key to remove.
     * @param result always gets an empty string, same deal as set().
     */
    void remove(std::string_view key, shared::QueryReadFn &&result) noexcept override {
        m_store.erase(std::string{key});
        std::move(result)("");
    }

  private:
    std::unordered_map<std::string, std::string> m_store;
};

} // namespace connector
