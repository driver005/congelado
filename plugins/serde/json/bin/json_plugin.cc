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
#ifdef CONGELADO_TEST
import boost.ut;
#endif

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

#ifdef CONGELADO_TEST
namespace json_plugin_tests {
using namespace boost::ut;

suite<"JsonPlugin"> json_plugin_suite = [] {
    "get_name returns json"_test = [] {
        JsonPlugin plugin;
        expect(plugin.get_name() == "json");
    };

    "get_version returns 0.1.0"_test = [] {
        JsonPlugin plugin;
        expect(plugin.get_version() == "0.1.0");
    };

    "capabilities reports CONGELADO_CAP_SERDE"_test = [] {
        JsonPlugin plugin;
        expect(plugin.capabilities() == CONGELADO_CAP_SERDE);
    };

    "serde_get returns a non-null pointer castable to ISerdeFormat"_test = [] {
        JsonPlugin plugin;
        void *raw = plugin.serde_get();
        expect(raw != nullptr) << fatal;
        auto *format = static_cast<interfaces::ISerdeFormat *>(raw);
        expect(format->format_name() == "json");
    };

    "content_type returns application/json"_test = [] {
        JsonPlugin plugin;
        expect(plugin.content_type() == "application/json");
    };

    "format_name returns json"_test = [] {
        JsonPlugin plugin;
        expect(plugin.format_name() == "json");
    };

    "encode/decode round-trips a string"_test = [] {
        JsonPlugin plugin;
        rfl::Generic value{std::string{"hello"}};
        auto encoded = plugin.encode(value);
        expect(encoded.has_value()) << fatal;
        expect(*encoded == R"("hello")");

        auto decoded = plugin.decode(*encoded);
        expect(decoded.has_value()) << fatal;
        auto text = decoded->to_string();
        expect(text.has_value()) << fatal;
        expect(*text == "hello");
    };

    "encode/decode round-trips a number"_test = [] {
        JsonPlugin plugin;
        rfl::Generic value{42};
        auto encoded = plugin.encode(value);
        expect(encoded.has_value()) << fatal;

        auto decoded = plugin.decode(*encoded);
        expect(decoded.has_value()) << fatal;
        auto number = decoded->to_int();
        expect(number.has_value()) << fatal;
        expect(*number == 42);
    };

    "encode/decode round-trips a bool"_test = [] {
        JsonPlugin plugin;
        rfl::Generic value{true};
        auto encoded = plugin.encode(value);
        expect(encoded.has_value()) << fatal;
        expect(*encoded == "true");

        auto decoded = plugin.decode(*encoded);
        expect(decoded.has_value()) << fatal;
        auto flag = decoded->to_bool();
        expect(flag.has_value()) << fatal;
        expect(*flag == true);
    };

    "encode/decode round-trips a nested object and array"_test = [] {
        JsonPlugin plugin;
        rfl::Generic::Object object;
        object.insert(std::string{"name"}, rfl::Generic{std::string{"alice"}});
        rfl::Generic::Array numbers{rfl::Generic{1}, rfl::Generic{2}, rfl::Generic{3}};
        object.insert(std::string{"numbers"}, rfl::Generic{numbers});
        rfl::Generic value{object};

        auto encoded = plugin.encode(value);
        expect(encoded.has_value()) << fatal;

        auto decoded = plugin.decode(*encoded);
        expect(decoded.has_value()) << fatal;
        auto decoded_object = decoded->to_object();
        expect(decoded_object.has_value()) << fatal;
        expect(decoded_object->size() == 2);
    };

    "decode fails on malformed JSON"_test = [] {
        JsonPlugin plugin;
        auto decoded = plugin.decode("{not valid json");
        expect(!decoded.has_value());
    };

    // rfl::json::read() is a recursive-descent parser with no nesting-depth limit applied before
    // it sees attacker bytes — deeply-nested input (thousands+ levels of `[`) is a classic
    // stack-overflow DoS vector. This test only exercises a MODERATE depth (100 levels), which is
    // well within any reasonable call-stack budget, to pin the current "no cap, decode just
    // succeeds" behavior. It deliberately does NOT probe the actual stack-overflow depth — doing
    // so would crash this shared test binary, which runs hundreds of other suites in the same
    // process.
    "decode accepts deeply nested arrays with no depth-limit rejection"_test = [] {
        JsonPlugin plugin;
        constexpr int depth = 100;
        std::string nested = std::string(depth, '[') + "1" + std::string(depth, ']');

        auto decoded = plugin.decode(nested);
        expect(decoded.has_value()) << fatal;
    };
};

} // namespace json_plugin_tests
#endif
