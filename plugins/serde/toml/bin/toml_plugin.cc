module;

#define CONGELADO_GUEST
#include <congelado/plugin.h>
#include <rfl/Generic.hpp>
#include <rfl/toml.hpp>

export module toml_plugin;

import congelado_plugin;
import interfaces;
import serde;
import core_events;
import core_logger;
import std;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

// The TOML wire format as a genuine plugin — formats are just plugins, no special category.
// encode()/decode() both call rfl::toml::write/read directly — the old FieldDesc-typed
// serde::Toml class (include/serde/toml.cppm) was deleted once nothing else called it
// directly; it never had a Generic-shaped path anyway, so this plugin never routed through it.
class TomlPlugin : public congelado::Plugin, public interfaces::ISerdeFormat {
  public:
    /**
     * @brief Plugin name reported to the host.
     * @return `"toml"`.
     */
    [[nodiscard]] std::string_view get_name() const noexcept override { return "toml"; }
    /**
     * @brief Version string for this build of the TOML format plugin.
     * @return `"0.1.0"`.
     */
    [[nodiscard]] std::string_view get_version() const noexcept override { return "0.1.0"; }
    /**
     * @brief Flags this as a serde-format-capable plugin, so the host wires `serde_get` into
     * the `_cap_dispatch` routing.
     * @return `CONGELADO_CAP_SERDE`.
     */
    [[nodiscard]] std::uint32_t capabilities() const noexcept override {
        return CONGELADO_CAP_SERDE;
    }

    /**
     * @brief Capability hook the host calls to get at this plugin's `ISerdeFormat` surface.
     * @return this instance, upcast to `interfaces::ISerdeFormat*`.
     */
    void *serde_get() noexcept { return static_cast<interfaces::ISerdeFormat *>(this); }

    /// @brief The content-type this format registers under. @return `"application/toml"`.
    [[nodiscard]] std::string_view content_type() const noexcept override {
        return "application/toml";
    }
    /// @brief Short human-readable format name. @return `"toml"`.
    [[nodiscard]] std::string_view format_name() const noexcept override { return "toml"; }

    /**
     * @brief Encodes a generic reflected value to TOML text.
     * @note TOML documents can only have a table at the root — reflect-cpp's TOML writer
     * `static_assert`s against writing `rfl::Generic` directly (it's a variant that could hold
     * a bare scalar), so this pulls the object out via `to_object()` first and writes that
     * instead; a non-table `value` is a real error, not a plugin bug.
     * @param value the value to encode.
     * @return the TOML-encoded text, or an error if `value` isn't a table at the root.
     */
    [[nodiscard]] std::expected<std::string, std::string>
    encode(const rfl::Generic &value) const override {
        auto object = value.to_object();
        if (!object) {
            core::logger::warning("toml", "encode failed: value isn't a table at the root");
            core::events::publish("serde.toml.encode_failed");
            return std::unexpected{std::string{"TOML requires a table at the document root"}};
        }
        auto result = rfl::toml::write(*object);
        core::logger::debug("toml", "encoded {} byte(s)", result.size());
        return result;
    }

    /**
     * @brief Decodes TOML text into a generic reflected value via `rfl::toml::read` directly.
     * @param data the raw TOML text to decode.
     * @return the decoded value, or an error message if `data` isn't valid TOML.
     */
    [[nodiscard]] std::expected<rfl::Generic, std::string>
    decode(std::string_view data) const override {
        core::logger::debug("toml", "decoding {} byte(s)", data.size());
        auto result = rfl::toml::read<rfl::Generic>(data);
        if (!result) {
            core::logger::warning("toml", "decode failed: {}", result.error().what());
            core::events::publish("serde.toml.decode_failed", {{"error", result.error().what()}});
            return std::unexpected{std::string{result.error().what()}};
        }
        return *result;
    }
};

CONGELADO_PLUGIN(TomlPlugin);

#ifdef CONGELADO_TEST
namespace toml_plugin_tests {
using namespace boost::ut;

suite<"TomlPlugin"> toml_plugin_suite = [] {
    "get_name returns toml"_test = [] {
        TomlPlugin plugin;
        expect(plugin.get_name() == "toml");
    };

    "get_version returns 0.1.0"_test = [] {
        TomlPlugin plugin;
        expect(plugin.get_version() == "0.1.0");
    };

    "capabilities reports CONGELADO_CAP_SERDE"_test = [] {
        TomlPlugin plugin;
        expect(plugin.capabilities() == CONGELADO_CAP_SERDE);
    };

    "serde_get returns a non-null pointer castable to ISerdeFormat"_test = [] {
        TomlPlugin plugin;
        void *raw = plugin.serde_get();
        expect(raw != nullptr) << fatal;
        auto *format = static_cast<interfaces::ISerdeFormat *>(raw);
        expect(format->format_name() == "toml");
    };

    "content_type returns application/toml"_test = [] {
        TomlPlugin plugin;
        expect(plugin.content_type() == "application/toml");
    };

    "format_name returns toml"_test = [] {
        TomlPlugin plugin;
        expect(plugin.format_name() == "toml");
    };

    "encode/decode round-trips a table at the root"_test = [] {
        TomlPlugin plugin;
        rfl::Generic::Object object;
        object.insert(std::string{"name"}, rfl::Generic{std::string{"alice"}});
        object.insert(std::string{"age"}, rfl::Generic{30});
        rfl::Generic value{object};

        auto encoded = plugin.encode(value);
        expect(encoded.has_value()) << fatal;

        auto decoded = plugin.decode(*encoded);
        expect(decoded.has_value()) << fatal;
        auto decoded_object = decoded->to_object();
        expect(decoded_object.has_value()) << fatal;
        expect(decoded_object->size() == 2);
    };

    "encode fails when value isn't a table at the root"_test = [] {
        TomlPlugin plugin;
        rfl::Generic value{std::string{"not a table"}};
        auto encoded = plugin.encode(value);
        expect(!encoded.has_value()) << fatal;
        expect(encoded.error() == "TOML requires a table at the document root");
    };

    "decode fails on malformed TOML"_test = [] {
        TomlPlugin plugin;
        auto decoded = plugin.decode("this = is [ not valid toml =");
        expect(!decoded.has_value());
    };
};

} // namespace toml_plugin_tests
#endif
