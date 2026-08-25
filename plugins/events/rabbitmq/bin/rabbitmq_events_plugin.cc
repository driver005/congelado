module;

#define CONGELADO_GUEST
#include <congelado/plugin.h>
#include <rabbitmq-c/amqp.h>
#include <rabbitmq-c/tcp_socket.h>

export module rabbitmq_events_plugin;

import congelado_plugin;
import interfaces;
import core_events;
import core_logger;
import std;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

/**
 * @brief `IEventSink` backed by RabbitMQ — same "link a blocking C client library directly,
 * bypass IClient/router" pattern `postgres_plugin` (libpq) and `elasticsearch_plugin` (libcurl)
 * already use. Publishes onto a configured exchange with the event name as routing key.
 */
class RabbitMqEventsPlugin : public congelado::Plugin, public interfaces::IEventSink {
  public:
    [[nodiscard]] std::string_view get_name() const noexcept override { return "rabbitmq"; }
    [[nodiscard]] std::string_view get_version() const noexcept override { return "0.1.0"; }
    [[nodiscard]] std::uint32_t capabilities() const noexcept override {
        return CONGELADO_CAP_EVENTS;
    }

    /**
     * @brief Opens a blocking connection/channel to the configured broker.
     * @warning A failed connect/login/channel-open is fatal: reaching the broker is a hard
     * requirement, so this throws rather than degrading to `m_channel_open == false` — same
     * "don't run without it" contract as `postgres_plugin`. The thrown exception is caught by
     * `CONGELADO_PLUGIN`'s `congelado_init` (returns -1), which fails the host's `build()` and
     * aborts startup. Only a channel lost *after* a successful open still degrades gracefully
     * (`publish()` keeps its existing `m_channel_open` check for that case).
     * @param host unused — this plugin doesn't read any host callback fields.
     * @param cfg this plugin's config view; reads `host` (default `localhost`), `port` (default
     * `5672`), `vhost` (default `/`), `user`/`password` (default `guest`/`guest`), `exchange`
     * (default `""`, the broker's default direct exchange).
     */
    void on_load(CongeladoHostCallbacks const & /*host*/,
                CongeladoConfigView const &cfg) override {
        auto host_str = congelado::config_get(cfg, "host").value_or("localhost");
        auto port_str = congelado::config_get(cfg, "port").value_or("5672");
        auto vhost = congelado::config_get(cfg, "vhost").value_or("/");
        auto user = congelado::config_get(cfg, "user").value_or("guest");
        auto password = congelado::config_get(cfg, "password").value_or("guest");
        m_exchange = congelado::config_get(cfg, "exchange").value_or("");

        int port = 5672;
        try {
            port = std::stoi(port_str);
        } catch (...) {
        }

        m_conn = amqp_new_connection();
        amqp_socket_t *socket = amqp_tcp_socket_new(m_conn);
        if (socket == nullptr || amqp_socket_open(socket, host_str.c_str(), port) != 0) {
            auto error_message = std::format("rabbitmq: connect to {}:{} failed", host_str, port);
            core::logger::error("rabbitmq", "{}", error_message);
            core::events::publish("rabbitmq.connect_failed",
                                  {{"host", host_str}, {"port", std::to_string(port)}});
            throw std::runtime_error(error_message);
        }
        auto login_reply =
            amqp_login(m_conn, vhost.c_str(), 0, 131072, 0, AMQP_SASL_METHOD_PLAIN, user.c_str(),
                      password.c_str());
        if (login_reply.reply_type != AMQP_RESPONSE_NORMAL) {
            auto error_message =
                std::format("rabbitmq: login to {}:{}{} failed", host_str, port, vhost);
            core::logger::error("rabbitmq", "{}", error_message);
            core::events::publish("rabbitmq.login_failed",
                                  {{"host", host_str}, {"port", std::to_string(port)}, {"vhost", vhost}});
            throw std::runtime_error(error_message);
        }
        amqp_channel_open(m_conn, 1);
        auto channel_reply = amqp_get_rpc_reply(m_conn);
        if (channel_reply.reply_type != AMQP_RESPONSE_NORMAL) {
            auto error_message = std::string{"rabbitmq: channel open failed"};
            core::logger::error("rabbitmq", "{}", error_message);
            core::events::publish("rabbitmq.channel_open_failed");
            throw std::runtime_error(error_message);
        }
        m_channel_open = true;
        core::logger::debug("rabbitmq", "connected to {}:{}{}, exchange='{}'", host_str, port,
                            vhost, m_exchange);
    }

    /// @brief Closes the channel/connection if one's open — clean teardown, no leaked socket.
    void on_unload() noexcept override {
        if (m_channel_open) {
            amqp_channel_close(m_conn, 1, AMQP_REPLY_SUCCESS);
            amqp_connection_close(m_conn, AMQP_REPLY_SUCCESS);
        }
        if (m_conn != nullptr) {
            amqp_destroy_connection(m_conn);
            m_conn = nullptr;
        }
    }

