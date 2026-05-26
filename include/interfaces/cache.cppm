export module interfaces:cache;

import std;
import shared;

export namespace interfaces {

class ICache {
  public:
    virtual ~ICache() = default;

    [[nodiscard]] virtual std::string_view backend_name() const noexcept = 0;
    [[nodiscard]] virtual bool required() const noexcept { return false; }

    virtual void get(std::string_view key, shared::QueryReadFn &&result) noexcept = 0;
    virtual void set(std::string_view key, std::string_view value, shared::QueryReadFn &&result) noexcept = 0;
    virtual void remove(std::string_view key, shared::QueryReadFn &&result) noexcept = 0;
};

} // namespace interfaces
