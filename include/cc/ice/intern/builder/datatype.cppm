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
    virtual void get_name(const ice::sonic::String& out_name) noexcept = 0;
    virtual void datatype_size(TFDataTypeEnum dt, size_t* out_size) noexcept = 0;

    void get_generic_vtable() noexcept
    {
        m_vtable = ::TF_DataTypeOps{
            .struct_size = TF_DATATYPE_STRUCT_SIZE,
            .get_name =
                [](TF_DataType* datatype, TF_String* out_name) noexcept
            {
                TF_DataTypeOps::from_handle(datatype).get_name(ice::sonic::String::wrap(out_name));
            },
            .datatype_size =
                [](TF_DataType* datatype, TFDataTypeEnum dt, size_t* out_size) noexcept
            {
                TF_DataTypeOps::from_handle(datatype).datatype_size(dt, out_size);
            },

        };
    }

    const ::TF_DataTypeOps& get_vtable() const noexcept
    {
        return m_vtable;
    }

    const TF_DataType& get_handle() const noexcept
    {
        return m_handle;
    }


private:
    ::TF_DataTypeOps m_vtable;
    TF_DataType m_handle;
};

} // namespace ice::builder
