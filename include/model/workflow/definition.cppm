export module model:workflow_def;

import std;
import :workflow_dag;
import :policies;
import serde;

export namespace model {

class WorkflowDef {
  public:
    WorkflowDef() = default;

    void add_node(TaskNode node) { m_nodes.push_back(std::move(node)); }
    void add_input_param(std::string param) { m_input_params.push_back(std::move(param)); }
    void add_output_mapping(OutputMapping mapping) {
        m_output_mappings.push_back(std::move(mapping));
    }

    void set_name(std::string name) { m_name = std::move(name); }
    void set_version(std::uint32_t version) noexcept { m_version = version; }
    void set_nodes(std::vector<TaskNode> nodes) { m_nodes = std::move(nodes); }
    void set_input_params(std::vector<std::string> params) { m_input_params = std::move(params); }
    void set_output_mappings(std::vector<OutputMapping> mappings) {
        m_output_mappings = std::move(mappings);
    }
    void set_failure_workflow(std::optional<std::string> failure_workflow) {
        m_failure_workflow = std::move(failure_workflow);
    }
    void set_timeout(std::optional<TimeoutPolicy> timeout) noexcept { m_timeout = timeout; }

    [[nodiscard]] std::uint32_t get_version() const noexcept { return m_version; }
    [[nodiscard]] const std::string &get_name() const noexcept { return m_name; }
    [[nodiscard]] const std::vector<TaskNode> &get_nodes() const noexcept { return m_nodes; }
    [[nodiscard]] const std::vector<std::string> &get_input_params() const noexcept {
        return m_input_params;
    }
    [[nodiscard]] const std::vector<OutputMapping> &get_output_mappings() const noexcept {
        return m_output_mappings;
    }
    [[nodiscard]] const std::optional<std::string> &get_failure_workflow() const noexcept {
        return m_failure_workflow;
    }
    [[nodiscard]] const std::optional<TimeoutPolicy> &get_timeout() const noexcept {
        return m_timeout;
    }

    [[nodiscard]] std::expected<void, std::string> validate() const noexcept {
        if (m_name.empty())
            return std::unexpected{"WorkflowDef name must not be empty"};
        if (m_version == 0)
            return std::unexpected{"WorkflowDef version must be at least 1"};
        if (m_nodes.empty())
            return std::unexpected{"WorkflowDef must have at least one node"};
        for (auto const &node : m_nodes)
            if (auto r = node.validate(); !r)
                return r;
        for (auto const &mapping : m_output_mappings)
            if (auto r = mapping.validate(); !r)
                return r;
        if (m_timeout)
            if (auto r = m_timeout->validate(); !r)
                return r;
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
    static constexpr std::string_view table_name() { return "workflow_definitions"; }
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
