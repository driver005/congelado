export module core_config:types;

import std;

export namespace core::config {

// Config for one plugin section. All TOML keys are stored verbatim in m_fields
// and forwarded to the plugin's on_load() via CongeladoConfigView.
// The plugin itself owns the interpretation of every key — no host-side schema.
class PluginConfig {
  public:
    PluginConfig() = default;

    void set_name(std::string name) { m_name = std::move(name); }
    void set_type(std::string type) { m_type = std::move(type); }
    void add_field(std::string key, std::string value) { m_fields[std::move(key)] = std::move(value); }

    [[nodiscard]] const std::string &get_name() const noexcept { return m_name; }
    [[nodiscard]] const std::string &get_type() const noexcept { return m_type; }
    [[nodiscard]] const std::unordered_map<std::string, std::string> &get_fields() const noexcept { return m_fields; }
    std::unordered_map<std::string, std::string> &get_fields() noexcept { return m_fields; }

  private:
    std::string m_name;
    std::string m_type;
    std::unordered_map<std::string, std::string> m_fields;
};

// Top-level config. Only [plugins.*] sections exist — logger is just another plugin.
class Config {
  public:
    Config() = default;

    void add_plugin(PluginConfig plugin) { m_plugins[plugin.get_name()] = std::move(plugin); }

    [[nodiscard]] const std::unordered_map<std::string, PluginConfig> &get_plugins() const noexcept {
        return m_plugins;
    }
    std::unordered_map<std::string, PluginConfig> &get_plugins() noexcept { return m_plugins; }

  private:
    std::unordered_map<std::string, PluginConfig> m_plugins;
};

} // namespace core::config
