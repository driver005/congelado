export module model:workflow_dag;

import std;
import serde;

export namespace model {

class InputMapping {
  public:
    InputMapping() = default;
    InputMapping(std::string source, std::string target)
        : m_source{std::move(source)}, m_target{std::move(target)} {}

    void set_source(std::string source) { m_source = std::move(source); }
    void set_target(std::string target) { m_target = std::move(target); }

    [[nodiscard]] const std::string &get_source() const noexcept { return m_source; }
    [[nodiscard]] const std::string &get_target() const noexcept { return m_target; }

    [[nodiscard]] std::expected<void, std::string> validate() const noexcept {
        if (m_source.empty())
            return std::unexpected{"InputMapping source must not be empty"};
        if (m_target.empty())
            return std::unexpected{"InputMapping target must not be empty"};
        return {};
    }

  private:
    std::string m_source;
    std::string m_target;
};

class OutputMapping {
  public:
    OutputMapping() = default;
    OutputMapping(std::string source, std::string target)
        : m_source{std::move(source)}, m_target{std::move(target)} {}

    void set_source(std::string source) { m_source = std::move(source); }
    void set_target(std::string target) { m_target = std::move(target); }

    [[nodiscard]] const std::string &get_source() const noexcept { return m_source; }
    [[nodiscard]] const std::string &get_target() const noexcept { return m_target; }

    [[nodiscard]] std::expected<void, std::string> validate() const noexcept {
        if (m_source.empty())
            return std::unexpected{"OutputMapping source must not be empty"};
        if (m_target.empty())
            return std::unexpected{"OutputMapping target must not be empty"};
        return {};
    }

  private:
    std::string m_source;
    std::string m_target;
};

class TaskEdge {
  public:
    TaskEdge() = default;

    void add_mapping(InputMapping mapping) { m_mappings.push_back(std::move(mapping)); }
    void set_to(std::string to) { m_to = std::move(to); }
    void set_from(std::string from) { m_from = std::move(from); }
    void set_condition(std::optional<std::string> cond) { m_condition = std::move(cond); }
    void set_mappings(std::vector<InputMapping> mappings) { m_mappings = std::move(mappings); }

    [[nodiscard]] const std::string &get_from() const noexcept { return m_from; }
    [[nodiscard]] const std::string &get_to() const noexcept { return m_to; }
    [[nodiscard]] const std::vector<InputMapping> &get_mappings() const noexcept {
        return m_mappings;
    }
    [[nodiscard]] const std::optional<std::string> &get_condition() const noexcept {
        return m_condition;
    }

    [[nodiscard]] std::expected<void, std::string> validate() const noexcept {
        if (m_from.empty())
            return std::unexpected{"TaskEdge from must not be empty"};
        if (m_to.empty())
            return std::unexpected{"TaskEdge to must not be empty"};
        for (auto const &m : m_mappings)
            if (auto r = m.validate(); !r)
                return r;
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
    TaskNode() = default;

    void add_edge(TaskEdge edge) { m_edges.push_back(std::move(edge)); }
    void set_task_def_name(std::string def_name) { m_def_name = std::move(def_name); }
    void set_edges(std::vector<TaskEdge> edges) { m_edges = std::move(edges); }

    [[nodiscard]] const std::string &get_def_name() const noexcept { return m_def_name; }
    [[nodiscard]] const std::vector<TaskEdge> &get_edges() const noexcept { return m_edges; }

    [[nodiscard]] std::expected<void, std::string> validate() const noexcept {
        if (m_def_name.empty())
            return std::unexpected{"TaskNode def_name must not be empty"};
        for (auto const &e : m_edges)
            if (auto r = e.validate(); !r)
                return r;
        return {};
    }

  private:
    std::string m_def_name;
    std::vector<TaskEdge> m_edges;
};

} // namespace model

template <>
struct serde::Serializable<model::InputMapping> {
    static constexpr auto fields() {
        return std::tuple{
            serde::field<"source", &model::InputMapping::get_source,
                       &model::InputMapping::set_source>(),
            serde::field<"target", &model::InputMapping::get_target,
                       &model::InputMapping::set_target>(),
        };
    }
};

template <>
struct serde::Serializable<model::OutputMapping> {
    static constexpr auto fields() {
        return std::tuple{
            serde::field<"source", &model::OutputMapping::get_source,
                       &model::OutputMapping::set_source>(),
            serde::field<"target", &model::OutputMapping::get_target,
                       &model::OutputMapping::set_target>(),
        };
    }
};

template <>
struct serde::Serializable<model::TaskEdge> {
    static constexpr auto fields() {
        return std::tuple{
            serde::field<"from", &model::TaskEdge::get_from, &model::TaskEdge::set_from>(),
            serde::field<"to", &model::TaskEdge::get_to, &model::TaskEdge::set_to>(),
            serde::field<"condition", &model::TaskEdge::get_condition,
                       &model::TaskEdge::set_condition>(),
            serde::field<"mappings", &model::TaskEdge::get_mappings,
                       &model::TaskEdge::set_mappings>(),
        };
    }
};

template <>
struct serde::Serializable<model::TaskNode> {
    static constexpr auto fields() {
        return std::tuple{
            serde::field<"task_def_name", &model::TaskNode::get_def_name,
                       &model::TaskNode::set_task_def_name>(),
            serde::field<"edges", &model::TaskNode::get_edges, &model::TaskNode::set_edges>(),
        };
    }
};
