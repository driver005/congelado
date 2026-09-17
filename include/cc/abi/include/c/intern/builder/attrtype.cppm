// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/intern/attrtype.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/intern/attrtype.h"

export module cc_abi_builder_intern;

import std;
import cc_abi_primitives;
import cc_abi_sonic_intern;

export namespace ice::builder {

class TF_AttrTypeOps
{
public:
    static TF_AttrTypeOps* create(void* ctx) noexcept
    {
        return static_cast<TF_AttrTypeOps*>(ctx);
    }

    template<typename HandleT>
    static TF_AttrTypeOps* create(HandleT* handle) noexcept
    {
        return static_cast<TF_AttrTypeOps*>(handle->plugin_data);
    }

    virtual ~TF_AttrTypeOps() = default;
    [[nodiscard]] std::expected<void, ice::Status>
    attrtype_name(TFAttrTypeEnum type, const ice::sonic::TF_StringOps& out_type_name) noexcept = 0;
    virtual ice::String get_name() const noexcept = 0;

    static TF_AttrTypeOps* get_generic_vtable()
    {
        static TF_AttrTypeOps vtable = {
            .struct_size = TF_ATTRTYPE_STRUCT_SIZE,

            .get_name =
                [](void* plugin_context, TF_String* out) noexcept
            {
                auto* self = TF_AttrTypeOps::create(plugin_context);
                auto result = self->get_name();
                result.to_c(out);
            },
            .attrtype_name =
                [](TF_AttrType* attrtype, TFAttrTypeEnum type, TF_String* out_type_name) noexcept
            {
                auto* self = TF_AttrTypeOps::create(attrtype);
                auto res = self->attrtype_name(type, ice::sonic::TF_StringOps::wrap(out_type_name));
                if (!res) {
                    res.error().to_c(status);
                }
            },

        };

        return &vtable;
    }
};

} // namespace ice::builder
