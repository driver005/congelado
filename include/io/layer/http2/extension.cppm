export module io_layer_http2:extension;

import std;
import interfaces;
import :settings;

export namespace io::layer::http2 {

/**
 * @brief Extra functionality bolted onto the HTTP/2 handler (`Session`). A `Session` holds any
 * number of these and calls their virtual hooks on every operation it performs — connection
 * lifecycle, settings, ping, window updates, stream lifecycle, headers, requests/responses,
 * data, and unknown frames. Hooks are pure observe/intercept points: an extension reacts to
 * what it's handed, and the only "action" it can take is mutating a reference passed in (e.g.
 * adding vendor setting ids to the outgoing `Settings` in `on_local_settings`). Extensions do
 * NOT own a write-back channel and do NOT take over streams. Every hook defaults to an inert
 * no-op, so an extension overrides only the ones it needs; `name()` is the sole pure virtual.
 */
class IHttpExtension {
  public:
    /**
     * @brief Virtual dtor so concrete extensions clean up right through the base pointer.
     */
    virtual ~IHttpExtension() = default;
    IHttpExtension() = default;
    IHttpExtension(const IHttpExtension &) = delete;
    IHttpExtension &operator=(const IHttpExtension &) = delete;
    IHttpExtension(IHttpExtension &&) = delete;
    IHttpExtension &operator=(IHttpExtension &&) = delete;

    /**
     * @brief This extension's self-identifying name (e.g. "websocket", "grpc").
     * @return the extension's name.
     */
    [[nodiscard]] virtual std::string_view name() const noexcept = 0;

    // ── Connection ──────────────────────────────────────────────────────────

    /**
     * @brief A new connection/session just opened.
     */
    virtual void on_connection_open() {}

    /**
     * @brief The connection is being torn down (GOAWAY sent or transport closed).
     * @param error_code the RFC 9113 error code the connection closed with.
     */
    virtual void on_connection_close(std::uint32_t error_code) {}

    /**
     * @brief A PING frame was received.
     * @param is_ack true if it was a PING ACK (reply to our own ping), false if the peer is
     * pinging us.
     */
    virtual void on_ping(bool is_ack) {}

    /**
     * @brief A WINDOW_UPDATE frame was received.
     * @param stream_id the stream it applied to, or 0 for a connection-level window update.
     * @param increment the flow-control window increment.
     */
    virtual void on_window_update(std::uint32_t stream_id, std::uint32_t increment) {}

    /**
     * @brief Our own SETTINGS were acknowledged by the peer (SETTINGS ACK received).
     */
    virtual void on_settings_ack() {}

    /**
     * @brief The local SETTINGS are about to be sent (handshake). The extension may add its own
     * vendor/extension setting ids by mutating `local` (e.g.
     * `local.add_local_setting_override(0x8, 1)` for RFC 8441).
     * @param local the outgoing local settings to (optionally) mutate before they're serialized.
     */
    virtual void on_local_settings(Settings &local) {}

    /**
     * @brief The peer's SETTINGS were received and applied — observe (including vendor ids via
     * `remote.get_vendor_settings()`).
     * @param remote the peer's just-applied settings.
     */
    virtual void on_remote_settings(const Settings &remote) {}

    // ── Stream lifecycle ────────────────────────────────────────────────────

    /**
     * @brief A new stream was created.
     * @param stream_id the new stream's id.
     */
    virtual void on_stream_open(std::uint32_t stream_id) {}

    /**
     * @brief The peer reset a stream (RST_STREAM received).
     * @param stream_id the reset stream's id.
     * @param error_code the RFC 9113 error code the peer reported.
     */
    virtual void on_stream_reset(std::uint32_t stream_id, std::uint32_t error_code) {}

    /**
     * @brief A stream reached its graceful teardown (closed / tombstoned).
     * @param stream_id the closed stream's id.
     */
    virtual void on_stream_close(std::uint32_t stream_id) {}

    // ── Headers / requests / responses (delivered decoded) ──────────────────

    /**
     * @brief One header field was decoded onto a stream's request/trailers. Fired per field
     * after the HEADERS block finishes decoding (HPACK is a shared, stateful codec, so
     * extensions can't decode raw header bytes themselves — they get the decoded fields).
     * @param stream_id the stream the field belongs to.
     * @param name the header field name.
     * @param value the header field value.
     */
    virtual void on_header_added(std::uint32_t stream_id, std::string_view name,
                                 std::string_view value) {}

