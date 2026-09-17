// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/jobber/jobber.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/jobber/jobber.h"

export module cc_abi_builder_jobber;

import std;
import cc_abi_primitives;
import cc_abi_sonic_intern;

export namespace ice::builder {

class TF_JobberOps
{
public:
    static TF_JobberOps* create(void* ctx) noexcept
    {
        return static_cast<TF_JobberOps*>(ctx);
    }

    template<typename HandleT>
    static TF_JobberOps* create(HandleT* handle) noexcept
    {
        return static_cast<TF_JobberOps*>(handle->plugin_data);
    }

    virtual ~TF_JobberOps() = default;

    static TF_JobberOps* get_generic_vtable()
    {
        static TF_JobberOps vtable = {
            .struct_size = TF_JOBBER_STRUCT_SIZE,

            .destroy =
                [](void* plugin_context) noexcept
            {
                delete TF_JobberOps::create(plugin_context);
            },

            .get_name =
                [](void* plugin_context, TF_String* out) noexcept
            {
                auto* self = TF_JobberOps::create(plugin_context);
                auto result = self->get_name();
                result.to_c(out);
            },

        };

        return &vtable;
    }
};

} // namespace ice::builder
