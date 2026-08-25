module;

#define CONGELADO_GUEST
#include <congelado/plugin.h>
#include <hiredis/hiredis.h>

export module redis_events_plugin;

import congelado_plugin;
import interfaces;
import shared;
import core_events;
import core_logger;
import std;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

/**
 * @brief `IEventSink` + `ICache` backed by Redis via hiredis — same "link a blocking C client
 * library directly, bypass IClient/router" pattern `postgres_plugin` (libpq) and
 * `elasticsearch_plugin` (libcurl) already use, and the same multi-capability shape as
 * `postgres_plugin` itself (`IDatabase` + `ISearchProvider` on one connection). Pub/sub
 * (`PUBLISH`) is fire-and-forget with no persistence — an event published with nobody subscribed
 * to the channel at that instant is simply gone. Deliberate, documented, not a bug — the
 * alternative RabbitMQ/Kafka sinks cover the durable-delivery case. `ICache`'s GET/SET/DEL share
 * this same blocking connection — none of them enter Redis's pub/sub subscriber mode, so mixing
 * cache commands with PUBLISH on one `redisContext` is safe.
 */
class RedisEventsPlugin : public congelado::Plugin,
                          public interfaces::IEventSink,
                          public interfaces::ICache {
  public:
    [[nodiscard]] std::string_view get_name() const noexcept override { return "redis"; }
    [[nodiscard]] std::string_view get_version() const noexcept override { return "0.1.0"; }
    /**
     * @brief Flags this as both event-sink AND cache-capable, so the host wires both
     * `event_get`/`cache_get` into the `_cap_dispatch` routing — this plugin reuses its own
     * `redisContext` connection for cache reads/writes rather than opening a second one, same
     * "both capabilities live on the same instance" story as `postgres_plugin`.
     * @return `CONGELADO_CAP_EVENTS | CONGELADO_CAP_CACHE`.
     */
    [[nodiscard]] std::uint32_t capabilities() const noexcept override {
        return CONGELADO_CAP_EVENTS | CONGELADO_CAP_CACHE;
    }

    /**
     * @brief Opens a blocking connection to the configured Redis instance.
     * @warning A failed connect doesn't take the process down — it just leaves `m_ctx` null, so
     * `publish()`/`get()`/`set()`/`remove()` all degrade to a warning + empty/failed result
     * instead of a hard failure, same "optional infra degrades gracefully" story as
     * `postgres_plugin`.
     * @param host unused — this plugin doesn't read any host callback fields.
     * @param cfg this plugin's config view; reads `host` (default `localhost`), `port` (default
     * `6379`), optional `password`, optional `db` (index, default unset/`0`), `channel_prefix`
     * (default `""`) prepended to every published channel name, and `key_prefix` (default `""`)
     * prepended to every cache key.
     */
    void on_load(CongeladoHostCallbacks const & /*host*/,
                CongeladoConfigView const &cfg) override {
        auto host_str = congelado::config_get(cfg, "host").value_or("localhost");
        auto port_str = congelado::config_get(cfg, "port").value_or("6379");
        auto password = congelado::config_get(cfg, "password").value_or("");
        auto db = congelado::config_get(cfg, "db").value_or("");
        m_channel_prefix = congelado::config_get(cfg, "channel_prefix").value_or("");
        m_key_prefix = congelado::config_get(cfg, "key_prefix").value_or("");

        int port = 6379;
        try {
            port = std::stoi(port_str);
        } catch (...) {
        }

        m_ctx = redisConnect(host_str.c_str(), port);
        if (m_ctx == nullptr || m_ctx->err != 0) {
            core::logger::warning("redis", "connect to {}:{} failed: {}", host_str, port,
                                  m_ctx != nullptr ? m_ctx->errstr : "null context");
            core::events::publish(
                "redis.connect_failed",
                {{"host", host_str},
                 {"port", std::to_string(port)},
                 {"error", std::string{m_ctx != nullptr ? m_ctx->errstr : "null context"}}});
            if (m_ctx != nullptr) {
                redisFree(m_ctx);
                m_ctx = nullptr;
            }
            return;
        }
        if (!password.empty()) {
            run_command(std::format("AUTH {}", password));
        }
        if (!db.empty()) {
            run_command(std::format("SELECT {}", db));
        }
        core::logger::debug("redis", "connected to {}:{}", host_str, port);
    }

    /// @brief Closes the connection if one's open — clean teardown, no leaked socket.
    void on_unload() noexcept override {
        if (m_ctx != nullptr) {
            redisFree(m_ctx);
            m_ctx = nullptr;
        }
    }

    /**
     * @brief Capability hook the host calls to get at this plugin's `IEventSink` surface.
     * @return this instance, upcast to `interfaces::IEventSink*`.
     */
    void *event_get() noexcept { return static_cast<interfaces::IEventSink *>(this); }
    /**
     * @brief Capability hook the host calls to get at this plugin's `ICache` surface.
     * @return this instance, upcast to `interfaces::ICache*`.
     */
    void *cache_get() noexcept { return static_cast<interfaces::ICache *>(this); }

    /**
     * @brief Publishes via `PUBLISH channel_prefix+event_name payload_json`.
     * @param event_name the published event's name, appended to `channel_prefix` for the
     * channel name.
     * @param payload_json the event's JSON-encoded payload, sent as the message.
     */
    void publish(std::string_view event_name, std::string_view payload_json) noexcept override {
        if (m_ctx == nullptr) {
            core::logger::warning("redis", "publish skipped, no live connection: {}", event_name);
            return;
        }
        try {
            auto *reply = static_cast<redisReply *>(redisCommand(
                m_ctx, "PUBLISH %s%s %s", m_channel_prefix.c_str(),
                std::string{event_name}.c_str(), std::string{payload_json}.c_str()));
            if (reply == nullptr) {
                core::logger::warning("redis", "publish failed for '{}': {}", event_name,
                                      m_ctx->errstr);
                return;
            }
            if (reply->type == REDIS_REPLY_ERROR) {
                core::logger::warning("redis", "publish error for '{}': {}", event_name,
                                      reply->str != nullptr ? reply->str : "unknown");
            }
            freeReplyObject(reply);
        } catch (...) {
            core::logger::warning("redis", "publish threw for '{}'", event_name);
        }
    }

    /**
     * @brief Identifies this cache backend.
     * @return the fixed string "redis".
     */
    [[nodiscard]] std::string_view backend_name() const noexcept override { return "redis"; }

    /**
     * @brief Looks up `key_prefix+key` via `GET`.
     * @note Fully synchronous under the hood despite the async-shaped signature — `result` fires
     * before this call even returns, same "blocking C client, no real async" story as
     * `postgres_plugin`'s `IDatabase` methods.
     * @param key the cache key to look up (before `key_prefix` gets prepended).
     * @param result gets the stored value, or an empty string on a cache miss, no live
     * connection, or a Redis-side error.
     */
    void get(std::string_view key, shared::QueryReadFn &&result) noexcept override {
        if (m_ctx == nullptr) {
            core::logger::warning("redis", "cache get skipped, no live connection: {}", key);
            core::events::publish("redis.cache.get_skipped", {{"key", std::string{key}}});
            std::move(result)("");
            return;
        }
        try {
            auto full_key = m_key_prefix + std::string{key};
            auto *reply = static_cast<redisReply *>(
                redisCommand(m_ctx, "GET %b", full_key.data(), full_key.size()));
            if (reply == nullptr) {
                core::logger::warning("redis", "cache get failed for '{}': {}", key, m_ctx->errstr);
                core::events::publish("redis.cache.get_failed",
                                      {{"key", std::string{key}}, {"error", m_ctx->errstr}});
                std::move(result)("");
                return;
            }
            if (reply->type == REDIS_REPLY_STRING) {
                std::move(result)(std::string_view{reply->str, static_cast<std::size_t>(reply->len)});
            } else {
                if (reply->type == REDIS_REPLY_ERROR) {
                    core::logger::warning("redis", "cache get error for '{}': {}", key,
                                          reply->str != nullptr ? reply->str : "unknown");
                    core::events::publish("redis.cache.get_error",
                                          {{"key", std::string{key}},
                                           {"error", reply->str != nullptr ? reply->str : "unknown"}});
                }
                std::move(result)("");
            }
            freeReplyObject(reply);
        } catch (...) {
            core::logger::warning("redis", "cache get threw for '{}'", key);
            core::events::publish("redis.cache.get_exception", {{"key", std::string{key}}});
            std::move(result)("");
        }
    }

    /**
     * @brief Writes `value` under `key_prefix+key` via `SET`.
     * @param key the cache key to write to (before `key_prefix` gets prepended).
     * @param value the value getting stashed — sent binary-safe (`%b`), so arbitrary bytes
     * (JSON blobs included) survive intact.
     * @param result gets `"ok"` on a successful `SET`, `""` on failure or with no live
     * connection.
     */
    void set(std::string_view key, std::string_view value,
             shared::QueryReadFn &&result) noexcept override {
        if (m_ctx == nullptr) {
            core::logger::warning("redis", "cache set skipped, no live connection: {}", key);
            core::events::publish("redis.cache.set_skipped", {{"key", std::string{key}}});
            std::move(result)("");
            return;
        }
        try {
            auto full_key = m_key_prefix + std::string{key};
            auto *reply = static_cast<redisReply *>(
                redisCommand(m_ctx, "SET %b %b", full_key.data(), full_key.size(), value.data(),
                            value.size()));
            if (reply == nullptr) {
                core::logger::warning("redis", "cache set failed for '{}': {}", key, m_ctx->errstr);
                core::events::publish("redis.cache.set_failed",
                                      {{"key", std::string{key}}, {"error", m_ctx->errstr}});
                std::move(result)("");
                return;
            }
            bool ok = reply->type == REDIS_REPLY_STATUS;
            if (!ok && reply->type == REDIS_REPLY_ERROR) {
                core::logger::warning("redis", "cache set error for '{}': {}", key,
                                      reply->str != nullptr ? reply->str : "unknown");
                core::events::publish("redis.cache.set_error",
                                      {{"key", std::string{key}},
                                       {"error", reply->str != nullptr ? reply->str : "unknown"}});
            }
            freeReplyObject(reply);
            std::move(result)(ok ? "ok" : "");
        } catch (...) {
            core::logger::warning("redis", "cache set threw for '{}'", key);
            core::events::publish("redis.cache.set_exception", {{"key", std::string{key}}});
            std::move(result)("");
        }
    }

    /**
     * @brief Removes `key_prefix+key` via `DEL`.
     * @param key the cache key to remove (before `key_prefix` gets prepended).
     * @param result gets `"ok"` whether or not the key actually existed — `DEL` succeeding on a
     * miss is still a successful removal, same "no error, no cap" story as `LocalCache::remove`;
     * `""` only on a Redis-side error or with no live connection.
     */
    void remove(std::string_view key, shared::QueryReadFn &&result) noexcept override {
        if (m_ctx == nullptr) {
            core::logger::warning("redis", "cache remove skipped, no live connection: {}", key);
            core::events::publish("redis.cache.remove_skipped", {{"key", std::string{key}}});
            std::move(result)("");
            return;
        }
        try {
            auto full_key = m_key_prefix + std::string{key};
            auto *reply = static_cast<redisReply *>(
                redisCommand(m_ctx, "DEL %b", full_key.data(), full_key.size()));
            if (reply == nullptr) {
                core::logger::warning("redis", "cache remove failed for '{}': {}", key,
                                      m_ctx->errstr);
                core::events::publish("redis.cache.remove_failed",
                                      {{"key", std::string{key}}, {"error", m_ctx->errstr}});
                std::move(result)("");
                return;
            }
            bool ok = reply->type == REDIS_REPLY_INTEGER || reply->type == REDIS_REPLY_STATUS;
            if (!ok && reply->type == REDIS_REPLY_ERROR) {
                core::logger::warning("redis", "cache remove error for '{}': {}", key,
                                      reply->str != nullptr ? reply->str : "unknown");
                core::events::publish("redis.cache.remove_error",
                                      {{"key", std::string{key}},
                                       {"error", reply->str != nullptr ? reply->str : "unknown"}});
            }
            freeReplyObject(reply);
            std::move(result)(ok ? "ok" : "");
        } catch (...) {
            core::logger::warning("redis", "cache remove threw for '{}'", key);
            core::events::publish("redis.cache.remove_exception", {{"key", std::string{key}}});
            std::move(result)("");
        }
    }

  private:
    redisContext *m_ctx{nullptr};
    std::string m_channel_prefix;
    std::string m_key_prefix;

    void run_command(std::string const &command) noexcept {
        auto *reply = static_cast<redisReply *>(redisCommand(m_ctx, command.c_str()));
        if (reply != nullptr) {
            freeReplyObject(reply);
        }
    }
};

