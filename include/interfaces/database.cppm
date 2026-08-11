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
     * @brief Says whether this backend actually has a live connection right now, as opposed to
     * merely being the resolved `IDatabase*` for a loaded storage plugin.
     * @note A plugin can be "attached" (resolved, non-null pointer wired into the connector)
     * while its underlying connection failed at startup — e.g. Postgres leaves `m_conn` null on
     * a failed connect and degrades every query to an empty result rather than crashing. Callers
     * that gate work on "is a database attached" (migrations, anything that can't tolerate a
     * silently-empty result) should check this, not just pointer-non-null.
     * @return true if not overridden — matches the previous nullptr-only gating behavior for any
     * backend that has no distinct connect step to fail.
     */
    [[nodiscard]] virtual bool is_connected() const noexcept { return true; }

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
