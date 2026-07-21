module;
#define UUID_SYSTEM_GENERATOR
#include <rfl.hpp>
#include <rfl/enums.hpp>
#include <simdjson.h>
#include <toml++/toml.hpp>
#include <uuid.h>

export module serde:converter;

import :core;
import std;

// ─── Forward declaration of TomlParser::from_toml_impl ───────────────────────

export namespace serde {
class TomlParser {
  public:
    /**
     * @brief Populates every reflected field of `T` from a parsed TOML table — the
     * recursive engine behind Toml::decode and every ISerializable FieldConverter's
     * `from_toml`. Forward-declared here so FieldConverter<ISerializable T> can call it
     * before its actual definition (further down this file, after Serializable<T> is fully
     * usable).
     * @tparam T the serializable type being populated.
     * @param table the already-parsed TOML table to read fields from (second param is the
     * output object, left unnamed here since this is just the declaration).
     * @return success, or an error message the moment any field fails to extract.
     */
    template <ISerializable T>
    static std::expected<void, std::string> from_toml_impl(const toml::table &table, T &object);
};
}

// Forward-declare so FieldConverter<ISerializable>::from_simdjson can call it via
// qualified lookup (phase-1). Definition is in export namespace simdjson below.
export namespace simdjson {
/**
 * @brief Forward declaration of simdjson's ADL customization point for deserializing an
 * ISerializable T straight out of a simdjson value — this is a free function by contract
 * (simdjson's `tag_invoke` mechanism finds it via argument-dependent lookup, it cannot live
 * on a class), so it's the one deliberate exception to this codebase's no-free-functions
 * rule. Declared here so FieldConverter<ISerializable T>::from_simdjson can call it via
 * qualified lookup before its real definition further down.
 * @tparam V the simdjson value-like type being deserialized from.
 * @tparam T the serializable type being deserialized into.
 * @param val the simdjson value to read.
 * @param obj the object to populate, mutated in place.
 * @return a simdjson error_code — SUCCESS if every field decoded clean.
 */
template <typename V, serde::ISerializable T>
error_code tag_invoke(deserialize_tag tag, V &json_value, T &object);
} // namespace simdjson

// ─── FieldConverter<VT> primary + concrete specializations ───────────────────

export namespace serde {

template <typename VT>
struct FieldConverter {
    using rfl_type = VT;

    /**
     * @brief Reads a field's value straight out of a simdjson value via its generic
     * `get(out)` — the fallback path for any VT simdjson natively knows how to extract
     * (primitives, mostly), no per-type logic needed here.
     * @param json_value the simdjson value to read from.
     * @param out the destination to write the decoded value into.
     * @return a simdjson error_code — SUCCESS on a clean decode.
     */
    static simdjson::error_code from_simdjson(simdjson::ondemand::value &json_value, VT &out) {
        return json_value.get(out);
    }

    /**
     * @brief Reads a field's value out of a TOML table by key, via toml++'s generic
     * `value<VT>()`.
     * @param table the TOML table to read from.
     * @param field_name the key to look up.
     * @return the decoded value, or an error message if the field's missing or its TOML
     * type doesn't convert to VT.
     */
    static std::expected<VT, std::string> from_toml(const toml::table &table,
                                                     std::string_view field_name) {
        // toml++'s generic value<VT>() covers the lookup + type-conversion in one shot — a
        // missing key and a wrong-typed value both land here as a single failure mode.
        auto toml_value = table[field_name].value<VT>();  // FIXME(clang-tidy): unchecked operator[], consider .at()
        if (!toml_value) {
            return std::unexpected{
                std::format("missing or invalid field '{}'", field_name)};
        }
        return *toml_value;
    }

    /// @brief Converts VT to its rfl-facing representation — identity for the generic case.
    /// @param value the value to convert.
    /// @return `value`, unchanged (rfl_type == VT here).
    static rfl_type to_rfl(const VT &value) { return value; }
    /// @brief Converts an rfl-facing value back to VT — identity for the generic case.
    /// @param value the rfl-side value to convert.
    /// @return `value`, unchanged.
    static VT       from_rfl(const rfl_type &value) { return value; }
};

// ─── uint32_t ─────────────────────────────────────────────────────────────────

template <>
struct FieldConverter<std::uint32_t> {
    using rfl_type = std::uint32_t;

    /**
     * @brief Reads a uint32 field by first pulling a uint64 out of simdjson (its native
     * integer width) then narrowing — simdjson has no direct `get_uint32`, so this is the
     * required detour.
     * @param json_value the simdjson value to read from.
     * @param out the destination to write the decoded value into.
     * @warning The narrowing `static_cast<std::uint32_t>` is silent — a JSON value between
     * `UINT32_MAX` and `UINT64_MAX` truncates with no error, no cap. Only safe if the wire
     * format is trusted to actually fit in 32 bits.
     * @return a simdjson error_code — SUCCESS on a clean decode.
     */
    static simdjson::error_code from_simdjson(simdjson::ondemand::value &json_value,
                                               std::uint32_t &out) {
        // simdjson has no get_uint32 — pull the native uint64 first.
        std::uint64_t tmp{};
        if (auto ec = json_value.get_uint64().get(tmp); ec) {
            return ec;
        }
        // Then narrow, silently, as written — no range check before the truncation.
        out = static_cast<std::uint32_t>(tmp);
        return simdjson::SUCCESS;
    }

    /**
     * @brief Reads a uint32 field out of a TOML table — toml++ only has signed integer
     * value types, so this goes through `int64_t` first, same detour as the simdjson path.
     * @param table the TOML table to read from.
     * @param field_name the key to look up.
     * @warning Same silent-narrowing footgun as `from_simdjson` — no range check before the
     * `static_cast`.
     * @return the decoded value, or an error message if the field's missing.
     */
    static std::expected<std::uint32_t, std::string> from_toml(const toml::table &table,
                                                                std::string_view field_name) {
        // toml++ only does signed ints — read as int64_t first, same detour as from_simdjson.
        auto toml_value = table[field_name].value<std::int64_t>();  // FIXME(clang-tidy): unchecked operator[], consider .at()
        if (!toml_value) {
            return std::unexpected{std::format("missing field '{}'", field_name)};
        }
        // Same silent narrowing here too, no range check before the cast.
        return static_cast<std::uint32_t>(*toml_value);
    }

