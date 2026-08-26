module;

#include <rfl/json.hpp>

export module events_worker;

import std;
import interfaces;
import core_events;
import serde;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace worker_events {

/// @brief Typed input for the `events` worker's fixed field, parsed from the task's dynamic
/// input value via `serde::Ser::from_value` — see the `Serializable<EventsInput>`
/// specialization below. Every OTHER input key becomes payload and doesn't fit a fixed field
/// list, so `EventsWorker:: execute` still scans the raw input object for those.
class EventsInput
{
public:
    void setEventName(std::string value)
    {
        m_event_name = std::move(value);
    }

    [[nodiscard]] const std::string& getEventName() const noexcept
    {
        return m_event_name;
    }

private:
    std::string m_event_name;
};

} // namespace worker_events

template<>
struct serde::Serializable<worker_events::EventsInput>
{
    static constexpr auto fields()
    {
        using worker_events::EventsInput;
        return std::tuple{
            serde::FieldDesc<
                "event_name", &EventsInput::getEventName, &EventsInput::setEventName>{},
        };
    }
};

export namespace worker_events {

/// @brief The `events` worker — publishes an event to the process event bus, a reusable IWorker
/// (generalizes Conductor's KAFKA_PUBLISH / event-publish system tasks). It links no broker
/// library: it calls `core::events::publish`, which the worker host fans out to every injected
/// `IEventSink` (kafka, rabbitmq, redis, memory — whichever sinks the host wired). This is the
/// "use injected infra" model — the broker/topic live in the sink's config, the worker only
/// supplies the event name + payload. Input: `event_name` (required) + any other keys become
/// the JSON payload. Output: `events_status` ("ok"/"error").
class EventsWorker final : public interfaces::IWorker
{
public:
    [[nodiscard]] std::string_view get_task_type() const noexcept override
    {
        return "events";
    }

    [[nodiscard]] interfaces::WorkerResult execute(const serde::Value& input) override
    {
        auto parsed = serde::Ser::from_value<EventsInput>(input);
        if (!parsed) {
            return std::unexpected{interfaces::WorkerError{parsed.error()}};
        }
        if (parsed->getEventName().empty()) {
            return std::unexpected{interfaces::WorkerError{"missing 'event_name'"}};
        }
        if (core::events::EventBusRegistry::get_active() == nullptr) {
            return std::unexpected{interfaces::WorkerError{"no event sink injected"}};
        }

        // Everything but the control key rides through as the published payload — string values
        // go in raw (matching the old flat-map behavior), anything else as its JSON encoding.
        std::unordered_map<std::string, std::string> payload;
        if (auto object = input.to_object()) {
            for (const auto& [key, value]: *object) {
                if (key != "event_name") {
                    if (auto as_string = value.to_string()) {
                        payload.emplace(key, *as_string);
                    } else {
                        payload.emplace(key, serde::Ser::encode_json(value));
                    }
                }
            }
        }
        core::events::publish(parsed->getEventName(), std::move(payload));
        return interfaces::WorkerOutput{{"events_status", "ok"}};
    }
};

} // namespace worker_events

#ifdef CONGELADO_TEST
namespace worker_events::events_worker_tests {
using namespace boost::ut;

class EventsWorkerFakeSink : public interfaces::IEventSink
{
public:
    [[nodiscard]] std::string_view get_name() const noexcept override
    {
        return "fake";
    }

    void publish(std::string_view event_name, std::string_view payload_json) noexcept override
    {
        m_last_event_name = std::string{event_name};
        m_last_payload_json = std::string{payload_json};
        ++m_publish_count;
    }

