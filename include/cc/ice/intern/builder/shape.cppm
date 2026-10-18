// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/intern/shape.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/intern/registration.h"
#include "include/c/intern/shape.h"
#include "include/c/intern/tstring.h"

export module cc_ice_intern_builder:shape;

import std;
import cc_ice_intern_sonic;

export namespace ice::builder {

class Shape
{
public:
    explicit Shape(const ::TF_StringOps* String_ops) noexcept :
        m_handle{.plugin_data = this}
    {
        m_String_ops = String_ops;
    }

    Shape(const Shape&) = delete;
    Shape& operator=(const Shape&) = delete;

    static Shape& from_handle(void* ctx) noexcept
    {
        return *static_cast<Shape*>(ctx);
    }

    template<typename HandleT>
    static Shape& from_handle(HandleT* handle) noexcept
    {
        return *static_cast<Shape*>(handle->plugin_data);
    }

    virtual ~Shape() = default;
    virtual void destroy() noexcept = 0;
    virtual void get_name(const ice::sonic::String& out_name) noexcept = 0;
    virtual void set_dims(std::span<const std::int64_t> dims) noexcept = 0;
    virtual void delete_shape() noexcept = 0;
    virtual void shape_num_dims(int& out_num_dims) noexcept = 0;
    virtual void shape_dim(int index, std::int64_t& out_dim) noexcept = 0;

    void get_generic_vtable(void (*create)(::TF_Shape*)) noexcept
    {
        m_vtable = ::TF_ShapeOps{
            .struct_size = TF_OFFSET_OF_END(::TF_ShapeOps, shape_dim),

            .create = create,
            .destroy =
                [](::TF_Shape* handle) noexcept
            {
                auto& self = Shape::from_handle(handle);
                self.destroy();
            },
            .get_name =
                [](::TF_Shape* shape, ::TF_String* out_name) noexcept
            {
                auto& self = Shape::from_handle(shape);
                self.get_name(self.wrap(std::type_identity<ice::sonic::String>{}, out_name));
            },
            .set_dims =
                [](::TF_Shape* shape, const int64_t* dims, int num_dims) noexcept
            {
                auto& self = Shape::from_handle(shape);
                self.set_dims(
                    std::span<const std::int64_t>{dims, static_cast<std::size_t>(num_dims)}
                );
            },
            .delete_shape =
                [](::TF_Shape* shape) noexcept
            {
                auto& self = Shape::from_handle(shape);
                self.delete_shape();
            },
            .shape_num_dims =
                [](const ::TF_Shape* shape, int* out_num_dims) noexcept
            {
                auto& self = Shape::from_handle(shape);
                self.shape_num_dims(*out_num_dims);
            },
            .shape_dim =
                [](const ::TF_Shape* shape, int index, int64_t* out_dim) noexcept
            {
                auto& self = Shape::from_handle(shape);
                self.shape_dim(index, *out_dim);
            },

        };
    }

    ice::sonic::String
    wrap(std::type_identity<ice::sonic::String>, const ::TF_String* handle) const noexcept
    {
        return ice::sonic::String{m_String_ops, handle};
    }

    const ::TF_ShapeOps& get_vtable() const noexcept
    {
        return m_vtable;
    }

    const ::TF_Shape& get_handle() const noexcept
    {
        return m_handle;
    }

    void register_ops(
        const ice::sonic::Registration& registry,
        const ice::sonic::String& type,
        const ice::sonic::String& provider
    ) const noexcept
    {
        registry.register_op(type, provider, const_cast<::TF_ShapeOps*>(&m_vtable));
    }

private:
    ::TF_ShapeOps m_vtable;
    ::TF_Shape m_handle;

    const ::TF_StringOps* m_String_ops{nullptr};
};

} // namespace ice::builder
