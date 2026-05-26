export module interfaces:cache_codec;

import std;

export namespace interfaces {

template <typename T>
class ICacheCodec {
  public:
    virtual ~ICacheCodec() = default;

    [[nodiscard]] virtual std::string key(T const &) const = 0;
    [[nodiscard]] virtual std::string encode(T const &) const = 0;

    virtual void decode(std::string_view value, T &out) const = 0;
};

template <typename Codec, typename T>
concept CacheCodec = std::derived_from<Codec, ICacheCodec<T>>;

} // namespace interfaces
