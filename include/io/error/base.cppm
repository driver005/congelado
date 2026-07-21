module;

#include <openssl/err.h>

export module io_error:base;

import std;

export namespace io::error {

template <bool UseException = false, bool AbortOnError = false, typename Fn = std::monostate>
    requires(std::is_same_v<Fn, std::monostate> || std::invocable<Fn, std::string_view, void *>)
inline void handle_error(std::string_view message, Fn &&callback = {}, void *ctx = nullptr) {
    // UseException mode just throws and bails, no cap — nothing below this runs.
    if constexpr (UseException) {
        throw std::runtime_error(std::string(message));
    }

    // Otherwise route the message: a real callback gets it, no callback falls back to stderr.
    if constexpr (!std::is_same_v<std::decay_t<Fn>, std::monostate>) {
        std::forward<Fn>(callback)(message, ctx);
    } else {
        std::println(stderr, "Default Log: {}", message);
    }

    // AbortOnError is the last word — if it's set, this never returns to the caller.
    if constexpr (AbortOnError) {
        std::abort();
    }
}


class TlsError : std::runtime_error {
  public:
    /**
     * @brief Builds a TlsError by pulling the actual OpenSSL error off the thread-local error
     * queue and tacking it onto `ctx` — no cap, this is why the message dynamically differs even
     * though the ctor signature looks static.
     * @param ctx short context string prefixed onto the pulled OpenSSL error text.
     */
    explicit TlsError(std::string_view ctx) : std::runtime_error(make_msg(ctx)) {}

  private:
    /**
     * @brief Drains the OpenSSL thread-local error queue via `ERR_get_error()`/
     * `ERR_error_string_n()` and stitches it onto `ctx` to build the final exception message.
     * @warning Only grabs one error off the queue — OpenSSL can stack multiple errors per
     * operation, so if something upstream threw more than one this only surfaces the first.
     * @param ctx short context string prefixed onto the resolved OpenSSL error text.
     * @return the assembled `"<ctx>: <openssl error>"` message.
     */
    static std::string make_msg(std::string_view ctx) {
        char buf[256];
        ::ERR_error_string_n(::ERR_get_error(), buf, sizeof(buf));  // FIXME(clang-tidy): array-to-pointer decay
        return std::string(ctx) + ": " + buf;  // FIXME(clang-tidy): array-to-pointer decay
    }
};

} // namespace io::error
