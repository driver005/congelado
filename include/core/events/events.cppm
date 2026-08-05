export module core_events;

import std;
import interfaces;
import core_logger;

export import :registry;

namespace core::events {

/**
 * @brief Escapes a value for embedding inside a JSON string literal — same minimal
 * quote/backslash/control-character set already hand-rolled in `postgres_plugin.cc`/
 * `elasticsearch_plugin.cc`'s own `escape_json()` helpers, for the same reason: this is a
 * low-level `core_*` module with no `serde`/JSON-library dependency (mirrors `core_logger`'s own
 * dependency-free posture).
 * @param value the raw text to escape.
 * @return `value`, JSON-string-literal-safe, still missing the surrounding quotes.
 */
[[nodiscard]] inline std::string escape_json(std::string_view value) {
    std::string out;
    out.reserve(value.size());
    for (char character : value) {
        switch (character) {
        case '"':
            out += "\\\"";
            break;
        case '\\':
            out += "\\\\";
            break;
        case '\n':
            out += "\\n";
            break;
        case '\r':
            out += "\\r";
            break;
        case '\t':
            out += "\\t";
            break;
        default:
            out += character;
        }
    }
    return out;
}

/// @brief Flattens a string/string payload map into a JSON object — the wire format every
/// `IEventSink::publish()` implementation receives.
[[nodiscard]] inline std::string
to_json(const std::unordered_map<std::string, std::string> &payload) {
    std::string out = "{";
    bool first = true;
    for (auto const &[key, value] : payload) {
        if (!first) {
            out += ",";
        }
        first = false;
        out += std::format(R"("{}":"{}")", escape_json(key), escape_json(value));
    }
    out += "}";
    return out;
}

} // namespace core::events

export namespace core::events {

/**
 * @brief Publishes one event to every registered `IEventSink` — the ambient, logger-style
 * facade this whole capability exists to provide. Fans out to every sink (memory ring buffer,
 * RabbitMQ, Kafka, Redis, whatever's registered), same broadcast shape `core::logger::*`
 * already uses.
 * @note Never throws (matches `core::logger`'s own `noexcept` posture) — a sink's own `publish()`
 * is itself `noexcept`, and this function's own JSON encoding can't throw for a flat string map.
 * @warning Unlike `core::logger` (mandatory at boot — no logger plugin found is a hard abort),
 * zero registered sinks here is NOT fatal — events are optional infra. Falls back to a single
 * `core::logger::debug("events", ...)` line so a publish with nothing listening isn't silently
 * lost during local dev with no event plugins configured.
 * @param event_name the published event's name — this codebase's own convention is a dotted,
 * hierarchical string (e.g. `"engine.workflow.started"`).
 * @param payload flat key/value payload, JSON-encoded internally before reaching any sink.
 */
inline void publish(std::string_view event_name,
                    std::unordered_map<std::string, std::string> payload = {}) noexcept {
    auto *registry = EventBusRegistry::get_active();
    if (registry == nullptr || !registry->has_sink()) {
        core::logger::debug("events", "no sink for '{}': {}", event_name, to_json(payload));
        return;
    }
    auto json = to_json(payload);
    for (auto const &sink : registry->get_sinks()) {
        sink->publish(event_name, json);
    }
}

} // namespace core::events
