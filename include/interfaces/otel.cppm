export module interfaces:otel;

import std;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace interfaces {

enum class SpanKind : std::uint8_t
{
    INTERNAL,
    SERVER,
    CLIENT,
    PRODUCER,
    CONSUMER
};

enum class SpanStatus : std::uint8_t
{
    UNSET,
    OK,
    ERROR
};

enum class LogSeverity : std::uint8_t
{
    DEBUG,
    INFO,
    WARN,
    ERROR,
    FATAL
};

/**
 * @brief One OTel attribute value — the small closed set the wire format actually supports
 * (string/int/float/bool), kept as a variant rather than a template so it can cross the plugin
 * ABI boundary without instantiating anything across a `.so` edge.
 */
using AttributeValue = std::variant<std::string_view, std::int64_t, double, bool>;

/**
 * @brief One key/value attribute attached to a span, event, log record, or metric point.
 */
struct Attribute
{
    std::string_view key;
    AttributeValue value;
};

/**
 * @brief W3C trace-context identifiers for one logical span — plain data, no virtuals. This is
 * what gets generated once by `core::otel`'s ambient layer and handed identically to every
 * registered `ITracerProvider` (so fan-out providers agree on the same span/trace ids),
 * captured by value across thread hops (e.g. `WorkerContext::resolve_response()`), and
 * serialized into the outbound `traceparent` wire header.
 * @warning `trace_id`/`span_id` all-zero means "no context" (root with nothing generated yet) —
 * mirrors the W3C spec's own "all-zero is invalid" convention, don't hand out an all-zero id as
 * if it were real.
 */
struct SpanContext
{
    std::array<std::byte, 16> trace_id{};
    std::array<std::byte, 8> span_id{};
    std::array<std::byte, 8> parent_span_id{};
    bool sampled{true};
};

/**
 * @brief One per-provider span handle — a logical span held by `core::otel::ScopedSpan` gets
 * one of these per currently-registered `ITracerProvider` (the fan-out itself lives one level
 * up, here it's just "this one provider's view of this one span").
 */
class ISpan
{
public:
    ISpan() = default;
    virtual ~ISpan() = default;
    ISpan(const ISpan&) = delete;
    ISpan& operator=(const ISpan&) = delete;
    ISpan(ISpan&&) = delete;
    ISpan& operator=(ISpan&&) = delete;

    /**
     * @brief Attaches or overwrites one attribute on this span.
     * @param key the attribute key.
     * @param value the attribute value.
     */
    virtual void set_attribute(std::string_view key, const AttributeValue& value) noexcept = 0;

    /**
     * @brief Records a timestamped event on this span.
     * @param name the event name.
     * @param attrs attributes carried on the event, if any.
     */
    virtual void add_event(std::string_view name, std::span<const Attribute> attrs) noexcept = 0;

    /**
     * @brief Sets this span's completion status.
     * @param status OK/ERROR/UNSET.
     * @param description optional human-readable detail, mainly meaningful alongside ERROR.
     */
    virtual void set_status(SpanStatus status, std::string_view description) noexcept = 0;

    /**
     * @brief Marks this span complete (records the end timestamp). Idempotent — a provider
     * should tolerate a second call quietly, since `ScopedSpan` may call this both explicitly
     * and again from its destructor.
     */
    virtual void end() noexcept = 0;
};

/**
 * @brief A tracing backend as a plugin capability — one of `IOtelProvider`'s three optional
 * sub-providers. Takes an already-generated `SpanContext` rather than minting its own ids: with
 * fan-out (multiple providers active at once), the same logical span must carry identical
 * trace/span ids across every provider, so `core::otel`'s ambient layer generates them once and
 * hands the same `SpanContext` to every registered provider's `start_span()`.
 */
class ITracerProvider
{
public:
    ITracerProvider() = default;
    virtual ~ITracerProvider() = default;
    ITracerProvider(const ITracerProvider&) = delete;
    ITracerProvider& operator=(const ITracerProvider&) = delete;
    ITracerProvider(ITracerProvider&&) = delete;
    ITracerProvider& operator=(ITracerProvider&&) = delete;

    /**
     * @brief Starts a new span under this provider.
     * @param name the span's operation name (e.g. `"POST /tasks/:id/result"`).
     * @param kind the span's role (server/client/internal/...).
     * @param ctx the already-generated trace/span/parent ids this span must record under.
     * @param attrs attributes to set on the span at start time.
     * @return this provider's handle onto the new span.
     */
    [[nodiscard]] virtual std::shared_ptr<ISpan> start_span(
        std::string_view name,
        SpanKind kind,
        const SpanContext& ctx,
        std::span<const Attribute> attrs
    ) = 0;
};

/**
 * @brief A single counter instrument under one provider — monotonically-increasing sums
 * (request counts, task completions, ...).
 */
class ICounter
{
public:
    ICounter() = default;
    virtual ~ICounter() = default;
    ICounter(const ICounter&) = delete;
    ICounter& operator=(const ICounter&) = delete;
    ICounter(ICounter&&) = delete;
    ICounter& operator=(ICounter&&) = delete;

    /**
     * @brief Adds `value` to this counter.
     * @param value the amount to add (non-negative for a true counter).
     * @param attrs attributes (dimensions) this data point carries.
     */
    virtual void add(double value, std::span<const Attribute> attrs) noexcept = 0;
};

/**
 * @brief A single histogram instrument under one provider — distributions (task duration,
 * request latency, ...).
 */
class IHistogram
{
public:
    IHistogram() = default;
    virtual ~IHistogram() = default;
    IHistogram(const IHistogram&) = delete;
    IHistogram& operator=(const IHistogram&) = delete;
    IHistogram(IHistogram&&) = delete;
    IHistogram& operator=(IHistogram&&) = delete;

    /**
     * @brief Records one observation into this histogram.
     * @param value the observed value.
     * @param attrs attributes (dimensions) this data point carries.
     */
    virtual void record(double value, std::span<const Attribute> attrs) noexcept = 0;
};

/**
 * @brief A metrics backend as a plugin capability — the second of `IOtelProvider`'s three
 * optional sub-providers. Instrument creation is by name; `core::otel::MeterRegistry` caches
 * the returned handles so repeated `counter_add("task.completed", ...)`-style facade calls
 * don't re-create the instrument every time.
 */
class IMeterProvider
{
public:
    IMeterProvider() = default;
    virtual ~IMeterProvider() = default;
    IMeterProvider(const IMeterProvider&) = delete;
    IMeterProvider& operator=(const IMeterProvider&) = delete;
    IMeterProvider(IMeterProvider&&) = delete;
    IMeterProvider& operator=(IMeterProvider&&) = delete;

    /**
     * @brief Creates (or looks up) a counter instrument by name.
     * @param name the instrument's name.
     * @param description human-readable description.
     * @param unit the instrument's unit (e.g. `"1"`, `"ms"`).
     * @return a handle to add values through.
     */
    [[nodiscard]] virtual std::shared_ptr<ICounter>
    create_counter(std::string_view name, std::string_view description, std::string_view unit) = 0;

    /**
     * @brief Creates (or looks up) a histogram instrument by name.
     * @param name the instrument's name.
     * @param description human-readable description.
     * @param unit the instrument's unit (e.g. `"ms"`).
     * @return a handle to record observations through.
     */
    [[nodiscard]] virtual std::shared_ptr<IHistogram> create_histogram(
        std::string_view name, std::string_view description, std::string_view unit
    ) = 0;
};

/**
 * @brief One OTel log record — what `OtelLogBridge` (an `interfaces::ILogger` that registers
 * into the existing `core::logger::LoggerRegistry`) builds out of every fanned-out
 * `core::logger::*` call, carrying whatever ambient trace/span context was active on the
 * calling thread at the time.
 */
struct LogRecord
{
    std::string_view body;
    LogSeverity severity{LogSeverity::INFO};
    std::span<const Attribute> attrs;
    std::array<std::byte, 16> trace_id{};
    std::array<std::byte, 8> span_id{};
};

/**
 * @brief A log-export backend as a plugin capability — the third of `IOtelProvider`'s optional
 * sub-providers. Distinct from `interfaces::ILogger`: `ILogger` is what `core::logger` fans
 * *existing* log calls out to; this is the actual OTel-logs export target `OtelLogBridge`
 * forwards into.
 */
class ILogRecordProvider
{
public:
    ILogRecordProvider() = default;
    virtual ~ILogRecordProvider() = default;
    ILogRecordProvider(const ILogRecordProvider&) = delete;
    ILogRecordProvider& operator=(const ILogRecordProvider&) = delete;
    ILogRecordProvider(ILogRecordProvider&&) = delete;
    ILogRecordProvider& operator=(ILogRecordProvider&&) = delete;

    /**
     * @brief Exports one log record.
     * @param record the record to export.
     */
    virtual void emit(const LogRecord& record) noexcept = 0;
};

/**
 * @brief The single aggregate capability a plugin implements to participate as an OTel provider
 * — mirrors how `IOtelProvider` is what crosses the plugin ABI (one `otel_get()` call, one
 * `CONGELADO_CAP_OTEL` bit), while the three signal-specific interfaces above are the actual
 * per-signal contracts. Each accessor defaults to `nullptr` so a plugin can support any subset
 * of the three signals — e.g. a metrics-only exporter just overrides `get_meter_provider()`.
 */
class IOtelProvider
{
public:
    IOtelProvider() = default;
    virtual ~IOtelProvider() = default;
    IOtelProvider(const IOtelProvider&) = delete;
    IOtelProvider& operator=(const IOtelProvider&) = delete;
    IOtelProvider(IOtelProvider&&) = delete;
    IOtelProvider& operator=(IOtelProvider&&) = delete;

    /**
     * @brief This provider's tracer, if it supports the traces signal.
     * @return the tracer provider, or `nullptr` if traces aren't supported.
     */
    [[nodiscard]] virtual ITracerProvider* get_tracer_provider() noexcept
    {
        return nullptr;
    }

    /**
     * @brief This provider's meter, if it supports the metrics signal.
     * @return the meter provider, or `nullptr` if metrics aren't supported.
     */
    [[nodiscard]] virtual IMeterProvider* get_meter_provider() noexcept
    {
        return nullptr;
    }

    /**
     * @brief This provider's log-record exporter, if it supports the logs signal.
     * @return the log-record provider, or `nullptr` if logs aren't supported.
     */
    [[nodiscard]] virtual ILogRecordProvider* get_log_provider() noexcept
    {
        return nullptr;
    }
};

} // namespace interfaces

