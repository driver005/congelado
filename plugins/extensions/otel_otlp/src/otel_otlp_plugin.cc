#define CONGELADO_GUEST

// The opentelemetry-cpp headers below pull in plain, non-modular `<memory>` (among others,
// transitively) — deliberately included here, textually, BEFORE `import std;`. Reversing that
// order (import std; first, legacy headers after) hits a real toolchain-level conflict in this
// build environment: libstdc++ 16's brand-new C++26 `std::indirect`/`std::polymorphic` alias
// templates get redeclared once via the `std` module's own BMI and a second time via
// opentelemetry-cpp's textual `#include <memory>`, and the two don't agree — "too many template
// arguments" errors deep in `<bits/indirect.h>`. Processing the plain-textual path first (so its
// include guards are already active before `import std;` ever touches the same header) avoids
// the double-declaration entirely.
//
// OPENTELEMETRY_PROTO_API: opentelemetry-cpp's generated `.pb.h` files (transitively pulled in
// by the OTLP exporter headers below) reference this macro on every exported proto type/symbol,
// but it is never actually `#define`d anywhere in the installed include tree — genuinely a
// build-internal macro (set via CMake `target_compile_definitions` when opentelemetry-cpp
// compiles its own .cpp sources), not one exposed to header consumers. Defining it ourselves as
// empty is safe here: this plugin compiles these types directly into its own single .so with no
// need for a separate cross-DSO export/visibility attribute on them.
#ifndef OPENTELEMETRY_PROTO_API
#  define OPENTELEMETRY_PROTO_API
#endif

#include <opentelemetry/common/key_value_iterable_view.h>
#include <opentelemetry/exporters/otlp/otlp_http_exporter.h>
#include <opentelemetry/exporters/otlp/otlp_http_exporter_options.h>
#include <opentelemetry/exporters/otlp/otlp_http_log_record_exporter.h>
#include <opentelemetry/exporters/otlp/otlp_http_log_record_exporter_options.h>
#include <opentelemetry/exporters/otlp/otlp_http_metric_exporter.h>
#include <opentelemetry/exporters/otlp/otlp_http_metric_exporter_options.h>
#include <opentelemetry/logs/logger.h>
#include <opentelemetry/logs/logger_provider.h>
#include <opentelemetry/logs/severity.h>
#include <opentelemetry/metrics/meter.h>
#include <opentelemetry/metrics/meter_provider.h>
#include <opentelemetry/metrics/sync_instruments.h>
#include <opentelemetry/sdk/common/global_log_handler.h>
#include <opentelemetry/sdk/logs/batch_log_record_processor.h>
#include <opentelemetry/sdk/logs/logger_provider.h>
#include <opentelemetry/sdk/metrics/export/periodic_exporting_metric_reader.h>
#include <opentelemetry/sdk/metrics/meter_provider.h>
#include <opentelemetry/sdk/resource/resource.h>
#include <opentelemetry/sdk/trace/batch_span_processor.h>
#include <opentelemetry/sdk/trace/tracer_provider.h>
#include <opentelemetry/trace/span.h>
#include <opentelemetry/trace/span_context.h>
#include <opentelemetry/trace/tracer.h>
#include <opentelemetry/trace/tracer_provider.h>

// POSIX sockets for the load-time endpoint reachability probe (EndpointProbe below). Textual,
// before `import std;`, same reason as the opentelemetry headers above.
#include <cerrno>
#include <fcntl.h>
#include <netdb.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <unistd.h>

import congelado_plugin;
#include <congelado/plugin.h>
import interfaces;
import core_events;
import core_logger;
import std;

// Real OpenTelemetry provider — the first concrete `IOtelProvider` implementation, using the
// actual `opentelemetry-cpp` SDK with an OTLP/HTTP exporter for all three signals. Built for a
// Grafana-stack setup: point `endpoint` at an OpenTelemetry Collector (which fans out to
// Prometheus/Tempo/Loki) or directly at a backend with a native OTLP receiver.
//
// @note gRPC transport was deliberately dropped, not just left unconfigured — `conan::grpc`'s
// own build (abseil/re2/c-ares/protobuf's C++ codegen, all from source) dominated this plugin's
// build time by a huge margin next to everything else in the project combined. OTLP/HTTP covers
// the same backends (any Collector, and Prometheus's native OTLP receiver already in this repo's
// docker stack) with no gRPC/protobuf-service-codegen dependency at all.
//
// @warning Attribute values cross into the SDK stringified (every `interfaces::AttributeValue`
// gets formatted to a plain string before being handed to `common::KeyValueIterableView`) —
// simpler and safer to get right for a first pass than threading the SDK's own typed
// `AttributeValue` variant through, at the cost of losing native int/float/bool typing on
// exported attributes. A reasonable follow-up, not required for the pipeline to work end to end.

