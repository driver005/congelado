export module model:workflow_dag;

import std;
import serde;

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
    /// @brief Sets the def_name of the node this edge points to.
    /// @param target_name the target node's def_name.
    void set_to(std::string target_name) { m_to = std::move(target_name); }
    /// @brief Sets the def_name of the node this edge originates from.
    /// @param from the source node's def_name.
    void set_from(std::string from) { m_from = std::move(from); }
    /// @brief Sets the optional condition expression gating this edge.
    /// @param cond the condition expression, or std::nullopt for an unconditional edge.
    void set_condition(std::optional<std::string> cond) { m_condition = std::move(cond); }
    /// @brief Replaces the whole mapping list wholesale.
    /// @param mappings the new list of input mappings.
    void set_mappings(std::vector<InputMapping> mappings) { m_mappings = std::move(mappings); }

    /// @brief Gets the def_name of the node this edge originates from.
    /// @return the source node's def_name.
    [[nodiscard]] const std::string &get_from() const noexcept { return m_from; }
    /// @brief Gets the def_name of the node this edge points to.
    /// @return the target node's def_name.
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

class TaskNode {
  public:
    /// @brief Default ctor — empty def_name, no edges. Bet you'll want set_task_def_name()
    /// right after this.
    TaskNode() = default;

    /// @brief Appends an outgoing edge to this node.
    /// @param edge the edge to add.
    void add_edge(TaskEdge edge) { m_edges.push_back(std::move(edge)); }
    /// @brief Sets the name of the TaskDef this node runs.
    /// @param def_name the task definition name.
    void set_task_def_name(std::string def_name) { m_def_name = std::move(def_name); }
    /// @brief Replaces the whole outgoing edge list wholesale.
    /// @param edges the new list of edges.
    void set_edges(std::vector<TaskEdge> edges) { m_edges = std::move(edges); }

    /// @brief Gets the name of the TaskDef this node runs.
    /// @return the task definition name.
    [[nodiscard]] const std::string &get_def_name() const noexcept { return m_def_name; }
    /// @brief Gets the node's outgoing edges.
    /// @return the configured edges.
    [[nodiscard]] const std::vector<TaskEdge> &get_edges() const noexcept { return m_edges; }

    /**
     * @brief Checks that def_name is set and every outgoing edge validates clean.
     * @warning Doesn't check that each edge's `from` actually matches this node's def_name, or
     * that `to` points at a node that exists anywhere in the containing WorkflowDef — that
     * cross-referencing is left entirely to the caller. A TaskNode can validate() fine while
     * its edges dangle into nowhere.
     * @return an empty expected if everything checks out, otherwise an unexpected describing
     * the first thing that's busted.
     */
    [[nodiscard]] std::expected<void, std::string> validate() const noexcept {
        // Node needs to know which TaskDef it's actually running.
        if (m_def_name.empty()) {
            return std::unexpected{"TaskNode def_name must not be empty"};
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
    std::vector<TaskEdge> m_edges;
};

} // namespace model

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
     * @brief Field-descriptor table wiring TaskNode's task_def_name/edges to their
     * getters/setters, for serde (de)serialization — bet, last table in this file.
     * @return the tuple of FieldDesc entries serde uses for this type.
     */
    static constexpr auto fields() {
        return std::tuple{
            serde::FieldDesc<"task_def_name", &model::TaskNode::get_def_name,
                       &model::TaskNode::set_task_def_name>{},
            serde::FieldDesc<"edges", &model::TaskNode::get_edges, &model::TaskNode::set_edges>{},
        };
    }
};
