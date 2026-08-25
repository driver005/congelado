export module model:workflow_dag;

import std;
import serde;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace model {

class InputMapping {
  public:
    /// @brief Default ctor — empty source/target, fill them in with the setters or use the
    /// two-arg ctor below.
    InputMapping() = default;
    /**
     * @brief Builds a mapping straight from a source/target pair.
     * @param source the source field path this mapping reads from.
     * @param target the target field path this mapping writes to.
     */
    InputMapping(std::string source, std::string target)
        : m_source{std::move(source)}, m_target{std::move(target)} {}

    /// @brief Sets the source field path.
    /// @param source the new source path.
    void set_source(std::string source) { m_source = std::move(source); }
    /// @brief Sets the target field path.
    /// @param target the new target path.
    void set_target(std::string target) { m_target = std::move(target); }

    /// @brief Gets the source field path.
    /// @return the configured source path.
    [[nodiscard]] const std::string &get_source() const noexcept { return m_source; }
    /// @brief Gets the target field path.
    /// @return the configured target path.
    [[nodiscard]] const std::string &get_target() const noexcept { return m_target; }

    /**
     * @brief Checks that both source and target are actually set — no cap, that's the whole
     * check.
     * @return an empty expected if both paths are non-empty, otherwise an unexpected naming
     * whichever one's blank.
     */
    [[nodiscard]] std::expected<void, std::string> validate() const noexcept {
        // Source has to be set — no data to pull from otherwise.
        if (m_source.empty()) {
            return std::unexpected{"InputMapping source must not be empty"};
        }
        // Target has to be set too, or the mapping's got nowhere to write to.
        if (m_target.empty()) {
            return std::unexpected{"InputMapping target must not be empty"};
        }
        return {};
    }

  private:
    std::string m_source;
    std::string m_target;
};

class OutputMapping {
  public:
    /// @brief Default ctor, bet — empty source/target, fill them in with the setters or use the
    /// two-arg ctor below.
    OutputMapping() = default;
    /**
     * @brief Builds a mapping straight from a source/target pair.
     * @param source the source field path this mapping reads from.
     * @param target the target field path this mapping writes to.
     */
    OutputMapping(std::string source, std::string target)
        : m_source{std::move(source)}, m_target{std::move(target)} {}

    /// @brief Sets the source field path.
    /// @param source the new source path.
    void set_source(std::string source) { m_source = std::move(source); }
    /// @brief Sets the target field path.
    /// @param target the new target path.
    void set_target(std::string target) { m_target = std::move(target); }

    /// @brief Gets the source field path.
    /// @return the configured source path.
    [[nodiscard]] const std::string &get_source() const noexcept { return m_source; }
    /// @brief Gets the target field path.
    /// @return the configured target path.
    [[nodiscard]] const std::string &get_target() const noexcept { return m_target; }

    /**
     * @brief Checks that both source and target are actually set.
     * @return an empty expected if both paths are non-empty, otherwise an unexpected naming
     * whichever one's blank.
     */
    [[nodiscard]] std::expected<void, std::string> validate() const noexcept {
        // Same shape as InputMapping's check — source can't be blank.
        if (m_source.empty()) {
            return std::unexpected{"OutputMapping source must not be empty"};
        }
        // Neither can target, or the mapped value has nowhere to land. W if both are set.
        if (m_target.empty()) {
            return std::unexpected{"OutputMapping target must not be empty"};
        }
        return {};
    }

  private:
    std::string m_source;
    std::string m_target;
};

class TaskEdge {
  public:
    /// @brief Default ctor — empty from/to, no condition, no mappings.
    TaskEdge() = default;

    /// @brief Appends an input mapping to this edge's mapping list — lowkey how data actually
    /// flows from one node's output into the next node's input.
    /// @param mapping the mapping to add.
    void add_mapping(InputMapping mapping) { m_mappings.push_back(std::move(mapping)); }
    /// @brief Sets the ref_name of the node this edge points to.
    /// @param target_name the target node's ref_name.
    void set_to(std::string target_name) { m_to = std::move(target_name); }
    /// @brief Sets the ref_name of the node this edge originates from.
    /// @param from the source node's ref_name.
    void set_from(std::string from) { m_from = std::move(from); }
    /// @brief Sets the optional condition expression gating this edge.
    /// @param cond the condition expression, or std::nullopt for an unconditional edge.
    void set_condition(std::optional<std::string> cond) { m_condition = std::move(cond); }
    /// @brief Replaces the whole mapping list wholesale.
    /// @param mappings the new list of input mappings.
    void set_mappings(std::vector<InputMapping> mappings) { m_mappings = std::move(mappings); }

    /// @brief Gets the ref_name of the node this edge originates from.
    /// @return the source node's ref_name.
    [[nodiscard]] const std::string &get_from() const noexcept { return m_from; }
    /// @brief Gets the ref_name of the node this edge points to.
    /// @return the target node's ref_name.
    [[nodiscard]] const std::string &get_to() const noexcept { return m_to; }
    /// @brief Gets the input mappings applied when this edge is traversed.
    /// @return the configured mappings.
    [[nodiscard]] const std::vector<InputMapping> &get_mappings() const noexcept {
        return m_mappings;
    }
    /// @brief Gets the optional condition expression gating this edge — motion only happens
    /// down this edge when it evaluates truthy.
    /// @return the condition expression, or std::nullopt if the edge is unconditional.
    [[nodiscard]] const std::optional<std::string> &get_condition() const noexcept {
        return m_condition;
    }

    /**
     * @brief Checks that from/to are set and every nested mapping validates clean.
     * @warning No syntax or existence check on the condition expression — a garbage or dangling
     * `m_condition` string sails right through validate(). That's on whatever evaluates it at
     * runtime, not here.
     * @return an empty expected if everything checks out, otherwise an unexpected describing
     * the first thing that's busted.
     */
    [[nodiscard]] std::expected<void, std::string> validate() const noexcept {
        // Edge needs a source node...
        if (m_from.empty()) {
            return std::unexpected{"TaskEdge from must not be empty"};
        }
        // ...and a target node, otherwise it's not really an edge, just a dangling def_name.
        if (m_to.empty()) {
            return std::unexpected{"TaskEdge to must not be empty"};
        }
        // Walk every input mapping on this edge and bail on the first one that's busted.
        for (auto const &mapping : m_mappings) {
            if (auto result = mapping.validate(); !result) {
                return result;
            }
        }
        return {};
    }

  private:
    std::string m_from;
    std::string m_to;
    std::optional<std::string> m_condition;
    std::vector<InputMapping> m_mappings;
};

enum class JoinType : std::uint8_t { ALL, ANY };

class TaskNode {
  public:
    /// @brief Default ctor — empty def_name/ref_name, no edges. Bet you'll want
    /// set_task_def_name()/set_ref_name() right after this.
    TaskNode() = default;

    /// @brief Appends an outgoing edge to this node.
    /// @param edge the edge to add.
    void add_edge(TaskEdge edge) { m_edges.push_back(std::move(edge)); }
    /// @brief Sets the name of the TaskDef this node runs.
    /// @param def_name the task definition name.
    void set_task_def_name(std::string def_name) { m_def_name = std::move(def_name); }
    /// @brief Sets this node's DAG-position reference name — unique within the containing
    /// WorkflowDef, distinct from def_name so the same TaskDef can run at more than one DAG
    /// position (needed for JOIN branches, DO_WHILE loop bodies, FORK_JOIN_DYNAMIC branches).
    /// TaskEdge::from/to and TaskInstance::node_ref key off this, not def_name.
    /// @param ref_name the new reference name.
    void set_ref_name(std::string ref_name) { m_ref_name = std::move(ref_name); }
    /// @brief Replaces the whole outgoing edge list wholesale.
    /// @param edges the new list of edges.
    void set_edges(std::vector<TaskEdge> edges) { m_edges = std::move(edges); }
    /// @brief Sets the ref_names this node waits on before it's eligible to spawn — a JOIN
    /// node's explicit predecessor list, carried separately from the DAG's static edges since
    /// dynamic/fork branches vary at runtime (Conductor's own JOIN works the same way). Empty
    /// means "not a join node" — eligibility falls back to the plain static-edge-predecessor
    /// check every other node uses.
    /// @param join_on the ref_names to wait on.
    void set_join_on(std::vector<std::string> join_on) { m_join_on = std::move(join_on); }
    /// @brief Sets whether this join fires once every join_on ref completes (ALL, plain JOIN)
    /// or once any one of them does (ANY, EXCLUSIVE_JOIN semantics).
    /// @param join_type ALL or ANY.
    void set_join_type(JoinType join_type) noexcept { m_join_type = join_type; }
    /// @brief Sets the ref_names of the nested subgraph this DO_WHILE node loops over — these
    /// must be ordinary nodes elsewhere in the same WorkflowDef's node list (DO_WHILE doesn't own
    /// a separate nested node list, it just re-runs this same set of refs each iteration). Empty
    /// means "not a loop node."
    /// @param loop_body the ref_names forming the loop body.
    void set_loop_body(std::vector<std::string> loop_body) { m_loop_body = std::move(loop_body); }
    /// @brief Sets the Lua boolean expression re-evaluated after each loop_body pass — true
    /// re-runs the loop, false (or unset) finishes the DO_WHILE node.
    /// @param loop_condition the Lua expression source, or std::nullopt to clear it.
    void set_loop_condition(std::optional<std::string> loop_condition) {
        m_loop_condition = std::move(loop_condition);
    }
    /// @brief Sets the workflow variable key holding a FORK_JOIN_DYNAMIC node's runtime branch
    /// list (a JSON array of `{task_ref, task_def_name, input}` objects, materialized into
    /// sibling nodes at spawn time). std::nullopt means "not a dynamic-fork node."
    /// @param key the workflow variable name, or std::nullopt to clear it.
    void set_dynamic_tasks_input_key(std::optional<std::string> key) {
        m_dynamic_tasks_input_key = std::move(key);
    }

    /// @brief Gets the name of the TaskDef this node runs.
    /// @return the task definition name.
    [[nodiscard]] const std::string &get_def_name() const noexcept { return m_def_name; }
    /// @brief Gets this node's DAG-position reference name.
    /// @return the reference name.
    [[nodiscard]] const std::string &get_ref_name() const noexcept { return m_ref_name; }
    /// @brief Gets the node's outgoing edges.
    /// @return the configured edges.
    [[nodiscard]] const std::vector<TaskEdge> &get_edges() const noexcept { return m_edges; }
    /// @brief Gets the ref_names this node waits on before it's eligible to spawn.
    /// @return the join predecessor ref_names, or empty if this isn't a join node.
    [[nodiscard]] const std::vector<std::string> &get_join_on() const noexcept { return m_join_on; }
    /// @brief Gets whether this join needs every join_on ref to complete, or just one.
    /// @return ALL or ANY.
    [[nodiscard]] JoinType get_join_type() const noexcept { return m_join_type; }
    /// @brief Gets the ref_names of this DO_WHILE node's loop body.
    /// @return the loop body ref_names, or empty if this isn't a loop node.
    [[nodiscard]] const std::vector<std::string> &get_loop_body() const noexcept {
        return m_loop_body;
    }
    /// @brief Gets the loop's re-iterate condition.
    /// @return the Lua expression source, or std::nullopt if unset.
    [[nodiscard]] const std::optional<std::string> &get_loop_condition() const noexcept {
        return m_loop_condition;
    }
    /// @brief Gets the workflow variable key holding this node's runtime branch list.
    /// @return the variable name, or std::nullopt if this isn't a dynamic-fork node.
    [[nodiscard]] const std::optional<std::string> &get_dynamic_tasks_input_key() const noexcept {
        return m_dynamic_tasks_input_key;
    }

    /**
     * @brief Checks that def_name/ref_name are set and every outgoing edge validates clean.
     * @warning Doesn't check that each edge's `from` actually matches this node's ref_name, that
     * `to` points at a node that exists anywhere in the containing WorkflowDef, or that ref_name
     * is actually unique among sibling nodes — that cross-referencing is left entirely to the
     * caller (WorkflowDef doesn't do it either, see its own validate() warning). A TaskNode can
     * validate() fine while its edges dangle into nowhere or its ref_name collides with another
     * node's.
     * @return an empty expected if everything checks out, otherwise an unexpected describing
     * the first thing that's busted.
     */
    [[nodiscard]] std::expected<void, std::string> validate() const noexcept {
        // Node needs to know which TaskDef it's actually running.
        if (m_def_name.empty()) {
            return std::unexpected{"TaskNode def_name must not be empty"};
        }
        // And needs its own DAG-position identity — edges/instances key off this, not def_name.
        if (m_ref_name.empty()) {
            return std::unexpected{"TaskNode ref_name must not be empty"};
        }
        // Then check every outgoing edge is internally sound too — first L found wins.
        for (auto const &edge : m_edges) {
            if (auto result = edge.validate(); !result) {
                return result;
            }
        }
        return {};
    }

  private:
    std::string m_def_name;
    std::string m_ref_name;
    std::vector<TaskEdge> m_edges;
    std::vector<std::string> m_join_on;
    JoinType m_join_type{JoinType::ALL};
    std::vector<std::string> m_loop_body;
    std::optional<std::string> m_loop_condition;
    std::optional<std::string> m_dynamic_tasks_input_key;
};

/// @brief One branch of a FORK_JOIN_DYNAMIC node's runtime task list — deserialized straight out
/// of the workflow variable named by TaskNode::get_dynamic_tasks_input_key() (a JSON array of
/// these), never persisted standalone. Orchestrator materializes one TaskNode per spec.
class DynamicTaskSpec {
  public:
    DynamicTaskSpec() = default;

    void set_task_ref(std::string task_ref) { m_task_ref = std::move(task_ref); }
    void set_task_def_name(std::string name) { m_task_def_name = std::move(name); }
    void set_input(std::unordered_map<std::string, std::string> input) {
        m_input = std::move(input);
    }

    [[nodiscard]] const std::string &get_task_ref() const noexcept { return m_task_ref; }
    [[nodiscard]] const std::string &get_task_def_name() const noexcept { return m_task_def_name; }
    [[nodiscard]] const std::unordered_map<std::string, std::string> &get_input() const noexcept {
        return m_input;
    }

  private:
    std::string m_task_ref;
    std::string m_task_def_name;
    std::unordered_map<std::string, std::string> m_input;
};

} // namespace model

