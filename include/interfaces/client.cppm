export module interfaces:client;

import std;
import io_shared;
import shared;
import :io;

export namespace interfaces {

class IClient {
  public:
    /**
     * @brief Virtual dtor, default's good — polymorphic cleanup through the base ptr, zero leaks,
     * zero drama.
     */
    virtual ~IClient() = default;

    // Disallow copying to ensure clean polymorphic handling
    /**
     * @brief Deleted, on purpose — copying a polymorphic client is a one-way ticket to object
     * slicing. Hard no, not happening, don't even try it.
     */
    IClient(const IClient &) = delete;
    /**
     * @brief Deleted — same L as the copy ctor above, don't even think about it.
     */
    IClient &operator=(const IClient &) = delete;

    /**
     * @brief Defaulted move ctor — sliding a client around ownership-wise is all good, copying
     * it is the thing that's banned, not moving it.
     */
    IClient(IClient &&) noexcept = default;
    /**
     * @brief Defaulted move assign — same energy as the move ctor, still totally fine.
     */
    IClient &operator=(IClient &&) noexcept = default;

    /**
     * @brief The hook every implementer has to fire when the underlying connection actually
     * comes up — wires in how to send bytes out, how to slam the connection shut, and hands
     * back the callback that's gonna catch every incoming read from here on out.
     * @param send callback the implementer calls to push bytes out over the wire.
     * @param close callback the implementer calls to tear the connection down.
     * @return the callback that should get invoked whenever data comes in on this connection.
     */
    virtual shared::ReadCallback on_connect(shared::SendCallback send,
                                            shared::CloseCallback close) = 0;

    /**
     * @brief Ships `request` out over this client's connection. That's it, that's the motion.
     * @param request the request to send — implementer decides how it gets serialized/dispatched.
     */
    virtual void send(io::IRequest &request) = 0;

    /**
     * @brief Builds a fresh request of whatever concrete type this client's protocol actually
     * deals in (e.g. io::layer::http2::HttpRequest), tagged with `stream_id`. This is the
     * protocol-agnostic escape hatch: `IRequest`'s own base-class virtuals all abort — only a
     * concrete subtype is ever actually usable — but generic code holding just an `IClient&`
     * has no way to name that concrete type. Calling this instead of constructing one directly
     * dispatches to whichever concrete client is actually behind the reference, no cast needed
     * anywhere in the generic caller.
     * @param stream_id the stream id to tag the new request with.
     * @return a heap-allocated request of this client's concrete protocol type.
     */
    [[nodiscard]] virtual std::unique_ptr<io::IRequest> create_request(std::uint32_t stream_id) = 0;

  protected:
    /**
     * @brief Protected default ctor — this base only ever gets built through a concrete
     * subclass, no spinning up a bare `IClient` standalone, that's not the move.
     */
    IClient() = default;
};

} // namespace interfaces
