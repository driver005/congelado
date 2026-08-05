export module core_otel:span;

import std;
import interfaces;
import :registry;

namespace core::otel {

// FIXME(clang-tidy): cppcoreguidelines-avoid-non-const-global-variables — thread_local nesting
// stack for the ambient "current span" concept, mirrors `shared::this_handler::current`/
// `current_id` — mutated by every `start_span()`/`ScopedSpan` pop, can't be const for that
// reason.
thread_local inline std::vector<interfaces::SpanContext> t_span_stack;

// FIXME(clang-tidy): cppcoreguidelines-avoid-non-const-global-variables — per-thread RNG state
// for generating trace/span ids, reseeded once per thread on first use.
thread_local inline std::mt19937_64 t_rng{std::random_device{}()};

/**
 * @brief Splits a 64-bit value into 8 big-endian bytes.
 * @param value the value to split.
 * @return the value's bytes, most-significant first.
 */
inline std::array<std::byte, 8> to_bytes(std::uint64_t value) {
    std::array<std::byte, 8> out{};
    for (int i = 0; i < 8; ++i) {
        out[static_cast<std::size_t>(i)] =
            static_cast<std::byte>((value >> (8 * (7 - i))) & 0xFFU);
    }
    return out;
}

/**
 * @brief Generates a fresh 128-bit trace id — two random 64-bit halves concatenated.
 * @return a new random trace id.
 */
inline std::array<std::byte, 16> generate_trace_id() {
    std::array<std::byte, 16> id{};
    auto hi = to_bytes(t_rng());
    auto lo = to_bytes(t_rng());
    std::ranges::copy(hi, id.begin());
    std::ranges::copy(lo, id.begin() + 8);
    return id;
}

/**
 * @brief Generates a fresh 64-bit span id.
 * @return a new random span id.
 */
inline std::array<std::byte, 8> generate_span_id() { return to_bytes(t_rng()); }

} // namespace core::otel

export namespace core::otel {
class ScopedSpan;
}

namespace core::otel::detail {

/// @brief The two pieces every new span needs: its ids, and one live `ISpan` per currently
/// registered `ITracerProvider`.
struct NewSpan {
    interfaces::SpanContext ctx;
    std::vector<std::shared_ptr<interfaces::ISpan>> spans;
};

/**
 * @brief Shared implementation behind every span-starting entry point (`start_span()`'s two
 * overloads, `start_detached_span()`) — builds the child `SpanContext` from `parent` (or a fresh
 * trace root if `parent` is `std::nullopt`) and starts one per-provider span on every currently-
 * registered `TracerRegistry` provider. Does not touch the ambient stack — callers that want
 * ambient nesting (`ScopedSpan`) push/pop it themselves around this.
 * @param name the span's operation name.
 * @param kind the span's role.
 * @param parent the parent context to derive from, or `std::nullopt` for a fresh trace root.
 * @param attrs attributes to set on the span at start time.
 * @return the new span's context and per-provider handles.
 */
inline NewSpan build_new_span(std::string_view name, interfaces::SpanKind kind,
                              const std::optional<interfaces::SpanContext> &parent,
                              std::span<const interfaces::Attribute> attrs) {
    interfaces::SpanContext ctx;
    if (parent.has_value()) {
        ctx.trace_id = parent->trace_id;
        ctx.parent_span_id = parent->span_id;
        ctx.sampled = parent->sampled;
    } else {
        ctx.trace_id = generate_trace_id();
        ctx.parent_span_id = {};
        ctx.sampled = true;
    }
    ctx.span_id = generate_span_id();

    std::vector<std::shared_ptr<interfaces::ISpan>> spans;
    if (auto *registry = TracerRegistry::get_active(); registry != nullptr) {
        spans.reserve(registry->get_providers().size());
        for (const auto &provider : registry->get_providers()) {
            if (auto span = provider->start_span(name, kind, ctx, attrs)) {
                spans.push_back(std::move(span));
            }
        }
    }

    return NewSpan{ctx, std::move(spans)};
}

/**
 * @brief `start_span()`'s implementation — delegates to `build_new_span()` then pushes the new
 * context onto the calling thread's ambient stack before handing back a `ScopedSpan`. Only
 * declared here (not yet defined — `ScopedSpan` isn't complete at this point in the file); the
 * out-of-line definition sits right after `ScopedSpan`'s own definition below. Declaring the
 * prototype here first is what lets `ScopedSpan`'s friend declaration bind to it by qualified
 * name.
 * @param name the span's operation name.
 * @param kind the span's role.
 * @param parent the parent context to derive from, or `std::nullopt` for a fresh trace root.
 * @param attrs attributes to set on the span at start time.
 * @return the new `ScopedSpan`.
 */
ScopedSpan start_span_impl(std::string_view name, interfaces::SpanKind kind,
                          const std::optional<interfaces::SpanContext> &parent,
                          std::span<const interfaces::Attribute> attrs);

} // namespace core::otel::detail