template <>
struct serde::Serializable<model::DynamicTaskSpec> {
    static constexpr auto fields() {
        return std::tuple{
            serde::FieldDesc<"task_ref", &model::DynamicTaskSpec::get_task_ref,
                       &model::DynamicTaskSpec::set_task_ref>{},
            serde::FieldDesc<"task_def_name", &model::DynamicTaskSpec::get_task_def_name,
                       &model::DynamicTaskSpec::set_task_def_name>{},
            serde::FieldDesc<"input", &model::DynamicTaskSpec::get_input,
                       &model::DynamicTaskSpec::set_input>{},
        };
    }
};

template <>
struct serde::Serializable<model::InputMapping> {
    /**
     * @brief Field-descriptor table wiring InputMapping's source/target to their
     * getters/setters, for serde (de)serialization.
     * @return the tuple of FieldDesc entries serde uses for this type.
     */
    static constexpr auto fields() {
        return std::tuple{
            serde::FieldDesc<"source", &model::InputMapping::get_source,
                       &model::InputMapping::set_source>{},
            serde::FieldDesc<"target", &model::InputMapping::get_target,
                       &model::InputMapping::set_target>{},
        };
    }
};

template <>
struct serde::Serializable<model::OutputMapping> {
    /**
     * @brief Field-descriptor table wiring OutputMapping's source/target to their
     * getters/setters, for serde (de)serialization.
     * @return the tuple of FieldDesc entries serde uses for this type.
     */
    static constexpr auto fields() {
        return std::tuple{
            serde::FieldDesc<"source", &model::OutputMapping::get_source,
                       &model::OutputMapping::set_source>{},
            serde::FieldDesc<"target", &model::OutputMapping::get_target,
                       &model::OutputMapping::set_target>{},
        };
    }
};