    /// @brief Converts a uint32 to its rfl-facing representation — identity here.
    /// @param value the value to convert.
    /// @return `value`, unchanged.
    static std::uint32_t to_rfl(std::uint32_t value) { return value; }
    /// @brief Converts an rfl-facing uint32 back — identity here.
    /// @param value the rfl-side value to convert.
    /// @return `value`, unchanged.
    static std::uint32_t from_rfl(std::uint32_t value) { return value; }
};

// ─── Enum types ───────────────────────────────────────────────────────────────

/**
 * @brief FieldConverter specialization for every enum type — round-trips enums as their
 * name string (via rfl::string_to_enum), never their underlying integer value.
 * @tparam E the enum type being converted; constrained to `std::is_enum_v<E>`.
 */
template <typename E>
    requires std::is_enum_v<E>
struct FieldConverter<E> {
    using rfl_type = E;

    /**
     * @brief Reads an enum field from a JSON string, mapping it back via
     * `rfl::string_to_enum` — enums always round-trip as their name, not their underlying
     * integer value, so `"Active"` not `1`.
     * @param json_value the simdjson value to read from — must hold a JSON string.
     * @param out the destination to write the decoded enum into.
     * @warning A string that doesn't name any enumerator maps to `INCORRECT_TYPE`, not a
     * more specific "unknown enum value" error — same generic error code you'd get from a
     * genuinely wrong JSON type. Debugging a stale/renamed enumerator looks identical to a
     * type mismatch, no cap.
     * @return a simdjson error_code — SUCCESS on a clean decode, INCORRECT_TYPE if the
     * string doesn't name a valid enumerator.
     */
    static simdjson::error_code from_simdjson(simdjson::ondemand::value &json_value, E &out) {
        // Enums always ride the wire as their name string, never a raw integer.
        std::string_view string_value;
        if (auto ec = json_value.get_string().get(string_value); ec) {
            return ec;
        }
        // Map the name back to its enumerator — an unknown name folds into the same generic
        // INCORRECT_TYPE a genuinely wrong JSON type would produce, no distinct error here.
        auto result = rfl::string_to_enum<E>(std::string{string_value});
        if (!result) {
            return simdjson::INCORRECT_TYPE;
        }
        out = *result;
        return simdjson::SUCCESS;
    }

    /**
     * @brief Reads an enum field from a TOML string, same name-based mapping as
     * `from_simdjson`.
     * @param table the TOML table to read from.
     * @param field_name the key to look up.
     * @return the decoded enum, or an error message if the field's missing or its string
     * doesn't name a valid enumerator (this path, unlike simdjson's, actually names the bad
     * value in the error).
     */
    static std::expected<E, std::string> from_toml(const toml::table &table,
                                                    std::string_view field_name) {
        auto string_value = table[field_name].value<std::string>();  // FIXME(clang-tidy): unchecked operator[], consider .at()
        if (!string_value) {
            return std::unexpected{std::format("missing field '{}'", field_name)};
        }
        // Same name-based mapping as from_simdjson, but this path actually names the bad
        // value in the error instead of collapsing to a generic error code.
        auto result = rfl::string_to_enum<E>(*string_value);
        if (!result) {
            return std::unexpected{
                std::format("invalid enum '{}' for field '{}'", *string_value, field_name)};
        }
        return *result;
    }

    /// @brief Converts an enum to its rfl-facing representation — identity here (rfl handles
    /// the string mapping internally via its own Reflector).
    /// @param value the value to convert.
    /// @return `value`, unchanged.
    static E to_rfl(const E &value) { return value; }
    /// @brief Converts an rfl-facing enum back — identity here.
    /// @param value the rfl-side value to convert.
    /// @return `value`, unchanged.
    static E from_rfl(const E &value) { return value; }
};

// ─── uuids::uuid ──────────────────────────────────────────────────────────────

template <>
struct FieldConverter<uuids::uuid> {
    using rfl_type = std::string;

    /**
     * @brief Reads a UUID field from a JSON string via `uuids::uuid::from_string`.
     * @param json_value the simdjson value to read from — must hold a JSON string.
     * @param out the destination to write the decoded UUID into.
     * @return a simdjson error_code — SUCCESS on a clean decode, INCORRECT_TYPE if the
     * string isn't a parseable UUID.
     */
    static simdjson::error_code from_simdjson(simdjson::ondemand::value &json_value,
                                               uuids::uuid &out) {
        // Must be a JSON string first — no string, no UUID to even attempt parsing.
        std::string_view string_value;
        if (auto ec = json_value.get_string().get(string_value); ec) {
            return ec;
        }
        // Then validate it actually parses as a UUID before accepting it.
        auto id = uuids::uuid::from_string(string_value);
        if (!id) {
            return simdjson::INCORRECT_TYPE;
        }
        out = *id;
        return simdjson::SUCCESS;
    }

    /**
     * @brief Reads a UUID field from a TOML string, same parse-and-validate motion as
     * `from_simdjson`.
     * @param table the TOML table to read from.
     * @param field_name the key to look up.
     * @return the decoded UUID, or an error message if the field's missing or its string
     * isn't a parseable UUID.
     */
    static std::expected<uuids::uuid, std::string> from_toml(const toml::table &table,
                                                               std::string_view field_name) {
        // Missing field is a hard error here — no silent nullopt fallback for a required UUID.
        auto string_value = table[field_name].value<std::string>();  // FIXME(clang-tidy): unchecked operator[], consider .at()
        if (!string_value) {
            return std::unexpected{std::format("missing field '{}'", field_name)};
        }
        // Present but unparseable is a distinct, named error.
        auto id = uuids::uuid::from_string(*string_value);
        if (!id) {
            return std::unexpected{std::format("invalid UUID for field '{}'", field_name)};
        }
        return *id;
    }

    /// @brief Converts a UUID to its rfl-facing representation — a plain string here.
    /// @param value the UUID to convert.
    /// @return the UUID's canonical string form.
    static std::string to_rfl(const uuids::uuid &value) { return uuids::to_string(value); }
    /**
     * @brief Converts an rfl-facing string back to a UUID.
     * @param str the string to parse.
     * @warning If `str` isn't a valid UUID, this silently falls back to a zero-valued
     * `uuids::uuid{}` instead of erroring — a corrupted rfl round-trip degrades quietly into
     * a nil UUID rather than a caught failure. Kinda cooked if that nil UUID then gets used
     * as a real primary key downstream.
     * @return the parsed UUID, or a default-constructed (nil) UUID if `str` is invalid.
     */
    static uuids::uuid from_rfl(const std::string &str) {
        return uuids::uuid::from_string(str).value_or(uuids::uuid{});
    }
};

// ─── std::optional<uuids::uuid> ───────────────────────────────────────────────

template <>
struct FieldConverter<std::optional<uuids::uuid>> {
    using rfl_type = std::optional<std::string>;