    /**
     * @brief Capability hook the host calls to get at this plugin's `IEventSink` surface.
     * @return this instance, upcast to `interfaces::IEventSink*`.
     */
    void *event_get() noexcept { return static_cast<interfaces::IEventSink *>(this); }

    /**
     * @brief Publishes via `amqp_basic_publish`, routing key = `event_name`.
     * @param event_name the published event's name, used as the routing key.
     * @param payload_json the event's JSON-encoded payload, sent as the message body.
     */
    void publish(std::string_view event_name, std::string_view payload_json) noexcept override {
        if (!m_channel_open) {
            core::logger::warning("rabbitmq", "publish skipped, no live channel: {}", event_name);
            return;
        }
        try {
            std::string routing_key{event_name};
            std::string body{payload_json};
            auto status = amqp_basic_publish(
                m_conn, 1, amqp_cstring_bytes(m_exchange.c_str()),
                amqp_cstring_bytes(routing_key.c_str()), 0, 0, nullptr,
                amqp_cstring_bytes(body.c_str()));
            if (status != AMQP_STATUS_OK) {
                core::logger::warning("rabbitmq", "publish failed for '{}': status={}", event_name,
                                      static_cast<int>(status));
            }
        } catch (...) {
            core::logger::warning("rabbitmq", "publish threw for '{}'", event_name);
        }
    }

  private:
    amqp_connection_state_t m_conn{nullptr};
    bool m_channel_open{false};
    std::string m_exchange;
};

CONGELADO_PLUGIN(RabbitMqEventsPlugin);

#ifdef CONGELADO_TEST
namespace rabbitmq_events_plugin_tests {
using namespace boost::ut;

// NOTE on coverage gaps, both deliberate (same reasoning as RedisEventsPlugin's/KafkaEventsPlugin's
// own test suites):
// - on_load() is untested here: its success path needs a real rabbitmq-c TCP connect/login/channel
//   open against a live broker, with no hermetic seam to fake m_channel_open true without doing
//   one for real. Every test below constructs a RabbitMqEventsPlugin and never calls on_load(),
//   leaving m_channel_open at its default-constructed false — exactly the state this plugin would
//   be in had on_load() thrown and aborted host startup, so the graceful-degrade branch of
//   publish() IS exercised, just via the no-on_load path.
// - The amqp_basic_publish() routing-key/body construction in publish() is gated behind the
//   `!m_channel_open` early return above it — not reachable without a live channel, so whether
//   event_name/payload_json get spliced into the AMQP frame safely can't be observed here either.
suite<"RabbitMqEventsPlugin"> rabbitmq_events_plugin_suite = [] {
    "get_name reports 'rabbitmq'"_test = [] {
        RabbitMqEventsPlugin plugin;
        expect(plugin.get_name() == "rabbitmq");
    };

    "get_version reports a non-empty version string"_test = [] {
        RabbitMqEventsPlugin plugin;
        expect(plugin.get_version() == "0.1.0");
    };

    "capabilities reports CONGELADO_CAP_EVENTS"_test = [] {
        RabbitMqEventsPlugin plugin;
        expect(plugin.capabilities() == CONGELADO_CAP_EVENTS);
    };

    "event_get returns this instance upcast to IEventSink*"_test = [] {
        RabbitMqEventsPlugin plugin;
        expect(plugin.event_get() == static_cast<interfaces::IEventSink *>(&plugin));
    };

    "publish with no live channel is a safe no-op"_test = [] {
        RabbitMqEventsPlugin plugin;
        plugin.publish("some.event", R"({"payload":true})");
        // No live channel (on_load never ran) — publish() must early-return without ever
        // touching rabbitmq-c. Reaching this line without crashing/hanging is the whole assertion.
        expect(true);
    };

    // Adversarial: a routing key/body shaped like it's trying to break AMQP framing — pins that
    // the guard clause swallows this before any of that matters on the no-connection path.
    "publish with no live channel tolerates injection-shaped event_name/payload"_test = [] {
        RabbitMqEventsPlugin plugin;
        expect(nothrow([&] {
            plugin.publish("routing.key\r\nX-Injected: 1", R"({"a":"'; DROP TABLE t; --"})");
        }));
    };

    "on_unload with no live connection is a safe no-op"_test = [] {
        RabbitMqEventsPlugin plugin;
        // m_conn is nullptr and m_channel_open is false (on_load never ran) — on_unload()'s
        // guards mean amqp_channel_close()/amqp_connection_close()/amqp_destroy_connection()
        // never get called.
        expect(nothrow([&] { plugin.on_unload(); }));
    };
};

} // namespace rabbitmq_events_plugin_tests
#endif