    /**
     * @brief A stream's first HEADERS block finished decoding into a request (server side; on
     * the client, an incoming response decodes here too — the codec is request-wired).
     * @param stream_id the stream the request arrived on.
     * @param request the decoded request.
     */
    virtual void on_request_incoming(std::uint32_t stream_id, interfaces::io::IRequest &request) {}

    /**
     * @brief A request is about to be framed and sent out (client side).
     * @param stream_id the stream the request is going out on.
     * @param request the request being sent (may be mutated before framing).
     */
    virtual void on_request_outgoing(std::uint32_t stream_id, interfaces::io::IRequest &request) {}

    /**
     * @brief A response is about to be framed and sent out (server side).
     * @param stream_id the stream the response is going out on.
     * @param response the response being sent (may be mutated before framing).
     */
    virtual void on_response_outgoing(std::uint32_t stream_id,
                                      interfaces::io::IResponse &response) {}

    /**
     * @brief A stream's trailers (a second HEADERS block after DATA) finished decoding.
     * @param stream_id the stream the trailers arrived on.
     * @param trailers the decoded trailer fields.
     */
    virtual void on_trailers(std::uint32_t stream_id, interfaces::io::IRequest &trailers) {}

    // ── Raw frames ──────────────────────────────────────────────────────────

    /**
     * @brief A frame the core doesn't turn into headers finished being read — DATA frames and
     * any unknown/custom frame type (RFC 9113 §4.1 ignore-and-discard). Raw payload bytes.
     * @param stream_id the stream the frame arrived on (0 for connection-level).
     * @param type the raw wire frame-type byte.
     * @param flags the frame's flag byte.
     * @param payload the raw frame payload — only valid for the duration of this call.
     * @param end_stream true if the frame carried END_STREAM.
     */
    virtual void on_frame_complete(std::uint32_t stream_id, std::uint8_t type, std::uint8_t flags,
                                   std::span<const std::byte> payload, bool end_stream) {}
};

/**
 * @brief Holds every registered `IHttpExtension` for one process — a plain fan-out container,
 * instance-owned by `congelado::heart::AppContext` and threaded by reference into `Session`.
 * The handler (`Session`/`Stream`/`Handshake`) iterates `get_extensions()` and calls the hooks
 * itself; the registry keeps no dispatch logic of its own. Empty registry (no extension plugin
 * configured) ⇒ every hook loop is a zero-iteration no-op, identical to the pre-extension path.
 */
class HttpExtensionRegistry {
  public:
    /**
     * @brief Registers an extension so it starts receiving hook calls.
     * @note No-op if `extension` is null. Once registered there's no unregister.
     * @param extension the extension instance to add.
     */
    void add_extension(std::shared_ptr<IHttpExtension> extension) {
        if (extension) {
            m_extensions.push_back(std::move(extension));
        }
    }

    /**
     * @brief Checks whether any extension is registered.
     * @return true if at least one extension is registered.
     */
    [[nodiscard]] bool has_extensions() const noexcept { return !m_extensions.empty(); }

    /**
     * @brief Gets every registered extension, in registration order.
     * @return the registered extensions.
     */
    [[nodiscard]] const std::vector<std::shared_ptr<IHttpExtension>> &
    get_extensions() const noexcept {
        return m_extensions;
    }

    /**
     * @brief Invokes `fn` on every registered extension, in registration order — the single
     * fan-out primitive the handler uses to fire a hook, e.g.
     * `registry.for_each([&](auto &e){ e->on_stream_open(id); })`. Zero iterations (hence a
     * no-op) when nothing is registered.
     * @tparam Fn a callable taking the registered `std::shared_ptr<IHttpExtension>` (by ref).
     * @param fn the functor to run against each extension.
     */
    template <typename Fn>
    void for_each(Fn &&fn) const {
        for (const auto &extension : m_extensions) {
            fn(extension);
        }
    }

  private:
    std::vector<std::shared_ptr<IHttpExtension>> m_extensions;
};

} // namespace io::layer::http2
