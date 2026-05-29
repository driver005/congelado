module;
#include <toml++/toml.hpp>

export module worker:config;

import std;

export namespace worker {

struct TaskConfig {
    std::string name;
    std::string worker_type;
};

class WorkerConfig {
  public:
    std::string              engine_url;
    std::string              worker_id;
    std::uint32_t            concurrency{0}; // 0 = std::thread::hardware_concurrency()
    std::vector<TaskConfig>  tasks;

    // Throws std::runtime_error on parse failure.
    static WorkerConfig from_file(std::string_view path) {
        toml::table tbl;
        try {
            tbl = toml::parse_file(path);
        } catch (toml::parse_error const &e) {
            throw std::runtime_error(
                std::format("worker: failed to parse '{}': {}", path, e.what()));
        }

        WorkerConfig cfg;

        if (auto v = tbl["engine_url"].value<std::string>()) cfg.engine_url = std::move(*v);
        if (auto v = tbl["worker_id"].value<std::string>())  cfg.worker_id  = std::move(*v);
        if (auto v = tbl["concurrency"].value<std::uint32_t>()) cfg.concurrency = *v;

        if (auto *arr = tbl["tasks"].as_array()) {
            for (auto &elem : *arr) {
                auto *t = elem.as_table();
                if (t == nullptr) continue;
                TaskConfig tc;
                if (auto v = (*t)["name"].value<std::string>())        tc.name        = std::move(*v);
                if (auto v = (*t)["worker_type"].value<std::string>())  tc.worker_type = std::move(*v);
                cfg.tasks.push_back(std::move(tc));
            }
        }

        return cfg;
    }
};

} // namespace worker