export namespace core::otel {

/**
 * @brief RAII handle for one logical span, holding one `interfaces::ISpan` per currently-
 * registered `ITracerProvider` (the fan-out itself) plus the `SpanContext` shared by all of
 * them. Forwards `set_attribute`/`add_event`/`set_status` to every held span. Ending (via
 * `end()`, or the destructor) ends every held span and pops this span back off the calling
 * thread's ambient stack — degrades to a pure context-tracking no-op (empty span list) when zero
 * providers are registered, matching the logger/serde registries' own graceful pre-registration
 * fallback.
 * @warning Expected to nest LIFO like any RAII scope guard — ending out of order (this isn't the
 * top of the calling thread's span stack when it ends) is tolerated (the stack entry matching
 * this span's id gets popped regardless of position) but is a real misuse smell worth noticing
 * if it ever happens.
 */
class ScopedSpan {
  public:
    /**
     * @brief Move ctor — steals `other`'s state and disarms it so its destructor becomes a
     * no-op.
     * @param other the span to move from.
     */
    ScopedSpan(ScopedSpan &&other) noexcept
        : m_context{other.m_context}, m_spans{std::move(other.m_spans)}, m_pushed{other.m_pushed},
          m_ended{other.m_ended} {
        other.m_pushed = false;
        other.m_ended = true;
    }

    /**
     * @brief Move assignment — ends this span first (if still live), then steals `other`'s
     * state and disarms it.
     * @param other the span to move from.
     * @return `*this`.
     */
    ScopedSpan &operator=(ScopedSpan &&other) noexcept {
        if (this != &other) {
            end();
            m_context = other.m_context;
            m_spans = std::move(other.m_spans);
            m_pushed = other.m_pushed;
            m_ended = other.m_ended;
            other.m_pushed = false;
            other.m_ended = true;
        }
        return *this;
    }

    ScopedSpan(const ScopedSpan &) = delete;
    ScopedSpan &operator=(const ScopedSpan &) = delete;

    /**
     * @brief Ends the span (if not already ended) on destruction — the normal RAII path.
     */
    ~ScopedSpan() { end(); }

    /**
     * @brief Attaches or overwrites one attribute on every held per-provider span.
     * @param key the attribute key.
     * @param value the attribute value.
     */
    void set_attribute(std::string_view key, const interfaces::AttributeValue &value) noexcept {
        for (const auto &span : m_spans) {
            span->set_attribute(key, value);
        }
    }

    /**
     * @brief Records a timestamped event on every held per-provider span.
     * @param name the event name.
     * @param attrs attributes carried on the event, if any.
     */
    void add_event(std::string_view name,
                   std::span<const interfaces::Attribute> attrs = {}) noexcept {
        for (const auto &span : m_spans) {
            span->add_event(name, attrs);
        }
    }

    /**
     * @brief Sets this span's completion status on every held per-provider span.
     * @param status OK/ERROR/UNSET.
     * @param description optional human-readable detail.
     */
    void set_status(interfaces::SpanStatus status, std::string_view description = "") noexcept {
        for (const auto &span : m_spans) {
            span->set_status(status, description);
        }
    }