    /**
     * @brief Reads an optional UUID field — a JSON `null` maps to `nullopt`, anything else
     * must be a parseable UUID string.
     * @param json_value the simdjson value to read from.
     * @param out the destination to write the decoded value into.
     * @return a simdjson error_code — SUCCESS whether `out` ends up set or `nullopt`,
     * INCORRECT_TYPE if a non-null value isn't a parseable UUID.
     */
    static simdjson::error_code from_simdjson(simdjson::ondemand::value &json_value,
                                               std::optional<uuids::uuid> &out) {
        // Null short-circuits straight to nullopt — everything past this point assumes a
        // present, string-typed value.
        if (json_value.is_null()) {
            out = std::nullopt;
            return simdjson::SUCCESS;
        }
        std::string_view string_value;
        if (auto ec = json_value.get_string().get(string_value); ec) {
            return ec;
        }
        auto id = uuids::uuid::from_string(string_value);
        if (!id) {
            return simdjson::INCORRECT_TYPE;
        }
        out = *id;
        return simdjson::SUCCESS;
    }

    /**
     * @brief Reads an optional UUID field from TOML.
     * @param table the TOML table to read from.
     * @param field_name the key to look up.
     * @note Unlike the non-optional UUID's `from_toml`, a missing field here is not an
     * error — it's treated the same as an absent value and maps to `nullopt`. Only an
     * invalid (present but unparseable) UUID string produces an error.
     * @return the decoded optional UUID, or an error message if the field's present but
     * isn't a parseable UUID.
     */
    static std::expected<std::optional<uuids::uuid>, std::string>
    from_toml(const toml::table &table, std::string_view field_name) {
        // Unlike the non-optional UUID, a missing field here is not an error — it just reads
        // as absent, same as an explicit nullopt.
        auto string_value = table[field_name].value<std::string>();  // FIXME(clang-tidy): unchecked operator[], consider .at()
        if (!string_value) {
            return std::nullopt;
        }
        // Present but unparseable still errors hard, though.
        auto id = uuids::uuid::from_string(*string_value);
        if (!id) {
            return std::unexpected{std::format("invalid UUID for field '{}'", field_name)};
        }
        return *id;
    }

    /// @brief Converts an optional UUID to its rfl-facing optional-string representation.
    /// @param value the optional UUID to convert.
    /// @return `nullopt` if `value` is empty, else its canonical string form.
    static rfl_type to_rfl(const std::optional<uuids::uuid> &value) {
        // No value, no string — everything else delegates to the canonical stringify.
        if (!value) {
            return std::nullopt;
        }
        return uuids::to_string(*value);
    }
    /**
     * @brief Converts an rfl-facing optional string back to an optional UUID.
     * @param str the optional string to parse.
     * @warning Same silent-fallback footgun as the non-optional `from_rfl` — an invalid
     * (non-empty but unparseable) string doesn't propagate an error, it degrades to a
     * present-but-nil `uuids::uuid{}` instead of `nullopt` or a thrown error.
     * @return `nullopt` if `str` is empty, else the parsed UUID (or a nil UUID if parsing
     * fails).
     */
    static std::optional<uuids::uuid> from_rfl(const rfl_type &str) {
        // No string in, no UUID out.
        if (!str) {
            return std::nullopt;
        }
        // A present-but-invalid string quietly degrades to a nil UUID rather than propagating
        // an error — matches the non-optional from_rfl's silent-fallback behavior.
        return uuids::uuid::from_string(*str).value_or(uuids::uuid{});
    }
};

// ─── std::chrono::system_clock::time_point ───────────────────────────────────

using TP = std::chrono::system_clock::time_point;

template <>
struct FieldConverter<TP> {
    using rfl_type = std::int64_t;

    /**
     * @brief Reads a time_point field as a millisecond-since-epoch integer.
     * @param json_value the simdjson value to read from — must hold a JSON integer.
     * @param out the destination to write the decoded time_point into.
     * @return a simdjson error_code — SUCCESS on a clean decode.
     */
    static simdjson::error_code from_simdjson(simdjson::ondemand::value &json_value, TP &out) {
        // Pull the raw millisecond count off the wire first...
        std::int64_t milliseconds{};
        if (auto ec = json_value.get_int64().get(milliseconds); ec) {
            return ec;
        }
        // ...then rebuild the time_point from it.
        out = TP{std::chrono::milliseconds{milliseconds}};
        return simdjson::SUCCESS;
    }

    /**
     * @brief Reads a time_point field from TOML as a millisecond-since-epoch integer — note
     * this is NOT toml++'s native `toml::date_time` type, it's a plain int64 by convention.
     * @param table the TOML table to read from.
     * @param field_name the key to look up.
     * @return the decoded time_point, or an error message if the field's missing.
     */
    static std::expected<TP, std::string> from_toml(const toml::table &table,
                                                     std::string_view field_name) {
        // Read as a plain int64 by convention — this is NOT toml++'s native date_time type.
        auto milliseconds = table[field_name].value<std::int64_t>();  // FIXME(clang-tidy): unchecked operator[], consider .at()
        if (!milliseconds) {
            return std::unexpected{std::format("missing field '{}'", field_name)};
        }
        return TP{std::chrono::milliseconds{*milliseconds}};
    }

    /// @brief Converts a time_point to milliseconds-since-epoch for the rfl-facing side.
    /// @param value the time_point to convert.
    /// @return `value`'s epoch offset, truncated to whole milliseconds.
    static std::int64_t to_rfl(const TP &value) {
        return std::chrono::duration_cast<std::chrono::milliseconds>(value.time_since_epoch())
            .count();
    }
    /// @brief Converts an rfl-facing millisecond count back to a time_point.
    /// @param milliseconds the epoch offset in milliseconds.
    /// @return the reconstructed time_point.
    static TP from_rfl(std::int64_t milliseconds) {
        return TP{std::chrono::milliseconds{milliseconds}};
    }
};

// ─── std::optional<time_point> ────────────────────────────────────────────────

template <>
struct FieldConverter<std::optional<TP>> {
    using rfl_type = std::optional<std::int64_t>;

