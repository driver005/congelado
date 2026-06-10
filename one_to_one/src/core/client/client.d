module core.client.client;
@nogc nothrow:

import interfaces.interfaces : ClientConcept, ReceiveDispatchFn;
import interfaces.request    : IRequest;
import util.optional         : Optional;
import util.alloc            : make, dispose;

// PORT-NOTE: C++ template<ClientConcept ClientType, typename Protocol>
// → D class Client(ClientType, Protocol) with constraints
// PORT-NOTE: std::optional<std::reference_wrapper<ClientType>> m_client
//   → Optional!(ClientType*) pointer + null flag
// PORT-NOTE: std::runtime_error → assert for @nogc

class Client(ClientType, Protocol)
    if (ClientConcept!ClientType)
{
  public:
    @disable this();

    ~this() {
        dispose(m_base_request);
    }

    @disable this(this); // No copy

    // Move semantics are default in D (reference semantics for classes)

    static Client!(ClientType, Protocol) get(const(char)[] path) {
        return make!(Client!(ClientType, Protocol))("GET", path);
    }
    static Client!(ClientType, Protocol) head(const(char)[] path) {
        return make!(Client!(ClientType, Protocol))("HEAD", path);
    }
    static Client!(ClientType, Protocol) post(const(char)[] path) {
        return make!(Client!(ClientType, Protocol))("POST", path);
    }
    static Client!(ClientType, Protocol) put(const(char)[] path) {
        return make!(Client!(ClientType, Protocol))("PUT", path);
    }
    static Client!(ClientType, Protocol) del(const(char)[] path) {
        return make!(Client!(ClientType, Protocol))("DELETE", path);
    }
    static Client!(ClientType, Protocol) patch(const(char)[] path) {
        return make!(Client!(ClientType, Protocol))("PATCH", path);
    }
    static Client!(ClientType, Protocol) options(const(char)[] path) {
        return make!(Client!(ClientType, Protocol))("OPTIONS", path);
    }

    Client!(ClientType, Protocol) with_runtime(ref ClientType client) {
        m_client = &client;
        return this;
    }

    Client!(ClientType, Protocol) on_receive(ReceiveDispatchFn func) {
        m_receive_dispatch_fn = func;
        return this;
    }

    void set_runtime(ref ClientType client)      { m_client = &client; }
    void set_on_receive(ReceiveDispatchFn func)  { m_receive_dispatch_fn = func; }

    void add_header(const(char)[] key, const(char)[] value) {
        m_base_request.add_header(key, value);
    }

    // TODO: add_body has to append to utils::buffering::BuggerView via get_body()
    void add_body(const(char)[] body_) {}

    void send() {
        if (m_client is null)
            assert(false, "Please set runtime first");
        auto requester = m_client.on_send();
        requester(m_base_request);
    }

  private:
    this(const(char)[] method, const(char)[] path) {
        m_path         = path;
        m_base_request = make!(IRequest!Protocol)();
        m_client       = null;
        m_base_request.add_header("method", method);
        m_base_request.add_header("authority", m_path);
    }

    const(char)[]        m_path;
    IRequest!Protocol    m_base_request;
    ReceiveDispatchFn    m_receive_dispatch_fn;
    ClientType*          m_client; // null = unset; PORT-NOTE: optional ref wrapper
}
