module;

#include "c/extern/protocol/protocol.h"
#include "c/intern/tf_status.h"
#include "c/intern/tf_tstring.h"
#include "c/intern/tf_bool.h"

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
    virtual ~Server() = default;

    virtual std::expected<void, ice::Status> start() = 0;
    virtual std::expected<void, ice::Status> stop() = 0;
    virtual std::expected<bool, ice::Status> is_running() = 0;
};

// Abstract base class for a protocol backend — pure interface, zero C-ABI/TF_* knowledge,
// mirrors ice::builder::Builder's role. A backend implements this directly and
// registers a factory function pointer into ice::sonic::RegistrationRuntime under
// type="protocol".
class Protocol
{
public:
    virtual ~Protocol() = default;

    virtual std::expected<std::unique_ptr<Server>, ice::Status> create_server() = 0;

    virtual ice::String get_name() const = 0;
    virtual ice::String get_bind_host() const = 0;
    virtual std::uint16_t get_bind_port() const = 0;
    virtual ice::String get_tls_cert() const = 0;
    virtual ice::String get_tls_key() const = 0;

    TF_Protocol* get_generic_vtable() {
        static TF_Protocol vtable = {
            .struct_size = sizeof(TF_Protocol),
            .destroy = [](void* ctx) {
                delete ctx_as<Protocol>(ctx);
            },
            .get_name = [](void* ctx, TF_String* out) {
                auto* self = ctx_as<Protocol>(ctx);
                auto name = self->get_name();
                name.to_c(out);
            },
            .get_bind_host = [](void* ctx, TF_String* out) {
                auto* self = ctx_as<Protocol>(ctx);
                auto name = self->get_bind_host();
                name.to_c(out);
            },
            .get_bind_port = [](void* ctx) -> uint16_t {
                auto* self = ctx_as<Protocol>(ctx);
                return self->get_bind_port();
            },
            .get_tls_cert = [](void* ctx, TF_String* out) {
                auto* self = ctx_as<Protocol>(ctx);
                auto name = self->get_tls_cert();
                name.to_c(out);
            },
            .get_tls_key = [](void* ctx, TF_String* out) {
                auto* self = ctx_as<Protocol>(ctx);
                auto name = self->get_tls_key();
                name.to_c(out);
            },
            .create_server = [](void* ctx, TF_Status* status) -> void* {
                auto* self = ctx_as<Protocol>(ctx);
                auto res = self->create_server();
                if (!res) {
                    res.error().to_c(status);
                    return nullptr;
                }
                return res->release();
            },
            .server__destroy = [](void* ctx) {
                delete ctx_as<Server>(ctx);
            },
            .server__start = [](void* ctx, TF_Status* status) {
                auto* self = ctx_as<Server>(ctx);
                auto res = self->start();
                if (!res) res.error().to_c(status);
            },
            .server__stop = [](void* ctx, TF_Status* status) {
                auto* self = ctx_as<Server>(ctx);
                auto res = self->stop();
                if (!res) res.error().to_c(status);
            },
            .server__is_running = [](void* ctx, TF_Status* status) -> TF_Bool {
                auto* self = ctx_as<Server>(ctx);
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