#ifdef CONGELADO_TEST
namespace interfaces::otel_tests {
using namespace boost::ut;

// IOtelProvider has no pure virtuals — every accessor defaults to nullptr, so the base class
// itself is concrete and worth exercising directly (no mock subclass needed).
suite<"IOtelProvider defaults"> otel_provider_suite = [] {
    "get_tracer_provider() defaults to nullptr when not overridden"_test = [] {
        IOtelProvider provider;
        expect(provider.get_tracer_provider() == nullptr);
    };

    "get_meter_provider() defaults to nullptr when not overridden"_test = [] {
        IOtelProvider provider;
        expect(provider.get_meter_provider() == nullptr);
    };

    "get_log_provider() defaults to nullptr when not overridden"_test = [] {
        IOtelProvider provider;
        expect(provider.get_log_provider() == nullptr);
    };
};

suite<"SpanContext defaults"> span_context_suite = [] {
    "a default-constructed SpanContext is all-zero ids and sampled"_test = [] {
        SpanContext ctx;
        expect(std::ranges::all_of(ctx.trace_id, [](std::byte val) {
            return val == std::byte{0};
        }));
        expect(std::ranges::all_of(ctx.span_id, [](std::byte val) {
            return val == std::byte{0};
        }));
        expect(std::ranges::all_of(ctx.parent_span_id, [](std::byte val) {
            return val == std::byte{0};
        }));
        expect(ctx.sampled);
    };
};

} // namespace interfaces::otel_tests
#endif