    std::string m_last_event_name;
    std::string m_last_payload_json;
    int m_publish_count{0};
};

suite<"EventsInput"> events_input_suite = [] {
    "setEventName/getEventName round-trips"_test = [] {
        EventsInput input;
        input.setEventName("app.started");
        expect(input.getEventName() == "app.started");
    };

    "default-constructed event_name is empty"_test = [] {
        EventsInput input;
        expect(input.getEventName().empty());
    };

    "from_value fails when 'event_name' is omitted"_test = [] {
        auto value = rfl::json::read<rfl::Generic>(R"({})").value();
        auto parsed = serde::Ser::from_value<EventsInput>(value);
        expect(!parsed.has_value()) << fatal;
        expect(parsed.error().contains("event_name")) << parsed.error();
    };

    "from_value succeeds when 'event_name' is present"_test = [] {
        auto value = rfl::json::read<rfl::Generic>(R"({"event_name":"x"})").value();
        auto parsed = serde::Ser::from_value<EventsInput>(value);
        expect(parsed.has_value()) << fatal;
        expect(parsed->getEventName() == "x");
    };
};

suite<"EventsWorker"> events_worker_suite = [] {
    "get_task_type reports 'events'"_test = [] {
        EventsWorker worker;
        expect(worker.get_task_type() == "events");
    };

    "execute fails with an empty 'event_name'"_test = [] {
        EventsWorker worker;
        auto value = rfl::json::read<rfl::Generic>(R"({"event_name":""})").value();
        auto result = worker.execute(value);
        expect(!result.has_value());
    };

    "execute fails when no event sink is injected"_test = [] {
        auto* previous = core::events::EventBusRegistry::get_active();
        core::events::EventBusRegistry::set_active(nullptr);

        EventsWorker worker;
        auto value = rfl::json::read<rfl::Generic>(R"({"event_name":"app.started"})").value();
        auto result = worker.execute(value);
        expect(!result.has_value());

        core::events::EventBusRegistry::set_active(previous);
    };

    "execute publishes the event_name and JSON payload to every registered sink"_test = [] {
        auto* previous = core::events::EventBusRegistry::get_active();
        core::events::EventBusRegistry registry;
        auto sink = std::make_shared<EventsWorkerFakeSink>();
        registry.add_sink(sink);
        core::events::EventBusRegistry::set_active(&registry);

        EventsWorker worker;
        auto value =
            rfl::json::read<rfl::Generic>(R"({"event_name":"user.created","id":"42"})").value();
        auto result = worker.execute(value);

        expect(result.has_value()) << fatal;
        expect(result->at("events_status") == "ok");
        expect(sink->m_publish_count == 1);
        expect(sink->m_last_event_name == "user.created");
        expect(sink->m_last_payload_json == R"({"id":"42"})");

        core::events::EventBusRegistry::set_active(previous);
    };

    "execute excludes 'event_name' from the published payload"_test = [] {
        auto* previous = core::events::EventBusRegistry::get_active();
        core::events::EventBusRegistry registry;
        auto sink = std::make_shared<EventsWorkerFakeSink>();
        registry.add_sink(sink);
        core::events::EventBusRegistry::set_active(&registry);

        EventsWorker worker;
        auto value = rfl::json::read<rfl::Generic>(R"({"event_name":"e"})").value();
        auto result = worker.execute(value);

        expect(result.has_value()) << fatal;
        expect(sink->m_last_payload_json == "{}");

        core::events::EventBusRegistry::set_active(previous);
    };

    "execute JSON-encodes a non-string payload field"_test = [] {
        auto* previous = core::events::EventBusRegistry::get_active();
        core::events::EventBusRegistry registry;
        auto sink = std::make_shared<EventsWorkerFakeSink>();
        registry.add_sink(sink);
        core::events::EventBusRegistry::set_active(&registry);

        EventsWorker worker;
        auto value = rfl::json::read<rfl::Generic>(R"({"event_name":"e","count":5})").value();
        auto result = worker.execute(value);

        expect(result.has_value()) << fatal;
        expect(sink->m_last_payload_json == R"({"count":"5"})");

        core::events::EventBusRegistry::set_active(previous);
    };

    "execute propagates the from_value error when 'event_name' is missing"_test = [] {
        EventsWorker worker;
        auto value = rfl::json::read<rfl::Generic>(R"({})").value();
        auto result = worker.execute(value);
        expect(!result.has_value());
    };
};

} // namespace worker_events::events_worker_tests
#endif
