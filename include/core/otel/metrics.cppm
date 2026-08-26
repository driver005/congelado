export module core_otel:metrics;

import std;
import interfaces;
import :registry;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

namespace core::otel::detail {

/**
 * @brief Per-instrument-name cache of one `ICounter` handle per currently-registered
 * `IMeterProvider`, so repeated `counter_add("task.completed", ...)`-style facade calls don't
 * re-create the instrument on every call. Mutex-guarded since counters get bumped from many
 * threads (HTTP handlers, poll-cycle contracts) concurrently.
 */
struct CounterCache
{
    std::mutex mutex;
    std::unordered_map<std::string, std::vector<std::shared_ptr<interfaces::ICounter>>> entries;
};

/// @brief Same caching deal as `CounterCache`, for histogram instruments.
struct HistogramCache
{
    std::mutex mutex;
    std::unordered_map<std::string, std::vector<std::shared_ptr<interfaces::IHistogram>>> entries;
};

inline CounterCache& counter_cache()
{
    static CounterCache cache;
    return cache;
}

inline HistogramCache& histogram_cache()
{
    static HistogramCache cache;
    return cache;
}

} // namespace core::otel::detail

export namespace core::otel {

/**
 * @brief Adds `value` to the named counter on every registered `IMeterProvider`, creating the
 * instrument on first use per provider. Never throws — degrades to a silent no-op if no
 * `MeterRegistry` is active, nothing's registered in it, or a provider's instrument creation
 * throws, matching `core::logger`'s own must-never-throw stance for telemetry call sites.
 * @param name the counter's name (e.g. `"task.completed"`).
 * @param value the amount to add.
 * @param attrs attributes (dimensions) this data point carries.
 */
inline void counter_add(
    std::string_view name, double value, std::span<const interfaces::Attribute> attrs = {}
) noexcept
{
    try {
        auto* registry = MeterRegistry::get_active();
        if (registry == nullptr || !registry->has_provider()) {
            return;
        }
        auto& cache = detail::counter_cache();
        std::scoped_lock lock(cache.mutex);
        auto& counters = cache.entries[std::string{name}];
        if (counters.empty()) {
            counters.reserve(registry->get_providers().size());
            for (const auto& provider: registry->get_providers()) {
                counters.push_back(provider->create_counter(name, "", ""));
            }
        }
        for (const auto& counter: counters) {
            if (counter) {
                counter->add(value, attrs);
            }
        }
    } catch (...) {
        // Telemetry must never take the caller down with it.
    }
}

/**
 * @brief Records one observation into the named histogram on every registered `IMeterProvider`,
 * creating the instrument on first use per provider. Same never-throws/graceful-degrade
 * contract as `counter_add()`.
 * @param name the histogram's name (e.g. `"task.duration_ms"`).
 * @param value the observed value.
 * @param attrs attributes (dimensions) this data point carries.
 */
inline void histogram_record(
    std::string_view name, double value, std::span<const interfaces::Attribute> attrs = {}
) noexcept
{
    try {
        auto* registry = MeterRegistry::get_active();
        if (registry == nullptr || !registry->has_provider()) {
            return;
        }
        auto& cache = detail::histogram_cache();
        std::scoped_lock lock(cache.mutex);
        auto& histograms = cache.entries[std::string{name}];
        if (histograms.empty()) {
            histograms.reserve(registry->get_providers().size());
            for (const auto& provider: registry->get_providers()) {
                histograms.push_back(provider->create_histogram(name, "", ""));
            }
        }
        for (const auto& histogram: histograms) {
            if (histogram) {
                histogram->record(value, attrs);
            }
        }
    } catch (...) {
        // Telemetry must never take the caller down with it.
    }
}

} // namespace core::otel

#ifdef CONGELADO_TEST
namespace core::otel::tests {
using namespace boost::ut;

class MetricsFakeCounter : public interfaces::ICounter
{
public:
    void add(double value, std::span<const interfaces::Attribute>) noexcept override
    {
        m_last_value = value;
        ++m_add_count;
    }

    double m_last_value{0.0};
    int m_add_count{0};
};

class MetricsFakeHistogram : public interfaces::IHistogram
{
public:
    void record(double value, std::span<const interfaces::Attribute>) noexcept override
    {
        m_last_value = value;
        ++m_record_count;
    }

    double m_last_value{0.0};
    int m_record_count{0};
};

// Every counter_add()/histogram_record() name goes through a process-wide static cache
// (detail::counter_cache()/histogram_cache()), so each test below uses its own unique
// instrument name to avoid tripping over a stale entry left by another test.
class MetricsFakeMeterProvider : public interfaces::IMeterProvider
{
public:
    [[nodiscard]] std::shared_ptr<interfaces::ICounter>
    create_counter(std::string_view, std::string_view, std::string_view) override
    {
        ++m_create_counter_count;
        m_counter = std::make_shared<MetricsFakeCounter>();
        return m_counter;
    }

    [[nodiscard]] std::shared_ptr<interfaces::IHistogram>
    create_histogram(std::string_view, std::string_view, std::string_view) override
    {
        ++m_create_histogram_count;
        m_histogram = std::make_shared<MetricsFakeHistogram>();
        return m_histogram;
    }

    int m_create_counter_count{0};
    int m_create_histogram_count{0};
    std::shared_ptr<MetricsFakeCounter> m_counter;
    std::shared_ptr<MetricsFakeHistogram> m_histogram;
};

suite<"otel::counter_add"> counter_add_suite = [] {
    "no-op and doesn't throw when no registry is active"_test = [] {
        auto* previous = MeterRegistry::get_active();
        MeterRegistry::set_active(nullptr);

        expect(nothrow([] {
            counter_add("test.metrics.counter.noop", 1.0);
        }));

        MeterRegistry::set_active(previous);
    };

    "creates the instrument once per provider and adds on every call"_test = [] {
        auto* previous = MeterRegistry::get_active();
        MeterRegistry registry;
        auto provider = std::make_shared<MetricsFakeMeterProvider>();
        registry.add_provider(provider);
        MeterRegistry::set_active(&registry);

        counter_add("test.metrics.counter.unique_1", 2.0);
        counter_add("test.metrics.counter.unique_1", 3.0);

        expect(provider->m_create_counter_count == 1);
        expect(provider->m_counter->m_add_count == 2);
        expect(provider->m_counter->m_last_value == 3.0);

        MeterRegistry::set_active(previous);
    };
};

suite<"otel::histogram_record"> histogram_record_suite = [] {
    "no-op and doesn't throw when no registry is active"_test = [] {
        auto* previous = MeterRegistry::get_active();
        MeterRegistry::set_active(nullptr);

        expect(nothrow([] {
            histogram_record("test.metrics.histogram.noop", 1.0);
        }));

        MeterRegistry::set_active(previous);
    };

    "creates the instrument once per provider and records on every call"_test = [] {
        auto* previous = MeterRegistry::get_active();
        MeterRegistry registry;
        auto provider = std::make_shared<MetricsFakeMeterProvider>();
        registry.add_provider(provider);
        MeterRegistry::set_active(&registry);

        histogram_record("test.metrics.histogram.unique_1", 5.0);
        histogram_record("test.metrics.histogram.unique_1", 7.0);

        expect(provider->m_create_histogram_count == 1);
        expect(provider->m_histogram->m_record_count == 2);
        expect(provider->m_histogram->m_last_value == 7.0);

        MeterRegistry::set_active(previous);
    };
};

} // namespace core::otel::tests
#endif
