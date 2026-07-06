module;

#include <congelado/abi.h>

export module core_heart:worker;

import std;
import core_plugin;
import worker;

export namespace core::heart {

class PluginWorker : public worker::ITaskWorker {
  public:
    PluginWorker(std::string_view type, core::plugin::types::WorkerExecuteFn exec)
        : m_type{type}, m_exec{exec} {}

    [[nodiscard]] std::string_view get_task_type() const noexcept override { return m_type; }

    [[nodiscard]] worker::TaskOutput execute(worker::TaskInput const &input) override {
        auto &data = input.get_data_map();

        std::vector<std::string> keys, values;
        std::vector<const char *> key_ptrs, val_ptrs;
        keys.reserve(data.size());
        values.reserve(data.size());
        key_ptrs.reserve(data.size());
        val_ptrs.reserve(data.size());

        for (auto &[k, v] : data) {
            keys.push_back(k);
            values.push_back(v);
            key_ptrs.push_back(keys.back().c_str());
            val_ptrs.push_back(values.back().c_str());
        }

        CongeladoConfigView in{key_ptrs.data(), val_ptrs.data(), keys.size()};
        CongeladoConfigView out = m_exec(&in);

        worker::TaskOutput result;
        for (std::size_t i = 0; i < out.count; ++i)
            result.set(std::string{out.keys[i]}, std::string{out.values[i]});

        return result;
    }

  private:
    std::string m_type;
    core::plugin::types::WorkerExecuteFn m_exec;
};

} // namespace core::heart
