// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from c/extern/shape_data/shape_data.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "c/extern/shape_data/shape_data.h"
#include "c/intern/tf_status.h"
#include "c/intern/tf_tstring.h"

export module cc_abi_builder_shape_data;

import std;
import cc_abi_primitives;
import cc_abi_sonic_intern;

export namespace ice::builder {

class Shape_data
{
public:
    static Shape_data* create(void* ctx) noexcept
    {
        return static_cast<Shape_data*>(ctx);
    }

    template<typename HandleT>
    static Shape_data* create(HandleT* handle) noexcept
    {
        return reinterpret_cast<Shape_data*>(handle);
    }

    virtual ~Shape_data() = default;
    virtual ice::String get_name() const noexcept = 0;

    static TF_Shape_Data* get_generic_vtable()
    {
        static TF_Shape_Data vtable = {
            .struct_size = TF_SHAPE_DATA_STRUCT_SIZE,

        };

        return &vtable;
    }
};

} // namespace ice::builder
