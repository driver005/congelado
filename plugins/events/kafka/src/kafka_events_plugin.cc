module;

#define CONGELADO_GUEST
#include <congelado/plugin.h>
#include <librdkafka/rdkafka.h>

export module kafka_events_plugin;

import congelado_plugin;
import interfaces;
import core_events;
import core_logger;
import std;

/**
 * @brief `IEventSink` backed by Kafka via librdkafka — same "link a blocking C client library
 * directly, bypass IClient/router" pattern `postgres_plugin` (libpq) and `elasticsearch_plugin`
 * (libcurl) already use. Publishes onto one fixed topic, `event_name` as the message key — a
 * single topic + key, not per-event-name topics, avoids runtime topic auto-creation surprises
 * (documented simplification, not a silent gap).
 */
class KafkaEventsPlugin : public congelado::Plugin, public interfaces::IEventSink {
  public:
    [[nodiscard]] std::string_view get_name() const noexcept override { return "kafka"; }
    [[nodiscard]] std::string_view get_version() const noexcept override { return "0.1.0"; }
    [[nodiscard]] std::uint32_t capabilities() const noexcept override {
        return CONGELADO_CAP_EVENTS;
    }

    /**
     * @brief Builds a librdkafka producer handle against the configured broker list.
     * @warning A failed producer creation doesn't take the process down — it just leaves
     * `m_producer` null, so `publish()` degrades to a warning instead of a hard failure, same
     * "optional infra degrades gracefully" story as `postgres_plugin`.
     * @param host unused — this plugin doesn't read any host callback fields.
     * @param cfg this plugin's config view; reads `bootstrap_servers` (default `localhost:9092`)
     * and `topic` (default `congelado-events`).
     */
    void on_load(CongeladoHostCallbacks const & /*host*/,
                CongeladoConfigView const &cfg) override {
        m_topic = congelado::config_get(cfg, "topic").value_or("congelado-events");
        auto brokers = congelado::config_get(cfg, "bootstrap_servers").value_or("localhost:9092");

        char errstr[512];
        rd_kafka_conf_t *conf = rd_kafka_conf_new();
        if (rd_kafka_conf_set(conf, "bootstrap.servers", brokers.c_str(), errstr,
                              sizeof(errstr)) != RD_KAFKA_CONF_OK) {
            core::logger::warning("kafka", "conf_set bootstrap.servers failed: {}", errstr);
            core::events::publish("kafka.config_failed", {{"error", std::string{errstr}}});
            rd_kafka_conf_destroy(conf);
            return;
        }

        // rd_kafka_new() takes ownership of conf on success; on failure, conf must still be
        // destroyed by the caller — only free it in the failure branch below.
        m_producer = rd_kafka_new(RD_KAFKA_PRODUCER, conf, errstr, sizeof(errstr));
        if (m_producer == nullptr) {
            core::logger::warning("kafka", "producer creation failed: {}", errstr);
            core::events::publish("kafka.producer_create_failed", {{"error", std::string{errstr}}});
            rd_kafka_conf_destroy(conf);
            return;
        }
        core::logger::debug("kafka", "producer ready, brokers='{}', topic='{}'", brokers, m_topic);
    }

    /// @brief Flushes any in-flight messages (bounded timeout) then destroys the producer handle.
    void on_unload() noexcept override {
        if (m_producer != nullptr) {
            rd_kafka_flush(m_producer, 5000);
            rd_kafka_destroy(m_producer);
            m_producer = nullptr;
        }
    }

    /**
     * @brief Capability hook the host calls to get at this plugin's `IEventSink` surface.
     * @return this instance, upcast to `interfaces::IEventSink*`.
     */
    void *event_get() noexcept { return static_cast<interfaces::IEventSink *>(this); }

    /**
     * @brief Publishes via `rd_kafka_producev` onto the fixed configured topic.
     * @param event_name the published event's name, sent as the Kafka message key.
     * @param payload_json the event's JSON-encoded payload, sent as the message value.
     */
    void publish(std::string_view event_name, std::string_view payload_json) noexcept override {
        if (m_producer == nullptr) {
            core::logger::warning("kafka", "publish skipped, no live producer: {}", event_name);
            return;
        }
        try {
            std::string key{event_name};
            std::string value{payload_json};
            auto err = rd_kafka_producev(
                m_producer, RD_KAFKA_V_TOPIC(m_topic.c_str()),
                RD_KAFKA_V_KEY(key.data(), key.size()),
                RD_KAFKA_V_VALUE(value.data(), value.size()),
                RD_KAFKA_V_MSGFLAGS(RD_KAFKA_MSG_F_COPY), RD_KAFKA_V_END);
            if (err != RD_KAFKA_RESP_ERR_NO_ERROR) {
                core::logger::warning("kafka", "publish failed for '{}': {}", event_name,
                                      rd_kafka_err2str(err));
                return;
            }
            // Non-blocking poll to serve delivery-report callbacks — bounded, never stalls the
            // caller waiting on a broker round trip.
            rd_kafka_poll(m_producer, 0);
        } catch (...) {
            core::logger::warning("kafka", "publish threw for '{}'", event_name);
        }
    }

  private:
    rd_kafka_t *m_producer{nullptr};
    std::string m_topic;
};

CONGELADO_PLUGIN(KafkaEventsPlugin);
