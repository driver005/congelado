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

    virtual ice::String get_name() const noexcept = 0;

    // Range-first: dims crosses the C ABI as (const int64_t*, int) — adapt to a span
    // immediately at the C++ interface; the vtable lambda below is the only place the
    // raw pair exists.
    [[nodiscard]] virtual std::expected<TF_Shape_Handle*, ice::Status>
    new_shape(std::span<const int64_t> dims) noexcept = 0;
    virtual void delete_shape(TF_Shape_Handle* shape) noexcept = 0;
    virtual int get_num_dims(const TF_Shape_Handle* shape) const noexcept = 0;
    virtual int64_t get_dim(const TF_Shape_Handle* shape, int index) const noexcept = 0;

    static TF_Shape* get_generic_vtable()
    {
        static TF_Shape vtable = {
            .struct_size = TF_SHAPE_STRUCT_SIZE,
            .get_name =
                [](void* plugin_context, TF_String* out) noexcept
            {
                Shape::create(plugin_context)->get_name().to_c(out);
            },
            .new_shape =
                [](void* plugin_context, const int64_t* dims, int num_dims) noexcept
                -> TF_Shape_Handle*
            {
                // A negative num_dims (unknown rank) must not become a giant span.
                const std::span<const int64_t> dim_span =
                    num_dims > 0 ? std::span{dims, static_cast<size_t>(num_dims)}
                                 : std::span<const int64_t>{};
                auto res = Shape::create(plugin_context)->new_shape(dim_span);
                if (!res) {
                    return nullptr; // no status slot — P1 allocator contract
                }
                return res.value();
            },
            .delete_shape =
                [](void* plugin_context, TF_Shape_Handle* shape) noexcept
            {
                Shape::create(plugin_context)->delete_shape(shape);
            },
            .shape_num_dims = [](void* plugin_context, const TF_Shape_Handle* shape) noexcept
                              -> int
            {
                return Shape::create(plugin_context)->get_num_dims(shape);
            },
            .shape_dim =
                [](void* plugin_context, const TF_Shape_Handle* shape, int index) noexcept
                -> int64_t
            {
                return Shape::create(plugin_context)->get_dim(shape, index);
            }
        };
        return &vtable;
    }
};

} // namespace ice::builder
