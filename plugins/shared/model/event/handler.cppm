export module model:event_handler;

import std;
import serde;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace model {

enum class EventActionType : std::uint8_t {
    START_WORKFLOW,
    COMPLETE_TASK,
    FAIL_TASK,
    TERMINATE_WORKFLOW,
    UPDATE_WORKFLOW_VARIABLES,
};

/// @brief One action an EventHandler fires when its event/condition match — a typed dispatch
/// into already-existing Orchestrator machinery (start()/on_task_terminal()/
/// on_execution_terminal()/variable merge), not a new state machine of its own. `payload` is a
/// flat key/value map whose expected keys depend on `type`:
///   START_WORKFLOW:            workflow_name, plus any variables to seed it with
///   COMPLETE_TASK/FAIL_TASK:   exec_id, task_ref, plus any output_data to attach
///   TERMINATE_WORKFLOW:        exec_id, optional status (COMPLETED/FAILED/TIMED_OUT/TERMINATED)
///   UPDATE_WORKFLOW_VARIABLES: exec_id, plus any variables to merge in
class EventAction {
  public:
    EventAction() = default;

    void set_type(EventActionType type) noexcept { m_type = type; }
    void set_payload(std::unordered_map<std::string, std::string> payload) {
        m_payload = std::move(payload);
    }

    [[nodiscard]] EventActionType get_type() const noexcept { return m_type; }
    [[nodiscard]] const std::unordered_map<std::string, std::string> &get_payload() const noexcept {
        return m_payload;
    }

  private:
    EventActionType m_type{EventActionType::START_WORKFLOW};
    std::unordered_map<std::string, std::string> m_payload;
};

/// @brief Subscribes to an internal event name (published via the EVENT task type's `event`
/// input key, e.g. `"order_shipped"`) and fires one or more EventActions when it matches — the
/// "react" counterpart to EVENT's "publish". Phase 5 only implements the in-process dispatch
/// path (no external MQ-backed sinks like SQS/Kafka — see Orchestrator::publish_event()'s docs).
class EventHandler {
  public:
    EventHandler() = default;

    void set_name(std::string name) { m_name = std::move(name); }
    void set_event(std::string event) { m_event = std::move(event); }
    /// @brief Sets the optional Lua boolean expression gating whether this handler fires, with
    /// the published event's payload bound as Lua globals. std::nullopt always fires.
    void set_condition(std::optional<std::string> condition) { m_condition = std::move(condition); }
    void add_action(EventAction action) { m_actions.push_back(std::move(action)); }
    void set_actions(std::vector<EventAction> actions) { m_actions = std::move(actions); }
    void set_active(bool active) noexcept { m_active = active; }

    [[nodiscard]] const std::string &get_name() const noexcept { return m_name; }
    [[nodiscard]] const std::string &get_event() const noexcept { return m_event; }
    [[nodiscard]] const std::optional<std::string> &get_condition() const noexcept {
        return m_condition;
    }
    [[nodiscard]] const std::vector<EventAction> &get_actions() const noexcept { return m_actions; }
    [[nodiscard]] bool get_active() const noexcept { return m_active; }

    /**
     * @brief Checks that name/event are set — no cap, that's the whole check. Doesn't validate
     * that each action's payload actually carries the keys its type needs; a malformed action
     * just quietly no-ops when Orchestrator::publish_event() can't find what it's looking for.
     * @return an empty expected if name/event are non-empty, otherwise an unexpected naming
     * whichever one's blank.
     */
    [[nodiscard]] std::expected<void, std::string> validate() const noexcept {
        if (m_name.empty()) {
            return std::unexpected{"EventHandler name must not be empty"};
        }
        if (m_event.empty()) {
            return std::unexpected{"EventHandler event must not be empty"};
        }
        return {};
    }

  private:
    std::string m_name;
    std::string m_event;
    std::optional<std::string> m_condition;
    std::vector<EventAction> m_actions;
    bool m_active{true};
};

} // namespace model

template <>
struct serde::Serializable<model::EventAction> {
    static constexpr auto fields() {
        return std::tuple{
            serde::FieldDesc<"type", &model::EventAction::get_type, &model::EventAction::set_type>{},
            serde::FieldDesc<"payload", &model::EventAction::get_payload,
                       &model::EventAction::set_payload>{},
        };
    }
};

template <>
struct serde::Serializable<model::EventHandler> {
    static constexpr std::string_view table_name() { return "event_handlers"; }
    static constexpr auto fields() {
        return std::tuple{
            serde::FieldDesc<"name", &model::EventHandler::get_name, &model::EventHandler::set_name,
                         serde::FieldOptions::init().with_db(serde::FieldOptionsDb::init().pk())>{},
            serde::FieldDesc<"event", &model::EventHandler::get_event,
                       &model::EventHandler::set_event>{},
            serde::FieldDesc<"condition", &model::EventHandler::get_condition,
                       &model::EventHandler::set_condition>{},
            serde::FieldDesc<"actions", &model::EventHandler::get_actions,
                       &model::EventHandler::set_actions>{},
            serde::FieldDesc<"active", &model::EventHandler::get_active,
                       &model::EventHandler::set_active>{},
        };
    }
};

#ifdef CONGELADO_TEST
namespace model::tests {
using namespace boost::ut;

suite<"EventAction"> event_action_suite = [] {
    "defaults to START_WORKFLOW and setters round-trip"_test = [] {
        EventAction action;
        expect(action.get_type() == EventActionType::START_WORKFLOW);

        action.set_type(EventActionType::COMPLETE_TASK);
        action.set_payload({{"exec_id", "abc"}});

        expect(action.get_type() == EventActionType::COMPLETE_TASK);
        expect(action.get_payload().at("exec_id") == "abc");
    };
};

suite<"EventHandler"> event_handler_suite = [] {
    "defaults to active, no actions, and fails validation"_test = [] {
        EventHandler handler;

        expect(handler.get_active());
        expect(handler.get_actions().empty());
        expect(not handler.validate().has_value());
    };
    "requires both name and event"_test = [] {
        EventHandler handler;
        handler.set_name("on_order_shipped");
        expect(not handler.validate().has_value());

        handler.set_event("order_shipped");
        expect(bool(handler.validate()));
    };
    "add_action accumulates"_test = [] {
        EventHandler handler;
        handler.add_action(EventAction{});
        handler.add_action(EventAction{});

        expect(handler.get_actions().size() == 2);
    };
};

} // namespace model::tests
#endif
