// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from c/extern/shape/shape.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "c/extern/shape/shape.h"
#include "c/intern/tf_status.h"
#include "c/intern/tf_tstring.h"

export module cc_abi_builder_shape;

import std;
import cc_abi_primitives;
import cc_abi_sonic_intern;

export namespace ice::builder {

class Shape
{
public:
    static Shape* create(void* ctx) noexcept
    {
        return static_cast<Shape*>(ctx);
    }

    template<typename HandleT>
    static Shape* create(HandleT* handle) noexcept
    {
        return reinterpret_cast<Shape*>(handle);
    }

    virtual ~Shape() = default;
    [[nodiscard]] std::expected<void, ice::Status>
    new_shape(const int64_t* dims, int num_dims) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status> delete_shape() noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status> shape_num_dims() noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status> shape_dim(int index) noexcept = 0;
    virtual ice::String get_name() const noexcept = 0;

    static TF_Shape* get_generic_vtable()
    {
        static TF_Shape vtable = {
            .struct_size = TF_SHAPE_STRUCT_SIZE,

            .get_name =
                [](void* plugin_context, TF_String* out) noexcept
            {
                auto* self = Shape::create(plugin_context);
                auto result = self->get_name();
                result.to_c(out);
            },
            .new_shape =
                [](void* plugin_context, const int64_t* dims, int num_dims) noexcept
            {
                auto* self = Shape::create(plugin_context);
                auto res = self->new_shape(dims, num_dims);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .delete_shape =
                [](TF_Shape_Handle* shape) noexcept
            {
                auto* self = Shape::create(shape);
                auto res = self->delete_shape();
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .shape_num_dims =
                [](const TF_Shape_Handle* shape) noexcept
            {
                auto* self = Shape::create(shape);
                auto res = self->shape_num_dims();
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .shape_dim =
                [](const TF_Shape_Handle* shape, int index) noexcept
            {
                auto* self = Shape::create(shape);
                auto res = self->shape_dim(index);
                if (!res) {
                    res.error().to_c(status);
                }
            },

        };

        return &vtable;
    }
};

} // namespace ice::builder