namespace {

namespace trace_api = opentelemetry::trace;
namespace logs_api = opentelemetry::logs;
namespace metrics_api = opentelemetry::metrics;
namespace otlp = opentelemetry::exporter::otlp;
namespace trace_sdk = opentelemetry::sdk::trace;
namespace metrics_sdk = opentelemetry::sdk::metrics;
namespace logs_sdk = opentelemetry::sdk::logs;
namespace resource_sdk = opentelemetry::sdk::resource;
namespace otel_internal_log = opentelemetry::sdk::common::internal_log;

trace_api::SpanKind to_otel_kind(interfaces::SpanKind kind) noexcept {
    switch (kind) {
    case interfaces::SpanKind::SERVER:
        return trace_api::SpanKind::kServer;
    case interfaces::SpanKind::CLIENT:
        return trace_api::SpanKind::kClient;
    case interfaces::SpanKind::PRODUCER:
        return trace_api::SpanKind::kProducer;
    case interfaces::SpanKind::CONSUMER:
        return trace_api::SpanKind::kConsumer;
    case interfaces::SpanKind::INTERNAL:
        return trace_api::SpanKind::kInternal;
    }
    return trace_api::SpanKind::kInternal;
}

trace_api::StatusCode to_otel_status(interfaces::SpanStatus status) noexcept {
    switch (status) {
    case interfaces::SpanStatus::OK:
        return trace_api::StatusCode::kOk;
    case interfaces::SpanStatus::ERROR:
        return trace_api::StatusCode::kError;
    case interfaces::SpanStatus::UNSET:
        return trace_api::StatusCode::kUnset;
    }
    return trace_api::StatusCode::kUnset;
}

logs_api::Severity to_otel_severity(interfaces::LogSeverity severity) noexcept {
    switch (severity) {
    case interfaces::LogSeverity::DEBUG:
        return logs_api::Severity::kDebug;
    case interfaces::LogSeverity::INFO:
        return logs_api::Severity::kInfo;
    case interfaces::LogSeverity::WARN:
        return logs_api::Severity::kWarn;
    case interfaces::LogSeverity::ERROR:
        return logs_api::Severity::kError;
    case interfaces::LogSeverity::FATAL:
        return logs_api::Severity::kFatal;
    }
    return logs_api::Severity::kInfo;
}

std::array<std::uint8_t, 16> to_u8(const std::array<std::byte, 16> &bytes) noexcept {
    std::array<std::uint8_t, 16> out{};
    for (std::size_t i = 0; i < 16; ++i) {
        out[i] = std::to_integer<std::uint8_t>(bytes[i]);
    }
    return out;
}

std::array<std::uint8_t, 8> to_u8(const std::array<std::byte, 8> &bytes) noexcept {
    std::array<std::uint8_t, 8> out{};
    for (std::size_t i = 0; i < 8; ++i) {
        out[i] = std::to_integer<std::uint8_t>(bytes[i]);
    }
    return out;
}

std::string attribute_to_string(const interfaces::AttributeValue &value) {
    return std::visit(
        [](const auto &held) -> std::string {
            using Held = std::decay_t<decltype(held)>;
            if constexpr (std::same_as<Held, std::string_view>) {
                return std::string{held};
            } else if constexpr (std::same_as<Held, bool>) {
                return held ? "true" : "false";
            } else {
                return std::format("{}", held);
            }
        },
        value);
}

std::map<std::string, std::string> to_label_map(std::span<const interfaces::Attribute> attrs) {
    std::map<std::string, std::string> out;
    for (const auto &attr : attrs) {
        out[std::string{attr.key}] = attribute_to_string(attr.value);
    }
    return out;
}

class SpanWrapper final : public interfaces::ISpan {
  public:
    explicit SpanWrapper(opentelemetry::nostd::shared_ptr<trace_api::Span> span)
        : m_span{std::move(span)} {}

    void set_attribute(std::string_view key, const interfaces::AttributeValue &value) noexcept override {
        try {
            m_span->SetAttribute(std::string{key}, attribute_to_string(value));
        } catch (...) {
        }
    }

    void add_event(std::string_view name, std::span<const interfaces::Attribute> attrs) noexcept override {
        try {
            auto labels = to_label_map(attrs);
            opentelemetry::common::KeyValueIterableView view{labels};
            m_span->AddEvent(std::string{name}, view);
        } catch (...) {
        }
    }

    void set_status(interfaces::SpanStatus status, std::string_view description) noexcept override {
        try {
            m_span->SetStatus(to_otel_status(status), std::string{description});
        } catch (...) {
        }
    }

    void end() noexcept override {
        try {
            m_span->End();
        } catch (...) {
        }
    }

  private:
    opentelemetry::nostd::shared_ptr<trace_api::Span> m_span;
};

class TracerBackend final : public interfaces::ITracerProvider {
  public:
    explicit TracerBackend(opentelemetry::nostd::shared_ptr<trace_api::Tracer> tracer)
        : m_tracer{std::move(tracer)} {}

