module;

#include <openssl/err.h>

export module io_error:base;

import std;

export namespace transport::error {

template <bool use_exception = false, bool abort_on_error = false, typename Fn = std::monostate>
    requires(std::is_same_v<Fn, std::monostate> || std::invocable<Fn, std::string_view, void *>)
inline void handle_error(std::string_view message, Fn &&callback = {}, void *ctx = nullptr) {
    if constexpr (use_exception) {
        throw std::runtime_error(std::string(message));
    }

    if constexpr (!std::is_same_v<std::decay_t<Fn>, std::monostate>) {
        callback(message, ctx);
    } else {
        std::println(stderr, "Default Log: {}", message);
    }

    if constexpr (abort_on_error) {
        std::abort();
    }
}


class TlsError : std::runtime_error {
  public:
    explicit TlsError(std::string_view ctx) : std::runtime_error(make_msg(ctx)) {}

  private:
    static std::string make_msg(std::string_view ctx) {
        char buf[256];
        ::ERR_error_string_n(::ERR_get_error(), buf, sizeof(buf));
        return std::string(ctx) + ": " + buf;
    }
};

} // namespace transport::error
