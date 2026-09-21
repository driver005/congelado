// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/parser/typeinfo.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/parser/typeinfo.h"

export module cc_ice_extern_parser_builder:typeinfo;

import std;

export namespace ice::builder {

class TFParserTypeInfoOps
{
public:
    static TFParserTypeInfoOps* create(void* ctx) noexcept
    {
        return static_cast<TFParserTypeInfoOps*>(ctx);
    }

    template<typename HandleT>
    static TFParserTypeInfoOps* create(HandleT* handle) noexcept
    {
        return static_cast<TFParserTypeInfoOps*>(handle->plugin_data);
    }

    virtual ~TFParserTypeInfoOps() = default;
    [[nodiscard]] virtual std::expected<void, ice::Status> get_dtype(int* out_dtype) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    get_shape(int64_t** out_dims, int* out_num_dims) noexcept = 0;

    static TFParserTypeInfoOps* get_generic_vtable()
    {
        static TFParserTypeInfoOps vtable = {
            .struct_size = TF_ARSERTYPEINFO_STRUCT_SIZE,
            .get_dtype =
                [](TFParserTypeInfo* typeinfo, int* out_dtype, TF_Status* out_status) noexcept
            {
                auto* self = TFParserTypeInfoOps::create(typeinfo);
                auto res = self->get_dtype(out_dtype);
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .get_shape =
                [](TFParserTypeInfo* typeinfo,
                   int64_t** out_dims,
                   int* out_num_dims,
                   TF_Status* out_status) noexcept
            {
                auto* self = TFParserTypeInfoOps::create(typeinfo);
                auto res = self->get_shape(out_dims, out_num_dims);
                if (!res) {
                    res.error().to_c(out_status);
                }
            },

        };

        return &vtable;
    }

    builder::String get_name() const noexcept
    {
        builder::String result;
        m_ops->get_name(get_handle(), result.get_handle());
        return result;
    }
};

} // namespace ice::builder
