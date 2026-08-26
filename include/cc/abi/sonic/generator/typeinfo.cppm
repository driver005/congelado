module;

#include "c/extern/generator/typeinfo.h"

export module cc_abi_sonic_generator:typeinfo;

import cc_abi_sonic_intern;
import cc_abi_builder_generator;

export namespace ice::sonic::generator {

// C ABI adapter: implements ice::builder::generator::TypeInfo by calling
// TF_Generator_TypeInfo_* functions.
class TypeInfo : public ice::builder::generator::TypeInfo
{
public:
    explicit TypeInfo(const TF_Generator_TypeInfo* handle) :
        m_handle(handle)
    {
    }

    ~TypeInfo() override = default;

    TypeInfo(const TypeInfo&) = default;
    TypeInfo& operator=(const TypeInfo&) = default;
    TypeInfo(TypeInfo&&) = default;
    TypeInfo& operator=(TypeInfo&&) = default;

    int get_data_type() const override
    {

        return TF_Generator_TypeInfo_GetDataType(m_handle);
    }

    StringRuntime get_type_attr_name() const override
    {

        return StringRuntime(TF_Generator_TypeInfo_GetTypeAttrName(m_handle));
    }

    bool is_read_only() const override
    {

        return TF_Generator_TypeInfo_IsReadOnly(m_handle);
    }

    bool is_list() const override
    {

        return TF_Generator_TypeInfo_IsList(m_handle);
    }

    const TF_Generator_TypeInfo* get_handle() const
    {
        return m_handle;
    }

private:
    const TF_Generator_TypeInfo* m_handle;
};

} // namespace ice::sonic::generator
