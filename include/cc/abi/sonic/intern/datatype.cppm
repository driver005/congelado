module;

#include "c/intern/tf_datatype.h"

export module cc_abi_sonic_intern:datatype;

import std;
import :runtime;
export namespace ice::sonic {

using DataTypeEnum = ice::builder::DataTypeEnum;

class DataType : public Runtime<DataType, TF_DataType, true>
{
public:
    static constexpr std::string_view domain_name = "datatype";

    size_t data_type_size(DataTypeEnum dt)
    {
        return m_ops->TF_DataTypeSize(m_host_context, data_type_to_c(dt));
    }
};

} // namespace ice::sonic
