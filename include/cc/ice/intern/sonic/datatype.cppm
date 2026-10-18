// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/intern/datatype.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/intern/datatype.h"
#include "include/c/intern/registration.h"

export module cc_ice_intern_sonic:datatype;

import std;
import :data_type_enum;
import :registration;
import :runtime;
import :tstring;

export namespace ice::sonic {

class DataType : public ice::sonic::Runtime<::TF_DataTypeOps, ::TF_DataType>
{
public:
    DataType(
        const ice::sonic::Registration& registry,
        const ice::sonic::String& type,
        const ice::sonic::String& provider
    ) noexcept :
        Runtime(
            *registry.get_ops(),
            *registry.get_handle(),
            *type.get_handle(),
            *provider.get_handle()
        )
    {
    }

    DataType(
        const ice::sonic::Registration& registry,
        const ::TF_DataType& handle,
        const ice::sonic::String& type,
        const ice::sonic::String& provider
    ) noexcept :
        Runtime(
            *registry.get_ops(),
            *registry.get_handle(),
            handle,
            *type.get_handle(),
            *provider.get_handle()
        )
    {
    }

    explicit DataType(const ::TF_DataTypeOps* ops) noexcept :
        Runtime(ops)
    {
    }

    DataType(const ::TF_DataTypeOps* ops, const ::TF_DataType* handle) noexcept :
        Runtime(ops, handle)
    {
    }

    void create() const noexcept
    {
        m_ops->create(get_handle());
    }

    void destroy() const noexcept
    {
        m_ops->destroy(get_handle());
    }

    void get_name(const ice::sonic::String& out_name) const noexcept
    {
        m_ops->get_name(get_handle(), out_name.get_handle());
    }

    void datatype_size(ice::sonic::DataTypeEnum dt, std::size_t& out_size) const noexcept
    {
        m_ops->datatype_size(get_handle(), dt.get_value(), &out_size);
    }
};

} // namespace ice::sonic