    [[nodiscard]] std::shared_ptr<interfaces::ISpan>
    start_span(std::string_view name, interfaces::SpanKind kind, const interfaces::SpanContext &ctx,
              std::span<const interfaces::Attribute> attrs) override {
        trace_api::StartSpanOptions options;
        options.kind = to_otel_kind(kind);

        static constexpr std::array<std::byte, 8> ZERO_SPAN{};
        if (ctx.parent_span_id != ZERO_SPAN) {
            // nostd::span has no implicit conversion from std::array<uint8_t, N> (the array
            // element type isn't `const`-qualified to match, and TraceId/SpanId's span-taking
            // constructors are `explicit` besides) — build the span explicitly from the
            // (pointer, count) constructor instead, off locals that outlive this statement.
            auto trace_id_bytes = to_u8(ctx.trace_id);
            auto span_id_bytes = to_u8(ctx.parent_span_id);
            trace_api::TraceId trace_id{
                opentelemetry::nostd::span<const std::uint8_t, trace_api::TraceId::kSize>(
                    trace_id_bytes.data(), trace_id_bytes.size())};
            trace_api::SpanId parent_id{
                opentelemetry::nostd::span<const std::uint8_t, trace_api::SpanId::kSize>(
                    span_id_bytes.data(), span_id_bytes.size())};
            trace_api::TraceFlags flags{static_cast<std::uint8_t>(ctx.sampled ? 1U : 0U)};
            // is_remote=true: the parent was propagated in from our own facade's ambient/wire
            // context, not started by this same Tracer instance.
            options.parent = trace_api::SpanContext(trace_id, parent_id, flags, true);
        }

        auto labels = to_label_map(attrs);
        opentelemetry::common::KeyValueIterableView view{labels};
        auto span = m_tracer->StartSpan(std::string{name}, view, options);
        return std::make_shared<SpanWrapper>(std::move(span));
    }

  private:
    opentelemetry::nostd::shared_ptr<trace_api::Tracer> m_tracer;
};

class CounterBackend final : public interfaces::ICounter {
  public:
    explicit CounterBackend(opentelemetry::nostd::unique_ptr<metrics_api::Counter<double>> counter)
        : m_counter{std::move(counter)} {}

    void add(double value, std::span<const interfaces::Attribute> attrs) noexcept override {
        try {
            auto labels = to_label_map(attrs);
            opentelemetry::common::KeyValueIterableView view{labels};
            m_counter->Add(value, view);
        } catch (...) {
        }
    }

  private:
    opentelemetry::nostd::unique_ptr<metrics_api::Counter<double>> m_counter;
};

class HistogramBackend final : public interfaces::IHistogram {
  public:
    explicit HistogramBackend(opentelemetry::nostd::unique_ptr<metrics_api::Histogram<double>> histogram)
        : m_histogram{std::move(histogram)} {}

    void record(double value, std::span<const interfaces::Attribute> attrs) noexcept override {
        try {
            auto labels = to_label_map(attrs);
            opentelemetry::common::KeyValueIterableView view{labels};
            m_histogram->Record(value, view, opentelemetry::context::Context{});
        } catch (...) {
        }
    }

  private:
    opentelemetry::nostd::unique_ptr<metrics_api::Histogram<double>> m_histogram;
};

class MeterBackend final : public interfaces::IMeterProvider {
  public:
    explicit MeterBackend(opentelemetry::nostd::shared_ptr<metrics_api::Meter> meter)
        : m_meter{std::move(meter)} {}

    [[nodiscard]] std::shared_ptr<interfaces::ICounter>
    create_counter(std::string_view name, std::string_view description, std::string_view unit) override {
        return std::make_shared<CounterBackend>(
            m_meter->CreateDoubleCounter(std::string{name}, std::string{description}, std::string{unit}));
    }

    [[nodiscard]] std::shared_ptr<interfaces::IHistogram>
    create_histogram(std::string_view name, std::string_view description, std::string_view unit) override {
        return std::make_shared<HistogramBackend>(
            m_meter->CreateDoubleHistogram(std::string{name}, std::string{description}, std::string{unit}));
    }

  private:
    opentelemetry::nostd::shared_ptr<metrics_api::Meter> m_meter;
};

class LogBackend final : public interfaces::ILogRecordProvider {
  public:
    explicit LogBackend(opentelemetry::nostd::shared_ptr<logs_api::Logger> logger)
        : m_logger{std::move(logger)} {}

    void emit(const interfaces::LogRecord &record) noexcept override {
        try {
            opentelemetry::nostd::string_view body{record.body.data(), record.body.size()};
            static constexpr std::array<std::byte, 16> ZERO_TRACE{};
            if (record.trace_id == ZERO_TRACE) {
                m_logger->Log(to_otel_severity(record.severity), body);
                return;
            }
            // Stamps this log record with the calling OtelLogBridge call's ambient trace/span
            // context (see sdk/heart/adapters.cppm's OtelLogBridge::write()) so Grafana can
            // correlate logs<->traces (Tempo's "logs for this span" / Loki's trace-id derived
            // field) — Logger::Log() alone has no way to attach a SpanContext, only
            // EmitLogRecord()'s typed-argument form does (see this SDK's logger.h doc comment
            // on EmitLogRecord's ArgumentType dispatch: "SpanContext -> span_id,trace_id and
            // trace_flags").
            auto trace_id_bytes = to_u8(record.trace_id);
            auto span_id_bytes = to_u8(record.span_id);
            trace_api::TraceId trace_id{
                opentelemetry::nostd::span<const std::uint8_t, trace_api::TraceId::kSize>(
                    trace_id_bytes.data(), trace_id_bytes.size())};
            trace_api::SpanId span_id{
                opentelemetry::nostd::span<const std::uint8_t, trace_api::SpanId::kSize>(
                    span_id_bytes.data(), span_id_bytes.size())};
            trace_api::SpanContext span_context{trace_id, span_id, trace_api::TraceFlags{}, true};
            m_logger->EmitLogRecord(to_otel_severity(record.severity), span_context, body);
        } catch (...) {
        }
    }

