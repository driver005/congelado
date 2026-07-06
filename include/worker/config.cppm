module;
#include <toml++/toml.hpp>

export module worker:config;

import std;

export namespace worker {

class TaskConfig {
  public:
    void set_name(std::string name) { m_name = std::move(name); }
    void set_worker_type(std::string worker_type) { m_worker_type = std::move(worker_type); }

    [[nodiscard]] std::string_view get_name() const noexcept { return m_name; }
    [[nodiscard]] std::string_view get_worker_type() const noexcept { return m_worker_type; }

  private:
    std::string m_name;
    std::string m_worker_type;
};

class WorkerConfig {
  public:
    // Throws std::runtime_error on parse failure.
    [[nodiscard]] static WorkerConfig from_file(std::string_view path) {
        toml::table tbl;
        try {
            tbl = toml::parse_file(path);
        } catch (toml::parse_error const &ex) {
            throw std::runtime_error(
                std::format("worker: failed to parse '{}': {}", path, ex.what()));
        }

        WorkerConfig cfg;

        if (auto val = tbl["engine_url"].value<std::string>())
            cfg.set_engine_url(std::move(*val));
        if (auto val = tbl["worker_id"].value<std::string>())
            cfg.set_worker_id(std::move(*val));
        if (auto val = tbl["concurrency"].value<std::uint32_t>())
            cfg.set_concurrency(*val);

        if (auto *arr = tbl["tasks"].as_array()) {
            for (auto &elem : *arr) {
                auto *tbl_ptr = elem.as_table();
                if (tbl_ptr == nullptr)
                    continue;
                TaskConfig task_cfg;
                if (auto val = (*tbl_ptr)["name"].value<std::string>())
                    task_cfg.set_name(std::move(*val));
                if (auto val = (*tbl_ptr)["worker_type"].value<std::string>())
                    task_cfg.set_worker_type(std::move(*val));
                cfg.add_task(std::move(task_cfg));
            }
        }

        return cfg;
    }

    void add_task(TaskConfig task) { m_tasks.push_back(std::move(task)); }

    void set_engine_url(std::string url) { m_engine_url = std::move(url); }
    void set_worker_id(std::string worker_id) { m_worker_id = std::move(worker_id); }
    void set_concurrency(std::uint32_t concurrency) { m_concurrency = concurrency; }

    [[nodiscard]] std::string_view get_engine_url() const noexcept { return m_engine_url; }
    [[nodiscard]] std::string_view get_worker_id() const noexcept { return m_worker_id; }
    [[nodiscard]] std::uint32_t get_concurrency() const noexcept { return m_concurrency; }
    [[nodiscard]] std::vector<TaskConfig> const &get_tasks() const noexcept { return m_tasks; }

  private:
    std::string m_engine_url;
    std::string m_worker_id;
    std::uint32_t m_concurrency{0};
    std::vector<TaskConfig> m_tasks;
};

} // namespace worker