    /**
     * @brief Reads an optional time_point field — a JSON `null` maps to `nullopt`, anything
     * else must be a millisecond-since-epoch integer.
     * @param json_value the simdjson value to read from.
     * @param out the destination to write the decoded value into.
     * @return a simdjson error_code — SUCCESS whether `out` ends up set or `nullopt`.
     */
    static simdjson::error_code from_simdjson(simdjson::ondemand::value &json_value,
                                               std::optional<TP> &out) {
        // Null maps straight to nullopt, no int parsing attempted.
        if (json_value.is_null()) {
            out = std::nullopt;
            return simdjson::SUCCESS;
        }
        // Otherwise same millisecond-count decode as the non-optional TP.
        std::int64_t milliseconds{};
        if (auto ec = json_value.get_int64().get(milliseconds); ec) {
            return ec;
        }
        out = TP{std::chrono::milliseconds{milliseconds}};
        return simdjson::SUCCESS;
    }

    /**
     * @brief Reads an optional time_point field from TOML.
     * @param table the TOML table to read from.
     * @param field_name the key to look up.
     * @note A missing field maps to `nullopt` here — no error, unlike the non-optional TP's
     * `from_toml` which treats a missing field as a hard failure.
     * @return the decoded optional time_point; `nullopt` if the field's absent.
     */
    static std::expected<std::optional<TP>, std::string>
    from_toml(const toml::table &table, std::string_view field_name) {
        // A missing field maps to nullopt here — no hard error, unlike non-optional TP's from_toml.
        auto milliseconds = table[field_name].value<std::int64_t>();  // FIXME(clang-tidy): unchecked operator[], consider .at()
        if (!milliseconds) {
            return std::nullopt;
        }
        return TP{std::chrono::milliseconds{*milliseconds}};
    }

    /// @brief Converts an optional time_point to an optional millisecond count.
    /// @param value the optional time_point to convert.
    /// @return `nullopt` if `value` is empty, else its epoch offset in whole milliseconds.
    static rfl_type to_rfl(const std::optional<TP> &value) {
        // Empty stays empty; otherwise reduce to whole milliseconds since epoch.
        if (!value) {
            return std::nullopt;
        }
        return std::chrono::duration_cast<std::chrono::milliseconds>(value->time_since_epoch())
            .count();
    }
    /// @brief Converts an rfl-facing optional millisecond count back to an optional
    /// time_point.
    /// @param milliseconds the optional epoch offset in milliseconds.
    /// @return `nullopt` if `milliseconds` is empty, else the reconstructed time_point.
    static std::optional<TP> from_rfl(const rfl_type &milliseconds) {
        // Mirror to_rfl's empty check, then rebuild the time_point from the raw count.
        if (!milliseconds) {
            return std::nullopt;
        }
        return TP{std::chrono::milliseconds{*milliseconds}};
    }
};

// ─── std::vector<std::string> ─────────────────────────────────────────────────

template <>
struct FieldConverter<std::vector<std::string>> {
    using rfl_type = std::vector<std::string>;

    /**
     * @brief Reads a string-array field, one element at a time — every element must itself
     * be a JSON string, mixed-type arrays bail on the first mismatch.
     * @param json_value the simdjson value to read from — must hold a JSON array.
     * @param out the destination vector, appended to (not cleared first).
     * @return a simdjson error_code — SUCCESS on a clean decode, propagates the first
     * element-level error otherwise.
     */
    static simdjson::error_code from_simdjson(simdjson::ondemand::value &json_value,
                                               std::vector<std::string> &out) {
        // Must actually be a JSON array before iterating it.
        simdjson::ondemand::array json_array;
        if (auto ec = json_value.get_array().get(json_array); ec) {
            return ec;
        }
        // Every element must itself be a string — mixed-type arrays bail on the first mismatch.
        for (auto element : json_array) {
            simdjson::ondemand::value element_value;
            if (auto ec = element.get(element_value); ec) {
                return ec;
            }
            std::string_view string_value;
            if (auto ec = element_value.get_string().get(string_value); ec) {
                return ec;
            }
            out.emplace_back(string_value);
        }
        return simdjson::SUCCESS;
    }

    /**
     * @brief Reads a string-array field from TOML.
     * @param table the TOML table to read from.
     * @param field_name the key to look up.
     * @note A missing field (no such array at all) quietly resolves to an empty vector, no
     * error — different from a present-but-wrong-typed element, which does error. Lowkey
     * inconsistent with most of this file's other `from_toml`s that treat a missing field as
     * a hard failure.
     * @return the decoded elements in order, or an error message the moment any element
     * isn't a TOML string.
     */
    static std::expected<std::vector<std::string>, std::string>
    from_toml(const toml::table &table, std::string_view field_name) {
        // Missing array is treated as empty, not an error — inconsistent with most other
        // from_toml's in this file, which treat a missing field as a hard failure.
        const auto *toml_array = table[field_name].as_array();  // FIXME(clang-tidy): unchecked operator[], consider .at()
        if (toml_array == nullptr) {
            return std::vector<std::string>{};
        }
        std::vector<std::string> result;
        result.reserve(toml_array->size());
        // But once present, every element must actually be a string — no exceptions there.
        for (const auto &element : *toml_array) {
            auto string_value = element.value<std::string>();
            if (!string_value) {
                return std::unexpected{
                    std::format("element of '{}' must be a string", field_name)};
            }
            result.push_back(std::move(*string_value));
        }
        return result;
    }

    /// @brief Converts a string vector to its rfl-facing representation — identity here.
    /// @param value the vector to convert.
    /// @return `value`, unchanged.
    static rfl_type                to_rfl(const std::vector<std::string> &value) { return value; }
    /// @brief Converts an rfl-facing string vector back — identity here.
    /// @param value the rfl-side vector to convert.
    /// @return `value`, unchanged.
    static std::vector<std::string> from_rfl(const rfl_type &value) { return value; }
};

// ─── std::optional<std::string> ───────────────────────────────────────────────

template <>
struct FieldConverter<std::optional<std::string>> {
    using rfl_type = std::optional<std::string>;

    /**
     * @brief Reads an optional string field — a JSON `null` maps to `nullopt`.
     * @param json_value the simdjson value to read from.
     * @param out the destination to write the decoded value into.
     * @return a simdjson error_code — SUCCESS whether `out` ends up set or `nullopt`,
     * propagates the underlying error if a non-null value isn't a JSON string.
     */
    static simdjson::error_code from_simdjson(simdjson::ondemand::value &json_value,
                                               std::optional<std::string> &out) {
        // Null maps to nullopt straight away.
        if (json_value.is_null()) {
            out = std::nullopt;
            return simdjson::SUCCESS;
        }
        // Anything else must be a JSON string.
        std::string_view string_value;
        if (auto ec = json_value.get_string().get(string_value); ec) {
            return ec;
        }
        out = std::string{string_value};
        return simdjson::SUCCESS;
    }

