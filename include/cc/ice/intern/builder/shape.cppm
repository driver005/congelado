// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/intern/shape.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/intern/shape.h"

export module cc_ice_builder_intern:shape;

import std;

export namespace ice::builder {

class TF_ShapeOps
{
public:
    static TF_ShapeOps* create(void* ctx) noexcept
    {
        return static_cast<TF_ShapeOps*>(ctx);
    }

    template<typename HandleT>
    static TF_ShapeOps* create(HandleT* handle) noexcept
    {
        return static_cast<TF_ShapeOps*>(handle->plugin_data);
    }

    virtual ~TF_ShapeOps() = default;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    set_dims(const int64_t* dims, int num_dims) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status> delete_shape() noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    shape_num_dims(int* out_num_dims) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    shape_dim(int index, int64_t* out_dim) noexcept = 0;

    static TF_ShapeOps* get_generic_vtable()
    {
        static TF_ShapeOps vtable = {
            .struct_size = TF_SHAPE_STRUCT_SIZE,

            .get_name =
                [](void* plugin_context, TF_String* out) noexcept
            {
                auto* self = TF_ShapeOps::create(plugin_context);
                auto result = self->get_name();
                result.to_c(out);
            },
            .set_dims =
                [](TF_Shape* shape, const int64_t* dims, int num_dims) noexcept
            {
                auto* self = TF_ShapeOps::create(shape);
                auto res = self->set_dims(dims, num_dims);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .delete_shape =
                [](TF_Shape* shape) noexcept
            {
                auto* self = TF_ShapeOps::create(shape);
                auto res = self->delete_shape();
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .shape_num_dims =
                [](const TF_Shape* shape, int* out_num_dims) noexcept
            {
                auto* self = TF_ShapeOps::create(shape);
                auto res = self->shape_num_dims(out_num_dims);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .shape_dim =
                [](const TF_Shape* shape, int index, int64_t* out_dim) noexcept
            {
                auto* self = TF_ShapeOps::create(shape);
                auto res = self->shape_dim(index, out_dim);
                if (!res) {
                    res.error().to_c(status);
                }
            },

        };

        return &vtable;
    }

    builder::String get_name() const noexcept
    {
        builder::String result;
        m_ops->get_name(get_handle(), result.get_handle());
        return result;
    }
};

} // namespace ice::builder
