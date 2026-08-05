module;

#define CONGELADO_GUEST
#include <congelado/plugin.h>

export module memory_events_plugin;

import congelado_plugin;
import interfaces;
import core_logger;
import std;

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
