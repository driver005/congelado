module;

#include "c/extern/protocol/protocol.h"
#include "c/intern/tf_status.h"
#include "c/intern/tf_tstring.h"

export module cc_abi_builder_protocol;

import std;
import cc_abi_primitives;
import cc_abi_sonic_intern;

export namespace ice::builder {

// Abstract base class for a running protocol server — created by Protocol::create_server(), no
// independent existence outside its owning Protocol (mirrors ice::builder::Function's
// relationship to Builder::enter_border_patrol).
class Server
{
public:
    // Recover the Server instance from the opaque void* context slot that every
    // C vtable callback receives.  Named accessor so the cast intent is explicit
    // at the call site and the static_cast appears exactly once, here.
    static Server* create(void* ctx) noexcept
    {
        return static_cast<Server*>(ctx);
    }

    virtual ~Server() = default;

    virtual [[nodiscard]] std::expected<void, ice::Status> start() = 0;
    virtual [[nodiscard]] std::expected<void, ice::Status> stop() = 0;
    virtual [[nodiscard]] std::expected<bool, ice::Status> is_running() = 0;
};

// Abstract base class for a protocol backend — pure interface, zero C-ABI/TF_* knowledge,
// mirrors ice::builder::Builder's role. A backend implements this directly and
// registers a factory function pointer into ice::sonic::RegistrationRuntime under
// type="protocol".
class Protocol
{
public:
    // Recover the Protocol instance from the opaque void* context slot that every
    // C vtable callback receives.  Named accessor so the cast intent is explicit
    // at the call site and the static_cast appears exactly once, here.
    static Protocol* create(void* ctx) noexcept
    {
        return static_cast<Protocol*>(ctx);
    }

    virtual ~Protocol() = default;

    virtual [[nodiscard]] std::expected<std::unique_ptr<Server>, ice::Status> create_server() = 0;

    virtual ice::String get_name() const = 0;
    virtual ice::String get_bind_host() const = 0;
    virtual std::uint16_t get_bind_port() const = 0;
    virtual ice::String get_tls_cert() const = 0;
    virtual ice::String get_tls_key() const = 0;

    static TF_Protocol* get_generic_vtable()
    {
        static TF_Protocol vtable = {
            .struct_size = sizeof(TF_Protocol),
            .destroy =
                [](void* plugin_context)
            {
                delete Protocol::create(plugin_context);
            },
            .get_name =
                [](void* plugin_context, TF_String* out)
            {
                auto* self = Protocol::create(plugin_context);
                auto name = self->get_name();
                name.to_c(out);
            },
            .get_bind_host =
                [](void* plugin_context, TF_String* out)
            {
                auto* self = Protocol::create(plugin_context);
                auto name = self->get_bind_host();
                name.to_c(out);
            },
            .get_bind_port = [](void* plugin_context) -> uint16_t
            {
                auto* self = Protocol::create(plugin_context);
                return self->get_bind_port();
            },
            .get_tls_cert =
                [](void* plugin_context, TF_String* out)
            {
                auto* self = Protocol::create(plugin_context);
                auto name = self->get_tls_cert();
                name.to_c(out);
            },
            .get_tls_key =
                [](void* plugin_context, TF_String* out)
            {
                auto* self = Protocol::create(plugin_context);
                auto name = self->get_tls_key();
                name.to_c(out);
            },
            .create_server = [](void* plugin_context, TF_Status* status) -> void*
            {
                auto* self = Protocol::create(plugin_context);
                auto res = self->create_server();
                if (!res) {
                    res.error().to_c(status);
                    return nullptr;
                }
                return res->release();
            },
            .server__destroy =
                [](void* server_context)
            {
                delete Server::create(server_context);
            },
            .server__start =
                [](void* server_context, TF_Status* status)
            {
                auto* self = Server::create(server_context);
                auto res = self->start();
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .server__stop =
                [](void* server_context, TF_Status* status)
            {
                auto* self = Server::create(server_context);
                auto res = self->stop();
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .server__is_running = [](void* server_context, TF_Status* status) -> TF_Bool
            {
                auto* self = Server::create(server_context);
                auto res = self->is_running();
                if (!res) {
                    res.error().to_c(status);
                    return 0;
                }
                return *res ? 1 : 0;
            }
        };
        return &vtable;
    }
};

} // namespace ice::builder