    /**
     * @brief Reads an optional string field from TOML — a missing field or a wrong-typed
     * value both quietly resolve to `nullopt`, no distinction between the two, bet.
     * @param table the TOML table to read from.
     * @param field_name the key to look up.
     * @return the decoded optional string; `nullopt` if absent or not a TOML string.
     */
    static std::expected<std::optional<std::string>, std::string>
    from_toml(const toml::table &table, std::string_view field_name) {
        // A missing key and a wrong-typed value both quietly resolve to nullopt here — no
        // distinction made between the two.
        auto string_value = table[field_name].value<std::string>();  // FIXME(clang-tidy): unchecked operator[], consider .at()
        if (!string_value) {
            return std::nullopt;
        }
        return *string_value;
    }

    /// @brief Converts an optional string to its rfl-facing representation — identity here.
    /// @param value the optional string to convert.
    /// @return `value`, unchanged.
    static rfl_type                    to_rfl(const std::optional<std::string> &value) {
        return value;
    }
    /// @brief Converts an rfl-facing optional string back — identity here.
    /// @param value the rfl-side optional string to convert.
    /// @return `value`, unchanged.
    static std::optional<std::string> from_rfl(const rfl_type &value) { return value; }
};

// ─── std::unordered_map<std::string, std::string> ────────────────────────────

template <>
struct FieldConverter<std::unordered_map<std::string, std::string>> {
    using rfl_type = std::map<std::string, std::string>;

    /**
     * @brief Reads a string-to-string map field from a JSON object — every key becomes a
     * map key, every value must itself be a JSON string.
     * @param json_value the simdjson value to read from — must hold a JSON object.
     * @param out the destination map, populated via `emplace` (not cleared first).
     * @return a simdjson error_code — SUCCESS on a clean decode, propagates the first
     * key/value-level error otherwise.
     */
    static simdjson::error_code from_simdjson(
        simdjson::ondemand::value &json_value,
        std::unordered_map<std::string, std::string> &out) {
        // Must be a JSON object before iterating its key/value pairs.
        simdjson::ondemand::object json_object;
        if (auto ec = json_value.get_object().get(json_object); ec) {
            return ec;
        }
        // Every key becomes a map key; every value must itself be a JSON string.
        for (auto field : json_object) {
            std::string_view key;
            std::string_view string_value;
            if (auto ec = field.unescaped_key().get(key); ec) {
                return ec;
            }
            if (auto ec = field.value().get_string().get(string_value); ec) {
                return ec;
            }
            out.emplace(key, string_value);
        }
        return simdjson::SUCCESS;
    }

    /**
     * @brief Reads a string-to-string map field from a TOML sub-table.
     * @param table the TOML table to read from.
     * @param field_name the key to look up.
     * @note A missing sub-table quietly resolves to an empty map, no error — same
     * missing-is-empty convention as the vector<string> specialization above.
     * @return the decoded map, or an error message the moment any value isn't a TOML
     * string.
     */
    static std::expected<std::unordered_map<std::string, std::string>, std::string>
    from_toml(const toml::table &table, std::string_view field_name) {
        // No sub-table at all is just an empty map, no error — same missing-is-empty
        // convention as the vector<string> specialization above.
        const auto *sub_table = table[field_name].as_table();  // FIXME(clang-tidy): unchecked operator[], consider .at()
        if (sub_table == nullptr) {
            return std::unordered_map<std::string, std::string>{};
        }
        std::unordered_map<std::string, std::string> result;
        // Every value under the sub-table must be a string, though, no exceptions there.
        for (auto &&[key, value] : *sub_table) {
            auto string_value = value.value<std::string>();
            if (!string_value) {
                return std::unexpected{
                    std::format("value in '{}' must be a string", field_name)};
            }
            result.emplace(std::string{key.str()}, std::move(*string_value));
        }
        return result;
    }

    /**
     * @brief Converts an unordered_map to its rfl-facing representation — an ordered
     * `std::map`, since rfl (and most wire formats downstream) wants deterministic key
     * ordering, unlike `unordered_map`'s bucket-order iteration.
     * @param value the unordered_map to convert.
     * @return an ordered `std::map` with the same key/value pairs.
     */
    static rfl_type to_rfl(const std::unordered_map<std::string, std::string> &value) {
        return rfl_type{value.begin(), value.end()};
    }
    /**
     * @brief Converts an rfl-facing ordered map back to an unordered_map.
     * @param map the ordered map to convert.
     * @return an `unordered_map` with the same key/value pairs, order no longer guaranteed.
     */
    static std::unordered_map<std::string, std::string> from_rfl(const rfl_type &map) {
        return {map.begin(), map.end()};
    }
};

// ─── NamedTuple builder ───────────────────────────────────────────────────────

/**
 * @brief Builds an `rfl::NamedTuple` snapshot of `object`, one named field per FieldDesc in
 * `Fds...` — each field's value goes through its FieldConverter::to_rfl first, so this is
 * the bridge between this codebase's FieldDesc reflection and rfl's own NamedTuple world.
 * @tparam T the serializable type being snapshotted.
 * @tparam Fds the FieldDesc pack describing `T`'s fields (deduced from the trailing
 * `std::tuple<Fds...>` argument, normally `Serializable<T>::fields()`).
 * @param object the instance to read field values from.
 * @return an `rfl::NamedTuple` with one named entry per field in `Fds...`.
 */
template <typename T, typename... Fds>
auto build_named_tuple(const T &object, std::tuple<Fds...> field_descriptors) {
    // Fold over every FieldDesc: call its getter, run the result through the matching
    // FieldConverter::to_rfl, and pack it into a named rfl field — one per Fd, in order.
    return std::apply(
        [&](auto... fields) {
            return rfl::NamedTuple(
                rfl::make_field<decltype(fields)::name>(
                    FieldConverter<typename decltype(fields)::ValueType>::to_rfl(
                        (object.*decltype(fields)::getter)()))...);
        },
        std::tuple<Fds...>{});
}

/**
 * @brief The inverse of build_named_tuple — walks `named_tuple` and writes each field back
 * onto `object` via its setter, running every value through FieldConverter::from_rfl first.
 * @tparam T the serializable type being populated.
 * @tparam NT the rfl::NamedTuple type produced by a matching `build_named_tuple` call.
 * @tparam Fds the FieldDesc pack describing `T`'s fields.
 * @param object the instance to write field values onto, mutated in place.
 * @param named_tuple the NamedTuple to read field values from.
 */
template <typename T, typename NT, typename... Fds>
void apply_named_tuple_to(T &object, const NT &named_tuple, std::tuple<Fds...> field_descriptors) {
    // The inverse fold: pull each named field back out of the tuple, run it through
    // FieldConverter::from_rfl, and write it onto `object` via that field's setter.
    std::apply(
        [&](auto... fields) {
            ((object.*decltype(fields)::setter)(
                 FieldConverter<typename decltype(fields)::ValueType>::from_rfl(
                     rfl::get<decltype(fields)::name>(named_tuple))),
             ...);
        },
        std::tuple<Fds...>{});
}

// ─── ISerializable FieldConverter specializations ────────────────────────────

/**
 * @brief FieldConverter specialization for nested ISerializable types — recursion point for
 * object-in-object fields. `rfl_type` here isn't a primitive, it's the whole NamedTuple
 * shape `build_named_tuple` produces for `VT`, since a nested object still needs to be
 * reflected all the way down for rfl's own machinery to handle it.
 * @tparam VT the nested serializable type being converted; constrained to `ISerializable`.
 */
template <typename VT>
    requires ISerializable<VT>
struct FieldConverter<VT> {
    using rfl_type =
        decltype(build_named_tuple(std::declval<const VT &>(), Serializable<VT>::fields()));