  private:
    opentelemetry::nostd::shared_ptr<logs_api::Logger> m_logger;
};

class OtelSdkLogHandler final : public otel_internal_log::LogHandler {
  public:
    // The SDK's own diagnostics (failed exports, connection refused, timeouts) otherwise vanish
    // silently — route them into this project's logger so they land next to every other log line.
    void Handle(otel_internal_log::LogLevel level, const char *file, int line, const char *msg,
               const opentelemetry::sdk::common::AttributeMap & /*attributes*/) noexcept override {
        switch (level) {
        case otel_internal_log::LogLevel::Error:
            core::logger::error("otel_sdk", "{}:{}: {}", file, line, msg);
            break;
        case otel_internal_log::LogLevel::Warning:
            core::logger::warning("otel_sdk", "{}:{}: {}", file, line, msg);
            break;
        case otel_internal_log::LogLevel::Info:
            core::logger::info("otel_sdk", "{}:{}: {}", file, line, msg);
            break;
        case otel_internal_log::LogLevel::Debug:
            core::logger::debug("otel_sdk", "{}:{}: {}", file, line, msg);
            break;
        case otel_internal_log::LogLevel::None:
            break;
        }
    }
};

// Thin decorator around the real OTLP span exporter — logs span count + result via this
// project's own logger on every Export() call, so "are traces actually being sent" is visible
// directly (at debug level) instead of having to infer it from OtelSdkLogHandler's output, which
// only ever surfaces failures, not routine successful batches.
class LoggingSpanExporter final : public trace_sdk::SpanExporter {
  public:
    explicit LoggingSpanExporter(std::unique_ptr<trace_sdk::SpanExporter> inner) noexcept
        : m_inner{std::move(inner)} {}

    [[nodiscard]] std::unique_ptr<trace_sdk::Recordable> MakeRecordable() noexcept override {
        return m_inner->MakeRecordable();
    }

    [[nodiscard]] opentelemetry::sdk::common::ExportResult
    Export(const opentelemetry::nostd::span<std::unique_ptr<trace_sdk::Recordable>> &spans) noexcept override {
        core::logger::debug("otel_traces", "exporting {} span(s) to Tempo", spans.size());
        auto result = m_inner->Export(spans);
        if (result == opentelemetry::sdk::common::ExportResult::kSuccess) {
            core::logger::debug("otel_traces", "sent {} span(s) successfully", spans.size());
        } else {
            core::logger::warning("otel_traces", "export failed for {} span(s), result={}", spans.size(),
                                  static_cast<int>(result));
            core::events::publish("otel_otlp.traces_export_failed",
                                  {{"count", std::to_string(spans.size())},
                                   {"result", std::to_string(static_cast<int>(result))}});
        }
        return result;
    }

    bool ForceFlush(std::chrono::microseconds timeout =
                         (std::chrono::microseconds::max)()) noexcept override {
        return m_inner->ForceFlush(timeout);
    }

    bool
    Shutdown(std::chrono::microseconds timeout = (std::chrono::microseconds::max)()) noexcept override {
        return m_inner->Shutdown(timeout);
    }

  private:
    std::unique_ptr<trace_sdk::SpanExporter> m_inner;
};

// Same idea as LoggingSpanExporter above, for the metrics pipeline — logs a debug line on every
// Export() call (batch size + result) so "are metrics actually reaching Prometheus" is visible
// without having to infer it from OtelSdkLogHandler's failure-only output.
class LoggingMetricExporter final : public metrics_sdk::PushMetricExporter {
  public:
    explicit LoggingMetricExporter(std::unique_ptr<metrics_sdk::PushMetricExporter> inner) noexcept
        : m_inner{std::move(inner)} {}

    [[nodiscard]] opentelemetry::sdk::common::ExportResult
    Export(const metrics_sdk::ResourceMetrics &data) noexcept override {
        std::size_t metric_count = 0;
        for (const auto &scope_metrics : data.scope_metric_data_) {
            metric_count += scope_metrics.metric_data_.size();
        }
        core::logger::debug("otel_metrics", "exporting {} metric(s) across {} scope(s) to Prometheus",
                            metric_count, data.scope_metric_data_.size());
        auto result = m_inner->Export(data);
        if (result == opentelemetry::sdk::common::ExportResult::kSuccess) {
            core::logger::debug("otel_metrics", "sent {} metric(s) successfully", metric_count);
        } else {
            core::logger::warning("otel_metrics", "export failed for {} metric(s), result={}",
                                  metric_count, static_cast<int>(result));
            core::events::publish("otel_otlp.metrics_export_failed",
                                  {{"count", std::to_string(metric_count)},
                                   {"result", std::to_string(static_cast<int>(result))}});
        }
        return result;
    }

    [[nodiscard]] metrics_sdk::AggregationTemporality
    GetAggregationTemporality(metrics_sdk::InstrumentType instrument_type) const noexcept override {
        return m_inner->GetAggregationTemporality(instrument_type);
    }

    bool ForceFlush(std::chrono::microseconds timeout =
                         (std::chrono::microseconds::max)()) noexcept override {
        return m_inner->ForceFlush(timeout);
    }

    bool
    Shutdown(std::chrono::microseconds timeout = (std::chrono::microseconds::max)()) noexcept override {
        return m_inner->Shutdown(timeout);
    }

