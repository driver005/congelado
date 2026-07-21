export module interfaces:cache_codec;

import std;

export namespace interfaces {

template <typename T>
class ICacheCodec {
  public:
    /**
     * @brief Virtual dtor, default's chill — polymorphic codecs clean up fine through the base,
     * no extra motion needed.
     */
    virtual ~ICacheCodec() = default;

    /**
     * @brief Copy ctor, defaulted — no data members of its own, so member-wise copy is trivially
     * correct.
     */
    ICacheCodec(ICacheCodec const &) = default;
    /**
     * @brief Copy assignment, defaulted alongside the copy ctor for the same reason.
     */
    ICacheCodec &operator=(ICacheCodec const &) = default;
    /**
     * @brief Move ctor, defaulted — same story, nothing owned that needs special handling.
     */
    ICacheCodec(ICacheCodec &&) = default;
    /**
     * @brief Move assignment, defaulted to round out the set.
     */
    ICacheCodec &operator=(ICacheCodec &&) = default;

    /**
     * @brief Derives the cache key a value of type `T` is supposed to live under. Every
     * implementer's gotta have opinions here — this is the whole vibe of the codec.
     * @param T the value to build a key for.
     * @return the cache key string for this value.
     */
    [[nodiscard]] virtual std::string key(T const &) const = 0;
    /**
     * @brief Turns a `T` into the string blob that actually gets stashed in the cache. This is
     * the encode half of the round trip, decode() better be able to undo it clean.
     * @param T the value to encode.
     * @return the encoded string ready to hand off to the cache backend.
     */
    [[nodiscard]] virtual std::string encode(T const &) const = 0;

    /**
     * @brief Reverses encode() — takes the raw cached string and writes the reconstructed value
     * into `out`. This is the make-or-break half; if encode/decode don't round-trip clean the
     * whole cache layer's cooked.
     * @param value the raw string pulled back from the cache.
     * @param[out] out gets overwritten with the decoded value. No return code, no throwing — if
     * the value's junk that's on the implementer to eat quietly, not blow up the caller.
     */
    virtual void decode(std::string_view value, T &out) const noexcept = 0;
};

template <typename Codec, typename T>
concept CacheCodec = std::derived_from<Codec, ICacheCodec<T>>;

} // namespace interfaces
