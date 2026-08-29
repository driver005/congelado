module;

#include "c/intern/tf_shape.h"

export module cc_abi_builder_intern:shape;

import std;
import cc_abi_primitives;

export namespace ice::builder {

class Shape
{
public:
    // Recover the Shape instance from the opaque void* context slot that every
    // C vtable callback receives.  Named accessor so the cast intent is explicit
    // at the call site and the static_cast appears exactly once, here.
    static Shape* create(void* ctx) noexcept
    {
        return static_cast<Shape*>(ctx);
    }

    virtual ~Shape() = default;

    virtual ice::String get_name() const = 0;

    virtual [[nodiscard]] std::expected<TF_Shape_Handle*, ice::Status>
    new_shape(const int64_t* dims, int num_dims) = 0;
    virtual void delete_shape(TF_Shape_Handle* shape) = 0;
    virtual int get_num_dims(const TF_Shape_Handle* shape) const = 0;
    virtual int64_t get_dim(const TF_Shape_Handle* shape, int index) const = 0;

    static TF_Shape* get_generic_vtable()
    {
        static TF_Shape vtable = {
            .struct_size = sizeof(TF_Shape),
            .get_name =
                [](void* plugin_context, TF_String* out)
            {
                Shape::create(plugin_context)->get_name().to_c(out);
            },
            .new_shape =
                [](void* plugin_context, const int64_t* dims, int num_dims) -> TF_Shape_Handle*
            {
                auto res = Shape::create(plugin_context)->new_shape(dims, num_dims);
                if (!res) {
                    return nullptr; // no status slot — P1 allocator contract
                }
                return res.value();
            },
            .delete_shape =
                [](void* plugin_context, TF_Shape_Handle* shape)
            {
                Shape::create(plugin_context)->delete_shape(shape);
            },
            .shape_num_dims = [](void* plugin_context, const TF_Shape_Handle* shape) -> int
            {
                return Shape::create(plugin_context)->get_num_dims(shape);
            },
            .shape_dim =
                [](void* plugin_context, const TF_Shape_Handle* shape, int index) -> int64_t
            {
                return Shape::create(plugin_context)->get_dim(shape, index);
            }
        };
        return &vtable;
    }
};

} // namespace ice::builder
