// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from c/extern/attrtype/attrtype.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "c/extern/attrtype/attrtype.h"
#include "c/intern/tf_status.h"
#include "c/intern/tf_tstring.h"

export module cc_abi_builder_attrtype;

import std;
import cc_abi_primitives;
import cc_abi_sonic_intern;

export namespace ice::builder {

class Attrtype
{
public:
    static Attrtype* create(void* ctx) noexcept
    {
        return static_cast<Attrtype*>(ctx);
    }

    template<typename HandleT>
    static Attrtype* create(HandleT* handle) noexcept
    {
        return reinterpret_cast<Attrtype*>(handle);
    }

    virtual ~Attrtype() = default;
    [[nodiscard]] std::expected<void, ice::Status>
    attrtype_name(TF_AttrType_Enum type, TF_String* out) noexcept = 0;
    virtual ice::String get_name() const noexcept = 0;

    static TF_AttrType* get_generic_vtable()
    {
        static TF_AttrType vtable = {
            .struct_size = TF_ATTRTYPE_STRUCT_SIZE,

            .get_name =
                [](void* plugin_context, TF_String* out) noexcept
            {
                auto* self = Attrtype::create(plugin_context);
                auto result = self->get_name();
                result.to_c(out);
            },
            .attrtype_name =
                [](void* plugin_context, TF_AttrType_Enum type, TF_String* out) noexcept
            {
                auto* self = Attrtype::create(plugin_context);
                auto res = self->attrtype_name(type, out);
                if (!res) {
                    res.error().to_c(status);
                }
            },

        };

        return &vtable;
    }
};

} // namespace ice::builder
