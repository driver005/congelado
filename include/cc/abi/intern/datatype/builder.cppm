// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from c/extern/datatype/datatype.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "c/extern/datatype/datatype.h"
#include "c/intern/tf_status.h"
#include "c/intern/tf_tstring.h"

export module cc_abi_builder_datatype;

import std;
import cc_abi_primitives;
import cc_abi_sonic_intern;

export namespace ice::builder {

class Datatype
{
public:
    static Datatype* create(void* ctx) noexcept
    {
        return static_cast<Datatype*>(ctx);
    }

    template<typename HandleT>
    static Datatype* create(HandleT* handle) noexcept
    {
        return reinterpret_cast<Datatype*>(handle);
    }

    virtual ~Datatype() = default;
    [[nodiscard]] std::expected<void, ice::Status> datatype_size(TF_DataType_Enum dt) noexcept = 0;
    virtual ice::String get_name() const noexcept = 0;

    static TF_DataType* get_generic_vtable()
    {
        static TF_DataType vtable = {
            .struct_size = TF_DATATYPE_STRUCT_SIZE,

            .get_name =
                [](void* plugin_context, TF_String* out) noexcept
            {
                auto* self = Datatype::create(plugin_context);
                auto result = self->get_name();
                result.to_c(out);
            },
            .datatype_size =
                [](void* plugin_context, TF_DataType_Enum dt) noexcept
            {
                auto* self = Datatype::create(plugin_context);
                auto res = self->datatype_size(dt);
                if (!res) {
                    res.error().to_c(status);
                }
            },

        };

        return &vtable;
    }
};

} // namespace ice::builder
