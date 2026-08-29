module;

#include "c/intern/tf_shape.h"

export module cc_abi_builder_intern:shape;

import std;
import :ctx;

export namespace ice::builder {

class Shape
{
public:
    virtual ~Shape() = default;

    virtual TF_Shape_Handle* new_shape(const int64_t* dims, int num_dims) = 0;
    virtual void delete_shape(TF_Shape_Handle* shape) = 0;
    virtual int get_num_dims(const TF_Shape_Handle* shape) const = 0;
    virtual int64_t get_dim(const TF_Shape_Handle* shape, int index) const = 0;

    static TF_Shape* get_generic_vtable()
    {
        static TF_Shape vtable = {
            .struct_size = sizeof(TF_Shape),
            .TF_NewShape = [](void* ctx, const int64_t* dims, int num_dims) -> TF_Shape_Handle* {
                return ctx_as<Shape>(ctx)->new_shape(dims, num_dims);
            },
            .TF_DeleteShape =
                [](void* ctx, TF_Shape_Handle* shape) {
                    ctx_as<Shape>(ctx)->delete_shape(shape);
                },
            .TF_ShapeNumDims = [](void* ctx, const TF_Shape_Handle* shape) -> int {
                return ctx_as<Shape>(ctx)->get_num_dims(shape);
            },
            .TF_ShapeDim = [](void* ctx, const TF_Shape_Handle* shape, int index) -> int64_t {
                return ctx_as<Shape>(ctx)->get_dim(shape, index);
            }
        };
        return &vtable;
    }
};

} // namespace ice::builder
