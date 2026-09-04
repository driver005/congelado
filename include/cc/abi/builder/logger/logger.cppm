module;

#include "c/extern/logger/logger.h"
#include "c/intern/tf_status.h"
#include "c/intern/tf_tstring.h"

export module cc_abi_builder_logger;

import std;
import cc_abi_primitives;
import cc_abi_sonic_intern;

export namespace ice::builder {

class Logger
{
public:
    static Logger* create(void* ctx) noexcept
    {
        return static_cast<Logger*>(ctx);
    }

    virtual ~Logger() = default;

    [[nodiscard]] virtual std::expected<void, ice::Status>
    debug(const ice::String& message) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    info(const ice::String& message) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    important(const ice::String& message) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    warning(const ice::String& message) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    error(const ice::String& message) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    fatal(const ice::String& message) noexcept = 0;

    virtual ice::String get_name() const noexcept = 0;

    static TF_Logger* get_generic_vtable()
    {
        static TF_Logger vtable = {
            .struct_size = TF_LOGGER_STRUCT_SIZE,
            .destroy =
                [](void* plugin_context) noexcept
            {
                delete Logger::create(plugin_context);
            },
            .get_name =
                [](void* plugin_context, TF_String* out) noexcept
            {
                auto* self = Logger::create(plugin_context);
                auto name = self->get_name();
                name.to_c(out);
            },
            .debug =
                [](void* plugin_context, const TF_TString* message, TF_Status* status) noexcept
            {
                auto* self = Logger::create(plugin_context);
                auto res = self->debug(ice::String::create(message));
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .info =
                [](void* plugin_context, const TF_TString* message, TF_Status* status) noexcept
            {
                auto* self = Logger::create(plugin_context);
                auto res = self->info(ice::String::create(message));
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .important =
                [](void* plugin_context, const TF_TString* message, TF_Status* status) noexcept
            {
                auto* self = Logger::create(plugin_context);
                auto res = self->important(ice::String::create(message));
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .warning =
                [](void* plugin_context, const TF_TString* message, TF_Status* status) noexcept
            {
                auto* self = Logger::create(plugin_context);
                auto res = self->warning(ice::String::create(message));
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .error =
                [](void* plugin_context, const TF_TString* message, TF_Status* status) noexcept
            {
                auto* self = Logger::create(plugin_context);
                auto res = self->error(ice::String::create(message));
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .fatal =
                [](void* plugin_context, const TF_TString* message, TF_Status* status) noexcept
            {
                auto* self = Logger::create(plugin_context);
                auto res = self->fatal(ice::String::create(message));
                if (!res) {
                    res.error().to_c(status);
                }
            },
        };
        return &vtable;
    }
};

} // namespace ice::builder
