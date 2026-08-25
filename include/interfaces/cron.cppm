export module interfaces:cron;

import std;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace interfaces {

/// @brief Pluggable scheduling engine (the "cron" capability). Single-active-backend, same shape
/// as `ISearchProvider`/`ICache` — a deployment picks exactly one cron backend at a time. Owns
/// both the cron-expression math (validate/next_after) and the actual timing loop: the host
/// registers named jobs, hands over one fire callback, and the backend invokes that callback with
/// a job's name when its schedule comes due. The backend knows nothing about what a "job" does —
/// the name is opaque, and the callback owner (the engine plugin) maps it back to a workflow.
class ICron {
  public:
    /**
     * @brief Virtual dtor, default's good — polymorphic cron backends clean up fine through the
     * base pointer.
     */
    virtual ~ICron() = default;
    ICron() = default;
    ICron(const ICron &) = delete;
    ICron &operator=(const ICron &) = delete;
    ICron(ICron &&) = delete;
    ICron &operator=(ICron &&) = delete;

    /**
     * @brief Tells you which cron backend is actually running the show (the built-in "local"
     * poll-sweep, an external cron engine, whatever got plugged in).
     * @return the backend's name.
     */
    [[nodiscard]] virtual std::string_view backend_name() const noexcept = 0;
    /**
     * @brief Says whether this backend is load-bearing. Unlike search/cache, cron defaults to
     * required — with no cron backend, schedules simply never fire, so the engine treats its
     * absence as a real misconfiguration to log, not a silent degrade.
     * @return true by default.
     */
    [[nodiscard]] virtual bool required() const noexcept { return true; }

    /**
     * @brief Checks whether a cron expression is well-formed for this backend.
     * @param cron_expression the expression to validate.
     * @return true if the backend can schedule against it, false otherwise.
     */
    [[nodiscard]] virtual bool validate(std::string_view cron_expression) const noexcept = 0;
    /**
     * @brief Computes the next fire time strictly after `base` for a cron expression — powers the
     * engine's next-few-runs preview without registering a job.
     * @param cron_expression the expression to evaluate.
     * @param base the time to search strictly after.
     * @return the next matching time_point, or std::nullopt if the expression is invalid or has no
     * match within the backend's horizon.
     */
    [[nodiscard]] virtual std::optional<std::chrono::system_clock::time_point>
    next_after(std::string_view cron_expression,
               std::chrono::system_clock::time_point base) const noexcept = 0;

    /**
     * @brief Installs the callback the backend invokes (with the job's name) whenever a registered
     * job comes due. Called once, before any job is registered.
     * @param callback the fire callback; takes the due job's name.
     */
    virtual void set_fire_callback(std::move_only_function<void(std::string_view)> callback) = 0;
    /**
     * @brief Registers or updates a named job. Re-registering an existing name replaces its
     * expression, keeping its firing history.
     * @param name the job's unique name (opaque to the backend).
     * @param cron_expression the schedule to fire it on.
     */
    virtual void upsert_job(std::string_view name, std::string_view cron_expression) = 0;
    /**
     * @brief Removes a named job so it stops firing. No-op if the name isn't registered.
     * @param name the job to remove.
     */
    virtual void remove_job(std::string_view name) = 0;
};

} // namespace interfaces

#ifdef CONGELADO_TEST
namespace interfaces::cron_tests {

// Minimal ICron fixture — every pure virtual gets a trivial body so required()'s default
// implementation can be exercised in isolation.
class MockCron final : public ICron {
  public:
    [[nodiscard]] std::string_view backend_name() const noexcept override { return "mock-cron"; }
    [[nodiscard]] bool validate(std::string_view) const noexcept override { return true; }
    [[nodiscard]] std::optional<std::chrono::system_clock::time_point>
    next_after(std::string_view, std::chrono::system_clock::time_point) const noexcept override {
        return std::nullopt;
    }
    void set_fire_callback(std::move_only_function<void(std::string_view)>) override {}
    void upsert_job(std::string_view, std::string_view) override {}
    void remove_job(std::string_view) override {}
};

using namespace boost::ut;

suite<"ICron"> cron_suite = [] {
    "required() defaults to true when not overridden"_test = [] {
        MockCron cron;
        expect(cron.required());
    };
};

} // namespace interfaces::cron_tests
#endif
