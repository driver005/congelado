export module core_logger:registry;

import std;
import interfaces;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace core::logger {

/**
 * @brief Holds every registered logger for one process. Instance-owned (not a static
 * singleton) — exactly one lives inside `congelado::heart::AppContext` (the engine) or a local
 * variable in the worker's `main()`, and `set_active()` points the ambient
 * `core::logger::error(...)`-style free-function facade (~417 call sites across the codebase,
 * deliberately kept untouched) at it. Only `s_active` — a single pointer, not the logger data
 * itself — is process-global.
 */
class LoggerRegistry {
  public:
    // Appends a logger. No-op if null. Multiple loggers all receive every message.
    /**
     * @brief Registers a logger so it starts catching every fanned-out log call.
     * @note No-op if `logger` is null — silently dropped, no error, no throw. Once
     * registered there's no unregister — it's riding with this instance for good.
     * @param logger the logger instance to add to the registry.
     */
    void add_logger(std::shared_ptr<interfaces::ILogger> logger) {
        if (logger) {
            m_loggers.push_back(std::move(logger));
        }
    }

    /**
     * @brief Checks whether the registry currently holds any logger.
     * @return true if at least one logger is registered, false if it's still empty.
     */
    [[nodiscard]] bool has_logger() const noexcept { return !m_loggers.empty(); }

    /**
     * @brief Gets every logger currently registered, in registration order.
     * @return the full list of registered loggers.
     */
    [[nodiscard]] const std::vector<std::shared_ptr<interfaces::ILogger>> &get_loggers() const noexcept {
        return m_loggers;
    }

    /**
     * @brief Points the ambient logging facade at this instance — call once, right after
     * constructing the process's one `LoggerRegistry`, before any `core::logger::*` call.
     * @param registry the instance to make active, or `nullptr` to clear it.
     */
    static void set_active(LoggerRegistry *registry) noexcept { s_active = registry; }

    /**
     * @brief Gets the currently active registry, if one was set.
     * @return the active `LoggerRegistry`, or `nullptr` if `set_active()` was never called.
     */
    [[nodiscard]] static LoggerRegistry *get_active() noexcept { return s_active; }

  private:
    std::vector<std::shared_ptr<interfaces::ILogger>> m_loggers;
    static inline LoggerRegistry *s_active{nullptr};
};

} // namespace core::logger

#ifdef CONGELADO_TEST
namespace core::logger::tests {
using namespace boost::ut;

class LoggerRegistryFakeLogger : public interfaces::ILogger {
  public:
    [[nodiscard]] std::string_view get_name() const noexcept override { return "fake"; }
    void write(interfaces::LogLevel, std::string_view) noexcept override {}
    void error(std::string_view) noexcept override {}
};

suite<"LoggerRegistry"> registry_suite = [] {
    "starts empty"_test = [] {
        LoggerRegistry registry;
        expect(not registry.has_logger());
        expect(registry.get_loggers().empty());
    };

    "add_logger registers a logger"_test = [] {
        LoggerRegistry registry;
        registry.add_logger(std::make_shared<LoggerRegistryFakeLogger>());

        expect(registry.has_logger());
        expect(registry.get_loggers().size() == 1);
    };

    "add_logger ignores a null logger"_test = [] {
        LoggerRegistry registry;
        registry.add_logger(nullptr);

        expect(not registry.has_logger());
    };

    "multiple loggers accumulate in registration order"_test = [] {
        LoggerRegistry registry;
        auto first = std::make_shared<LoggerRegistryFakeLogger>();
        auto second = std::make_shared<LoggerRegistryFakeLogger>();
        registry.add_logger(first);
        registry.add_logger(second);

        expect(registry.get_loggers().size() == 2);
        expect(registry.get_loggers()[0] == first);
        expect(registry.get_loggers()[1] == second);
    };

    "set_active/get_active round-trip"_test = [] {
        auto *previous = LoggerRegistry::get_active();

        LoggerRegistry registry;
        LoggerRegistry::set_active(&registry);
        expect(LoggerRegistry::get_active() == &registry);

        LoggerRegistry::set_active(nullptr);
        expect(LoggerRegistry::get_active() == nullptr);

        LoggerRegistry::set_active(previous);
    };

    // Documents that nothing in LoggerRegistry's lifecycle clears s_active automatically:
    // destroying the actively-registered instance leaves the ambient pointer dangling until a
    // caller explicitly calls set_active(nullptr). This test demonstrates the gap by performing
    // that cleanup itself, from a fresh scope, after the instance is already gone -- it never
    // reads get_active() while the pointer is dangling.
    "no automatic cleanup: destroying the active instance leaves s_active dangling until cleared"_test = [] {
        auto *previous = LoggerRegistry::get_active();

        {
            LoggerRegistry registry;
            LoggerRegistry::set_active(&registry);
            expect(LoggerRegistry::get_active() == &registry);
        } // registry destroyed here -- s_active still points at the freed instance, nothing
          // clears it automatically

        // Explicit cleanup the class itself never performs on destruction.
        LoggerRegistry::set_active(nullptr);
        expect(LoggerRegistry::get_active() == nullptr);

        LoggerRegistry::set_active(previous);
    };
};

} // namespace core::logger::tests
#endif
