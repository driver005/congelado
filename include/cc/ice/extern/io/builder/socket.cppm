// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/io/socket.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/io/socket.h"
#include "include/c/intern/status.h"
#include "include/c/intern/tstring.h"

export module cc_ice_extern_io_builder:socket;

import std;
import cc_ice_extern_io_sonic;
import cc_ice_intern_sonic;

export namespace ice::builder {

class TF_SocketOps
{
public:
    explicit TF_SocketOps(
        const ::TF_SocketOps* TF_SocketOps_ops,
        const ::TF_StatusOps* Status_ops,
        const ::TF_StringOps* String_ops
    ) noexcept :
        m_handle{.plugin_data = this}
    {
        m_TF_SocketOps_ops = TF_SocketOps_ops;
        m_Status_ops = Status_ops;
        m_String_ops = String_ops;
    }

    TF_SocketOps(const TF_SocketOps&) = delete;
    TF_SocketOps& operator=(const TF_SocketOps&) = delete;

    static TF_SocketOps& from_handle(void* ctx) noexcept
    {
        return *static_cast<TF_SocketOps*>(ctx);
    }

    template<typename HandleT>
    static TF_SocketOps& from_handle(HandleT* handle) noexcept
    {
        return *static_cast<TF_SocketOps*>(handle->plugin_data);
    }

    virtual ~TF_SocketOps() = default;
    virtual void destroy() noexcept = 0;
    virtual void get_name(const ice::sonic::String& out_name) noexcept = 0;
    virtual void close_socket() noexcept = 0;
    virtual void set_non_blocking(int enabled, const ice::sonic::Status& out_status) noexcept = 0;
    virtual void set_reuse_address(int enabled, const ice::sonic::Status& out_status) noexcept = 0;
    virtual void set_broadcast(int enabled, const ice::sonic::Status& out_status) noexcept = 0;
    virtual void set_tcp_no_delay(int enabled, const ice::sonic::Status& out_status) noexcept = 0;
    virtual void load_certificate(
        const ice::sonic::String& cert_path,
        const ice::sonic::String& key_path,
        const ice::sonic::Status& out_status
    ) noexcept = 0;
    virtual void generate_certificate(
        const ice::sonic::String& cert_path,
        const ice::sonic::String& key_path,
        const ice::sonic::Status& out_status
    ) noexcept = 0;
    virtual void set_verify_peer(int enabled, const ice::sonic::Status& out_status) noexcept = 0;
    virtual void bind(int allow_unauthorized, const ice::sonic::Status& out_status) noexcept = 0;
    virtual void listen(int backlog, const ice::sonic::Status& out_status) noexcept = 0;
    virtual void join_multicast(
        const ice::sonic::String& group,
        const ice::sonic::Status& out_status
    ) noexcept = 0;
    virtual void accept(
        const ice::sonic::TF_SocketOps& out_accepted,
        const ice::sonic::Status& out_status
    ) noexcept = 0;
    virtual void accept_async(
        TFSocketAcceptFn completion,
        void* user_data,
        const ice::sonic::Status& out_status
    ) noexcept = 0;
    virtual void connect(int64_t timeout_ms, const ice::sonic::Status& out_status) noexcept = 0;
    virtual void connect_async(
        int64_t timeout_ms,
        TFSocketAckFn completion,
        void* user_data,
        const ice::sonic::Status& out_status
    ) noexcept = 0;
    virtual void send(
        const void* data,
        size_t length,
        size_t* out_bytes_sent,
        const ice::sonic::Status& out_status
    ) noexcept = 0;
    virtual void send_async(
        const void* data,
        size_t length,
        TFSocketTransferFn completion,
        void* user_data,
        const ice::sonic::Status& out_status
    ) noexcept = 0;
    virtual void receive(
        void* out_buffer,
        size_t buffer_size,
        size_t* out_bytes_received,
        const ice::sonic::Status& out_status
    ) noexcept = 0;
    virtual void receive_async(
        void* out_buffer,
        size_t buffer_size,
        TFSocketTransferFn completion,
        void* user_data,
        const ice::sonic::Status& out_status
    ) noexcept = 0;
    virtual void send_to(
        const void* data,
        size_t length,
        const ice::sonic::String& dest_host,
        uint16_t dest_port,
        size_t* out_bytes_sent,
        const ice::sonic::Status& out_status
    ) noexcept = 0;
    virtual void receive_from(
        void* out_buffer,
        size_t buffer_size,
        size_t* out_bytes_received,
        const ice::sonic::String& out_sender_host,
        uint16_t* out_sender_port,
        const ice::sonic::Status& out_status
    ) noexcept = 0;
    virtual void
    set_send_timeout(int64_t timeout_ms, const ice::sonic::Status& out_status) noexcept = 0;
    virtual void
    set_receive_timeout(int64_t timeout_ms, const ice::sonic::Status& out_status) noexcept = 0;
    virtual void shutdown(int how, const ice::sonic::Status& out_status) noexcept = 0;
    virtual void get_status(int* out_status) noexcept = 0;
    virtual void get_error_code(int* out_error_code) noexcept = 0;
    virtual void get_local_endpoint(
        const ice::sonic::String& out_host,
        uint16_t* out_port,
        const ice::sonic::Status& out_status
    ) noexcept = 0;
    virtual void get_remote_endpoint(
        const ice::sonic::String& out_host,
        uint16_t* out_port,
        const ice::sonic::Status& out_status
    ) noexcept = 0;
    virtual void get_protocol(TFSocketProtocol* out_protocol) noexcept = 0;
    virtual void get_fd(intptr_t* out_fd) noexcept = 0;
    virtual void is_valid(int* out_valid) noexcept = 0;

