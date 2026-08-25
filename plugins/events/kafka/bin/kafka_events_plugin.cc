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
#ifdef CONGELADO_TEST
import boost.ut;
#endif

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
     * @warning A failed producer creation is fatal: reaching the broker is a hard requirement,
     * so this throws rather than degrading to a null `m_producer` — same "don't run without it"
     * contract as `postgres_plugin`. The thrown exception is caught by `CONGELADO_PLUGIN`'s
     * `congelado_init` (returns -1), which fails the host's `build()` and aborts startup. Only a
     * connection lost *after* a successful producer creation still degrades gracefully
     * (`publish()` keeps its existing null-`m_producer` check for that case).
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
            auto error_message = std::format("kafka: conf_set bootstrap.servers failed: {}", errstr);
            core::logger::error("kafka", "{}", error_message);
            core::events::publish("kafka.config_failed", {{"error", std::string{errstr}}});
            rd_kafka_conf_destroy(conf);
            throw std::runtime_error(error_message);
        }

        // rd_kafka_new() takes ownership of conf on success; on failure, conf must still be
        // destroyed by the caller — only free it in the failure branch below.
        m_producer = rd_kafka_new(RD_KAFKA_PRODUCER, conf, errstr, sizeof(errstr));
        if (m_producer == nullptr) {
            auto error_message = std::format("kafka: producer creation failed: {}", errstr);
            core::logger::error("kafka", "{}", error_message);
            core::events::publish("kafka.producer_create_failed", {{"error", std::string{errstr}}});
            rd_kafka_conf_destroy(conf);
            throw std::runtime_error(error_message);
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

#ifdef CONGELADO_TEST
namespace kafka_events_plugin_tests {
using namespace boost::ut;

// NOTE on coverage gaps, both deliberate (same reasoning as RedisEventsPlugin's own test suite):
// - on_load() is untested here: its success path needs a real librdkafka producer connect against
//   a live broker (rd_kafka_new/rd_kafka_conf_set reach out for a real handle), and there's no
//   hermetic seam to fake m_producer non-null without constructing one for real. Every test below
//   constructs a KafkaEventsPlugin and never calls on_load(), leaving m_producer at its
//   default-constructed nullptr — exactly the state this plugin would be in had on_load() thrown
//   and aborted host startup, so the graceful-degrade branch of publish() IS exercised, just via
//   the no-on_load path.
// - The RD_KAFKA_V_TOPIC/RD_KAFKA_V_KEY/RD_KAFKA_V_VALUE message-building in publish() is gated
//   behind the `m_producer == nullptr` early return above it, same as the injection-shaped
//   collection/id URL-building in ElasticsearchPlugin's index()/remove()/search() — not reachable
//   without a live producer, so whether event_name/payload_json get spliced into the Kafka
//   key/value safely can't be observed here either.
suite<"KafkaEventsPlugin"> kafka_events_plugin_suite = [] {
    "get_name reports 'kafka'"_test = [] {
        KafkaEventsPlugin plugin;
        expect(plugin.get_name() == "kafka");
    };

    "get_version reports a non-empty version string"_test = [] {
        KafkaEventsPlugin plugin;
        expect(plugin.get_version() == "0.1.0");
    };

    "capabilities reports CONGELADO_CAP_EVENTS"_test = [] {
        KafkaEventsPlugin plugin;
        expect(plugin.capabilities() == CONGELADO_CAP_EVENTS);
    };

    "event_get returns this instance upcast to IEventSink*"_test = [] {
        KafkaEventsPlugin plugin;
        expect(plugin.event_get() == static_cast<interfaces::IEventSink *>(&plugin));
    };

    "publish with no live producer is a safe no-op"_test = [] {
        KafkaEventsPlugin plugin;
        plugin.publish("some.event", R"({"payload":true})");
        // No live m_producer (on_load never ran) — publish() must early-return without ever
        // touching librdkafka. Reaching this line without crashing/hanging is the whole assertion.
        expect(true);
    };

    // Adversarial: an event_name/payload shaped like it's trying to break out of the Kafka
    // key/value framing (rd_kafka_producev takes key/value as raw byte buffers, not delimited
    // text, so there's nothing to "break out" of even on the live path) — pins that the guard
    // clause swallows this before any of that matters.
    "publish with no live producer tolerates injection-shaped event_name/payload"_test = [] {
        KafkaEventsPlugin plugin;
        expect(nothrow([&] {
            plugin.publish("topic\0hijack", R"({"a":"'; DROP TABLE t; --\n\r\x00"})");
        }));
    };

    "on_unload with no live producer is a safe no-op"_test = [] {
        KafkaEventsPlugin plugin;
        // m_producer is nullptr (on_load never ran) — on_unload()'s null-guard means
        // rd_kafka_flush()/rd_kafka_destroy() never get called.
        expect(nothrow([&] { plugin.on_unload(); }));
    };
};

} // namespace kafka_events_plugin_tests
#endif
