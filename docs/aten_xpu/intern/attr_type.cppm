module;

#include "include/c/intern/attrtype.h"
#include "include/c/intern/tstring.h"

export module aten_xpu_intern:attr_type;

import std;
import cc_ice_intern_builder;
import cc_ice_intern_sonic;

export namespace aten_xpu {

// Names of the attribute type enum.
class AttrType : public ice::builder::AttrType
{
public:
    explicit AttrType(const ::TF_StringOps* string_ops) noexcept :
        ice::builder::AttrType{string_ops}
    {
    }

    ~AttrType() override = default;
    AttrType(const AttrType&) = delete;
    AttrType& operator=(const AttrType&) = delete;
    AttrType(AttrType&&) = delete;
    AttrType& operator=(AttrType&&) = delete;

    void destroy() noexcept override
    {
        delete this;
    }

    void get_name(const ice::sonic::String& out_name) noexcept override
    {
        out_name.copy("AttrType");
    }

    void attrtype_name(
        ice::sonic::AttrTypeEnum type,
        const ice::sonic::String& out_type_name
    ) noexcept override
    {
        out_type_name.copy(name_of(type));
    }

private:
    static std::string_view name_of(ice::sonic::AttrTypeEnum type) noexcept
    {
        switch (type) {
            case ice::sonic::AttrTypeEnum::k_string:
                return "string";
            case ice::sonic::AttrTypeEnum::k_int:
                return "int";
            case ice::sonic::AttrTypeEnum::k_float:
                return "float";
            case ice::sonic::AttrTypeEnum::k_bool:
                return "bool";
            case ice::sonic::AttrTypeEnum::k_type:
                return "type";
            case ice::sonic::AttrTypeEnum::k_shape:
                return "shape";
            case ice::sonic::AttrTypeEnum::k_tensor:
                return "tensor";
            case ice::sonic::AttrTypeEnum::k_placeholder:
                return "placeholder";
            case ice::sonic::AttrTypeEnum::k_func:
                return "func";
            default:
                return "unknown";
        }
    }
};

} // namespace aten_xpu
