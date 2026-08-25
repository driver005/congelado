export module core_events:registry;

import std;
import interfaces;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace core::events {

/**
 * @brief Holds every registered event sink for one process. Instance-owned (not a static
 * singleton) — same shape as `core::logger::LoggerRegistry`: exactly one lives inside
 * `congelado::heart::AppContext`, and `set_active()` points the ambient `core::events::publish()`
 * free-function facade at it. Only `s_active` — a single pointer, not the sink data itself — is
 * process-global. Multiple sinks can be registered at once and all receive every published
 * event, same fan-out story as `LoggerRegistry`.
 */
class EventBusRegistry {
  public:
    /**
     * @brief Registers a sink so it starts receiving every published event.
     * @note No-op if `sink` is null — silently dropped, no error, no throw. Once registered
     * there's no unregister — it's riding with this instance for good.
     * @param sink the sink instance to add to the registry.
     */
    void add_sink(std::shared_ptr<interfaces::IEventSink> sink) {
        if (sink) {
            m_sinks.push_back(std::move(sink));
        }
    }

    /**
     * @brief Checks whether the registry currently holds any sink.
     * @return true if at least one sink is registered, false if it's still empty.
     */
    [[nodiscard]] bool has_sink() const noexcept { return !m_sinks.empty(); }

    /**
     * @brief Gets every sink currently registered, in registration order.
     * @return the full list of registered sinks.
     */
    [[nodiscard]] const std::vector<std::shared_ptr<interfaces::IEventSink>> &
    get_sinks() const noexcept {
        return m_sinks;
    }

    /**
     * @brief Points the ambient publish facade at this instance — call once, right after
     * constructing the process's one `EventBusRegistry`, before any `core::events::publish()`
     * call.
     * @param registry the instance to make active, or `nullptr` to clear it.
     */
    static void set_active(EventBusRegistry *registry) noexcept { s_active = registry; }

    /**
     * @brief Gets the currently active registry, if one was set.
     * @return the active `EventBusRegistry`, or `nullptr` if `set_active()` was never called.
     */
    [[nodiscard]] static EventBusRegistry *get_active() noexcept { return s_active; }

  private:
    std::vector<std::shared_ptr<interfaces::IEventSink>> m_sinks;
    static inline EventBusRegistry *s_active{nullptr};
};

} // namespace core::events

#ifdef CONGELADO_TEST
namespace core::events::tests {
using namespace boost::ut;

class EventBusRegistryFakeSink : public interfaces::IEventSink {
  public:
    [[nodiscard]] std::string_view get_name() const noexcept override { return "fake"; }
    void publish(std::string_view, std::string_view) noexcept override {}
};

suite<"EventBusRegistry"> registry_suite = [] {
    "starts empty"_test = [] {
        EventBusRegistry registry;
        expect(not registry.has_sink());
        expect(registry.get_sinks().empty());
    };

    "add_sink registers a sink"_test = [] {
        EventBusRegistry registry;
        registry.add_sink(std::make_shared<EventBusRegistryFakeSink>());

        expect(registry.has_sink());
        expect(registry.get_sinks().size() == 1);
    };

    "add_sink ignores a null sink"_test = [] {
        EventBusRegistry registry;
        registry.add_sink(nullptr);

        expect(not registry.has_sink());
    };

    "multiple sinks accumulate in registration order"_test = [] {
        EventBusRegistry registry;
        auto first = std::make_shared<EventBusRegistryFakeSink>();
        auto second = std::make_shared<EventBusRegistryFakeSink>();
        registry.add_sink(first);
        registry.add_sink(second);

        expect(registry.get_sinks().size() == 2);
        expect(registry.get_sinks()[0] == first);
        expect(registry.get_sinks()[1] == second);
    };

    "set_active/get_active round-trip"_test = [] {
        auto *previous = EventBusRegistry::get_active();

        EventBusRegistry registry;
        EventBusRegistry::set_active(&registry);
        expect(EventBusRegistry::get_active() == &registry);

        EventBusRegistry::set_active(nullptr);
        expect(EventBusRegistry::get_active() == nullptr);

        EventBusRegistry::set_active(previous);
    };

    // Documents that nothing in EventBusRegistry's lifecycle clears s_active automatically:
    // destroying the actively-registered instance leaves the ambient pointer dangling until a
    // caller explicitly calls set_active(nullptr). This test demonstrates the gap by performing
    // that cleanup itself, from a fresh scope, after the instance is already gone -- it never
    // reads get_active() while the pointer is dangling.
    "no automatic cleanup: destroying the active instance leaves s_active dangling until cleared"_test = [] {
        auto *previous = EventBusRegistry::get_active();

        {
            EventBusRegistry registry;
            EventBusRegistry::set_active(&registry);
            expect(EventBusRegistry::get_active() == &registry);
        } // registry destroyed here -- s_active still points at the freed instance, nothing
          // clears it automatically

        // Explicit cleanup the class itself never performs on destruction.
        EventBusRegistry::set_active(nullptr);
        expect(EventBusRegistry::get_active() == nullptr);

        EventBusRegistry::set_active(previous);
    };
};

} // namespace core::events::tests
#endif
