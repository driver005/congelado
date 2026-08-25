module;

#define CONGELADO_GUEST
#include <congelado/plugin.h>

export module memory_events_plugin;

import congelado_plugin;
import interfaces;
import core_logger;
import std;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

/**
 * @brief Default `IEventSink` — a bounded, thread-safe in-process ring buffer plus a debug log
 * line per publish. Zero external dependencies, always buildable — the fallback backend so
 * `core::events::publish(...)` has somewhere to go even with no message broker configured.
 * @warning No inspection route in this pass (see the event-bus plan's "out of scope" section) —
 * the buffer exists for future use and is visible today only via its own debug logging.
 */
class MemoryEventsPlugin : public congelado::Plugin, public interfaces::IEventSink {
  public:
    [[nodiscard]] std::string_view get_name() const noexcept override { return "memory"; }
    [[nodiscard]] std::string_view get_version() const noexcept override { return "0.1.0"; }
    [[nodiscard]] std::uint32_t capabilities() const noexcept override {
        return CONGELADO_CAP_EVENTS;
    }

    /**
     * @brief Reads the ring buffer's max size out of config.
     * @param host unused — this plugin doesn't read any host callback fields.
     * @param cfg this plugin's config view; reads `max_events` (default `200`).
     */
    void on_load(CongeladoHostCallbacks const & /*host*/,
                CongeladoConfigView const &cfg) override {
        auto max_str = congelado::config_get(cfg, "max_events").value_or("200");
        try {
            m_max_events = static_cast<std::size_t>(std::stoul(max_str));
        } catch (...) {
            m_max_events = 200;
        }
        core::logger::debug("events", "memory sink ready, max_events={}", m_max_events);
    }

    /**
     * @brief Capability hook the host calls to get at this plugin's `IEventSink` surface.
     * @return this instance, upcast to `interfaces::IEventSink*`.
     */
    void *event_get() noexcept { return static_cast<interfaces::IEventSink *>(this); }

    /**
     * @brief Records `event_name`/`payload_json` in the bounded ring buffer (oldest evicted once
     * `max_events` is exceeded) and logs it at debug level — this sink's whole job.
     * @param event_name the published event's name.
     * @param payload_json the event's JSON-encoded payload.
     */
    void publish(std::string_view event_name, std::string_view payload_json) noexcept override {
        core::logger::debug("events", "[{}] {}", event_name, payload_json);
        try {
            std::lock_guard lock{m_mutex};
            m_recent.emplace_back(std::string{event_name}, std::string{payload_json});
            while (m_recent.size() > m_max_events) {
                m_recent.pop_front();
            }
        } catch (...) {
            // Ring-buffer bookkeeping must never take the process down over an allocation
            // failure — the debug log line above already happened, that's the durable part.
        }
    }

  private:
    std::mutex m_mutex;
    std::deque<std::pair<std::string, std::string>> m_recent;
    std::size_t m_max_events{200};
};

CONGELADO_PLUGIN(MemoryEventsPlugin);

#ifdef CONGELADO_TEST
namespace memory_events_plugin_tests {
using namespace boost::ut;

/// @brief Small test-only helper class — keeps the "class-only, no free functions" convention
/// even for test scaffolding. Builds a `CongeladoConfigView` over caller-owned key/value arrays.
class MemoryEventsTestHelper {
  public:
    MemoryEventsTestHelper() = delete;

