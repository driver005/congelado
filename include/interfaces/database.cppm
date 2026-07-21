export module interfaces:database;

import std;
import shared;

export namespace interfaces {

class IDatabase {
  public:
    /**
     * @brief Virtual dtor, default's good — polymorphic db backends clean up fine through the
     * base pointer, no extra motion needed.
     */
    virtual ~IDatabase() = default;
    IDatabase() = default;
    IDatabase(const IDatabase &) = delete;
    IDatabase &operator=(const IDatabase &) = delete;
    IDatabase(IDatabase &&) = delete;
    IDatabase &operator=(IDatabase &&) = delete;

    /**
     * @brief Tells you which db backend is actually running the show behind this interface
     * (postgres, sqlite, whatever got plugged in).
     * @return the backend's name.
     */
    [[nodiscard]] virtual std::string_view backend_name() const noexcept = 0;
    /**
     * @brief Says whether this db is load-bearing or just optional infra riding along.
     * @note Defaults to `false` — flip to `true` if the app genuinely can't function without
     * this backend being up, so failures here get treated as the fatal L they are instead of
     * getting shrugged off like nothing happened.
     * @return true if this db is a hard requirement, false if it's optional motion.
     */
    [[nodiscard]] virtual bool required() const noexcept { return false; }

    /**
     * @brief Fires an async query against the backend — no blocking, result comes back through
     * the callback whenever it's actually ready.
     * @param payload the query payload (already encoded, backend-specific format).
     * @param result callback that gets the query outcome.
     */
    virtual void query(std::string_view payload, shared::QueryReadFn &&result) noexcept = 0;
    /**
     * @brief Fires an async insert against the backend, fresh row going in, result comes back
     * through the callback.
     * @param payload the insert payload (already encoded, backend-specific format).
     * @param result callback that gets the insert outcome.
     */
    virtual void insert(std::string_view payload, shared::QueryReadFn &&result) noexcept = 0;
    /**
     * @brief Fires an async update against the backend, result comes back through the callback.
     * @param payload the update payload (already encoded, backend-specific format).
     * @param result callback that gets the update outcome.
     */
    virtual void update(std::string_view payload, shared::QueryReadFn &&result) noexcept = 0;
    /**
     * @brief Fires an async delete against the backend — that row's not coming back, result
     * shows up through the callback.
     * @param payload the remove payload (already encoded, backend-specific format).
     * @param result callback that gets the removal outcome.
     */
    virtual void remove(std::string_view payload, shared::QueryReadFn &&result) noexcept = 0;
};

} // namespace interfaces
