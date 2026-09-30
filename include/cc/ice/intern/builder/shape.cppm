// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/intern/shape.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/intern/shape.h"

export module cc_ice_intern_builder:shape;

import std;

export namespace ice::builder {

class TF_ShapeOps
{
public:
    TF_ShapeOps() noexcept :
        m_handle{.plugin_data = this}
    {
    }

    TF_ShapeOps(const TF_ShapeOps&) = delete;
    TF_ShapeOps& operator=(const TF_ShapeOps&) = delete;

    static TF_ShapeOps& from_handle(void* ctx) noexcept
    {
        return *static_cast<TF_ShapeOps*>(ctx);
    }

    template<typename HandleT>
    static TF_ShapeOps& from_handle(HandleT* handle) noexcept
    {
        return *static_cast<TF_ShapeOps*>(handle->plugin_data);
    }

    virtual ~TF_ShapeOps() = default;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    set_dims(const int64_t* dims, int num_dims) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status> delete_shape() noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    shape_num_dims(int* out_num_dims) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    shape_dim(int index, int64_t* out_dim) noexcept = 0;

    void get_generic_vtable() noexcept
    {
        m_vtable = ::TF_ShapeOps{
            .struct_size = TF_SHAPE_STRUCT_SIZE,

            .get_name =
                [](void* plugin_context, TF_String* out) noexcept
            {
                auto result = TF_ShapeOps::from_handle(plugin_context).get_name();
                result.to_c(out);
            },
            .set_dims =
                [](TF_Shape* shape, const int64_t* dims, int num_dims) noexcept
            {
                auto res = TF_ShapeOps::from_handle(shape).set_dims(dims, num_dims);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .delete_shape =
                [](TF_Shape* shape) noexcept
            {
                auto res = TF_ShapeOps::from_handle(shape).delete_shape();
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .shape_num_dims =
                [](const TF_Shape* shape, int* out_num_dims) noexcept
            {
                auto res = TF_ShapeOps::from_handle(shape).shape_num_dims(out_num_dims);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .shape_dim =
                [](const TF_Shape* shape, int index, int64_t* out_dim) noexcept
            {
                auto res = TF_ShapeOps::from_handle(shape).shape_dim(index, out_dim);
                if (!res) {
                    res.error().to_c(status);
                }
            },

        };
    }

    const ::TF_ShapeOps& get_vtable() const noexcept
    {
        return m_vtable;
    }

    virtual builder::String get_name() const noexcept = 0;

    const TF_Shape& get_handle() const noexcept
    {
        return m_handle;
    }


private:
    ::TF_ShapeOps m_vtable;
    TF_Shape m_handle;
};

} // namespace ice::builder