  private:
    std::unique_ptr<metrics_sdk::PushMetricExporter> m_inner;
};

// Same idea as LoggingSpanExporter above, for the logs pipeline — logs a debug line on every
// Export() call (batch size + result) so "are logs actually reaching Loki" is visible without
// having to infer it from OtelSdkLogHandler's failure-only output.
class LoggingLogRecordExporter final : public logs_sdk::LogRecordExporter {
  public:
    explicit LoggingLogRecordExporter(std::unique_ptr<logs_sdk::LogRecordExporter> inner) noexcept
        : m_inner{std::move(inner)} {}

    [[nodiscard]] std::unique_ptr<logs_sdk::Recordable> MakeRecordable() noexcept override {
        return m_inner->MakeRecordable();
    }

    [[nodiscard]] opentelemetry::sdk::common::ExportResult
    Export(const opentelemetry::nostd::span<std::unique_ptr<logs_sdk::Recordable>> &records) noexcept override {
        // The "otel_logs"-tagged line below (and LoggingSpanExporter's/LoggingMetricExporter's
        // own "otel_traces"/"otel_metrics" lines) never loop back here: OtelLogBridge::emit()
        // (sdk/heart/adapters.cppm) drops every "otel*"-tagged line before it ever reaches the
        // OTel logs pipeline this exporter feeds — see that method's own doc comment for why.
        core::logger::debug("otel_logs", "exporting {} log record(s) to Loki", records.size());
        auto result = m_inner->Export(records);
        if (result == opentelemetry::sdk::common::ExportResult::kSuccess) {
            core::logger::debug("otel_logs", "sent {} log record(s) successfully", records.size());
        } else {
            core::logger::warning("otel_logs", "export failed for {} log record(s), result={}",
                                  records.size(), static_cast<int>(result));
            core::events::publish("otel_otlp.logs_export_failed",
                                  {{"count", std::to_string(records.size())},
                                   {"result", std::to_string(static_cast<int>(result))}});
        }
        return result;
    }

    bool ForceFlush(std::chrono::microseconds timeout =
                         (std::chrono::microseconds::max)()) noexcept override {
        return m_inner->ForceFlush(timeout);
    }

    bool
    Shutdown(std::chrono::microseconds timeout = (std::chrono::microseconds::max)()) noexcept override {
        return m_inner->Shutdown(timeout);
    }

  private:
    std::unique_ptr<logs_sdk::LogRecordExporter> m_inner;
};

// Decides at load time whether an OTLP endpoint is actually a working receiver, instead of letting
// the SDK spam per-export "connection failed / 404 page not found" errors forever against a dead
// or misconfigured backend. A plain TCP connect isn't enough — a wrong endpoint path answers the
// TCP handshake but returns HTTP 404 — so this speaks minimal HTTP: connect, POST the OTLP path,
// read the status line. "Reachable" means we got an HTTP response that wasn't 404. Connection
// refused, DNS miss, timeout, or a 404 all count as unreachable. https endpoints can't be
// HTTP-probed here (no TLS), so a successful TCP connect is accepted for those.
class EndpointProbe {
  public:
    /// @brief Checks whether an OTLP URL points at a live receiver (see class comment).
    /// @param url the OTLP endpoint URL (e.g. "http://loki:3100/otlp/v1/logs").
    /// @param timeout per-attempt deadline (connect and HTTP exchange each bounded by it).
    /// @return true if the endpoint answered HTTP with a non-404 status (or, for https, accepted a
    /// TCP connection); false on refused/DNS-miss/timeout/404.
    [[nodiscard]] static bool reachable(const std::string &url,
                                        std::chrono::milliseconds timeout) noexcept {
        auto parts = parse_url(url);
        if (parts.host.empty() || parts.port.empty()) {
            return false;
        }
        addrinfo hints{};
        hints.ai_family = AF_UNSPEC;
        hints.ai_socktype = SOCK_STREAM;
        addrinfo *result = nullptr;
        if (::getaddrinfo(parts.host.c_str(), parts.port.c_str(), &hints, &result) != 0) {
            return false;
        }
        bool ok = false;
        for (addrinfo *addr = result; addr != nullptr && !ok; addr = addr->ai_next) {
            int socket_fd = ::socket(addr->ai_family, addr->ai_socktype, addr->ai_protocol);
            if (socket_fd < 0) {
                continue;
            }
            if (connect_with_timeout(socket_fd, addr, timeout)) {
                // Can't HTTP-probe a TLS port without a handshake — accept the TCP connect for
                // https and let the SDK's real exporter surface any deeper error.
                ok = parts.is_https ? true : http_ok(socket_fd, parts, timeout);
            }
            ::close(socket_fd);
        }
        ::freeaddrinfo(result);
        return ok;
    }

  private:
    class UrlParts {
      public:
        std::string host;
        std::string port;
        std::string path;
        bool is_https = false;
    };

