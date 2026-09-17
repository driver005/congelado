// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from c/extern/jobber/jobber.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "c/extern/jobber/jobber.h"
#include "c/intern/tf_status.h"
#include "c/intern/tf_tstring.h"

export module cc_abi_builder_jobber;

import std;
import cc_abi_primitives;
import cc_abi_sonic_intern;

export namespace ice::builder {

class Jobber
{
public:
    static Jobber* create(void* ctx) noexcept
    {
        return static_cast<Jobber*>(ctx);
    }

    template<typename HandleT>
    static Jobber* create(HandleT* handle) noexcept
    {
        return static_cast<Jobber*>(handle->plugin_data);
    }

    virtual ~Jobber() = default;
    virtual ice::String get_name() const noexcept = 0;

    static TF_JobberOps* get_generic_vtable()
    {
        static TF_JobberOps vtable = {
            .struct_size = TF_JOBBER_STRUCT_SIZE,

            .destroy =
                [](void* plugin_context) noexcept
            {
                delete Jobber::create(plugin_context);
            },

            .get_name =
                [](void* plugin_context, TF_String* out) noexcept
            {
                auto* self = Jobber::create(plugin_context);
                auto result = self->get_name();
                result.to_c(out);
            },

        };

        return &vtable;
    }
};

} // namespace ice::builder