    [[nodiscard]] static CongeladoConfigView single_field(const char *const *keys,
                                                           const char *const *values,
                                                           std::size_t count) noexcept {
        return CongeladoConfigView{.keys = keys, .values = values, .count = count};
    }
};

// NOTE on coverage gap, deliberate: m_recent (the bounded ring buffer) has no inspection route —
// per this class's own @warning, IEventSink exposes no getter for it, only publish()'s debug log
// line observes an event ever landed. So `publish()`'s eviction behavior (oldest dropped once
// m_max_events is exceeded) and on_load()'s parsed m_max_events value are both untestable via the
// public surface — every test below instead confirms the safe, no-crash shape of these calls
// (including adversarial input), same spirit as the ring-buffer-bookkeeping catch-all in
// publish() itself.
suite<"MemoryEventsPlugin"> memory_events_plugin_suite = [] {
    "get_name reports 'memory'"_test = [] {
        MemoryEventsPlugin plugin;
        expect(plugin.get_name() == "memory");
    };

    "get_version reports a non-empty version string"_test = [] {
        MemoryEventsPlugin plugin;
        expect(plugin.get_version() == "0.1.0");
    };

    "capabilities reports CONGELADO_CAP_EVENTS"_test = [] {
        MemoryEventsPlugin plugin;
        expect(plugin.capabilities() == CONGELADO_CAP_EVENTS);
    };

    "event_get returns this instance upcast to IEventSink*"_test = [] {
        MemoryEventsPlugin plugin;
        expect(plugin.event_get() == static_cast<interfaces::IEventSink *>(&plugin));
    };

    "on_load with no config keys falls back to the default max_events without throwing"_test = [] {
        MemoryEventsPlugin plugin;
        CongeladoHostCallbacks host{};
        auto cfg = MemoryEventsTestHelper::single_field(nullptr, nullptr, 0);
        expect(nothrow([&] { plugin.on_load(host, cfg); }));
    };

    "on_load accepts a valid numeric max_events"_test = [] {
        MemoryEventsPlugin plugin;
        CongeladoHostCallbacks host{};
        const char *keys[] = {"max_events"};       // NOLINT(cppcoreguidelines-avoid-c-arrays)
        const char *values[] = {"5"};               // NOLINT(cppcoreguidelines-avoid-c-arrays)
        auto cfg = MemoryEventsTestHelper::single_field(keys, values, 1);
        expect(nothrow([&] { plugin.on_load(host, cfg); }));
    };

    // std::stoul throws on non-numeric input — on_load's catch(...) swallows it and falls back
    // to the default 200, rather than propagating the exception out of on_load (which would
    // abort the whole host's plugin-load sequence over a typo'd config value).
    "on_load with a non-numeric max_events falls back silently instead of throwing"_test = [] {
        MemoryEventsPlugin plugin;
        CongeladoHostCallbacks host{};
        const char *keys[] = {"max_events"};       // NOLINT(cppcoreguidelines-avoid-c-arrays)
        const char *values[] = {"not-a-number"};    // NOLINT(cppcoreguidelines-avoid-c-arrays)
        auto cfg = MemoryEventsTestHelper::single_field(keys, values, 1);
        expect(nothrow([&] { plugin.on_load(host, cfg); }));
    };

    // std::stoul throws std::out_of_range on a value too large for unsigned long — same
    // catch-and-fall-back path as the non-numeric case above.
    "on_load with an out-of-range max_events falls back silently instead of throwing"_test = [] {
        MemoryEventsPlugin plugin;
        CongeladoHostCallbacks host{};
        const char *keys[] = {"max_events"};                              // NOLINT(cppcoreguidelines-avoid-c-arrays)
        const char *values[] = {"999999999999999999999999999999"};        // NOLINT(cppcoreguidelines-avoid-c-arrays)
        auto cfg = MemoryEventsTestHelper::single_field(keys, values, 1);
        expect(nothrow([&] { plugin.on_load(host, cfg); }));
    };

    "publish before on_load is a safe no-op (default max_events still applies)"_test = [] {
        MemoryEventsPlugin plugin;
        expect(nothrow([&] { plugin.publish("some.event", R"({"payload":true})"); }));
    };

    "publish tolerates an empty event_name and empty payload"_test = [] {
        MemoryEventsPlugin plugin;
        expect(nothrow([&] { plugin.publish("", ""); }));
    };

    // Adversarial: a payload shaped like an injection/format-string attempt against whatever
    // downstream sink eventually reads m_recent back out — this sink itself does no parsing or
    // interpolation of the payload, so this just pins that garbage bytes don't crash publish().
    "publish tolerates format-string- and control-character-shaped payloads"_test = [] {
        MemoryEventsPlugin plugin;
        expect(nothrow([&] {
            plugin.publish("evil.event", "%s%s%s%n{{7*7}}\x00\x01\x1b[31m");
        }));
    };

    // Exceeds any reasonable max_events (default 200) many times over — proves the eviction loop
    // in publish() doesn't blow the stack/heap or hang; the actual evicted state is unobservable
    // (see the suite-level NOTE above).
    "publish with far more events than the default max_events doesn't crash or hang"_test = [] {
        MemoryEventsPlugin plugin;
        for (int index = 0; index < 500; ++index) {
            auto name = std::string{"event."} + std::to_string(index);
            plugin.publish(name, R"({"n":1})");
        }
        expect(true);
    };

    "on_unload before on_load is a safe no-op"_test = [] {
        MemoryEventsPlugin plugin;
        // No on_load call — MemoryEventsPlugin declares no on_unload override, so this resolves
        // to congelado::Plugin's default no-op.
        expect(nothrow([&] { plugin.on_unload(); }));
    };
};

} // namespace memory_events_plugin_tests
#endif