    /// @brief Splits an OTLP URL into host, port and path. Port defaults from the scheme when the
    /// URL omits it; path defaults to "/".
    static UrlParts parse_url(const std::string &url) {
        UrlParts parts;
        std::string_view view{url};
        parts.is_https = view.starts_with("https://");
        if (auto scheme = view.find("://"); scheme != std::string_view::npos) {
            view.remove_prefix(scheme + 3);
        }
        std::string_view authority = view;
        if (auto slash = view.find('/'); slash != std::string_view::npos) {
            authority = view.substr(0, slash);
            parts.path = std::string{view.substr(slash)};
        } else {
            parts.path = "/";
        }
        if (auto colon = authority.rfind(':'); colon != std::string_view::npos) {
            parts.host = std::string{authority.substr(0, colon)};
            parts.port = std::string{authority.substr(colon + 1)};
        } else {
            parts.host = std::string{authority};
            parts.port = parts.is_https ? "443" : "80";
        }
        return parts;
    }

    /// @brief Non-blocking connect bounded by `timeout` via select() — a plain blocking connect
    /// could hang far longer than the caller's deadline on an unreachable host.
    static bool connect_with_timeout(int socket_fd, addrinfo *addr,
                                     std::chrono::milliseconds timeout) noexcept {
        int flags = ::fcntl(socket_fd, F_GETFL, 0);
        ::fcntl(socket_fd, F_SETFL, flags | O_NONBLOCK);
        if (::connect(socket_fd, addr->ai_addr, addr->ai_addrlen) == 0) {
            return true;
        }
        if (errno != EINPROGRESS) {
            return false;
        }
        fd_set write_set;
        FD_ZERO(&write_set);
        FD_SET(socket_fd, &write_set);
        timeval deadline = to_timeval(timeout);
        if (::select(socket_fd + 1, nullptr, &write_set, nullptr, &deadline) <= 0) {
            return false;
        }
        int socket_error = 0;
        socklen_t error_len = sizeof(socket_error);
        if (::getsockopt(socket_fd, SOL_SOCKET, SO_ERROR, &socket_error, &error_len) < 0 ||
            socket_error != 0) {
            return false;
        }
        return true;
    }

    /// @brief POSTs the OTLP path and returns true when the endpoint answers HTTP with any status
    /// other than 404. Send/recv are bounded by `timeout` via socket timeouts. A missing or
    /// unparseable response, or a 404, is treated as "not a working endpoint".
    static bool http_ok(int socket_fd, const UrlParts &parts,
                        std::chrono::milliseconds timeout) noexcept {
        // Back to blocking, with send/recv deadlines so a silent peer can't hang the probe.
        int flags = ::fcntl(socket_fd, F_GETFL, 0);
        ::fcntl(socket_fd, F_SETFL, flags & ~O_NONBLOCK);
        timeval deadline = to_timeval(timeout);
        ::setsockopt(socket_fd, SOL_SOCKET, SO_SNDTIMEO, &deadline, sizeof(deadline));
        ::setsockopt(socket_fd, SOL_SOCKET, SO_RCVTIMEO, &deadline, sizeof(deadline));

        auto request = std::format("POST {} HTTP/1.1\r\nHost: {}\r\nContent-Type: "
                                   "application/x-protobuf\r\nContent-Length: 0\r\n"
                                   "Connection: close\r\n\r\n",
                                   parts.path, parts.host);
        if (::send(socket_fd, request.data(), request.size(), MSG_NOSIGNAL) < 0) {
            return false;
        }
        std::array<char, 64> buffer{};
        auto received = ::recv(socket_fd, buffer.data(), buffer.size() - 1, 0);
        if (received <= 0) {
            return false;
        }
        // Status line looks like "HTTP/1.1 404 Not Found" — pull the 3-digit code.
        std::string_view response{buffer.data(), static_cast<std::size_t>(received)};
        auto space = response.find(' ');
        if (space == std::string_view::npos || space + 4 > response.size()) {
            return false;
        }
        auto status = response.substr(space + 1, 3);
        return status != "404";
    }

    static timeval to_timeval(std::chrono::milliseconds timeout) noexcept {
        timeval value{};
        value.tv_sec = static_cast<time_t>(timeout.count() / 1000);
        value.tv_usec = static_cast<suseconds_t>((timeout.count() % 1000) * 1000);
        return value;
    }
};

class OtelOtlpPlugin final : public congelado::Plugin, public interfaces::IOtelProvider {
  public:
    [[nodiscard]] std::string_view get_name() const noexcept override { return "OtelOtlpPlugin"; }
    [[nodiscard]] std::string_view get_version() const noexcept override { return "1.0.0"; }
    [[nodiscard]] std::string_view get_unique_type() const noexcept override { return "otel"; }
    [[nodiscard]] std::uint32_t capabilities() const noexcept override { return CONGELADO_CAP_OTEL; }

    /// @brief Bridges the `_cap_dispatch::has_otel_get`/`otel_get()` SFINAE convention — returns
    /// this instance as its own `IOtelProvider`.
    void *otel_get() noexcept { return static_cast<interfaces::IOtelProvider *>(this); }

    [[nodiscard]] interfaces::ITracerProvider *get_tracer_provider() noexcept override {
        return m_tracer_backend.get();
    }
    [[nodiscard]] interfaces::IMeterProvider *get_meter_provider() noexcept override {
        return m_meter_backend.get();
    }
    [[nodiscard]] interfaces::ILogRecordProvider *get_log_provider() noexcept override {
        return m_log_backend.get();
    }

