module;

#include "include/c/intern/datatype.h"
#include "include/c/intern/tstring.h"

export module aten_xpu_intern:data_type;

import std;
import cc_ice_intern_builder;
import cc_ice_intern_sonic;
import :datatype;

export namespace aten_xpu {

// Element sizes of the data type enum; the table itself lives in SyclDataType.
class DataType : public ice::builder::DataType
{
public:
    explicit DataType(const ::TF_StringOps* string_ops) noexcept :
        ice::builder::DataType{string_ops}
    {
    }

    ~DataType() override = default;
    DataType(const DataType&) = delete;
    DataType& operator=(const DataType&) = delete;
    DataType(DataType&&) = delete;
    DataType& operator=(DataType&&) = delete;

    void destroy() noexcept override
    {
        delete this;
    }

    void get_name(const ice::sonic::String& out_name) noexcept override
    {
        out_name.copy("DataType");
    }

    void datatype_size(ice::sonic::DataTypeEnum dt, std::size_t& out_size) noexcept override
    {
        out_size = m_sizes.element_size(dt);
    }

private:
    SyclDataType m_sizes;
};

} // namespace aten_xpu