CONGELADO_PLUGIN(RedisEventsPlugin);

#ifdef CONGELADO_TEST
namespace redis_events_plugin_tests {
using namespace boost::ut;

/// @brief Small test-only helper class — keeps the "class-only, no free functions" convention
/// even for test scaffolding.
class RedisEventsTestHelper {
  public:
    RedisEventsTestHelper() = delete;

    /// @brief Runs `fn` synchronously against an `ICache`-shaped op that reports through a
    /// `shared::QueryReadFn` callback, and hands back whatever the callback got — every one of
    /// this plugin's cache ops fires its callback before returning (blocking hiredis calls, or
    /// the immediate "no live connection" degrade path), so there's no real async to wait on.
    [[nodiscard]] static std::string run_sync(auto &&fn) {
        std::string captured;
        fn([&](std::string_view value) { captured = std::string{value}; });
        return captured;
    }
};

// NOTE on coverage gaps, both deliberate:
// - on_load() is untested here: the only way to exercise its success path is a real TCP connect
//   to a live Redis instance (redisConnect), and even its failure path still issues a real
//   blocking connect attempt (DNS + TCP) before degrading — not hermetic, not something this
//   suite reaches for. Instead, every test below constructs a `RedisEventsPlugin` and never
//   calls on_load(), which leaves `m_ctx` at its default-constructed `nullptr` — exactly the
//   post-"failed connect" state on_load() itself falls back to — so the graceful-degrade branch
//   of publish()/get()/set()/remove() below IS exercised, just via the no-on_load path rather
//   than a live failed connect.
// - run_command() is private and only ever reached from on_load()'s AUTH/SELECT calls, which
//   require a live connection to test meaningfully; skipped for the same reason as on_load().
suite<"RedisEventsPlugin"> redis_events_plugin_suite = [] {
    "get_name reports 'redis'"_test = [] {
        RedisEventsPlugin plugin;
        expect(plugin.get_name() == "redis");
    };

    "get_version reports a non-empty version string"_test = [] {
        RedisEventsPlugin plugin;
        expect(plugin.get_version() == "0.1.0");
    };

    "capabilities reports both EVENTS and CACHE bits"_test = [] {
        RedisEventsPlugin plugin;
        expect(plugin.capabilities() == (CONGELADO_CAP_EVENTS | CONGELADO_CAP_CACHE));
    };

    "event_get returns this instance upcast to IEventSink*"_test = [] {
        RedisEventsPlugin plugin;
        expect(plugin.event_get() == static_cast<interfaces::IEventSink *>(&plugin));
    };

    "cache_get returns this instance upcast to ICache*"_test = [] {
        RedisEventsPlugin plugin;
        expect(plugin.cache_get() == static_cast<interfaces::ICache *>(&plugin));
    };

    "backend_name reports 'redis'"_test = [] {
        RedisEventsPlugin plugin;
        expect(plugin.backend_name() == "redis");
    };

    "publish with no live connection is a safe no-op"_test = [] {
        RedisEventsPlugin plugin;
        plugin.publish("some.event", R"({"payload":true})");
        // No live m_ctx (on_load never ran) — publish() must early-return without touching
        // hiredis at all. Reaching this line without crashing/hanging is the whole assertion.
        expect(true);
    };

    "get with no live connection reports an empty string"_test = [] {
        RedisEventsPlugin plugin;
        auto result =
            RedisEventsTestHelper::run_sync([&](shared::QueryReadFn &&cb) { plugin.get("key", std::move(cb)); });
        expect(result.empty());
    };

    "set with no live connection reports an empty string (write failed)"_test = [] {
        RedisEventsPlugin plugin;
        auto result = RedisEventsTestHelper::run_sync(
            [&](shared::QueryReadFn &&cb) { plugin.set("key", "value", std::move(cb)); });
        expect(result.empty());
    };

    "remove with no live connection reports an empty string"_test = [] {
        RedisEventsPlugin plugin;
        auto result =
            RedisEventsTestHelper::run_sync([&](shared::QueryReadFn &&cb) { plugin.remove("key", std::move(cb)); });
        expect(result.empty());
    };

    "on_unload with no live connection is a safe no-op"_test = [] {
        RedisEventsPlugin plugin;
        plugin.on_unload();
        // m_ctx is nullptr (on_load never ran) — on_unload()'s null-guard means redisFree()
        // never gets called. Reaching this line without crashing is the whole assertion.
        expect(true);
    };
};

} // namespace redis_events_plugin_tests
#endif
