export module serde:cache;

import :core;
import :converter;
import :json;
import std;

namespace serde {

/**
 * @brief Stringifies a single field's value via its FieldConverter's rfl representation —
 * the reflected type is either already a std::string (passed through untouched) or
 * something std::format can handle, no cap.
 * @tparam VT the field's declared value type.
 * @param value the field value to stringify.
 * @return the string form of `value`, used to build primary-key strings for cache keys.
 */
template <typename VT>
std::string field_value_to_string(const VT &value) {
    using RflType = FieldConverter<VT>::rfl_type;
    RflType rfl_value = FieldConverter<VT>::to_rfl(value);
    // Already a string? Hand it back as-is — otherwise std::format does the stringify work.
    if constexpr (std::same_as<RflType, std::string>) {
        return rfl_value;
    } else {
        return std::format("{}", rfl_value);
    }
}

} // namespace serde

export namespace serde {

class Cache {
  public:
    /**
     * @brief Extracts the string form of `value`'s primary-key field by walking every
     * reflected field and matching on `options.m_db.m_primary_key` — lowkey a linear scan
     * disguised as a fold expression.
     * @tparam T the connectable (serializable + table-named) type being cached.
     * @param value the instance to pull the primary key off of.
     * @warning If no field is marked `primary_key` in `T`'s FieldOptionsDb, this quietly
     * returns an empty string instead of erroring — no cap, that's a silent footgun for any
     * type that forgot to mark a PK.
     * @return the primary key's value, stringified.
     */
    template <IConnectable T>
    [[nodiscard]] static std::string pk_string(const T &value) {
        std::string result;
        // Fold over every reflected field — whichever one's flagged primary_key wins, lowkey a
        // linear scan wearing a fold-expression trenchcoat.
        std::apply(
            [&](auto... fields) {
                ([&](auto field) {
                    if constexpr (decltype(field)::options.m_db.m_primary_key) {
                        result = field_value_to_string((value.*decltype(field)::getter)());
                    }
                }(fields), ...);
            },
            Serializable<T>::fields());
        return result;
    }

    /**
     * @brief Builds the cache key for a live instance — `"<table_name>:<pk_string>"`.
     * @tparam T the connectable type being cached.
     * @param value the instance to key.
     * @return the `table:pk` cache key, ready to hand to whatever cache backend's in play.
     */
    template <IConnectable T>
    [[nodiscard]] static std::string cache_key(const T &value) {
        return std::format("{}:{}", Serializable<T>::table_name(), pk_string(value));
    }

    /**
     * @brief Builds the cache key from a raw primary-key string, no instance needed —
     * handy for lookups where you've only got the id, bet.
     * @tparam T the connectable type whose table name to key under.
     * @param pk_value the already-known primary-key value.
     * @return the `table:pk` cache key.
     */
    template <IConnectable T>
    [[nodiscard]] static std::string cache_key(std::string_view pk_value) {
        return std::format("{}:{}", Serializable<T>::table_name(), pk_value);
    }

    /**
     * @brief Serializes `value` to the string that actually gets stored under the cache key —
     * just JSON-encodes it via Json::encode, no separate cache-specific format.
     * @tparam T the connectable type being cached.
     * @param value the instance to serialize.
     * @return the JSON-encoded cache payload.
     */
    template <IConnectable T>
    [[nodiscard]] static std::string cache_value(const T &value) {
        return Json::encode(value);
    }
};

} // namespace serde
