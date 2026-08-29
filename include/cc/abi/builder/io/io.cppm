module;

#include "c/extern/io/io.h"
#include "c/intern/tf_status.h"
#include "c/intern/tf_tstring.h"

export module cc_abi_builder_io;

export import :leaves;
export import :enums;
import std;
import cc_abi_primitives;
import cc_abi_sonic_intern;
import :leaves;


export namespace ice::builder {

// Abstract base class for an IO backend — pure interface, zero C-ABI/TF_* knowledge, mirrors
// ice::builder::Builder's role. A backend implements this directly and registers a
// factory function pointer into ice::sonic::RegistrationRuntime under type="io".
class Io
{
public:
    virtual ~Io() = default;

    virtual std::expected<std::unique_ptr<Request>, ice::Status> create_request() = 0;
    virtual std::expected<std::unique_ptr<Response>, ice::Status> create_response() = 0;

    TF_IO* get_generic_vtable() {
        static TF_IO vtable = {
            .struct_size = sizeof(TF_IO),
            .destroy = [](void* ctx) {
                delete ctx_as<Io>(ctx);
            },
            .create_request = [](void* ctx, TF_Status* status) -> void* {
                auto* self = ctx_as<Io>(ctx);
                auto res = self->create_request();
                if (!res) {
                    res.error().to_c(status);
                    return nullptr;
                }
                return res->release();
            },
            .request__destroy = [](void* ctx) {
                delete ctx_as<Request>(ctx);
            },
            .create_response = [](void* ctx, TF_Status* status) -> void* {
                auto* self = ctx_as<Io>(ctx);
                auto res = self->create_response();
                if (!res) {
                    res.error().to_c(status);
                    return nullptr;
                }
                return res->release();
            },
            .response__destroy = [](void* ctx) {
                delete ctx_as<Response>(ctx);
            },
            .request__get_method = [](void* ctx) -> TF_IO_Method {
                auto* self = ctx_as<Request>(ctx);
                return method_to_c(self->get_method());
            },
            .request__get_path = [](void* ctx, TF_String* out) {
                auto* self = ctx_as<Request>(ctx);
                auto name = self->get_path();
                name.to_c(out);
            },
            .request__set_header = [](void* ctx, const TF_TString* name, const TF_TString* value, TF_Status* status) {
                auto* self = ctx_as<Request>(ctx);
                auto res = self->set_header(
                    ice::String::create(name),
                    ice::String::create(value)
                );
                if (!res) res.error().to_c(status);
            },
            .request__set_body = [](void* ctx, const TF_TString* body, TF_Status* status) {
                auto* self = ctx_as<Request>(ctx);
                auto res = self->set_body(
                    ice::String::create(body)
                );
                if (!res) res.error().to_c(status);
            },
            .response__set_status = [](void* ctx, int32_t status_code, TF_Status* status) {
                auto* self = ctx_as<Response>(ctx);
                auto res = self->set_status(status_code);
                if (!res) res.error().to_c(status);
            },
            .response__set_header = [](void* ctx, const TF_TString* name, const TF_TString* value, TF_Status* status) {
                auto* self = ctx_as<Response>(ctx);
                auto res = self->set_header(
                    ice::String::create(name),
                    ice::String::create(value)
                );
                if (!res) res.error().to_c(status);
            },
            .response__set_body = [](void* ctx, const TF_TString* body, TF_Status* status) {
                auto* self = ctx_as<Response>(ctx);
                auto res = self->set_body(
                    ice::String::create(body)
                );
                if (!res) res.error().to_c(status);
            }
        };
        return &vtable;
    }
};

} // namespace ice::builder
