export module model:workflow_def;

import std;
import :workflow_dag;
import :policies;
import serde;

export namespace model {

class WorkflowDef {
  public:
    /// @brief Default ctor, bet — empty name, version 1, no nodes/params/mappings, no
    /// failure_workflow or timeout configured.
    WorkflowDef() = default;

    /// @brief Appends a node to the DAG — no cap, order in this vector doesn't determine
    /// execution order, the edges do.
    /// @param node the node to add.
    void add_node(TaskNode node) { m_nodes.push_back(std::move(node)); }
    /// @brief Appends a name to the list of expected input parameters.
    /// @param param the input parameter name to add.
    void add_input_param(std::string param) { m_input_params.push_back(std::move(param)); }
    /// @brief Appends a mapping to the list of output mappings.
    /// @param mapping the mapping to add.
    void add_output_mapping(OutputMapping mapping) {
        m_output_mappings.push_back(std::move(mapping));
    }

    /// @brief Sets the workflow definition's name.
    /// @param name the new name — also the primary key when this def gets persisted.
    void set_name(std::string name) { m_name = std::move(name); }
    /// @brief Sets the definition's version number.
    /// @param version the new version.
    void set_version(std::uint32_t version) noexcept { m_version = version; }
    /// @brief Replaces the whole node list wholesale.
    /// @param nodes the new set of DAG nodes.
    void set_nodes(std::vector<TaskNode> nodes) { m_nodes = std::move(nodes); }
    /// @brief Replaces the whole input parameter list wholesale.
    /// @param params the new set of expected input parameter names.
    void set_input_params(std::vector<std::string> params) { m_input_params = std::move(params); }
    /// @brief Replaces the whole output mapping list wholesale.
    /// @param mappings the new set of output mappings.
    void set_output_mappings(std::vector<OutputMapping> mappings) {
        m_output_mappings = std::move(mappings);
    }
    /// @brief Sets the workflow to run on failure, if any — lowkey a manual fallback, not
    /// automatic rollback.
    /// @param failure_workflow the failure workflow's name, or std::nullopt for none.
    void set_failure_workflow(std::optional<std::string> failure_workflow) {
        m_failure_workflow = std::move(failure_workflow);
    }
    /// @brief Sets the overall workflow timeout policy, if any.
    /// @param timeout the new timeout policy, or std::nullopt for no overall timeout.
    void set_timeout(std::optional<TimeoutPolicy> timeout) noexcept { m_timeout = timeout; }

    /// @brief Gets the definition's version number.
    /// @return the configured version.
    [[nodiscard]] std::uint32_t get_version() const noexcept { return m_version; }
    /// @brief Gets the workflow definition's name.
    /// @return the configured name.
    [[nodiscard]] const std::string &get_name() const noexcept { return m_name; }
    /// @brief Gets the DAG's nodes.
    /// @return the configured nodes.
    [[nodiscard]] const std::vector<TaskNode> &get_nodes() const noexcept { return m_nodes; }
    /// @brief Gets the expected input parameter names.
    /// @return the configured input parameters.
    [[nodiscard]] const std::vector<std::string> &get_input_params() const noexcept {
        return m_input_params;
    }
    /// @brief Gets the output mappings.
    /// @return the configured output mappings.
    [[nodiscard]] const std::vector<OutputMapping> &get_output_mappings() const noexcept {
        return m_output_mappings;
    }
    /// @brief Gets the workflow to run on failure, if any.
    /// @return the failure workflow's name, or std::nullopt if none is configured.
    [[nodiscard]] const std::optional<std::string> &get_failure_workflow() const noexcept {
        return m_failure_workflow;
    }
    /// @brief Gets the overall workflow timeout policy, if any.
    /// @return the configured timeout policy, or std::nullopt if unset.
    [[nodiscard]] const std::optional<TimeoutPolicy> &get_timeout() const noexcept {
        return m_timeout;
    }

