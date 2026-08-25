module;

#include <simdjson.h>
#include <toml++/toml.hpp>

export module core_config:loader;

import std;
import core_events;
import :types;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

namespace core::config {

// Helper for parse_json() below — pulled out purely to keep parse_json()'s own cognitive
// complexity down (the per-key string/int/bool dispatch was the bulk of it). Not exported,
// only ever used by this translation unit.
class JsonPluginFieldParser {
  public:
    /**
     * @brief Decodes one JSON plugin-section key/value pair and stashes it on `plugin_config`,
     * trying string/int/bool in that order — same three-way dispatch parse_toml() does for
     * TOML values. "type" also gets promoted onto its own dedicated field.
     * @param plugin_config the plugin config being built; mutated in place.
     * @param key the section key.
     * @param value the raw simdjson element to decode.
     */
    static void parse_field(PluginConfig &plugin_config, std::string_view key,
                            simdjson::dom::element value) {
        std::string_view view;
        std::int64_t int_value = 0;
        bool bool_value = false;

        if (value.get_string().get(view) == 0U) {
            if (key == "type") {
                plugin_config.set_type(std::string{view});
            }
            plugin_config.add_field(std::string{key}, std::string{view});
        } else if (value.get_int64().get(int_value) == 0U) {
            plugin_config.add_field(std::string{key}, std::to_string(int_value));
        } else if (value.get_bool().get(bool_value) == 0U) {
            plugin_config.add_field(std::string{key}, bool_value ? "true" : "false");
        }
    }
};

static std::expected<Config, std::string> parse_toml(const std::filesystem::path &path) {
    Config config{};

    // Parse the file first — bail early with a wrapped error if the TOML itself is busted.
    toml::table toml_table;
    try {
        toml_table = toml::parse_file(path.string());
    } catch (const toml::parse_error &e) {
        core::events::publish("config.load.parse_failed",
                              {{"path", path.string()}, {"error", e.what()}});
        return std::unexpected(std::format("TOML parse error in '{}': {}", path.string(), e.what()));
    }

    // Top-level `threads` scalar — the process-wide worker-thread count. Absent falls back to
    // Config's own default (1). Drives the app context's contract thread pool and is injected as
    // the per-plugin `threads` default downstream (see App::load_plugins).
    if (auto threads = toml_table["threads"].value<std::int64_t>(); threads && *threads > 0) {  // FIXME(clang-tidy): unchecked operator[], consider .at()
        config.set_threads(static_cast<std::size_t>(*threads));
    }

    // Top-level `migrations_dir` scalar — where the host-owned global migration runner looks
    // for `.sql` files. Absent falls back to Config's own default ("migrations").
    if (auto migrations_dir = toml_table["migrations_dir"].value<std::string>()) {  // FIXME(clang-tidy): unchecked operator[], consider .at()
        config.set_migrations_dir(*migrations_dir);
    }

    // No [plugins] table at all is fine — just means an empty config, nothing more to do.
    if (auto *plugins = toml_table["plugins"].as_table()) {  // FIXME(clang-tidy): unchecked operator[], consider .at()
        for (auto &[name, section] : *plugins) {
            // Each plugin section has to itself be a table — skip anything that isn't.
            auto *table = section.as_table();
            if (table == nullptr) {
                continue;
            }

            PluginConfig plugin_config;
            plugin_config.set_name(std::string{name.str()});

            // Walk every key in the section and stash it verbatim, string/int/bool alike —
            // "type" gets a little extra love since it's pulled out onto its own field too.
            for (auto &[section_key, section_value] : *table) {
                std::string key{section_key.str()};
                if (auto view = section_value.value<std::string>()) {
                    if (key == "type") {
                        plugin_config.set_type(*view);
                    }
                    plugin_config.add_field(key, *view);
                } else if (auto value = section_value.value<std::int64_t>()) {
                    plugin_config.add_field(key, std::to_string(*value));
                } else if (auto js_bool = section_value.value<bool>()) {
                    plugin_config.add_field(key, *js_bool ? "true" : "false");
                }
            }

            // Section's fully parsed — register it under its own name, W.
            config.get_plugins()[plugin_config.get_name()] = std::move(plugin_config);
        }
    }

    // No [providers] table at all is fine — every capability resolution falls back to its own
    // "first one found" default. Each key's value is either a bare string (`database =
    // "postgres"`) or an array (`logger = ["file_logger", "otel_otlp_plugin"]`) — normalized to
    // a list either way, see Config::add_provider()'s own docs on why.
    if (auto *providers = toml_table["providers"].as_table()) {  // FIXME(clang-tidy): unchecked operator[], consider .at()
        for (auto &[capability, value] : *providers) {
            std::vector<std::string> names;
            if (auto single = value.value<std::string>()) {
                names.push_back(*single);
            } else if (auto *array = value.as_array()) {
                for (auto &&element : *array) {
                    if (auto name = element.value<std::string>()) {
                        names.push_back(*name);
                    }
                }
            }
            if (!names.empty()) {
                config.add_provider(std::string{capability.str()}, std::move(names));
            }
        }
    }

    return config;
}

static std::expected<Config, std::string> parse_json(const std::filesystem::path &path) {
    Config config{};

    // Load + parse the file — simdjson hands back a nonzero error code instead of throwing.
    simdjson::dom::parser parser;
    simdjson::dom::element doc;
    if (auto ec = parser.load(path.string()).get(doc); ec) {
        return std::unexpected(std::format("JSON parse error in '{}': {}", path.string(), simdjson::error_message(ec)));
    }

    // Top-level "threads" scalar — same process-wide worker-thread count as parse_toml() reads;
    // absent falls back to Config's own default (1).
    std::int64_t threads_value = 0;
    if (doc["threads"].get_int64().get(threads_value) == 0U && threads_value > 0) {  // FIXME(clang-tidy): unchecked operator[], consider .at()
        config.set_threads(static_cast<std::size_t>(threads_value));
    }

    // Top-level "migrations_dir" scalar — same host-owned global migration directory as
    // parse_toml() reads; absent falls back to Config's own default ("migrations").
    std::string_view migrations_dir_value;
    if (doc["migrations_dir"].get_string().get(migrations_dir_value) == 0U) {  // FIXME(clang-tidy): unchecked operator[], consider .at()
        config.set_migrations_dir(std::string{migrations_dir_value});
    }

    // No "plugins" key, or it's not actually an object — either way, empty config's the move.
    simdjson::dom::element plugins_element;
    if (doc["plugins"].get(plugins_element) == 0U) {  // FIXME(clang-tidy): unchecked operator[], consider .at()
        simdjson::dom::object plugins_obj;

        if (plugins_element.get(plugins_obj) == 0U) {
            for (auto [pname, psection] : plugins_obj) {
                // Each plugin section has to itself be an object — skip anything that isn't.
                simdjson::dom::object section_obj;
                if (psection.get(section_obj) != 0U) {
                    continue;
                }

                PluginConfig plugin_config;
                plugin_config.set_name(std::string{pname});

                // Walk every key in the section — dispatch pulled into JsonPluginFieldParser
                // above to keep this function's own complexity down.
                for (auto [section_key, section_value] : section_obj) {
                    JsonPluginFieldParser::parse_field(plugin_config, std::string_view{section_key},
                                                       section_value);
                }

                // Section's fully parsed — register it under its own name.
                config.get_plugins()[plugin_config.get_name()] = std::move(plugin_config);
            }
        }
    }

    // Same "providers" table, JSON shape — see parse_toml()'s own comment for the
    // string-or-array normalization reasoning.
    simdjson::dom::element providers_element;
    if (doc["providers"].get(providers_element) == 0U) {  // FIXME(clang-tidy): unchecked operator[], consider .at()
        simdjson::dom::object providers_obj;
        if (providers_element.get(providers_obj) == 0U) {
            for (auto [capability, value] : providers_obj) {
                std::vector<std::string> names;
                std::string_view single;
                simdjson::dom::array array;
                if (value.get_string().get(single) == 0U) {
                    names.emplace_back(single);
                } else if (value.get_array().get(array) == 0U) {
                    for (auto element : array) {
                        std::string_view name;
                        if (element.get_string().get(name) == 0U) {
                            names.emplace_back(name);
                        }
                    }
                }
                if (!names.empty()) {
                    config.add_provider(std::string{capability}, std::move(names));
                }
            }
        }
    }

    return config;
}

} // namespace core::config