    /**
     * @brief Ends every held per-provider span and pops this span off the calling thread's
     * ambient stack. Safe to call more than once (and safe to let the destructor call it again)
     * — the second call is a no-op.
     */
    void end() noexcept {
        if (m_ended) {
            return;
        }
        m_ended = true;
        for (const auto &span : m_spans) {
            span->end();
        }
        if (m_pushed) {
            auto &stack = t_span_stack;
            // Normal case: this span is the top of the stack, pop it. Out-of-order end (this
            // span isn't the top) is tolerated — find and erase it wherever it sits rather than
            // corrupting the stack for spans that outlive this one.
            auto it = std::ranges::find_if(
                stack, [this](const auto &ctx) { return ctx.span_id == m_context.span_id; });
            if (it != stack.end()) {
                stack.erase(it);
            }
            m_pushed = false;
        }
    }

    /**
     * @brief This span's trace/span/parent ids — what a caller injects into an outbound
     * `traceparent` header, or captures across a thread hop for later reattachment.
     * @return the span's context.
     */
    [[nodiscard]] const interfaces::SpanContext &context() const noexcept { return m_context; }

  private:
    friend ScopedSpan detail::start_span_impl(std::string_view, interfaces::SpanKind,
                                              const std::optional<interfaces::SpanContext> &,
                                              std::span<const interfaces::Attribute>);

    ScopedSpan(interfaces::SpanContext context,
              std::vector<std::shared_ptr<interfaces::ISpan>> spans, bool pushed)
        : m_context{context}, m_spans{std::move(spans)}, m_pushed{pushed} {}

    interfaces::SpanContext m_context;
    std::vector<std::shared_ptr<interfaces::ISpan>> m_spans;
    bool m_pushed{false};
    bool m_ended{false};
};

/**
 * @brief Reads the calling thread's current ambient span context without mutating anything —
 * this is what gets captured by value into a lambda at a thread-hop point (e.g.
 * `WorkerContext::call_engine`'s pending-map entry) for later reattachment via the
 * explicit-parent `start_span()` overload on whichever thread the response actually resolves on.
 * @return the top of the calling thread's span stack, or `std::nullopt` if nothing's active.
 */
[[nodiscard]] inline std::optional<interfaces::SpanContext> current_context() noexcept {
    if (t_span_stack.empty()) {
        return std::nullopt;
    }
    return t_span_stack.back();
}

} // namespace core::otel

namespace core::otel::detail {

// Out-of-line definition of the prototype declared above, now that ScopedSpan is complete.
inline ScopedSpan start_span_impl(std::string_view name, interfaces::SpanKind kind,
                                  const std::optional<interfaces::SpanContext> &parent,
                                  std::span<const interfaces::Attribute> attrs) {
    auto built = build_new_span(name, kind, parent, attrs);
    t_span_stack.push_back(built.ctx);
    return ScopedSpan{built.ctx, std::move(built.spans), true};
}

} // namespace core::otel::detail

export namespace core::otel {

/**
 * @brief Starts a new span, parented to the calling thread's current ambient span if one exists,
 * or a fresh trace root otherwise.
 * @param name the span's operation name (e.g. `"POST /tasks/:id/result"`).
 * @param kind the span's role — defaults to `INTERNAL`.
 * @param attrs attributes to set on the span at start time.
 * @return the new `ScopedSpan`.
 */
[[nodiscard]] inline ScopedSpan
start_span(std::string_view name, interfaces::SpanKind kind = interfaces::SpanKind::INTERNAL,
          std::span<const interfaces::Attribute> attrs = {}) {
    return detail::start_span_impl(name, kind, current_context(), attrs);
}

/**
 * @brief Starts a new span with an explicit parent context rather than reading the calling
 * thread's ambient stack — for the (rarer) case of resuming ambient nesting on a thread that
 * didn't itself start the parent span, e.g. a fresh root span per poll-cycle contract
 * reschedule that still wants to record which trace it logically continues.
 * @param name the span's operation name.
 * @param kind the span's role.
 * @param parent the parent context this span is a child of.
 * @param attrs attributes to set on the span at start time.
 * @return the new `ScopedSpan`.
 */
[[nodiscard]] inline ScopedSpan start_span(std::string_view name, interfaces::SpanKind kind,
                                           const interfaces::SpanContext &parent,
                                           std::span<const interfaces::Attribute> attrs = {}) {
    return detail::start_span_impl(name, kind, parent, attrs);
}

/**
 * @brief RAII-ish span handle that, unlike `ScopedSpan`, never touches the ambient thread-local
 * stack — for spans whose lifetime spans a genuine async gap (started on one thread, ended when
 * a callback fires later on a possibly different thread), where "ambient nesting" doesn't make
 * sense in the first place since nothing runs "inside" it on any one thread. Used by
 * `ClientRuntime::send()`/`dispatch()` for the typed-client async round-trip: the span is moved
 * into the pending-callback closure at send time and ended from inside `dispatch()` once the
 * response arrives, on whatever thread that turns out to be.
 */
class DetachedSpan {
  public:
    DetachedSpan(DetachedSpan &&) noexcept = default;
    DetachedSpan &operator=(DetachedSpan &&) noexcept = default;
    DetachedSpan(const DetachedSpan &) = delete;
    DetachedSpan &operator=(const DetachedSpan &) = delete;

