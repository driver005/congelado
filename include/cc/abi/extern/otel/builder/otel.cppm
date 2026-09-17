// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from c/extern/otel/otel.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "c/extern/otel/otel.h"
#include "c/intern/tf_status.h"
#include "c/intern/tf_tstring.h"

export module cc_abi_builder_otel;

import std;
import cc_abi_primitives;
import cc_abi_sonic_intern;

export namespace ice::builder {

class Otel
{
public:
    static Otel* create(void* ctx) noexcept
    {
        return static_cast<Otel*>(ctx);
    }

    template<typename HandleT>
    static Otel* create(HandleT* handle) noexcept
    {
        return static_cast<Otel*>(handle->plugin_data);
    }

    virtual ~Otel() = default;
    virtual ice::String get_name() const noexcept = 0;

    static TF_OtelOps* get_generic_vtable()
    {
        static TF_OtelOps vtable = {
            .struct_size = TF_OTEL_STRUCT_SIZE,

            .destroy =
                [](void* plugin_context) noexcept
            {
                delete Otel::create(plugin_context);
            },

            .get_name =
                [](void* plugin_context, TF_String* out) noexcept
            {
                auto* self = Otel::create(plugin_context);
                auto result = self->get_name();
                result.to_c(out);
            },

        };

        return &vtable;
    }
};

} // namespace ice::builder