template <>
struct serde::Serializable<model::TaskEdge> {
    /**
     * @brief Field-descriptor table wiring TaskEdge's from/to/condition/mappings to their
     * getters/setters, for serde (de)serialization — no cap, this is the full edge shape.
     * @return the tuple of FieldDesc entries serde uses for this type.
     */
    static constexpr auto fields() {
        return std::tuple{
            serde::FieldDesc<"from", &model::TaskEdge::get_from, &model::TaskEdge::set_from>{},
            serde::FieldDesc<"to", &model::TaskEdge::get_to, &model::TaskEdge::set_to>{},
            serde::FieldDesc<"condition", &model::TaskEdge::get_condition,
                       &model::TaskEdge::set_condition>{},
            serde::FieldDesc<"mappings", &model::TaskEdge::get_mappings,
                       &model::TaskEdge::set_mappings>{},
        };
    }
};

template <>
struct serde::Serializable<model::TaskNode> {
    /**
     * @brief Field-descriptor table wiring TaskNode's task_def_name/ref_name/edges to their
     * getters/setters, for serde (de)serialization — bet, last table in this file.
     * @return the tuple of FieldDesc entries serde uses for this type.
     */
    static constexpr auto fields() {
        return std::tuple{
            serde::FieldDesc<"task_def_name", &model::TaskNode::get_def_name,
                       &model::TaskNode::set_task_def_name>{},
            serde::FieldDesc<"ref_name", &model::TaskNode::get_ref_name,
                       &model::TaskNode::set_ref_name>{},
            serde::FieldDesc<"edges", &model::TaskNode::get_edges, &model::TaskNode::set_edges>{},
            serde::FieldDesc<"join_on", &model::TaskNode::get_join_on,
                       &model::TaskNode::set_join_on>{},
            serde::FieldDesc<"join_type", &model::TaskNode::get_join_type,
                       &model::TaskNode::set_join_type>{},
            serde::FieldDesc<"loop_body", &model::TaskNode::get_loop_body,
                       &model::TaskNode::set_loop_body>{},
            serde::FieldDesc<"loop_condition", &model::TaskNode::get_loop_condition,
                       &model::TaskNode::set_loop_condition>{},
            serde::FieldDesc<"dynamic_tasks_input_key", &model::TaskNode::get_dynamic_tasks_input_key,
                       &model::TaskNode::set_dynamic_tasks_input_key>{},
        };
    }
};

