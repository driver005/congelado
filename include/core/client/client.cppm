export module core_client;

import std;
import interfaces;

export namespace core::client {

template <typename Derived>
class ClientHandler {
  public:
    using ResponseFn = std::function<void(int status, std::string body)>;

    // send<Protocol> — Protocol at call site, dispatches to Derived::do_send
    template <typename Protocol>
    void send(interfaces::IRequest<Protocol>& req, ResponseFn cb) {
        static_cast<Derived&>(*this).do_send(req, std::move(cb));
    }

    // Generic factory — method as runtime string; body optional
    template <typename Protocol>
    [[nodiscard]] auto request(std::string_view method, std::string_view path,
                 std::string_view body = {}) {
        return static_cast<Derived&>(*this).template make_request<Protocol>(method, path, body);
    }

    // Verb convenience — all body-free; use request<P> directly for body
    template <typename Protocol>
    [[nodiscard]] auto get(std::string_view path)   { return request<Protocol>("GET",    path); }
    template <typename Protocol>
    [[nodiscard]] auto post(std::string_view path)  { return request<Protocol>("POST",   path); }
    template <typename Protocol>
    [[nodiscard]] auto put(std::string_view path)   { return request<Protocol>("PUT",    path); }
    template <typename Protocol>
    [[nodiscard]] auto del(std::string_view path)   { return request<Protocol>("DELETE", path); }
    template <typename Protocol>
    [[nodiscard]] auto patch(std::string_view path) { return request<Protocol>("PATCH",  path); }
    template <typename Protocol>
    [[nodiscard]] auto head(std::string_view path)  { return request<Protocol>("HEAD",   path); }
};

} // namespace core::client
