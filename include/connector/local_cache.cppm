export module connector:local_cache;

import interfaces;
import shared;
import std;

export namespace connector {

class LocalCache : public interfaces::ICache {
  public:
    [[nodiscard]] std::string_view backend_name() const noexcept override { return "local"; }

    [[nodiscard]] bool required() const noexcept override { return false; }

    void get(std::string_view key, shared::QueryReadFn &&result) noexcept override {
        auto iterator = m_store.find(std::string{key});
        if (iterator == m_store.end())
            result("");
        else
            result(iterator->second);
    }

    void set(std::string_view key, std::string_view value,
             shared::QueryReadFn &&result) noexcept override {
        m_store.insert_or_assign(std::string{key}, std::string{value});
        result("");
    }

    void remove(std::string_view key, shared::QueryReadFn &&result) noexcept override {
        m_store.erase(std::string{key});
        result("");
    }

  private:
    std::unordered_map<std::string, std::string> m_store;
};

} // namespace connector
