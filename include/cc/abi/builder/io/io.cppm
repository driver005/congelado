module;

#include "c/extern/io/io.h"
#include "c/intern/tf_status.h"
#include "c/intern/tf_tstring.h"

export module cc_abi_builder_io;

export import :leaves;
import std;
import cc_abi_primitives;
import cc_abi_sonic_intern;

export namespace ice::builder {

// Abstract base class for an IO backend — pure interface, zero C-ABI/TF_* knowledge, mirrors
// ice::builder::Generator's role. A backend implements this directly and registers a
// factory function pointer into ice::sonic::Registration under type="io".
class Io
{
public:
    // Recover the Io instance from the opaque void* context slot that every
    // C vtable callback receives.  Named accessor so the cast intent is explicit
    // at the call site and the static_cast appears exactly once, here.
    static Io* create(void* ctx) noexcept
    {
        return static_cast<Io*>(ctx);
    }

    virtual ~Io() = default;

    virtual ice::String get_name() const noexcept = 0;

    [[nodiscard]] virtual std::expected<std::unique_ptr<Request>, ice::Status> create_request() noexcept = 0;
    [[nodiscard]] virtual std::expected<std::unique_ptr<Response>, ice::Status>
    create_response() noexcept = 0;

    static TF_IO* get_generic_vtable()
    {
        static TF_IO vtable = {
            .struct_size = TF_IO_STRUCT_SIZE,
            .destroy =
                [](void* plugin_context) noexcept
            {
                delete Io::create(plugin_context);
            },
            .get_name =
                [](void* plugin_context, TF_String* out) noexcept
            {
                Io::create(plugin_context)->get_name().to_c(out);
            },
            .create_request = [](void* plugin_context, TF_Status* status) noexcept -> TF_IO_Request*
            {
                auto* self = Io::create(plugin_context);
                auto res = self->create_request();
                if (!res) {
                    res.error().to_c(status);
                    return nullptr;
                }
                return static_cast<TF_IO_Request*>(static_cast<void*>(res->release()));
            },
            .request_destroy =
                [](TF_IO_Request* request_context) noexcept
            {
                delete Request::create(request_context);
            },
            .create_response = [](void* plugin_context, TF_Status* status) noexcept -> TF_IO_Response*
            {
                auto* self = Io::create(plugin_context);
                auto res = self->create_response();
                if (!res) {
                    res.error().to_c(status);
                    return nullptr;
                }
                return static_cast<TF_IO_Response*>(static_cast<void*>(res->release()));
            },
            .response_destroy =
                [](TF_IO_Response* response_context) noexcept
            {
                delete Response::create(response_context);
            },
            .request_get_method = [](TF_IO_Request* request_context) noexcept -> TF_IO_Method
            {
                auto* self = Request::create(request_context);
                return ice::method_to_c(self->get_method());
            },
            .request_get_path =
                [](TF_IO_Request* request_context, TF_String* out) noexcept
            {
                auto* self = Request::create(request_context);
                auto name = self->get_path();
                name.to_c(out);
            },
            .request_set_header =
                [](TF_IO_Request* request_context,
                   const TF_TString* name,
                   const TF_TString* value,
                   TF_Status* status) noexcept
            {
                auto* self = Request::create(request_context);
                auto res = self->set_header(ice::String::create(name), ice::String::create(value));
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .request_set_body =
                [](TF_IO_Request* request_context, const TF_TString* body, TF_Status* status) noexcept
            {
                auto* self = Request::create(request_context);
                auto res = self->set_body(ice::String::create(body));
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .response_set_status =
                [](TF_IO_Response* response_context, int32_t status_code, TF_Status* status) noexcept
            {
                auto* self = Response::create(response_context);
                auto res = self->set_status(status_code);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .response_set_header =
                [](TF_IO_Response* response_context,
                   const TF_TString* name,
                   const TF_TString* value,
                   TF_Status* status) noexcept
            {
                auto* self = Response::create(response_context);
                auto res = self->set_header(ice::String::create(name), ice::String::create(value));
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .response_set_body =
                [](TF_IO_Response* response_context, const TF_TString* body, TF_Status* status) noexcept
            {
                auto* self = Response::create(response_context);
                auto res = self->set_body(ice::String::create(body));
                if (!res) {
                    res.error().to_c(status);
                }
            }
        };
        return &vtable;
    }
};

} // namespace ice::builder
