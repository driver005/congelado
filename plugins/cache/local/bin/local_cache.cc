module;

#define CONGELADO_GUEST
#include <congelado/plugin.h>

export module local_cache_plugin;

import congelado_plugin;
import interfaces;
import shared;
import std;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

/**
 * @brief The in-process fallback cache (`ICache`), now packaged as a loadable plugin — same
 * in-memory `unordered_map` store Connector keeps built in as `LocalCache`
 * (`include/connector/local_cache.cppm`), exposed here as the `CONGELADO_CAP_CACHE` capability so a
 * deployment can select `local` as its cache backend the same way it selects redis. No mutex: cache
 * calls run inside ASIO handlers, already serialized (see the project's connector conventions).
 */
class LocalCachePlugin : public congelado::Plugin, public interfaces::ICache {
  public:
    [[nodiscard]] std::string_view get_name() const noexcept override { return "local"; }
    [[nodiscard]] std::string_view get_version() const noexcept override { return "0.1.0"; }

    /**
     * @brief Flags this as cache-capable, so the host wires `cache_get` into the `_cap_dispatch`
     * routing and resolves this plugin's ICache* for the connector.
     * @return `CONGELADO_CAP_CACHE`.
     */
    [[nodiscard]] std::uint32_t capabilities() const noexcept override {
        return CONGELADO_CAP_CACHE;
    }

    /**
     * @brief Capability hook the host calls to get at this plugin's `ICache` surface.
     * @return this instance, upcast to `interfaces::ICache*`.
     */
    void *cache_get() noexcept { return static_cast<interfaces::ICache *>(this); }

    /**
     * @brief Identifies this cache backend.
     * @return the fixed string "local" — in-process, no external service behind it.
     */
    [[nodiscard]] std::string_view backend_name() const noexcept override { return "local"; }

    /**
     * @brief Looks up `key` in the in-memory store.
     * @note Fully synchronous despite the async-shaped signature — `result` fires before this call
     * returns.
     * @param key the key to look up.
     * @param result gets the stored value, or an empty string if `key` isn't present.
     */
    void get(std::string_view key, shared::QueryReadFn &&result) noexcept override {
        auto iterator = m_store.find(std::string{key});
        if (iterator == m_store.end()) {
            std::move(result)("");
        } else {
            std::move(result)(iterator->second);
        }
    }

    /**
     * @brief Writes `value` under `key`, overwriting whatever was there.
     * @param key the key to write to.
     * @param value the value getting stashed.
     * @param result always gets an empty string — this backend never fails a write.
     */
    void set(std::string_view key, std::string_view value,
             shared::QueryReadFn &&result) noexcept override {
        m_store.insert_or_assign(std::string{key}, std::string{value});
        std::move(result)("");
    }

    /**
     * @brief Removes whatever's stored under `key`. No-op, no error, if the key wasn't present.
     * @param key the key to remove.
     * @param result always gets an empty string, same as set().
     */
    void remove(std::string_view key, shared::QueryReadFn &&result) noexcept override {
        m_store.erase(std::string{key});
        std::move(result)("");
    }

  private:
    std::unordered_map<std::string, std::string> m_store;
};

CONGELADO_PLUGIN(LocalCachePlugin);

#ifdef CONGELADO_TEST
namespace local_cache_plugin_tests {
using namespace boost::ut;

/// @brief Small test-only helper class — keeps the "class-only, no free functions" convention
/// even for test scaffolding.
class LocalCacheTestHelper {
  public:
    LocalCacheTestHelper() = delete;

