export module interfaces:db_codec;

import std;

export namespace interfaces {

template <typename T>
class IDbCodec {
  public:
    /**
     * @brief Virtual dtor, default's fine — db codecs clean up fine through the base pointer,
     * nothing fancy needed, no cap.
     */
    virtual ~IDbCodec() = default;
    IDbCodec() = default;
    IDbCodec(const IDbCodec &) = delete;
    IDbCodec &operator=(const IDbCodec &) = delete;
    IDbCodec(IDbCodec &&) = delete;
    IDbCodec &operator=(IDbCodec &&) = delete;

    /**
     * @brief Encodes a `T` into whatever payload the backend needs to run a query/select for it.
     * Four encode_* methods below, four different jobs — don't mix them up.
     * @param T the value driving the query.
     * @return the encoded query payload.
     */
    [[nodiscard]] virtual std::string encode_query(T const &) const = 0;
    /**
     * @brief Encodes a `T` into the payload needed to insert it into the backend, fresh record,
     * no cap.
     * @param T the value to insert.
     * @return the encoded insert payload.
     */
    [[nodiscard]] virtual std::string encode_insert(T const &) const = 0;
    /**
     * @brief Encodes a `T` into the payload needed to update an existing record with it — no
     * new row, just refreshing what's already there.
     * @param T the value carrying the update.
     * @return the encoded update payload.
     */
    [[nodiscard]] virtual std::string encode_update(T const &) const = 0;
    /**
     * @brief Encodes a `T` into the payload needed to remove the matching record from the
     * backend. One-way trip, that record's gone after this.
     * @param T the value identifying what to remove.
     * @return the encoded remove payload.
     */
    [[nodiscard]] virtual std::string encode_remove(T const &) const = 0;

    /**
     * @brief Reverses the encode_* calls — takes a raw backend result and writes the
     * reconstructed value into `out`. This is the other half of the round trip, gotta stay in
     * sync with whatever encode_query() actually asked for.
     * @param result the raw payload that came back from the db backend.
     * @param[out] out gets overwritten with the decoded value. Noexcept, so bad data needs to
     * get handled quietly by the implementer — no throwing your way out of this one.
     */
    virtual void decode(std::string_view result, T &out) const noexcept = 0;
};

template <typename Codec, typename T>
concept DbCodec = std::derived_from<Codec, IDbCodec<T>>;

} // namespace interfaces
