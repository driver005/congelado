export module cc_abi_builder_otel:leaves;

import std;
import cc_abi_primitives;
import cc_abi_sonic_intern;

export namespace ice::builder::otel {

// Counter/Histogram/Span — produced by Meter::create_counter/create_histogram and
// Tracer::start_span respectively, no independent existence outside their producing type
// (mirrors ice::builder::Function's relationship to Builder::enter_border_patrol).

class Counter
{
public:
    virtual ~Counter() = default;

    virtual std::expected<void, ice::Status> add(double value) = 0;
};

class Histogram
{
public:
    virtual ~Histogram() = default;

    virtual std::expected<void, ice::Status> record(double value) = 0;
};

class Span
{
public:
    virtual ~Span() = default;

    virtual std::expected<void, ice::Status> set_attribute(
        const ice::String& key, const ice::String& value
    ) = 0;

    virtual std::expected<void, ice::Status>
    set_status(int status, const ice::String& description) = 0;

    virtual std::expected<void, ice::Status> end() = 0;
};

} // namespace ice::builder