    /**
     * @brief Reads a nested-object field by delegating straight to `VT`'s own
     * `simdjson::tag_invoke` overload — this is the recursive step, one object deep per
     * call.
     * @param json_value the simdjson value to read from — must hold a JSON object.
     * @param out the destination to write the decoded nested object into.
     * @return a simdjson error_code — SUCCESS on a clean decode.
     */
    static simdjson::error_code from_simdjson(simdjson::ondemand::value &json_value, VT &out) {
        return simdjson::tag_invoke(simdjson::deserialize_tag{}, json_value, out);
    }

    /**
     * @brief Reads a nested-object field from a TOML sub-table, delegating to
     * TomlParser::from_toml_impl — the TOML-side recursive step.
     * @param table the TOML table to read from.
     * @param field_name the key to look up.
     * @return the decoded nested object, or an error message if the field isn't a TOML
     * table or any of its own fields fail to extract.
     */
    static std::expected<VT, std::string> from_toml(const toml::table &table,
                                                     std::string_view field_name) {
        // Nested object needs a nested TOML table — anything else is an immediate type error.
        const auto *sub_table = table[field_name].as_table();  // FIXME(clang-tidy): unchecked operator[], consider .at()
        if (sub_table == nullptr) {
            return std::unexpected{
                std::format("field '{}' must be a TOML table", field_name)};
        }
        // Recurse one level deeper into VT's own fields.
        VT result;
        if (auto outcome = TomlParser::from_toml_impl(*sub_table, result); !outcome) {
            return std::unexpected{outcome.error()};
        }
        return result;
    }

    /// @brief Converts a nested object to its full NamedTuple rfl representation.
    /// @param value the nested object to convert.
    /// @return the NamedTuple built from `value`'s reflected fields.
    static rfl_type to_rfl(const VT &value) {
        return build_named_tuple(value, Serializable<VT>::fields());
    }
    /// @brief Converts an rfl-facing NamedTuple back into a nested object.
    /// @param named_tuple the NamedTuple to read field values from.
    /// @return the reconstructed nested object.
    static VT from_rfl(const rfl_type &named_tuple) {
        // Build a fresh default VT, then populate it field-by-field from the tuple.
        VT result;
        apply_named_tuple_to(result, named_tuple, Serializable<VT>::fields());
        return result;
    }
};

/**
 * @brief FieldConverter specialization for an optional nested ISerializable object — layers
 * null-handling on top of the plain-VT specialization above, delegating the actual
 * object-decoding work to it.
 * @tparam VT the nested serializable type being converted; constrained to `ISerializable`.
 */
template <typename VT>
    requires ISerializable<VT>
struct FieldConverter<std::optional<VT>> {
    using InnerRfl = FieldConverter<VT>::rfl_type;
    using rfl_type = std::optional<InnerRfl>;

    /**
     * @brief Reads an optional nested-object field — a JSON `null` maps to `nullopt`,
     * anything else delegates to `FieldConverter<VT>::from_simdjson`.
     * @param json_value the simdjson value to read from.
     * @param out the destination to write the decoded value into.
     * @return a simdjson error_code — SUCCESS whether `out` ends up set or `nullopt`.
     */
    static simdjson::error_code from_simdjson(simdjson::ondemand::value &json_value,
                                               std::optional<VT> &out) {
        // Null skips the whole nested-object decode entirely.
        if (json_value.is_null()) {
            out = std::nullopt;
            return simdjson::SUCCESS;
        }
        // Otherwise delegate to the plain-VT specialization for the actual field walk.
        VT result;
        if (auto ec = FieldConverter<VT>::from_simdjson(json_value, result); ec) {
            return ec;
        }
        out = std::move(result);
        return simdjson::SUCCESS;
    }

    /**
     * @brief Reads an optional nested-object field from TOML.
     * @param table the TOML table to read from.
     * @param field_name the key to look up.
     * @note A missing sub-table maps to `nullopt`, no error — but a present sub-table that
     * fails its own field extraction still propagates that error, same asymmetry pattern as
     * the other optional-of-X specializations in this file.
     * @return the decoded optional nested object; `nullopt` if the field's absent.
     */
    static std::expected<std::optional<VT>, std::string>
    from_toml(const toml::table &table, std::string_view field_name) {
        // No sub-table means no value at all — nullopt, not an error.
        const auto *sub_table = table[field_name].as_table();  // FIXME(clang-tidy): unchecked operator[], consider .at()
        if (sub_table == nullptr) {
            return std::nullopt;
        }
        // Present sub-table still has to decode clean, same asymmetry pattern as the other
        // optional-of-X specializations in this file.
        VT result;
        if (auto outcome = TomlParser::from_toml_impl(*sub_table, result); !outcome) {
            return std::unexpected{outcome.error()};
        }
        return result;
    }

