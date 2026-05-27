export module model:workflow_dag;

import std;

export namespace model {

class InputMapping {
  public:
    InputMapping(std::string source, std::string target) : m_source{std::move(source)}, m_target{std::move(target)} {}

    void set_source(std::string source) { m_source = std::move(source); }
    void set_target(std::string target) { m_target = std::move(target); }

    [[nodiscard]] const std::string &get_source() const noexcept { return m_source; }
    [[nodiscard]] const std::string &get_target() const noexcept { return m_target; }

  private:
    std::string m_source;
    std::string m_target;
};

class OutputMapping {
  public:
    OutputMapping(std::string source, std::string target) : m_source{std::move(source)}, m_target{std::move(target)} {}

    void set_source(std::string source) { m_source = std::move(source); }
    void set_target(std::string target) { m_target = std::move(target); }

    [[nodiscard]] const std::string &get_source() const noexcept { return m_source; }
    [[nodiscard]] const std::string &get_target() const noexcept { return m_target; }

  private:
    std::string m_source;
    std::string m_target;
};

class TaskEdge {
  public:
    TaskEdge() = default;

    void add_mapping(InputMapping mapping) { m_mappings.push_back(std::move(mapping)); }

    void set_to(std::string too) { m_to = std::move(too); }
    void set_from(std::string from) { m_from = std::move(from); }
    void set_condition(std::optional<std::string> condition) { m_condition = std::move(condition); }
    void set_mappings(std::vector<InputMapping> mappings) { m_mappings = std::move(mappings); }

    [[nodiscard]] const std::string &get_from() const noexcept { return m_from; }
    [[nodiscard]] const std::string &get_to() const noexcept { return m_to; }
    [[nodiscard]] const std::vector<InputMapping> &get_mappings() const noexcept { return m_mappings; }
    [[nodiscard]] const std::optional<std::string> &get_condition() const noexcept { return m_condition; }

  private:
    std::string m_from;
    std::string m_to;
    std::optional<std::string> m_condition;
    std::vector<InputMapping> m_mappings;
};

class TaskNode {
  public:
    TaskNode() = default;

    void add_edge(TaskEdge edge) { m_edges.push_back(std::move(edge)); }

    void set_task_def_name(std::string def_name) { m_def_name = std::move(def_name); }
    void set_edges(std::vector<TaskEdge> edges) { m_edges = std::move(edges); }

    [[nodiscard]] const std::string &get_def_name() const noexcept { return m_def_name; }
    [[nodiscard]] const std::vector<TaskEdge> &get_edges() const noexcept { return m_edges; }

  private:
    std::string m_def_name;
    std::vector<TaskEdge> m_edges;
};

} // namespace model