    /// @brief Runs `fn` synchronously against a `LocalCachePlugin` op that reports through a
    /// `shared::QueryReadFn` callback, and hands back whatever the callback got — every op on
    /// this backend fires its callback before returning, so there's no real async to wait on.
    [[nodiscard]] static std::string run_sync(auto &&fn) {
        std::string captured;
        fn([&](std::string_view value) { captured = std::string{value}; });
        return captured;
    }
};

suite<"LocalCachePlugin"> local_cache_plugin_suite = [] {
    "get_name reports 'local'"_test = [] {
        LocalCachePlugin plugin;
        expect(plugin.get_name() == "local");
    };

    "get_version reports a non-empty version string"_test = [] {
        LocalCachePlugin plugin;
        expect(plugin.get_version() == "0.1.0");
    };

    "capabilities reports CONGELADO_CAP_CACHE"_test = [] {
        LocalCachePlugin plugin;
        expect(plugin.capabilities() == CONGELADO_CAP_CACHE);
    };

    "cache_get returns this instance upcast to ICache*"_test = [] {
        LocalCachePlugin plugin;
        expect(plugin.cache_get() == static_cast<interfaces::ICache *>(&plugin));
    };

    "backend_name reports 'local'"_test = [] {
        LocalCachePlugin plugin;
        expect(plugin.backend_name() == "local");
    };

    "get on a fresh store misses, reporting an empty string"_test = [] {
        LocalCachePlugin plugin;
        auto result = LocalCacheTestHelper::run_sync([&](shared::QueryReadFn &&cb) { plugin.get("missing", std::move(cb)); });
        expect(result.empty());
    };

    "set-then-get round-trips the stored value"_test = [] {
        LocalCachePlugin plugin;
        auto set_result =
            LocalCacheTestHelper::run_sync([&](shared::QueryReadFn &&cb) { plugin.set("key", "value", std::move(cb)); });
        expect(set_result.empty());

        auto get_result = LocalCacheTestHelper::run_sync([&](shared::QueryReadFn &&cb) { plugin.get("key", std::move(cb)); });
        expect(get_result == "value");
    };

    "set overwrites whatever was previously stored under the same key"_test = [] {
        LocalCachePlugin plugin;
        LocalCacheTestHelper::run_sync([&](shared::QueryReadFn &&cb) { plugin.set("key", "first", std::move(cb)); });
        LocalCacheTestHelper::run_sync([&](shared::QueryReadFn &&cb) { plugin.set("key", "second", std::move(cb)); });

        auto get_result = LocalCacheTestHelper::run_sync([&](shared::QueryReadFn &&cb) { plugin.get("key", std::move(cb)); });
        expect(get_result == "second");
    };

    "remove drops a present key, later get misses"_test = [] {
        LocalCachePlugin plugin;
        LocalCacheTestHelper::run_sync([&](shared::QueryReadFn &&cb) { plugin.set("key", "value", std::move(cb)); });

        auto remove_result =
            LocalCacheTestHelper::run_sync([&](shared::QueryReadFn &&cb) { plugin.remove("key", std::move(cb)); });
        expect(remove_result.empty());

        auto get_result = LocalCacheTestHelper::run_sync([&](shared::QueryReadFn &&cb) { plugin.get("key", std::move(cb)); });
        expect(get_result.empty());
    };

    "remove on an absent key is a harmless no-op"_test = [] {
        LocalCachePlugin plugin;
        auto remove_result =
            LocalCacheTestHelper::run_sync([&](shared::QueryReadFn &&cb) { plugin.remove("never-set", std::move(cb)); });
        expect(remove_result.empty());
    };

    // m_store (std::unordered_map<std::string, std::string>) has no size cap or eviction policy —
    // no LRU, no TTL, nothing. This backend is documented (see the class comment above) as an
    // intentionally simple in-process fallback, not something meant to need a Contract, so this
    // gap is an acknowledged design trade-off rather than a new finding. This test pins that
    // behavior at a moderate, safe scale: 1000 distinct keys, well short of anything that would
    // stress memory in a shared test binary, all still present and retrievable afterward with
    // nothing silently evicted.
    "set with many distinct keys retains all entries with no eviction"_test = [] {
        LocalCachePlugin plugin;
        constexpr int count = 1000;
        for (int index = 0; index < count; ++index) {
            auto key = std::string{"key-"} + std::to_string(index);
            auto value = std::string{"value-"} + std::to_string(index);
            LocalCacheTestHelper::run_sync(
                [&](shared::QueryReadFn &&cb) { plugin.set(key, value, std::move(cb)); });
        }

        for (int index = 0; index < count; ++index) {
            auto key = std::string{"key-"} + std::to_string(index);
            auto expected_value = std::string{"value-"} + std::to_string(index);
            auto get_result =
                LocalCacheTestHelper::run_sync([&](shared::QueryReadFn &&cb) { plugin.get(key, std::move(cb)); });
            expect(get_result == expected_value);
        }
    };
};

} // namespace local_cache_plugin_tests
#endif
