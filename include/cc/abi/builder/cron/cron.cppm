module;

#include "c/extern/cron/cron.h"
#include "c/intern/tf_status.h"
#include "c/intern/tf_tstring.h"

export module cc_abi_builder_cron;

import std;
import cc_abi_primitives;
import cc_abi_sonic_intern;

export namespace ice::builder {

// Abstract base class for a cron scheduler backend — pure interface, zero C-ABI/TF_* knowledge,
// mirrors ice::builder::Generator's role. A backend implements this directly and
// registers a factory function pointer into ice::sonic::RegistrationRuntime under type="cron".
class Cron
{
public:
    // Recover the Cron instance from the opaque void* context slot that every
    // C vtable callback receives.  Named accessor so the cast intent is explicit
    // at the call site and the static_cast appears exactly once, here.
    static Cron* create(void* ctx) noexcept
    {
        return static_cast<Cron*>(ctx);
    }

    virtual ~Cron() = default;


    [[nodiscard]] virtual std::expected<bool, ice::Status>
    validate(const ice::String& expression) noexcept = 0;

    // out_time_ms — filled with the next fire time when the returned bool is true.
    [[nodiscard]] virtual std::expected<bool, ice::Status> next_after(
        const ice::String& expression,
        std::int64_t base_time_ms,
        std::int64_t* out_time_ms
    ) noexcept = 0;

    [[nodiscard]] virtual std::expected<void, ice::Status>
    upsert_job(const ice::String& name, const ice::String& expression) noexcept = 0;

    [[nodiscard]] virtual std::expected<void, ice::Status> remove_job(const ice::String& name) noexcept = 0;

    virtual ice::String get_name() const noexcept = 0;

    static TF_Cron* get_generic_vtable()
    {
        static TF_Cron vtable = {
            .struct_size = TF_CRON_STRUCT_SIZE,
            .destroy =
                [](void* plugin_context) noexcept
            {
                delete Cron::create(plugin_context);
            },
            .get_name =
                [](void* plugin_context, TF_String* out) noexcept
            {
                auto* self = Cron::create(plugin_context);
                auto name = self->get_name();
                name.to_c(out);
            },
            .validate =
                [](void* plugin_context, const TF_TString* expression, TF_Status* status) noexcept -> bool
            {
                auto* self = Cron::create(plugin_context);
                auto res = self->validate(ice::String::create(expression));
                if (!res) {
                    res.error().to_c(status);
                    return 0;
                }
                return *res ? 1 : 0;
            },
            .next_after = [](void* plugin_context,
                             const TF_TString* expression,
                             int64_t base_time_ms,
                             int64_t* out_time_ms,
                             TF_Status* status) noexcept -> bool
            {
                auto* self = Cron::create(plugin_context);
                auto res =
                    self->next_after(ice::String::create(expression), base_time_ms, out_time_ms);
                if (!res) {
                    res.error().to_c(status);
                    return 0;
                }
                return *res ? 1 : 0;
            },
            .upsert_job =
                [](void* plugin_context,
                   const TF_TString* name,
                   const TF_TString* expression,
                   TF_Status* status) noexcept
            {
                auto* self = Cron::create(plugin_context);
                auto res =
                    self->upsert_job(ice::String::create(name), ice::String::create(expression));
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .remove_job =
                [](void* plugin_context, const TF_TString* name, TF_Status* status) noexcept
            {
                auto* self = Cron::create(plugin_context);
                auto res = self->remove_job(ice::String::create(name));
                if (!res) {
                    res.error().to_c(status);
                }
            }
        };
        return &vtable;
    }
};

} // namespace ice::builder