export namespace core::config {

[[nodiscard]] std::expected<Config, std::string> load(const std::filesystem::path &path) {
    // No path given — lowkey not an error, it just means an empty/default config.
    if (path.empty()) {
        return Config{};
    }

    // Dispatch on extension to the matching parser.
    auto ext = path.extension().string();
    if (ext == ".toml") {
        return parse_toml(path);
    }
    if (ext == ".json") {
        return parse_json(path);
    }

    return std::unexpected(std::format("unknown config extension '{}': use .toml or .json", ext));
}

} // namespace core::config

#ifdef CONGELADO_TEST
namespace core::config::tests {
using namespace boost::ut;

suite<"load"> load_suite = [] {
    "empty path yields an empty default config"_test = [] {
        auto result = load("");

        expect(result.has_value());
        expect(result->get_plugins().empty());
        expect(result->get_migrations_dir() == "migrations");
    };

    "unknown extension returns an error"_test = [] {
        auto result = load(std::filesystem::path{"config.xyz"});

        expect(not result.has_value());
        expect(result.error().contains("unknown config extension"));
    };

    "valid toml parses threads, plugins, and providers"_test = [] {
        auto path = std::filesystem::temp_directory_path() / "congelado_loader_test.toml";
        {
            std::ofstream out{path};
            out << "threads = 4\n"
                   "migrations_dir = \"db/migrations\"\n"
                   "[plugins.file_logger]\n"
                   "type = \"logger\"\n"
                   "path = \"/var/log/app.log\"\n"
                   "[providers]\n"
                   "database = \"postgres\"\n"
                   "logger = [\"file_logger\", \"otel_otlp_plugin\"]\n";
        }

        auto result = load(path);
        std::filesystem::remove(path);

        expect(result.has_value());
        expect(result->get_threads().value() == 4);
        expect(result->get_migrations_dir() == "db/migrations");
        expect(result->get_plugins().contains("file_logger"));
        expect(result->get_plugins().at("file_logger").get_type() == "logger");
        expect(result->get_plugins().at("file_logger").get_fields().at("path") == "/var/log/app.log");
        expect(result->get_providers().at("database").size() == 1);
        expect(result->get_providers().at("database")[0] == "postgres");
        expect(result->get_providers().at("logger").size() == 2);
    };

    "malformed toml returns a parse error"_test = [] {
        auto path = std::filesystem::temp_directory_path() / "congelado_loader_test_bad.toml";
        {
            std::ofstream out{path};
            out << "this is not [ valid toml\n";
        }

        auto result = load(path);
        std::filesystem::remove(path);

        expect(not result.has_value());
        expect(result.error().contains("TOML parse error"));
    };

    "valid json parses threads, plugins, and providers"_test = [] {
        auto path = std::filesystem::temp_directory_path() / "congelado_loader_test.json";
        {
            std::ofstream out{path};
            out << R"({
                "threads": 2,
                "migrations_dir": "custom_migrations",
                "plugins": {
                    "postgres": {"type": "database", "host": "localhost"}
                },
                "providers": {
                    "database": "postgres",
                    "logger": ["file_logger", "otel_otlp_plugin"]
                }
            })";
        }

        auto result = load(path);
        std::filesystem::remove(path);

        expect(result.has_value());
        expect(result->get_threads().value() == 2);
        expect(result->get_migrations_dir() == "custom_migrations");
        expect(result->get_plugins().contains("postgres"));
        expect(result->get_plugins().at("postgres").get_type() == "database");
        expect(result->get_providers().at("logger").size() == 2);
    };
};

} // namespace core::config::tests
#endif
