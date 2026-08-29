module;

#include "c/intern/tf_datatype.h"

export module cc_abi_sonic_intern:datatype;

import std;
import :runtime;
import cc_abi_primitives;

export namespace ice::sonic {

using DataTypeEnum = ice::DataTypeEnum;

class DataType : public Runtime<DataType, TF_DataTypeOps>
{
public:
    static constexpr std::string_view domain_name = "datatype";

    ice::String get_name() const
    {
        ice::String out;
        m_ops->get_name(m_host_context, out.get_handle());
        return out;
    }

    size_t data_type_size(DataTypeEnum dt)
    {
        return m_ops->data_type_size(m_host_context, data_type_to_c(dt));
    }
};

} // namespace ice::sonic
