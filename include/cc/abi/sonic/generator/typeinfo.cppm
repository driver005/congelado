module;

#include "c/extern/generator/typeinfo.h"

export module cc_abi_sonic_generator:typeinfo;

import cc_abi_sonic_intern;
export namespace ice::sonic {

// C ABI adapter: implements ice::builder::TypeInfo by calling
// TF_Generator_TypeInfo_* functions.
class TypeInfo : public ice::builder::TypeInfo
{
public:
    explicit TypeInfo(const TF_Generator_TypeInfo* handle) :
        m_handle(handle)
    {
    }

    ~TypeInfo() = default;

    TypeInfo(const TypeInfo&) = default;
    TypeInfo& operator=(const TypeInfo&) = default;
    TypeInfo(TypeInfo&&) = default;
    TypeInfo& operator=(TypeInfo&&) = default;

    int get_data_type() const
    {

        return TF_Generator_TypeInfo_GetDataType(m_handle);
    }

    String get_type_attr_name() const
    {

        return String(TF_Generator_TypeInfo_GetTypeAttrName(m_handle));
    }

    bool is_read_only() const
    {

        return TF_Generator_TypeInfo_IsReadOnly(m_handle);
    }

    bool is_list() const
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

} // namespace ice::sonic
