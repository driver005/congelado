export module core_config:types;

import std;

export namespace core::config {

// Config for one plugin section. All TOML keys are stored verbatim in m_fields
// and forwarded to the plugin's on_load() via CongeladoConfigView.
// The plugin itself owns the interpretation of every key — no host-side schema.
class PluginConfig {
  public:
    /**
     * @brief Defaulted default ctor — empty name/type/fields, ready to get filled in by the
     * loader.
     */
    PluginConfig() = default;

    /**
     * @brief Sets the plugin's name.
     * @param name the plugin name to store.
     */
    void set_name(std::string name) { m_name = std::move(name); }
    /**
     * @brief Sets the plugin's type (the `type` key pulled out of its section, if present).
     * @param type the plugin type to store.
     */
    void set_type(std::string type) { m_type = std::move(type); }
    /**
     * @brief Stashes one raw config key/value pair, verbatim, no interpretation happening here
     * — that's on the plugin's on_load() to figure out later. No cap, this class doesn't know
     * or care what any key means.
     * @param key the config key.
     * @param value the config value, stored as-is (already stringified by the caller).
     */
    void add_field(std::string key, std::string value) { m_fields[std::move(key)] = std::move(value); }

    /**
     * @brief Grabs the plugin's name.
     * @return the plugin name.
     */
    [[nodiscard]] const std::string &get_name() const noexcept { return m_name; }
    /**
     * @brief Grabs the plugin's type.
     * @return the plugin type.
     */
    [[nodiscard]] const std::string &get_type() const noexcept { return m_type; }
    /**
     * @brief Read-only view over every field stashed for this plugin.
     * @return all key/value fields for this plugin section.
     */
    [[nodiscard]] const std::unordered_map<std::string, std::string> &get_fields() const noexcept { return m_fields; }
    /**
     * @brief Mutable overload — same fields, but writable directly, no copy required.
     * @return all key/value fields for this plugin section.
     */
    std::unordered_map<std::string, std::string> &get_fields() noexcept { return m_fields; }

  private:
    std::string m_name;
    std::string m_type;
    std::unordered_map<std::string, std::string> m_fields;
};

// Top-level config. Only [plugins.*] sections exist — logger is just another plugin.
class Config {
  public:
    /**
     * @brief Defaulted default ctor — starts with zero plugins registered.
     */
    Config() = default;

    /**
     * @brief Registers a plugin under its own name, overwriting whatever was already there
     * under that name. Lowkey the only way plugins ever get into this config.
     * @param plugin the plugin config to add, keyed by its get_name().
     */
    void add_plugin(PluginConfig plugin) { m_plugins[plugin.get_name()] = std::move(plugin); }

    /**
     * @brief Read-only view over every registered plugin, keyed by name.
     * @return all plugin configs, name → PluginConfig.
     */
    [[nodiscard]] const std::unordered_map<std::string, PluginConfig> &get_plugins() const noexcept {
        return m_plugins;
    }
    /**
     * @brief Mutable overload — same map, but writable directly, no copy required. Straight up
     * the escape hatch when you need to poke a plugin's config after the fact.
     * @return all plugin configs, name → PluginConfig.
     */
    std::unordered_map<std::string, PluginConfig> &get_plugins() noexcept { return m_plugins; }

  private:
    std::unordered_map<std::string, PluginConfig> m_plugins;
};

} // namespace core::config
