export module interfaces:database;

import std;
import shared;

export namespace interfaces {

class IDatabase {
  public:
    virtual ~IDatabase() = default;

    [[nodiscard]] virtual std::string_view backend_name() const noexcept = 0;
    [[nodiscard]] virtual bool required() const noexcept { return false; }

    virtual void query(std::string_view payload, shared::QueryReadFn result) noexcept = 0;
    virtual void insert(std::string_view payload, shared::QueryReadFn result) noexcept = 0;
    virtual void update(std::string_view payload, shared::QueryReadFn result) noexcept = 0;
    virtual void remove(std::string_view payload, shared::QueryReadFn result) noexcept = 0;
};

} // namespace interfaces
