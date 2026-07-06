export module core_client;

import std;
import interfaces;

export namespace core::client {

class Client {
  public:
    Client() = delete;
    ~Client() = default;

    Client(const Client &) = delete;
    Client &operator=(const Client &) = delete;

    Client(Client &&) = default;
    Client &operator=(Client &&) = default;

    static Client get(std::string_view path) { return {"GET", path}; }
    static Client head(std::string_view path) { return {"HEAD", path}; }
    static Client post(std::string_view path) { return {"POST", path}; }
    static Client put(std::string_view path) { return {"PUT", path}; }
    static Client del(std::string_view path) { return {"DELETE", path}; }
    static Client patch(std::string_view path) { return {"PATCH", path}; }
    static Client options(std::string_view path) { return {"OPTIONS", path}; }

    Client &&with_runtime(interfaces::IClient &client) && {
        m_client = client;
        return std::move(*this);
    }

    Client &&on_receive(interfaces::io::ReceiveDispatchFn &&func) && {
        m_receive_dispatch_fn = std::move(func);
        return std::move(*this);
    }

    void set_runtime(interfaces::IClient &client) { m_client = client; }

    void set_on_receive(interfaces::io::ReceiveDispatchFn &&func) {
        m_receive_dispatch_fn = std::move(func);
    }

    void add_header(std::string_view key, std::string_view value) {
        m_base_request.add_header(key, value);
    }

    // TODO: add_body has to append to utils::buffering::BuggerView via get_body()
    void add_body(std::string_view body) {}


    void send() {
        if (!m_client.has_value()) {
            throw std::runtime_error("Please set runtime first");
        }
        auto &client = m_client.value().get();
        client.send(m_base_request);
    }


  private:
    Client(std::string_view method, std::string_view path)
        : m_path{path}, m_base_request{}, m_client{} {
        m_base_request.add_header("method", method);
        m_base_request.add_header("authority", m_path);
    }

    std::string m_path;
    interfaces::io::IRequest m_base_request;
    interfaces::io::ReceiveDispatchFn m_receive_dispatch_fn;
    std::optional<std::reference_wrapper<interfaces::IClient>> m_client;
};

} // namespace core::client