    /// @brief Converts an optional nested object to its optional-NamedTuple rfl form.
    /// @param value the optional nested object to convert.
    /// @return `nullopt` if `value` is empty, else its NamedTuple representation.
    static rfl_type to_rfl(const std::optional<VT> &value) {
        // Empty stays empty, otherwise delegate to the plain-VT converter.
        if (!value) {
            return std::nullopt;
        }
        return FieldConverter<VT>::to_rfl(*value);
    }
    /// @brief Converts an rfl-facing optional NamedTuple back into an optional nested
    /// object.
    /// @param named_tuple the optional NamedTuple to read field values from.
    /// @return `nullopt` if `named_tuple` is empty, else the reconstructed nested object.
    static std::optional<VT> from_rfl(const rfl_type &named_tuple) {
        // Same empty-stays-empty motion in reverse.
        if (!named_tuple) {
            return std::nullopt;
        }
        return FieldConverter<VT>::from_rfl(*named_tuple);
    }
};

/**
 * @brief FieldConverter specialization for a vector of nested ISerializable objects —
 * decodes/encodes element by element, delegating each one to the plain-VT specialization.
 * @tparam VT the nested serializable element type; constrained to `ISerializable`.
 */
template <typename VT>
    requires ISerializable<VT>
struct FieldConverter<std::vector<VT>> {
    using InnerRfl = FieldConverter<VT>::rfl_type;
    using rfl_type = std::vector<InnerRfl>;

    /**
     * @brief Reads a vector-of-nested-objects field, decoding each array element via
     * `FieldConverter<VT>::from_simdjson`.
     * @param json_value the simdjson value to read from — must hold a JSON array.
     * @param out the destination vector, appended to via `push_back` (not cleared first).
     * @return a simdjson error_code — SUCCESS on a clean decode, propagates the first
     * element-level error otherwise.
     */
    static simdjson::error_code from_simdjson(simdjson::ondemand::value &json_value,
                                               std::vector<VT> &out) {
        // Must be a JSON array first.
        simdjson::ondemand::array json_array;
        if (auto ec = json_value.get_array().get(json_array); ec) {
            return ec;
        }
        // Decode each element as a full nested VT, one recursive call per element.
        for (auto element : json_array) {
            simdjson::ondemand::value element_value;
            if (auto ec = element.get(element_value); ec) {
                return ec;
            }
            VT result;
            if (auto ec = FieldConverter<VT>::from_simdjson(element_value, result); ec) {
                return ec;
            }
            out.push_back(std::move(result));
        }
        return simdjson::SUCCESS;
    }

    /**
     * @brief Reads a vector-of-nested-objects field from a TOML array of tables.
     * @param table the TOML table to read from.
     * @param field_name the key to look up.
     * @note A missing array resolves to an empty vector, no error — but every present
     * element must be a TOML table, no exceptions, that part does error hard.
     * @return the decoded elements in order, or an error message the moment any element
     * isn't a TOML table or fails its own field extraction.
     */
    static std::expected<std::vector<VT>, std::string>
    from_toml(const toml::table &table, std::string_view field_name) {
        // Missing array resolves to empty, no error.
        const auto *toml_array = table[field_name].as_array();  // FIXME(clang-tidy): unchecked operator[], consider .at()
        if (toml_array == nullptr) {
            return std::vector<VT>{};
        }
        std::vector<VT> result;
        result.reserve(toml_array->size());
        // Every present element must be a TOML table and must decode as a full VT — no
        // exceptions on either front.
        for (const auto &element : *toml_array) {
            const auto *sub_table = element.as_table();
            if (sub_table == nullptr) {
                return std::unexpected{
                    std::format("element of '{}' must be a TOML table", field_name)};
            }
            VT value;
            if (auto outcome = TomlParser::from_toml_impl(*sub_table, value); !outcome) {
                return std::unexpected{outcome.error()};
            }
            result.push_back(std::move(value));
        }
        return result;
    }

    /// @brief Converts a vector of nested objects to a vector of their NamedTuple forms.
    /// @param value the vector of nested objects to convert.
    /// @return a vector with one NamedTuple per element, same order.
    static rfl_type to_rfl(const std::vector<VT> &value) {
        // Reserve up front, then convert each element via the plain-VT converter.
        rfl_type result;
        result.reserve(value.size());
        for (const auto &element : value) {
            result.push_back(FieldConverter<VT>::to_rfl(element));
        }
        return result;
    }
    /// @brief Converts an rfl-facing vector of NamedTuples back into nested objects.
    /// @param named_tuple the vector of NamedTuples to convert.
    /// @return a vector with one reconstructed object per NamedTuple, same order.
    static std::vector<VT> from_rfl(const rfl_type &named_tuple) {
        // Same reserve-then-convert motion, in reverse.
        std::vector<VT> result;
        result.reserve(named_tuple.size());
        for (const auto &element : named_tuple) {
            result.push_back(FieldConverter<VT>::from_rfl(element));
        }
        return result;
    }
};

} // namespace serde

// ─── Per-field helpers + from_toml_impl + rfl_build_from ─────────────────────