    void get_generic_vtable(void (*create)(::TF_Socket*)) noexcept
    {
        m_vtable = ::TF_SocketOps{
            .struct_size = TF_OFFSET_OF_END(::TF_SocketOps, is_valid),

            .create = create,
            .destroy =
                [](TF_Socket* handle) noexcept
            {
                auto& self = TF_SocketOps::from_handle(handle);
                self.destroy();
            },
            .get_name =
                [](TF_Socket* socket, TF_String* out_name) noexcept
            {
                auto& self = TF_SocketOps::from_handle(socket);
                self.get_name(self.wrap(std::type_identity<ice::sonic::String>{}, out_name));
            },
            .close_socket =
                [](TF_Socket* socket) noexcept
            {
                auto& self = TF_SocketOps::from_handle(socket);
                self.close_socket();
            },
            .set_non_blocking =
                [](TF_Socket* socket, int enabled, TF_Status* out_status) noexcept
            {
                auto& self = TF_SocketOps::from_handle(socket);
                self.set_non_blocking(
                    enabled,
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .set_reuse_address =
                [](TF_Socket* socket, int enabled, TF_Status* out_status) noexcept
            {
                auto& self = TF_SocketOps::from_handle(socket);
                self.set_reuse_address(
                    enabled,
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .set_broadcast =
                [](TF_Socket* socket, int enabled, TF_Status* out_status) noexcept
            {
                auto& self = TF_SocketOps::from_handle(socket);
                self.set_broadcast(
                    enabled,
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .set_tcp_no_delay =
                [](TF_Socket* socket, int enabled, TF_Status* out_status) noexcept
            {
                auto& self = TF_SocketOps::from_handle(socket);
                self.set_tcp_no_delay(
                    enabled,
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .load_certificate =
                [](TF_Socket* socket,
                   const TF_String* cert_path,
                   const TF_String* key_path,
                   TF_Status* out_status) noexcept
            {
                auto& self = TF_SocketOps::from_handle(socket);
                self.load_certificate(
                    self.wrap(std::type_identity<ice::sonic::String>{}, cert_path),
                    self.wrap(std::type_identity<ice::sonic::String>{}, key_path),
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .generate_certificate =
                [](TF_Socket* socket,
                   const TF_String* cert_path,
                   const TF_String* key_path,
                   TF_Status* out_status) noexcept
            {
                auto& self = TF_SocketOps::from_handle(socket);
                self.generate_certificate(
                    self.wrap(std::type_identity<ice::sonic::String>{}, cert_path),
                    self.wrap(std::type_identity<ice::sonic::String>{}, key_path),
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .set_verify_peer =
                [](TF_Socket* socket, int enabled, TF_Status* out_status) noexcept
            {
                auto& self = TF_SocketOps::from_handle(socket);
                self.set_verify_peer(
                    enabled,
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .bind =
                [](TF_Socket* socket, int allow_unauthorized, TF_Status* out_status) noexcept
            {
                auto& self = TF_SocketOps::from_handle(socket);
                self.bind(
                    allow_unauthorized,
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .listen =
                [](TF_Socket* socket, int backlog, TF_Status* out_status) noexcept
            {
                auto& self = TF_SocketOps::from_handle(socket);
                self.listen(
                    backlog,
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .join_multicast =
                [](TF_Socket* socket, const TF_String* group, TF_Status* out_status) noexcept
            {
                auto& self = TF_SocketOps::from_handle(socket);
                self.join_multicast(
                    self.wrap(std::type_identity<ice::sonic::String>{}, group),
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .accept =
                [](TF_Socket* socket, TF_Socket* out_accepted, TF_Status* out_status) noexcept
            {
                auto& self = TF_SocketOps::from_handle(socket);
                self.accept(
                    self.wrap(std::type_identity<ice::sonic::TF_SocketOps>{}, out_accepted),
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .accept_async =
                [](TF_Socket* socket,
                   TFSocketAcceptFn completion,
                   void* user_data,
                   TF_Status* out_status) noexcept
            {
                auto& self = TF_SocketOps::from_handle(socket);
                self.accept_async(
                    completion,
                    user_data,
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .connect =
                [](TF_Socket* socket, int64_t timeout_ms, TF_Status* out_status) noexcept
            {
                auto& self = TF_SocketOps::from_handle(socket);
                self.connect(
                    timeout_ms,
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .connect_async =
                [](TF_Socket* socket,
                   int64_t timeout_ms,
                   TFSocketAckFn completion,
                   void* user_data,
                   TF_Status* out_status) noexcept
            {
                auto& self = TF_SocketOps::from_handle(socket);
                self.connect_async(
                    timeout_ms,
                    completion,
                    user_data,
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .send =
                [](TF_Socket* socket,
                   const void* data,
                   size_t length,
                   size_t* out_bytes_sent,
                   TF_Status* out_status) noexcept
            {
                auto& self = TF_SocketOps::from_handle(socket);
                self.send(
                    data,
                    length,
                    out_bytes_sent,
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .send_async =
                [](TF_Socket* socket,
                   const void* data,
                   size_t length,
                   TFSocketTransferFn completion,
                   void* user_data,
                   TF_Status* out_status) noexcept
            {
                auto& self = TF_SocketOps::from_handle(socket);
                self.send_async(
                    data,
                    length,
                    completion,
                    user_data,
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .receive =
                [](TF_Socket* socket,
                   void* out_buffer,
                   size_t buffer_size,
                   size_t* out_bytes_received,
                   TF_Status* out_status) noexcept
            {
                auto& self = TF_SocketOps::from_handle(socket);
                self.receive(
                    out_buffer,
                    buffer_size,
                    out_bytes_received,
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .receive_async =
                [](TF_Socket* socket,
                   void* out_buffer,
                   size_t buffer_size,
                   TFSocketTransferFn completion,
                   void* user_data,
                   TF_Status* out_status) noexcept
            {
                auto& self = TF_SocketOps::from_handle(socket);
                self.receive_async(
                    out_buffer,
                    buffer_size,
                    completion,
                    user_data,
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .send_to =
                [](TF_Socket* socket,
                   const void* data,
                   size_t length,
                   const TF_String* dest_host,
                   uint16_t dest_port,
                   size_t* out_bytes_sent,
                   TF_Status* out_status) noexcept
            {
                auto& self = TF_SocketOps::from_handle(socket);
                self.send_to(
                    data,
                    length,
                    self.wrap(std::type_identity<ice::sonic::String>{}, dest_host),
                    dest_port,
                    out_bytes_sent,
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .receive_from =
                [](TF_Socket* socket,
                   void* out_buffer,
                   size_t buffer_size,
                   size_t* out_bytes_received,
                   TF_String* out_sender_host,
                   uint16_t* out_sender_port,
                   TF_Status* out_status) noexcept
            {
                auto& self = TF_SocketOps::from_handle(socket);
                self.receive_from(
                    out_buffer,
                    buffer_size,
                    out_bytes_received,
                    self.wrap(std::type_identity<ice::sonic::String>{}, out_sender_host),
                    out_sender_port,
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .set_send_timeout =
                [](TF_Socket* socket, int64_t timeout_ms, TF_Status* out_status) noexcept
            {
                auto& self = TF_SocketOps::from_handle(socket);
                self.set_send_timeout(
                    timeout_ms,
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .set_receive_timeout =
                [](TF_Socket* socket, int64_t timeout_ms, TF_Status* out_status) noexcept
            {
                auto& self = TF_SocketOps::from_handle(socket);
                self.set_receive_timeout(
                    timeout_ms,
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .shutdown =
                [](TF_Socket* socket, int how, TF_Status* out_status) noexcept
            {
                auto& self = TF_SocketOps::from_handle(socket);
                self.shutdown(how, self.wrap(std::type_identity<ice::sonic::Status>{}, out_status));
            },
            .get_status =
                [](TF_Socket* socket, int* out_status) noexcept
            {
                auto& self = TF_SocketOps::from_handle(socket);
                self.get_status(out_status);
            },
            .get_error_code =
                [](TF_Socket* socket, int* out_error_code) noexcept
            {
                auto& self = TF_SocketOps::from_handle(socket);
                self.get_error_code(out_error_code);
            },
            .get_local_endpoint =
                [](TF_Socket* socket,
                   TF_String* out_host,
                   uint16_t* out_port,
                   TF_Status* out_status) noexcept
            {
                auto& self = TF_SocketOps::from_handle(socket);
                self.get_local_endpoint(
                    self.wrap(std::type_identity<ice::sonic::String>{}, out_host),
                    out_port,
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .get_remote_endpoint =
                [](TF_Socket* socket,
                   TF_String* out_host,
                   uint16_t* out_port,
                   TF_Status* out_status) noexcept
            {
                auto& self = TF_SocketOps::from_handle(socket);
                self.get_remote_endpoint(
                    self.wrap(std::type_identity<ice::sonic::String>{}, out_host),
                    out_port,
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .get_protocol =
                [](TF_Socket* socket, TFSocketProtocol* out_protocol) noexcept
            {
                auto& self = TF_SocketOps::from_handle(socket);
                self.get_protocol(out_protocol);
            },
            .get_fd =
                [](TF_Socket* socket, intptr_t* out_fd) noexcept
            {
                auto& self = TF_SocketOps::from_handle(socket);
                self.get_fd(out_fd);
            },
            .is_valid =
                [](TF_Socket* socket, int* out_valid) noexcept
            {
                auto& self = TF_SocketOps::from_handle(socket);
                self.is_valid(out_valid);
            },

        };
    }

    ice::sonic::TF_SocketOps
    wrap(std::type_identity<ice::sonic::TF_SocketOps>, const ::TF_Socket* handle) const noexcept
    {
        return ice::sonic::TF_SocketOps{m_TF_SocketOps_ops, const_cast<::TF_Socket*>(handle)};
    }

    ice::sonic::Status
    wrap(std::type_identity<ice::sonic::Status>, const ::TF_Status* handle) const noexcept
    {
        return ice::sonic::Status{m_Status_ops, const_cast<::TF_Status*>(handle)};
    }

    ice::sonic::String
    wrap(std::type_identity<ice::sonic::String>, const ::TF_String* handle) const noexcept
    {
        return ice::sonic::String{m_String_ops, const_cast<::TF_String*>(handle)};
    }

    const ::TF_SocketOps& get_vtable() const noexcept
    {
        return m_vtable;
    }

    const ::TF_Socket& get_handle() const noexcept
    {
        return m_handle;
    }

    template<typename Registry, typename StringType>
    void register_ops(
        Registry& registry,
        const StringType& type,
        const StringType& provider
    ) const noexcept
    {
        registry.register_op(type, provider, const_cast<::TF_SocketOps*>(&m_vtable));
    }

private:
    ::TF_SocketOps m_vtable;
    ::TF_Socket m_handle;

    const ::TF_SocketOps* m_TF_SocketOps_ops{nullptr};

    const ::TF_StatusOps* m_Status_ops{nullptr};

    const ::TF_StringOps* m_String_ops{nullptr};
};

} // namespace ice::builder
