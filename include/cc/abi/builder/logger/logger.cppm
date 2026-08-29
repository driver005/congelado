module;

#include "c/extern/logger/logger.h"
#include "c/intern/tf_status.h"
#include "c/intern/tf_tstring.h"
#include "c/intern/tf_bool.h"


export module cc_abi_builder_logger;

import std;
import cc_abi_primitives;
import cc_abi_sonic_intern;






export namespace ice::builder {

// Abstract base class for a logger backend — pure interface, zero C-ABI/TF_* knowledge, mirrors
// ice::builder::Builder's role. A backend implements this directly and registers a
// factory function pointer into ice::sonic::RegistrationRuntime under type="logger".
class Logger
{
public:
    virtual ~Logger() = default;

    virtual std::expected<void, ice::Status> debug(const ice::String& message) = 0;
    virtual std::expected<void, ice::Status> info(const ice::String& message) = 0;
    virtual std::expected<void, ice::Status> important(const ice::String& message) = 0;
    virtual std::expected<void, ice::Status> warning(const ice::String& message) = 0;
    virtual std::expected<void, ice::Status> error(const ice::String& message) = 0;
    virtual std::expected<void, ice::Status> fatal(const ice::String& message) = 0;


    virtual ice::String get_name() const = 0;

    TF_Logger* get_generic_vtable() {
        static TF_Logger vtable = {
            .struct_size = sizeof(TF_Logger),
            .destroy = [](void* ctx) {
                delete ctx_as<Logger>(ctx);
            },
            .get_name = [](void* ctx, TF_String* out) {
                auto* self = ctx_as<Logger>(ctx);
                auto name = self->get_name();
                name.to_c(out);
            },
            .debug = [](void* ctx, const TF_TString* message, TF_Status* status) {
                auto res = ctx_as<Logger>(ctx)->debug(ice::String::create(message));
                if (!res) res.error().to_c(status);
            },
            .info = [](void* ctx, const TF_TString* message, TF_Status* status) {
                auto res = ctx_as<Logger>(ctx)->info(ice::String::create(message));
                if (!res) res.error().to_c(status);
            },
            .important = [](void* ctx, const TF_TString* message, TF_Status* status) {
                auto res = ctx_as<Logger>(ctx)->important(ice::String::create(message));
                if (!res) res.error().to_c(status);
            },
            .warning = [](void* ctx, const TF_TString* message, TF_Status* status) {
                auto res = ctx_as<Logger>(ctx)->warning(ice::String::create(message));
                if (!res) res.error().to_c(status);
            },
            .error = [](void* ctx, const TF_TString* message, TF_Status* status) {
                auto res = ctx_as<Logger>(ctx)->error(ice::String::create(message));
                if (!res) res.error().to_c(status);
            },
            .fatal = [](void* ctx, const TF_TString* message, TF_Status* status) {
                auto res = ctx_as<Logger>(ctx)->fatal(ice::String::create(message));
                if (!res) res.error().to_c(status);
            },};
        return &vtable;
    }
};

} // namespace ice::builder