namespace serde {

/**
 * @brief Extracts one field (described by `Fd`) from a simdjson object and writes it onto
 * `out` via the field's setter — the per-field unit that `tag_invoke` fans out over for
 * every field in `Serializable<T>::fields()`.
 * @tparam Fd the FieldDesc describing which field to extract.
 * @param json_object the simdjson object to look the field up in.
 * @param out the object to write the extracted field onto, mutated in place.
 * @note A missing field (`NO_SUCH_FIELD`) is treated as SUCCESS, not an error — this codec
 * is lenient about absent JSON keys by design, whatever default the setter's target already
 * holds just stays put.
 * @return a simdjson error_code — SUCCESS if the field was absent or decoded clean.
 */
template <typename Fd>
simdjson::error_code extract_simdjson_field(simdjson::ondemand::object &json_object,
                                            typename Fd::ClassType &out, Fd field_descriptor) {
    using VT = Fd::ValueType;
    // Look the field up by name — a missing key is fine, this codec is lenient about absent
    // JSON keys by design, so it counts as SUCCESS, not an error.
    simdjson::ondemand::value field_value;
    auto ec = json_object.find_field_unordered(Fd::name.string_view()).get(field_value);
    if (ec == simdjson::NO_SUCH_FIELD) {
        return simdjson::SUCCESS;
    }
    if (ec) {
        return ec;
    }
    // Present — decode through the field's own FieldConverter, then write it via the setter.
    VT value{};
    if (auto inner_error_code = FieldConverter<VT>::from_simdjson(field_value, value); inner_error_code) {
        return inner_error_code;
    }
    (out.*Fd::setter)(std::move(value));
    return simdjson::SUCCESS;
}

/**
 * @brief The TOML-side counterpart to extract_simdjson_field — extracts one field
 * (described by `Fd`) from a TOML table and writes it onto `out` via the field's setter.
 * @tparam Fd the FieldDesc describing which field to extract.
 * @param table the TOML table to look the field up in.
 * @param out the object to write the extracted field onto, mutated in place.
 * @note Unlike the simdjson twin, a missing field here is NOT automatically SUCCESS — that
 * call is deferred entirely to `FieldConverter<VT>::from_toml`, whose missing-field
 * behavior varies per specialization (some error, some default to empty/nullopt). Check the
 * specific FieldConverter before assuming leniency.
 * @return success, or an error message if the field's value fails to extract or convert.
 */
template <typename Fd>
std::expected<void, std::string> extract_toml_field(const toml::table &table,
                                                    typename Fd::ClassType &out, Fd field_descriptor) {
    using VT = Fd::ValueType;
    // Missing-vs-error leniency is entirely up to VT's own FieldConverter::from_toml — this
    // function just propagates whatever it decides.
    auto result = FieldConverter<VT>::from_toml(table, Fd::name.string_view());
    if (!result) {
        return std::unexpected{result.error()};
    }
    (out.*Fd::setter)(std::move(*result));
    return {};
}

/**
 * @brief Builds the rfl::NamedTuple representation of `object` — a thin named wrapper over
 * build_named_tuple, kept separate so rfl::Reflector<T>::from can call it with a clean,
 * concept-constrained entry point.
 * @tparam T the serializable type being converted; constrained to `ISerializable`.
 * @param object the instance to convert.
 * @return the NamedTuple built from `object`'s reflected fields.
 */
template <ISerializable T>
auto rfl_build_from(const T &object) {
    return build_named_tuple(object, Serializable<T>::fields());
}

} // namespace serde

export namespace serde {

/**
 * @brief Out-of-line definition of TomlParser::from_toml_impl (forward-declared at the top
 * of this file) — walks every reflected field of `T` and short-circuits on the first
 * extraction failure, bet.
 * @tparam T the serializable type being populated.
 * @param table the already-parsed TOML table to read fields from.
 * @param object the object to populate, mutated in place.
 * @note The `result && (result = extract_toml_field(...))` fold stops calling
 * `extract_toml_field` for remaining fields the instant one fails — `result` stays at that
 * first error, later fields (even ones that would've succeeded) are simply never attempted.
 * @return success, or the first field-extraction error encountered.
 */
template <ISerializable T>
std::expected<void, std::string> TomlParser::from_toml_impl(const toml::table &table, T &object) {
    std::expected<void, std::string> result{};
    // Fold over every field in order — `result && (...)` means the moment one extraction
    // fails, `result` latches onto that first error and every remaining field is skipped
    // entirely rather than attempted.
    std::apply(
        [&](auto... fields) {
            ((result ? (result = extract_toml_field(table, object, fields)) : result), ...);
        },
        Serializable<T>::fields());
    return result;
}

} // namespace serde

// ─── rfl::Reflector + simdjson::tag_invoke ────────────────────────────────────
// Exported so :json can use them when instantiating to_json/from_json.

export namespace rfl {

/**
 * @brief rfl's own Reflector customization point, specialized for every ISerializable T —
 * this is what lets rfl's generic machinery (rfl::json::write, rfl::toml::write, etc.) work
 * on this codebase's FieldDesc-based types without them ever inheriting from or wrapping
 * anything rfl-specific.
 * @tparam T the serializable type being reflected; constrained to `serde::ISerializable`.
 */
template <serde::ISerializable T>
struct Reflector<T> {
    using ReflType = decltype(serde::rfl_build_from(std::declval<const T &>()));

    /**
     * @brief Reconstructs a T from its NamedTuple representation — called by rfl whenever
     * it needs to materialize a T out of reflected data (e.g. deserializing).
     * @param named_tuple the NamedTuple to read field values from.
     * @return the reconstructed T.
     */
    static T to(const ReflType &named_tuple) noexcept {
        // Default-construct T, then let apply_named_tuple_to fill in every reflected field.
        T object;
        serde::apply_named_tuple_to(object, named_tuple, serde::Serializable<T>::fields());
        return object;
    }

    /**
     * @brief Builds the NamedTuple representation of a T — called by rfl whenever it needs
     * to walk a T's fields (e.g. serializing).
     * @param object the instance to convert.
     * @return the NamedTuple built from `object`'s reflected fields.
     */
    static ReflType from(const T &object) { return serde::rfl_build_from(object); }
};

} // namespace rfl

export namespace simdjson {

/**
 * @brief Definition of the ADL customization point forward-declared at the top of this
 * file — the actual entry point simdjson calls (directly, or via FieldConverter<VT
 * ISerializable>::from_simdjson recursing into a nested type) to deserialize any
 * ISerializable T out of a JSON object.
 * @tparam V the simdjson value-like type being deserialized from.
 * @tparam T the serializable type being deserialized into; constrained to
 * `serde::ISerializable`.
 * @param json_value the simdjson value to read — must hold a JSON object.
 * @param object the object to populate, mutated in place.
 * @note Fields are extracted in `Serializable<T>::fields()` order, and the fold
 * short-circuits on the first failure — once `result != SUCCESS`, every remaining field is
 * skipped entirely rather than attempted, no cap.
 * @return a simdjson error_code — SUCCESS if every field decoded clean.
 */
template <typename V, serde::ISerializable T>
error_code tag_invoke(deserialize_tag tag, V &json_value, T &object) {
    // Must actually be a JSON object before any field extraction can start.
    ondemand::object json_object;
    if (auto ec = json_value.get_object().get(json_object); ec) {
        return ec;
    }

    // Fold across every reflected field, in Serializable<T>::fields() order — once `result`
    // stops being SUCCESS, remaining fields are skipped entirely rather than attempted.
    error_code result = SUCCESS;
    std::apply(
        [&](auto... fields) {
            ((result == SUCCESS
                  ? (result = serde::extract_simdjson_field(json_object, object, fields))
                  : SUCCESS),
             ...);
        },
        serde::Serializable<T>::fields());
    return result;
}

} // namespace simdjson