#ifdef CONGELADO_TEST
namespace model::tests {
using namespace boost::ut;

suite<"InputMapping"> input_mapping_suite = [] {
    "two-arg ctor sets source/target"_test = [] {
        InputMapping mapping{"$.order.id", "order_id"};

        expect(mapping.get_source() == "$.order.id");
        expect(mapping.get_target() == "order_id");
        expect(bool(mapping.validate()));
    };
    "default ctor leaves both empty and fails validation"_test = [] {
        InputMapping mapping;
        expect(not mapping.validate().has_value());
    };
    "an empty target still fails validation"_test = [] {
        InputMapping mapping{"$.order.id", ""};
        expect(not mapping.validate().has_value());
    };
};

suite<"OutputMapping"> output_mapping_suite = [] {
    "two-arg ctor sets source/target"_test = [] {
        OutputMapping mapping{"$.result", "output_value"};

        expect(mapping.get_source() == "$.result");
        expect(mapping.get_target() == "output_value");
        expect(bool(mapping.validate()));
    };
    "default ctor leaves both empty and fails validation"_test = [] {
        OutputMapping mapping;
        expect(not mapping.validate().has_value());
    };
};

suite<"TaskEdge"> task_edge_suite = [] {
    "requires both from and to"_test = [] {
        TaskEdge edge;
        expect(not edge.validate().has_value());

        edge.set_from("validate_order");
        expect(not edge.validate().has_value());

        edge.set_to("charge_payment");
        expect(bool(edge.validate()));
    };
    "add_mapping accumulates and propagates a nested validation failure"_test = [] {
        TaskEdge edge;
        edge.set_from("validate_order");
        edge.set_to("charge_payment");
        edge.add_mapping(InputMapping{"$.order.id", "order_id"});
        expect(edge.get_mappings().size() == 1);
        expect(bool(edge.validate()));

        edge.add_mapping(InputMapping{});
        expect(edge.get_mappings().size() == 2);
        expect(not edge.validate().has_value());
    };
};

suite<"TaskNode"> task_node_suite = [] {
    "requires both def_name and ref_name"_test = [] {
        TaskNode node;
        expect(not node.validate().has_value());

        node.set_task_def_name("validate_order");
        expect(not node.validate().has_value());

        node.set_ref_name("validate_order_1");
        expect(bool(node.validate()));
    };
    "add_edge accumulates and propagates a nested validation failure"_test = [] {
        TaskNode node;
        node.set_task_def_name("validate_order");
        node.set_ref_name("validate_order_1");

        TaskEdge good_edge;
        good_edge.set_from("validate_order_1");
        good_edge.set_to("charge_payment_1");
        node.add_edge(good_edge);
        expect(node.get_edges().size() == 1);
        expect(bool(node.validate()));

        node.add_edge(TaskEdge{});
        expect(node.get_edges().size() == 2);
        expect(not node.validate().has_value());
    };
    "join_type defaults to ALL"_test = [] {
        TaskNode node;
        expect(node.get_join_type() == JoinType::ALL);
    };
};

suite<"DynamicTaskSpec"> dynamic_task_spec_suite = [] {
    "setters round-trip through their getters"_test = [] {
        DynamicTaskSpec spec;
        spec.set_task_ref("branch_1");
        spec.set_task_def_name("send_email");
        spec.set_input({{"to", "a@example.com"}});

        expect(spec.get_task_ref() == "branch_1");
        expect(spec.get_task_def_name() == "send_email");
        expect(spec.get_input().at("to") == "a@example.com");
    };
};

} // namespace model::tests
#endif
