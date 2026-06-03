export module serde:cache;

import :core;
import :converter;
import :json;
import std;

namespace serde {

template <typename VT>
std::string field_value_to_string(const VT &value) {
    using RflType = typename FieldConverter<VT>::rfl_type;
    RflType rfl_value = FieldConverter<VT>::to_rfl(value);
    if constexpr (std::same_as<RflType, std::string>)
        return rfl_value;
    else
        return std::format("{}", rfl_value);
}

} // namespace serde

export namespace serde {

class Cache {
  public:
    template <IConnectable T>
    [[nodiscard]] static std::string pk_string(const T &value) {
        std::string result;
        std::apply(
            [&](auto... fds) {
                ([&](auto fd) {
                    if constexpr (decltype(fd)::options.db.primary_key)
                        result = field_value_to_string((value.*decltype(fd)::getter)());
                }(fds), ...);
            },
            Serializable<T>::fields());
        return result;
    }

    template <IConnectable T>
    [[nodiscard]] static std::string cache_key(const T &value) {
        return std::format("{}:{}", Serializable<T>::table_name(), pk_string(value));
    }

    template <IConnectable T>
    [[nodiscard]] static std::string cache_key(std::string_view pk_value) {
        return std::format("{}:{}", Serializable<T>::table_name(), pk_value);
    }

    template <IConnectable T>
    [[nodiscard]] static std::string cache_value(const T &value) {
        return Json::encode(value);
    }
};

} // namespace serde
