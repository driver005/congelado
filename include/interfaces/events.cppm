export module interfaces:events;

import std;

export namespace interfaces {

/// @brief A pluggable outbound event sink — the "publish" half of an app-wide event bus,
/// modeled on `IDatabase`/`ISearchProvider`'s shape (resolved once via `GET`, then called
/// directly as an in-process C++ virtual from then on — unlike `ILogger`, which crosses the raw
/// `congelado_call` ABI on every single call since `congelado::Plugin` itself implements it).
/// Unlike `IDatabase`/`ISearchProvider` (single-active-backend), multiple `IEventSink`s can be
/// active simultaneously — same fan-out shape `ILogger`/`LoggerRegistry` already use, since
/// publishing to more than one queue at once (e.g. an in-memory ring buffer AND RabbitMQ AND
/// Kafka) is a genuinely common, meaningful configuration, not a coin-flip like picking one
/// database.
class IEventSink {
  public:
    /**
     * @brief Virtual dtor, default's good — polymorphic sinks clean up fine through the base
     * pointer, no extra motion needed.
     */
    virtual ~IEventSink() = default;
    IEventSink() = default;
    IEventSink(const IEventSink &) = delete;
    IEventSink &operator=(const IEventSink &) = delete;
    IEventSink(IEventSink &&) = delete;
    IEventSink &operator=(IEventSink &&) = delete;

    /**
     * @brief Tells you which sink you're actually holding onto (memory, RabbitMQ, Kafka, Redis,
     * whatever got plugged in).
     * @return the sink's name.
     */
    [[nodiscard]] virtual std::string_view get_name() const noexcept = 0;

    /**
     * @brief Publishes one event. Fire-and-forget from the caller's perspective.
     * @note Reaching the backend at connect time is a hard requirement for network-backed sinks
     * (redis/kafka/rabbitmq) — their `on_load()` throws and aborts host startup if it can't
     * connect, same "don't run without it" contract `IDatabase`'s postgres backend already has.
     * Once connected, though, a sink that later loses that connection mid-flight still degrades
     * gracefully here in `publish()` — drops the event and logs its own warning, rather than
     * crashing the app on every transient broker hiccup.
     * @param event_name the published event's name (this codebase's own convention is a dotted,
     * hierarchical string, e.g. `"engine.workflow.started"` — not enforced here, just a
     * convention callers follow).
     * @param payload_json the event's payload, already JSON-encoded by the caller — same
     * "opaque string in" idiom `IDatabase`/`ISearchProvider` already use, so this interface
     * needs no dependency on any model type.
     */
    virtual void publish(std::string_view event_name, std::string_view payload_json) noexcept = 0;
};

} // namespace interfaces
