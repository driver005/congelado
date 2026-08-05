export module core_otel:metrics;

import std;
import interfaces;
import :registry;

namespace core::otel::detail {

/**
 * @brief Per-instrument-name cache of one `ICounter` handle per currently-registered
 * `IMeterProvider`, so repeated `counter_add("task.completed", ...)`-style facade calls don't
 * re-create the instrument on every call. Mutex-guarded since counters get bumped from many
 * threads (HTTP handlers, poll-cycle contracts) concurrently.
 */
struct CounterCache {
    std::mutex mutex;
    std::unordered_map<std::string, std::vector<std::shared_ptr<interfaces::ICounter>>> entries;
};

/// @brief Same caching deal as `CounterCache`, for histogram instruments.
struct HistogramCache {
    std::mutex mutex;
    std::unordered_map<std::string, std::vector<std::shared_ptr<interfaces::IHistogram>>> entries;
};

inline CounterCache &counter_cache() {
    static CounterCache cache;
    return cache;
}

inline HistogramCache &histogram_cache() {
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
inline void counter_add(std::string_view name, double value,
                        std::span<const interfaces::Attribute> attrs = {}) noexcept {
    try {
        auto *registry = MeterRegistry::get_active();
        if (registry == nullptr || !registry->has_provider()) {
            return;
        }
        auto &cache = detail::counter_cache();
        std::scoped_lock lock(cache.mutex);
        auto &counters = cache.entries[std::string{name}];
        if (counters.empty()) {
            counters.reserve(registry->get_providers().size());
            for (const auto &provider : registry->get_providers()) {
                counters.push_back(provider->create_counter(name, "", ""));
            }
        }
        for (const auto &counter : counters) {
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
 * creating the instrument on first use per provider. Same never-throws/graceful-degrade contract
 * as `counter_add()`.
 * @param name the histogram's name (e.g. `"task.duration_ms"`).
 * @param value the observed value.
 * @param attrs attributes (dimensions) this data point carries.
 */
inline void histogram_record(std::string_view name, double value,
                             std::span<const interfaces::Attribute> attrs = {}) noexcept {
    try {
        auto *registry = MeterRegistry::get_active();
        if (registry == nullptr || !registry->has_provider()) {
            return;
        }
        auto &cache = detail::histogram_cache();
        std::scoped_lock lock(cache.mutex);
        auto &histograms = cache.entries[std::string{name}];
        if (histograms.empty()) {
            histograms.reserve(registry->get_providers().size());
            for (const auto &provider : registry->get_providers()) {
                histograms.push_back(provider->create_histogram(name, "", ""));
            }
        }
        for (const auto &histogram : histograms) {
            if (histogram) {
                histogram->record(value, attrs);
            }
        }
    } catch (...) {
        // Telemetry must never take the caller down with it.
    }
}

} // namespace core::otel