    /**
     * @brief Reads `endpoint` (defaults to `http://localhost:4318`, the standard Collector
     * OTLP/HTTP port) and `service_name` (defaults to `"congelado"`) out of config, then builds
     * the trace/metrics/logs OTLP/HTTP export pipelines.
     * @note Each signal's exporter needs the *full* per-signal path (`/v1/traces`, `/v1/metrics`,
     * `/v1/logs`) — `OtlpHttp*ExporterOptions::url` is "the exact endpoint to POST to," not
     * auto-suffixed by the SDK. `endpoint` gets that standard suffix appended per signal by
     * default; `traces_endpoint`/`metrics_endpoint`/`logs_endpoint` config fields override any
     * one of them individually, for backends with a non-standard OTLP path (e.g. Prometheus's
     * native OTLP receiver at `/api/v1/otlp/v1/metrics` rather than the plain `/v1/metrics` a
     * Collector expects).
     * @note `service_name` becomes the `service.name` resource attribute on every exported span/
     * metric/log — without it every process using this same plugin reports under the SDK's
     * generic default, making server and worker indistinguishable in Grafana/Loki/Tempo. Set
     * differently per process in each process's own toml (see config/congelado.toml vs
     * worker.toml).
     * @param cfg this plugin's config view.
     */
    void on_load(CongeladoHostCallbacks const & /*host*/, CongeladoConfigView const &cfg) override {
        // Must land before any Provider is constructed — GlobalLogHandler is a lazy singleton
        // that TracerProvider/MeterProvider/LoggerProvider's ctors touch first for their own
        // debug logging, so registering late would miss whatever they log on the way up.
        otel_internal_log::GlobalLogHandler::SetLogHandler(
            opentelemetry::nostd::shared_ptr<otel_internal_log::LogHandler>(new OtelSdkLogHandler()));
        otel_internal_log::GlobalLogHandler::SetLogLevel(otel_internal_log::LogLevel::Warning);

        std::string endpoint = "http://localhost:4318";
        if (auto val = congelado::config_get(cfg, "endpoint")) {
            endpoint = *val;
        }
        std::string service_name = "congelado";
        if (auto val = congelado::config_get(cfg, "service_name")) {
            service_name = *val;
        }
        auto resource = resource_sdk::Resource::Create({{"service.name", service_name}});

        auto signal_endpoint = [&](const char *config_key, const char *suffix) {
            std::string result = endpoint + suffix;
            if (auto val = congelado::config_get(cfg, config_key)) {
                result = *val;
            }
            return result;
        };
        auto traces_endpoint = signal_endpoint("traces_endpoint", "/v1/traces");
        auto metrics_endpoint = signal_endpoint("metrics_endpoint", "/v1/metrics");
        auto logs_endpoint = signal_endpoint("logs_endpoint", "/v1/logs");

        // Probe every signal's endpoint before wiring its exporter, so we never spin up a pipeline
        // that can only ever emit per-export "connection failed / 404" errors. `required` decides
        // what an unreachable endpoint means:
        //   required = true  (default): a hard plugin-load failure — throw, which propagates out
        //                    through congelado_init (the CONGELADO_PLUGIN macro) as an init error
        //                    the host treats as fatal. Surfaces broken observability wiring loudly.
        //   required = false: skip just that signal's exporter (no pipeline, no export spam) and
        //                    load the plugin anyway. For a bare-metal run where some/all backends
        //                    aren't up — telemetry for reachable signals still flows.
        // Defaults to true. Accept the several falsey spellings this value can arrive as: the host
        // config loader serializes a TOML boolean `false` as "0" (toml++ coerces bool->int before
        // the string branch), while a JSON/string source may give "false". Treat "0"/"false"/"no"/
        // "off" as false, anything else as true.
        bool required = true;
        if (auto val = congelado::config_get(cfg, "required")) {
            required = !(*val == "0" || *val == "false" || *val == "no" || *val == "off");
        }

        constexpr auto PROBE_TIMEOUT = std::chrono::milliseconds{2000};
        auto probe_signal = [&](const char *signal, const std::string &url) -> bool {
            if (EndpointProbe::reachable(url, PROBE_TIMEOUT)) {
                return true;
            }
            if (required) {
                // Log the concrete reason before throwing — the CONGELADO_PLUGIN macro catches the
                // exception and collapses it to a generic "congelado_init failed", so what() alone
                // would never reach the operator.
                auto message =
                    std::format("otel_otlp: {} endpoint unreachable: {} (set required = false to "
                                "load anyway)",
                                signal, url);
                core::logger::error("otel_otlp", "{}", message);
                std::println(stderr, "[otel_otlp] {}", message);
                throw std::runtime_error(message);
            }
            core::logger::warning("otel_otlp",
                                  "{} endpoint unreachable: {} — skipping (required = false)", signal,
                                  url);
            return false;
        };

        bool traces_ok = probe_signal("traces", traces_endpoint);
        bool metrics_ok = probe_signal("metrics", metrics_endpoint);
        bool logs_ok = probe_signal("logs", logs_endpoint);

        if (traces_ok) {
            setup_traces(traces_endpoint, resource);
        }
        if (metrics_ok) {
            setup_metrics(metrics_endpoint, resource);
        }
        if (logs_ok) {
            setup_logs(logs_endpoint, resource);
        }

        if (!traces_ok && !metrics_ok && !logs_ok) {
            core::logger::warning(
                "otel_otlp", "no reachable OTLP endpoint — plugin loaded but exporting nothing");
            return;
        }
        core::logger::important(
            "otel_otlp", "exporting via OTLP/HTTP — service={} traces={} metrics={} logs={}",
            service_name, traces_ok ? traces_endpoint : "(skipped)",
            metrics_ok ? metrics_endpoint : "(skipped)", logs_ok ? logs_endpoint : "(skipped)");
    }

