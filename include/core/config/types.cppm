export module core_config:types;

import std;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

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

    /**
     * @brief Registers the chosen provider(s) for one capability, from the top-level
     * `[providers]` table — e.g. `database = "postgres"` or `logger = ["file_logger",
     * "otel_otlp_plugin"]`. Every value is stored as a list regardless of source shape (a bare
     * TOML/JSON string becomes a one-element list) since some capabilities genuinely allow more
     * than one active provider at once (logger fans out to every registered sink) while others
     * only ever use the first entry (database/search pick a single active backend) — the list
     * shape is uniform, the "how many actually get used" policy lives with each capability's own
     * resolution code, not here.
     * @param capability the capability name (e.g. `"database"`, `"search"`, `"logger"`).
     * @param providers the plugin stem name(s) chosen for it, in preference order.
     */
    void add_provider(std::string capability, std::vector<std::string> providers) {
        m_providers[std::move(capability)] = std::move(providers);
    }

    /**
     * @brief Read-only view over every capability's chosen provider list.
     * @return all provider selections, capability name → ordered list of plugin stem names.
     */
    [[nodiscard]] const std::unordered_map<std::string, std::vector<std::string>> &
    get_providers() const noexcept {
        return m_providers;
    }

    /**
     * @brief Sets the process-wide thread count — the optional top-level `threads` key. Drives
     * the app context's contract thread pool size. Absent falls back to
     * `std::thread::hardware_concurrency()` at read time (see App::run).
     * @param threads the worker-thread count to run.
     */
    void set_threads(std::size_t threads) noexcept { m_threads = threads; }

    /**
     * @brief Grabs the configured process-wide thread count, if the top-level `threads` key was
     * set — otherwise empty, in which case callers fall back to
     * `std::thread::hardware_concurrency()`.
     * @return the configured thread count, or `std::nullopt` if unset.
     */
    [[nodiscard]] const std::optional<std::size_t> &get_threads() const noexcept { return m_threads; }

    /**
     * @brief Sets the directory the host-owned global migration runner scans for
     * `<timestamp>_description.sql` files — the optional top-level `migrations_dir` key.
     * @param migrations_dir the directory path to scan, relative or absolute.
     */
    void set_migrations_dir(std::string migrations_dir) noexcept {
        m_migrations_dir = std::move(migrations_dir);
    }

    /**
     * @brief Grabs the configured migrations directory.
     * @return the configured directory, or `"migrations"` if the top-level `migrations_dir` key
     * was never set.
     */
    [[nodiscard]] const std::string &get_migrations_dir() const noexcept { return m_migrations_dir; }

  private:
    std::unordered_map<std::string, PluginConfig> m_plugins;
    std::unordered_map<std::string, std::vector<std::string>> m_providers;
    std::optional<std::size_t> m_threads;
    std::string m_migrations_dir{"migrations"};
};

} // namespace core::config

#ifdef CONGELADO_TEST
namespace core::config::tests {
using namespace boost::ut;

suite<"PluginConfig"> plugin_config_suite = [] {
    "name and type round-trip"_test = [] {
        PluginConfig plugin;
        plugin.set_name("file_logger");
        plugin.set_type("logger");

        expect(plugin.get_name() == "file_logger");
        expect(plugin.get_type() == "logger");
    };

    "fields accumulate by key, last write wins"_test = [] {
        PluginConfig plugin;
        plugin.add_field("path", "/var/log/app.log");
        plugin.add_field("level", "info");
        plugin.add_field("level", "debug");

        expect(plugin.get_fields().size() == 2);
        expect(plugin.get_fields().at("path") == "/var/log/app.log");
        expect(plugin.get_fields().at("level") == "debug");
    };
};

suite<"Config"> config_suite = [] {
    "starts empty"_test = [] {
        Config cfg;

        expect(cfg.get_plugins().empty());
        expect(cfg.get_providers().empty());
        expect(not cfg.get_threads().has_value());
        expect(cfg.get_migrations_dir() == "migrations");
    };

    "add_plugin keys by the plugin's own name"_test = [] {
        Config cfg;
        PluginConfig plugin;
        plugin.set_name("postgres");
        cfg.add_plugin(plugin);

        expect(cfg.get_plugins().contains("postgres"));
        expect(cfg.get_plugins().at("postgres").get_name() == "postgres");
    };

    "add_provider stores an ordered list under its capability"_test = [] {
        Config cfg;
        cfg.add_provider("logger", {"file_logger", "otel_otlp_plugin"});

        expect(cfg.get_providers().at("logger").size() == 2);
        expect(cfg.get_providers().at("logger")[0] == "file_logger");
        expect(cfg.get_providers().at("logger")[1] == "otel_otlp_plugin");
    };

    "threads and migrations_dir setters"_test = [] {
        Config cfg;
        cfg.set_threads(4);
        cfg.set_migrations_dir("db/migrations");

        expect(cfg.get_threads().value() == 4);
        expect(cfg.get_migrations_dir() == "db/migrations");
    };
};

} // namespace core::config::tests
#endif