    /// @brief Ends the span (if not already ended) on destruction — same RAII contract as
    /// `ScopedSpan`, minus any ambient stack involvement.
    ~DetachedSpan() { end(); }

    /// @brief Attaches or overwrites one attribute on every held per-provider span.
    void set_attribute(std::string_view key, const interfaces::AttributeValue &value) noexcept {
        for (const auto &span : m_spans) {
            span->set_attribute(key, value);
        }
    }

    /// @brief Records a timestamped event on every held per-provider span.
    void add_event(std::string_view name,
                   std::span<const interfaces::Attribute> attrs = {}) noexcept {
        for (const auto &span : m_spans) {
            span->add_event(name, attrs);
        }
    }

    /// @brief Sets this span's completion status on every held per-provider span.
    void set_status(interfaces::SpanStatus status, std::string_view description = "") noexcept {
        for (const auto &span : m_spans) {
            span->set_status(status, description);
        }
    }

    /// @brief Ends every held per-provider span. Safe to call more than once.
    void end() noexcept {
        if (m_ended) {
            return;
        }
        m_ended = true;
        for (const auto &span : m_spans) {
            span->end();
        }
    }

    /// @brief This span's trace/span/parent ids, e.g. for injecting into an outbound
    /// `traceparent` header before the request that owns this span is actually sent.
    [[nodiscard]] const interfaces::SpanContext &context() const noexcept { return m_context; }

  private:
    friend DetachedSpan start_detached_span(std::string_view, interfaces::SpanKind,
                                            const std::optional<interfaces::SpanContext> &,
                                            std::span<const interfaces::Attribute>);

    DetachedSpan(interfaces::SpanContext context,
                std::vector<std::shared_ptr<interfaces::ISpan>> spans)
        : m_context{context}, m_spans{std::move(spans)} {}

    interfaces::SpanContext m_context;
    std::vector<std::shared_ptr<interfaces::ISpan>> m_spans;
    bool m_ended{false};
};

/**
 * @brief Starts a new span that never touches the ambient thread-local stack — see
 * `DetachedSpan`'s own doc comment for when to reach for this instead of `start_span()`.
 * @param name the span's operation name.
 * @param kind the span's role.
 * @param parent the parent context to derive from; defaults to the calling thread's current
 * ambient context (read once, at call time) so a detached span still nests under whatever
 * ambient span happens to be active right before the async boundary — pass `std::nullopt`
 * explicitly for a fresh trace root instead.
 * @param attrs attributes to set on the span at start time.
 * @return the new `DetachedSpan`.
 */
[[nodiscard]] inline DetachedSpan
start_detached_span(std::string_view name, interfaces::SpanKind kind,
                    const std::optional<interfaces::SpanContext> &parent = current_context(),
                    std::span<const interfaces::Attribute> attrs = {}) {
    auto built = detail::build_new_span(name, kind, parent, attrs);
    return DetachedSpan{built.ctx, std::move(built.spans)};
}

} // namespace core::otel
