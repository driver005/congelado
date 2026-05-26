export module interfaces:db_codec;

import std;

export namespace interfaces {

template <typename T>
class IDbCodec {
  public:
    virtual ~IDbCodec() = default;

    [[nodiscard]] virtual std::string encode_query(T const &) const = 0;
    [[nodiscard]] virtual std::string encode_insert(T const &) const = 0;
    [[nodiscard]] virtual std::string encode_update(T const &) const = 0;
    [[nodiscard]] virtual std::string encode_remove(T const &) const = 0;

    virtual void decode(std::string_view result, T &out) const noexcept = 0;
};

template <typename Codec, typename T>
concept DbCodec = std::derived_from<Codec, IDbCodec<T>>;

} // namespace interfaces
