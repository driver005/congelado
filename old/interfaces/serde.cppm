module;
#include <rfl/Generic.hpp>

export module interfaces:serde;

import std;

export namespace interfaces {

// The dynamic JSON-like value type (string/bool/int/double/array/object) — reflect-cpp's
// own `rfl::Generic`, canonical here so it can flow to IWorker's input and serde's wire
// boundary without `interfaces` importing `serde` back (serde imports `interfaces`, so the
// alias flows: `interfaces` owns it, `serde` re-exports it as `serde::Value` — never the
// reverse, which would cycle).
using Value = rfl::Generic;

// A wire-format backend (JSON, TOML, ...) as a genuine plugin capability. The host converts
// any FieldDesc-reflected T to/from rfl::Generic itself (via rfl::to_generic/rfl::from_generic
// — that machinery stays compiled into the host, unaffected by which formats are loaded);
// only the Generic-to-bytes step, the part that actually differs per wire format, crosses the
// plugin ABI boundary. This is what lets a dlopen'd .so serialize an arbitrary host-defined T
// without ever instantiating a template across the shared-library boundary.
class ISerdeFormat
{
public:
    /**
     * @brief Virtual dtor, default's good — format backends clean up fine through the base
     * pointer, no extra motion needed.
     */
    virtual ~ISerdeFormat() = default;
    ISerdeFormat() = default;
    ISerdeFormat(const ISerdeFormat&) = delete;
    ISerdeFormat& operator=(const ISerdeFormat&) = delete;
    ISerdeFormat(ISerdeFormat&&) = delete;
    ISerdeFormat& operator=(ISerdeFormat&&) = delete;

    /**
     * @brief The wire content-type this format answers to (e.g. `"application/json"`) — this
     * is the key `SerdeFormatRegistry` looks formats up by.
     * @return the content-type string.
     */
    [[nodiscard]] virtual std::string_view content_type() const noexcept = 0;
    /**
     * @brief A short human-readable name for this format, for logs/diagnostics.
     * @return the format's name (e.g. `"json"`).
     */
    [[nodiscard]] virtual std::string_view format_name() const noexcept = 0;
    /**
     * @brief Encodes a generic reflected value to this format's wire bytes.
     * @param value the value to encode, already reduced to `rfl::Generic` by the host.
     * @return the encoded wire text, or an error message if `value` can't be represented in
     * this format.
     */
    [[nodiscard]] virtual std::expected<std::string, std::string>
    encode(const rfl::Generic& value) const = 0;
    /**
     * @brief Decodes wire bytes in this format into a generic reflected value.
     * @param data the raw wire text to decode.
     * @return the decoded `rfl::Generic`, or an error message if `data` isn't valid for this
     * format.
     */
    [[nodiscard]] virtual std::expected<rfl::Generic, std::string>
    decode(std::string_view data) const = 0;
};

} // namespace interfaces
