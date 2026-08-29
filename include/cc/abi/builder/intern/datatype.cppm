module;

#include "c/intern/tf_datatype.h"

export module cc_abi_builder_intern:datatype;

import std;
import cc_abi_primitives;

export namespace ice::builder {

class DataType
{
public:
    // Recover the DataType instance from the opaque void* context slot that every
    // C vtable callback receives.  Named accessor so the cast intent is explicit
    // at the call site and the static_cast appears exactly once, here.
    static DataType* create(void* ctx) noexcept
    {
        return static_cast<DataType*>(ctx);
    }

    virtual ~DataType() = default;

    virtual ice::String get_name() const = 0;

    virtual size_t data_type_size(ice::DataTypeEnum dt) = 0;

    static TF_DataTypeOps* get_generic_vtable()
    {
        static TF_DataTypeOps vtable = {
            .struct_size = sizeof(TF_DataTypeOps),
            .get_name =
                [](void* plugin_context, TF_String* out)
            {
                DataType::create(plugin_context)->get_name().to_c(out);
            },
            .data_type_size = [](void* plugin_context, TF_DataType_Enum dt) -> size_t
            {
                return DataType::create(plugin_context)->data_type_size(ice::data_type_from_c(dt));
            }
        };
        return &vtable;
    }
};

} // namespace ice::builder
