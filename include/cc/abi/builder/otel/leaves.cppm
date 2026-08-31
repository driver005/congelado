export module cc_abi_builder_otel:leaves;

import std;
import cc_abi_primitives;
import cc_abi_sonic_intern;

export namespace ice::builder {

// Counter/Histogram/Span — produced by Meter::create_counter/create_histogram and
// Tracer::start_span respectively, no independent existence outside their producing type
// (mirrors ice::builder::Function's relationship to Generator::create_function).

class Counter
{
public:
    // Recover the Counter instance from the opaque void* context slot that every
    // C vtable callback receives.  Named accessor so the cast intent is explicit
    // at the call site and the static_cast appears exactly once, here.
    static Counter* create(void* ctx) noexcept
    {
        return static_cast<Counter*>(ctx);
    }

    virtual ~Counter() = default;

    [[nodiscard]] virtual std::expected<void, ice::Status> add(double value) noexcept = 0;
};

class Histogram
{
public:
    // Recover the Histogram instance from the opaque void* context slot that every
    // C vtable callback receives.  Named accessor so the cast intent is explicit
    // at the call site and the static_cast appears exactly once, here.
    static Histogram* create(void* ctx) noexcept
    {
        return static_cast<Histogram*>(ctx);
    }

    virtual ~Histogram() = default;

    [[nodiscard]] virtual std::expected<void, ice::Status> record(double value) noexcept = 0;
};

class Span
{
public:
    // Recover the Span instance from the opaque void* context slot that every
    // C vtable callback receives.  Named accessor so the cast intent is explicit
    // at the call site and the static_cast appears exactly once, here.
    static Span* create(void* ctx) noexcept
    {
        return static_cast<Span*>(ctx);
    }

    virtual ~Span() = default;

    [[nodiscard]] virtual std::expected<void, ice::Status>
    set_attribute(const ice::String& key, const ice::String& value) noexcept = 0;

    [[nodiscard]] virtual std::expected<void, ice::Status>
    set_status(ice::SpanStatus status, const ice::String& description) noexcept = 0;

    [[nodiscard]] virtual std::expected<void, ice::Status> end() noexcept = 0;
};

} // namespace ice::builder
