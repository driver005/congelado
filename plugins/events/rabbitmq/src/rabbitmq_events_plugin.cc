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
     * @warning A failed connect doesn't take the process down — it just leaves `m_channel_open`
     * false, so `publish()` degrades to a warning instead of a hard failure, same "optional
     * infra degrades gracefully" story as `postgres_plugin`.
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
            core::logger::warning("rabbitmq", "connect to {}:{} failed", host_str, port);
            core::events::publish("rabbitmq.connect_failed",
                                  {{"host", host_str}, {"port", std::to_string(port)}});
            return;
        }
        auto login_reply =
            amqp_login(m_conn, vhost.c_str(), 0, 131072, 0, AMQP_SASL_METHOD_PLAIN, user.c_str(),
                      password.c_str());
        if (login_reply.reply_type != AMQP_RESPONSE_NORMAL) {
            core::logger::warning("rabbitmq", "login to {}:{}{} failed", host_str, port, vhost);
            core::events::publish("rabbitmq.login_failed",
                                  {{"host", host_str}, {"port", std::to_string(port)}, {"vhost", vhost}});
            return;
        }
        amqp_channel_open(m_conn, 1);
        auto channel_reply = amqp_get_rpc_reply(m_conn);
        if (channel_reply.reply_type != AMQP_RESPONSE_NORMAL) {
            core::logger::warning("rabbitmq", "channel open failed");
            core::events::publish("rabbitmq.channel_open_failed");
            return;
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
