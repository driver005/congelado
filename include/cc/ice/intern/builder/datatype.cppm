// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/intern/datatype.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/intern/datatype.h"

export module cc_ice_intern_builder:datatype;

import std;

export namespace ice::builder {

class TF_DataTypeOps
{
public:
    TF_DataTypeOps() noexcept :
        m_handle{.plugin_data = this}
    {
    }

    TF_DataTypeOps(const TF_DataTypeOps&) = delete;
    TF_DataTypeOps& operator=(const TF_DataTypeOps&) = delete;

    static TF_DataTypeOps& from_handle(void* ctx) noexcept
    {
        return *static_cast<TF_DataTypeOps*>(ctx);
    }

    template<typename HandleT>
    static TF_DataTypeOps& from_handle(HandleT* handle) noexcept
    {
        return *static_cast<TF_DataTypeOps*>(handle->plugin_data);
    }

    virtual ~TF_DataTypeOps() = default;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    datatype_size(TFDataTypeEnum dt, size_t* out_size) noexcept = 0;

    void get_generic_vtable() noexcept
    {
        m_vtable = ::TF_DataTypeOps{
            .struct_size = TF_DATATYPE_STRUCT_SIZE,

            .get_name =
                [](void* plugin_context, TF_String* out) noexcept
            {
                auto result = TF_DataTypeOps::from_handle(plugin_context).get_name();
                result.to_c(out);
            },
            .datatype_size =
                [](TF_DataType* datatype, TFDataTypeEnum dt, size_t* out_size) noexcept
            {
                auto res = TF_DataTypeOps::from_handle(datatype).datatype_size(dt, out_size);
                if (!res) {
                    res.error().to_c(status);
                }
            },

        };
    }

    const ::TF_DataTypeOps& get_vtable() const noexcept
    {
        return m_vtable;
    }

    virtual builder::String get_name() const noexcept = 0;

    const TF_DataType& get_handle() const noexcept
    {
        return m_handle;
    }


private:
    ::TF_DataTypeOps m_vtable;
    TF_DataType m_handle;
};

} // namespace ice::builder