    /**
     * @brief Checks the def is internally consistent — non-empty name, version at least 1, at
     * least one node, and every nested node/mapping/timeout validates clean.
     * @warning This is NOT a DAG-integrity check. It never confirms a TaskEdge's `from`/`to`
     * actually resolve to def_names present in m_nodes, never checks for cycles, and never
     * confirms failure_workflow names a workflow that exists. A WorkflowDef with edges pointing
     * into the void or a straight-up cyclic graph can validate() clean here — that's lowkey a
     * trap if you're relying on this as the only integrity gate before persisting/running it.
     * @return an empty expected if everything checks out, otherwise an unexpected describing
     * the first thing that's busted.
     */
    [[nodiscard]] std::expected<void, std::string> validate() const noexcept {
        // Name's the persistence PK, same rule as TaskDef — can't be blank.
        if (m_name.empty()) {
            return std::unexpected{"WorkflowDef name must not be empty"};
        }
        // Version starts at 1, not 0 — zero would mean this def was never actually versioned.
        if (m_version == 0) {
            return std::unexpected{"WorkflowDef version must be at least 1"};
        }
        // A DAG with zero nodes doesn't run anything, so that's checked before anything else.
        if (m_nodes.empty()) {
            return std::unexpected{"WorkflowDef must have at least one node"};
        }
        // Sweep every node's own validate() — first busted one short-circuits the whole thing.
        for (auto const &node : m_nodes) {
            if (auto result = node.validate(); !result) {
                return result;
            }
        }
        // Same sweep, but over the output mappings this time.
        for (auto const &mapping : m_output_mappings) {
            if (auto result = mapping.validate(); !result) {
                return result;
            }
        }
        // Overall timeout is optional — only validate it if one's actually configured, bet.
        if (m_timeout) {
            if (auto result = m_timeout->validate(); !result) {
                return result;
            }
        }
        return {};
    }

  private:
    std::string m_name;
    std::uint32_t m_version{1};
    std::vector<TaskNode> m_nodes;
    std::vector<std::string> m_input_params;
    std::vector<OutputMapping> m_output_mappings;
    std::optional<std::string> m_failure_workflow;
    std::optional<TimeoutPolicy> m_timeout;
};

} // namespace model

template <>
struct serde::Serializable<model::WorkflowDef> {
    /// @brief The DB table this def gets persisted to — bet, mirrors TaskDef's table_name()
    /// pattern.
    /// @return the table name, "workflow_definitions".
    static constexpr std::string_view table_name() { return "workflow_definitions"; }
    /**
     * @brief Field-descriptor table wiring WorkflowDef's columns (name, version, nodes, params,
     * mappings, failure_workflow, timeout) to their getters/setters, for serde
     * (de)serialization — name is the PK.
     * @return the tuple of FieldDesc entries serde uses for this type.
     */
    static constexpr auto fields() {
        return std::tuple{
            serde::FieldDesc<"name", &model::WorkflowDef::get_name, &model::WorkflowDef::set_name,
                         serde::FieldOptions::init().with_db(serde::FieldOptionsDb::init().pk())>{},
            serde::FieldDesc<"version", &model::WorkflowDef::get_version,
                       &model::WorkflowDef::set_version>{},
            serde::FieldDesc<"nodes", &model::WorkflowDef::get_nodes, &model::WorkflowDef::set_nodes>{},
            serde::FieldDesc<"input_params", &model::WorkflowDef::get_input_params,
                       &model::WorkflowDef::set_input_params>{},
            serde::FieldDesc<"output_mappings", &model::WorkflowDef::get_output_mappings,
                       &model::WorkflowDef::set_output_mappings>{},
            serde::FieldDesc<"failure_workflow", &model::WorkflowDef::get_failure_workflow,
                       &model::WorkflowDef::set_failure_workflow>{},
            serde::FieldDesc<"timeout", &model::WorkflowDef::get_timeout,
                       &model::WorkflowDef::set_timeout>{},
        };
    }
};
