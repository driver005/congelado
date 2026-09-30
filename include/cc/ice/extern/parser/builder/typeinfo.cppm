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
    TFParserTypeInfoOps() noexcept :
        m_handle{.plugin_data = this}
    {
    }

    TFParserTypeInfoOps(const TFParserTypeInfoOps&) = delete;
    TFParserTypeInfoOps& operator=(const TFParserTypeInfoOps&) = delete;

    static TFParserTypeInfoOps& from_handle(void* ctx) noexcept
    {
        return *static_cast<TFParserTypeInfoOps*>(ctx);
    }

    template<typename HandleT>
    static TFParserTypeInfoOps& from_handle(HandleT* handle) noexcept
    {
        return *static_cast<TFParserTypeInfoOps*>(handle->plugin_data);
    }

    virtual ~TFParserTypeInfoOps() = default;
    [[nodiscard]] virtual std::expected<void, ice::sonic::Status>
    get_dtype(int* out_dtype) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::sonic::Status>
    get_shape(int64_t** out_dims, int* out_num_dims) noexcept = 0;

    void get_generic_vtable() noexcept
    {
        m_vtable = ::TFParserTypeInfoOps{
            .struct_size = TF_ARSERTYPEINFO_STRUCT_SIZE,
            .get_dtype =
                [](TFParserTypeInfo* typeinfo, int* out_dtype, TF_Status* out_status) noexcept
            {
                auto res = TFParserTypeInfoOps::from_handle(typeinfo).get_dtype(out_dtype);
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
                auto res =
                    TFParserTypeInfoOps::from_handle(typeinfo).get_shape(out_dims, out_num_dims);
                if (!res) {
                    res.error().to_c(out_status);
                }
            },

        };
    }

    const ::TFParserTypeInfoOps& get_vtable() const noexcept
    {
        return m_vtable;
    }

    const TFParserTypeInfo& get_handle() const noexcept
    {
        return m_handle;
    }


private:
    ::TFParserTypeInfoOps m_vtable;
    TFParserTypeInfo m_handle;
};

} // namespace ice::builder
