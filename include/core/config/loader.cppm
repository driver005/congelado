module;

#include <simdjson.h>
#include <toml++/toml.hpp>

export module core_config:loader;

import std;
import :types;

namespace core::config {

static std::expected<Config, std::string> parse_toml(const std::filesystem::path &path) {
    Config config{};

    toml::table toml_table;
    try {
        toml_table = toml::parse_file(path.string());
    } catch (const toml::parse_error &e) {
        return std::unexpected(std::format("TOML parse error in '{}': {}", path.string(), e.what()));
    }

    if (auto *plugins = toml_table["plugins"].as_table()) {
        for (auto &[name, section] : *plugins) {
            auto *table = section.as_table();
            if (table == nullptr) {
                continue;
            }

            PluginConfig plugin_config;
            plugin_config.set_name(std::string{name.str()});

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

            config.get_plugins()[plugin_config.get_name()] = std::move(plugin_config);
        }
    }

    return config;
}

static std::expected<Config, std::string> parse_json(const std::filesystem::path &path) {
    Config config{};

    simdjson::dom::parser parser;
    simdjson::dom::element doc;
    if (auto ec = parser.load(path.string()).get(doc); ec) {
        return std::unexpected(std::format("JSON parse error in '{}': {}", path.string(), simdjson::error_message(ec)));
    }

    simdjson::dom::element plugins_element;
    if (doc["plugins"].get(plugins_element) == 0U) {
        simdjson::dom::object plugins_obj;

        if (plugins_element.get(plugins_obj) == 0U) {
            for (auto [pname, psection] : plugins_obj) {
                simdjson::dom::object section_obj;
                if (psection.get(section_obj) != 0U) {
                    continue;
                }

                PluginConfig plugin_config;
                plugin_config.set_name(std::string{pname});

                for (auto [section_key, section_value] : section_obj) {
                    std::string key{section_key};
                    std::string_view view;
                    std::int64_t value = 0;
                    bool js_bool = false;

                    if (section_value.get_string().get(view) == 0U) {
                        if (key == "type") {
                            plugin_config.set_type(std::string{view});
                        }
                        plugin_config.add_field(key, std::string{view});
                    } else if (section_value.get_int64().get(value) == 0U) {
                        plugin_config.add_field(key, std::to_string(value));
                    } else if (section_value.get_bool().get(js_bool) == 0U) {
                        plugin_config.add_field(key, js_bool ? "true" : "false");
                    }
                }

                config.get_plugins()[plugin_config.get_name()] = std::move(plugin_config);
            }
        }
    }

    return config;
}

} // namespace core::config

export namespace core::config {

[[nodiscard]] std::expected<Config, std::string> load(const std::filesystem::path &path) {
    if (path.empty()) {
        return Config{};
    }

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
