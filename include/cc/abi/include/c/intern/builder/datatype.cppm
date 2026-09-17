// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/intern/datatype.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/intern/datatype.h"

export module cc_abi_builder_intern;

import std;
import cc_abi_primitives;
import cc_abi_sonic_intern;

export namespace ice::builder {

class TF_DataTypeOps
{
public:
    static TF_DataTypeOps* create(void* ctx) noexcept
    {
        return static_cast<TF_DataTypeOps*>(ctx);
    }

    template<typename HandleT>
    static TF_DataTypeOps* create(HandleT* handle) noexcept
    {
        return static_cast<TF_DataTypeOps*>(handle->plugin_data);
    }

    virtual ~TF_DataTypeOps() = default;
    [[nodiscard]] std::expected<void, ice::Status>
    datatype_size(TFDataTypeEnum dt, size_t* out_size) noexcept = 0;
    virtual ice::String get_name() const noexcept = 0;

    static TF_DataTypeOps* get_generic_vtable()
    {
        static TF_DataTypeOps vtable = {
            .struct_size = TF_DATATYPE_STRUCT_SIZE,

            .get_name =
                [](void* plugin_context, TF_String* out) noexcept
            {
                auto* self = TF_DataTypeOps::create(plugin_context);
                auto result = self->get_name();
                result.to_c(out);
            },
            .datatype_size =
                [](TF_DataType* datatype, TFDataTypeEnum dt, size_t* out_size) noexcept
            {
                auto* self = TF_DataTypeOps::create(datatype);
                auto res = self->datatype_size(dt, out_size);
                if (!res) {
                    res.error().to_c(status);
                }
            },

        };

        return &vtable;
    }
};

} // namespace ice::builder
