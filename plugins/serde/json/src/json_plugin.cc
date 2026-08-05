module;

#define CONGELADO_GUEST
#include <congelado/plugin.h>
#include <rfl/Generic.hpp>
#include <rfl/json.hpp>

export module json_plugin;

import congelado_plugin;
import interfaces;
import core_events;
import core_logger;
import std;

// The JSON wire format as a genuine plugin — formats are just plugins, no special category.
// Calls rfl::json::write/read directly (the old shared serde::Json/Document classes were
// deleted once this plugin was their only real caller) — this only adds the rfl::Generic <->
// wire-text boundary the plugin ABI needs, per interfaces::ISerdeFormat.
class JsonPlugin : public congelado::Plugin, public interfaces::ISerdeFormat {
  public:
    /**
     * @brief Plugin name reported to the host.
     * @return `"json"`.
     */
    [[nodiscard]] std::string_view get_name() const noexcept override { return "json"; }
    /**
     * @brief Version string for this build of the JSON format plugin.
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

    /// @brief The content-type this format registers under. @return `"application/json"`.
    [[nodiscard]] std::string_view content_type() const noexcept override {
        return "application/json";
    }
    /// @brief Short human-readable format name. @return `"json"`.
    [[nodiscard]] std::string_view format_name() const noexcept override { return "json"; }

    /**
     * @brief Encodes a generic reflected value to JSON text via `rfl::json::write`.
     * @param value the value to encode.
     * @return the JSON-encoded text.
     */
    [[nodiscard]] std::expected<std::string, std::string>
    encode(const rfl::Generic &value) const override {
        // rfl::json::write() returns a plain std::string (not rfl::Result<T>, unlike read()
        // below) — encoding an already-in-memory rfl::Generic can't fail the way parsing
        // arbitrary text can, so there's no failure branch to log here.
        auto result = rfl::json::write(value);
        core::logger::debug("json", "encoded {} byte(s)", result.size());
        return result;
    }

    /**
     * @brief Decodes JSON text into a generic reflected value via `rfl::json::read` directly.
     * @param data the raw JSON text to decode.
     * @return the decoded value, or an error message if `data` isn't valid JSON.
     */
    [[nodiscard]] std::expected<rfl::Generic, std::string>
    decode(std::string_view data) const override {
        core::logger::debug("json", "decoding {} byte(s)", data.size());
        auto result = rfl::json::read<rfl::Generic>(data);
        if (!result) {
            core::logger::warning("json", "decode failed: {}", result.error().what());
            core::events::publish("serde.json.decode_failed", {{"error", result.error().what()}});
            return std::unexpected{result.error().what()};
        }
        return *result;
    }
};

CONGELADO_PLUGIN(JsonPlugin);
