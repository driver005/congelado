module;

#include <congelado/abi.h>

export module core_heart:adapters;

import std;
import interfaces;
import core_logger;
import core_plugin;

namespace core::heart {

using core::plugin::PluginRef;

class LoggerAdapter final : public interfaces::ILogger,
                            public std::enable_shared_from_this<LoggerAdapter> {
  public:
    using LoggerWriteFn = void (*)(int, const char *, size_t) noexcept;

    explicit LoggerAdapter(std::string name, LoggerWriteFn write_fn)
        : m_name{std::move(name)}, m_write{write_fn} {}

    [[nodiscard]] std::string_view get_name() const noexcept override { return m_name; }

    void write(interfaces::LogLevel level, std::string_view msg) noexcept override {
        m_write(static_cast<int>(level), msg.data(), msg.size());
    }

    void error(std::string_view msg) noexcept override { m_write(4, msg.data(), msg.size()); }

    void register_logger() {
        auto self = shared_from_this();
        core::logger::LoggerRegistry::register_logger(self);
    }

    static std::shared_ptr<LoggerAdapter> register_from(PluginRef &ref) {
        auto cap_it = ref.m_data.find("congelado_capabilities");
        if (cap_it == ref.m_data.end())
            return nullptr;

        auto caps = std::any_cast<uint32_t>(cap_it->second);
        if (!(caps & 1u)) // CONGELADO_CAP_LOGGER
            return nullptr;

        auto write_it = ref.m_data.find("congelado_logger_write");
        if (write_it == ref.m_data.end())
            return nullptr;

        auto *raw = std::any_cast<void *>(write_it->second);
        auto write_fn = reinterpret_cast<LoggerWriteFn>(raw);

        std::string pname;
        if (auto name_it = ref.m_data.find("congelado_plugin_name"); name_it != ref.m_data.end())
            pname = std::any_cast<const std::string &>(name_it->second);
        else
            pname = "unnamed";

        return std::make_shared<LoggerAdapter>(std::move(pname), write_fn);
    }

  private:
    std::string m_name;
    LoggerWriteFn m_write;
};

} // namespace core::heart