    // Fires from SharedLibrary::close_all() on process shutdown — ForceFlush before Shutdown so
    // whatever's still batched (not yet hit the 5s/512-span auto-flush) gets drained, not dropped.
    //
    // Each provider is reset() right here, immediately after its own Shutdown() — not left for
    // ~OtelOtlpPlugin()'s member destructors to pick up later. The OTel SDK's provider dtors run
    // their own internal Shutdown() too, as a safety net for callers who never shut down
    // explicitly; harmless on its own (logged as "invoked only once" and skipped), but member
    // declaration order here (tracer, meter, logger) destructs in the *opposite* order — logger,
    // meter, tracer — of the sequence just shut down above, so providers ended up torn down while
    // sibling providers destructing around them were in a different state than this function's
    // own explicit sequence intended. Resetting right after each Shutdown() closes that gap: one
    // signal fully flushes, shuts down, and destructs before the next one starts, full stop —
    // confirmed live as the fix for a segfault during close_all().
    void on_unload() noexcept override {
        constexpr auto TIMEOUT = std::chrono::seconds(5);
        if (m_tracer_provider) {
            m_tracer_provider->ForceFlush(TIMEOUT);
            m_tracer_provider->Shutdown(TIMEOUT);
            m_tracer_backend.reset();
            m_tracer_provider.reset();
        }
        if (m_meter_provider) {
            m_meter_provider->ForceFlush(TIMEOUT);
            m_meter_provider->Shutdown(TIMEOUT);
            m_meter_backend.reset();
            m_meter_provider.reset();
        }
        if (m_logger_provider) {
            m_logger_provider->ForceFlush(TIMEOUT);
            m_logger_provider->Shutdown(TIMEOUT);
            m_log_backend.reset();
            m_logger_provider.reset();
        }
    }

  private:
    void setup_traces(const std::string &endpoint, const resource_sdk::Resource &resource) {
        otlp::OtlpHttpExporterOptions options;
        options.url = endpoint;
        auto exporter = std::make_unique<otlp::OtlpHttpExporter>(options);
        auto logging_exporter = std::make_unique<LoggingSpanExporter>(std::move(exporter));
        auto processor = std::make_unique<trace_sdk::BatchSpanProcessor>(
            std::move(logging_exporter), trace_sdk::BatchSpanProcessorOptions{});
        m_tracer_provider =
            std::make_shared<trace_sdk::TracerProvider>(std::move(processor), resource);
        m_tracer_backend = std::make_unique<TracerBackend>(m_tracer_provider->GetTracer("congelado"));
    }

    void setup_metrics(const std::string &endpoint, const resource_sdk::Resource &resource) {
        otlp::OtlpHttpMetricExporterOptions options;
        options.url = endpoint;
        auto exporter = std::make_unique<otlp::OtlpHttpMetricExporter>(options);
        auto logging_exporter = std::make_unique<LoggingMetricExporter>(std::move(exporter));
        metrics_sdk::PeriodicExportingMetricReaderOptions reader_options;
        auto reader = std::make_unique<metrics_sdk::PeriodicExportingMetricReader>(
            std::move(logging_exporter), reader_options);
        auto provider = std::make_shared<metrics_sdk::MeterProvider>(
            std::unique_ptr<metrics_sdk::ViewRegistry>(new metrics_sdk::ViewRegistry()), resource);
        provider->AddMetricReader(std::move(reader));
        m_meter_provider = provider;
        m_meter_backend = std::make_unique<MeterBackend>(m_meter_provider->GetMeter("congelado"));
    }

    void setup_logs(const std::string &endpoint, const resource_sdk::Resource &resource) {
        otlp::OtlpHttpLogRecordExporterOptions options;
        options.url = endpoint;
        auto exporter = std::make_unique<otlp::OtlpHttpLogRecordExporter>(options);
        auto logging_exporter = std::make_unique<LoggingLogRecordExporter>(std::move(exporter));
        auto processor = std::make_unique<logs_sdk::BatchLogRecordProcessor>(std::move(logging_exporter));
        m_logger_provider =
            std::make_shared<logs_sdk::LoggerProvider>(std::move(processor), resource);
        m_log_backend =
            std::make_unique<LogBackend>(m_logger_provider->GetLogger("congelado", "congelado"));
    }

    std::shared_ptr<trace_sdk::TracerProvider> m_tracer_provider;
    std::shared_ptr<metrics_sdk::MeterProvider> m_meter_provider;
    std::shared_ptr<logs_sdk::LoggerProvider> m_logger_provider;
    std::unique_ptr<TracerBackend> m_tracer_backend;
    std::unique_ptr<MeterBackend> m_meter_backend;
    std::unique_ptr<LogBackend> m_log_backend;
};

} // namespace

CONGELADO_PLUGIN(OtelOtlpPlugin)
