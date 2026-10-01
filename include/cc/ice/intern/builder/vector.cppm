// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/intern/vector.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/intern/status.h"
#include "include/c/intern/vector.h"

export module cc_ice_intern_builder:vector;

import std;
import cc_ice_intern_sonic;

export namespace ice::builder {

class TF_VectorOps
{
public:
    explicit TF_VectorOps(const ::TF_StatusOps* Status_ops) noexcept :
        m_handle{.plugin_data = this}
    {
        m_Status_ops = Status_ops;
    }

    TF_VectorOps(const TF_VectorOps&) = delete;
    TF_VectorOps& operator=(const TF_VectorOps&) = delete;

    static TF_VectorOps& from_handle(void* ctx) noexcept
    {
        return *static_cast<TF_VectorOps*>(ctx);
    }

    template<typename HandleT>
    static TF_VectorOps& from_handle(HandleT* handle) noexcept
    {
        return *static_cast<TF_VectorOps*>(handle->plugin_data);
    }

    virtual ~TF_VectorOps() = default;
    virtual void destroy() noexcept = 0;
    virtual void set_element_size(size_t element_size) noexcept = 0;
    virtual void push_back(const void* value) noexcept = 0;
    virtual void
    get(size_t index, const void** out_value, const ice::sonic::Status& out_status) noexcept = 0;
    virtual void
    set(size_t index, const void* value, const ice::sonic::Status& out_status) noexcept = 0;
    virtual void size(size_t* out_size) noexcept = 0;
    virtual void capacity(size_t* out_capacity) noexcept = 0;
    virtual void reserve(size_t new_capacity, const ice::sonic::Status& out_status) noexcept = 0;
    virtual void data(void** out_data) noexcept = 0;

    void get_generic_vtable(void (*create)(::TF_Vector*)) noexcept
    {
        m_vtable = ::TF_VectorOps{
            .struct_size = TF_OFFSET_OF_END(::TF_VectorOps, data),

            .create = create,
            .destroy =
                [](TF_Vector* handle) noexcept
            {
                auto& self = TF_VectorOps::from_handle(handle);
                self.destroy();
            },
            .set_element_size =
                [](TF_Vector* vector, size_t element_size) noexcept
            {
                auto& self = TF_VectorOps::from_handle(vector);
                self.set_element_size(element_size);
            },
            .push_back =
                [](TF_Vector* vector, const void* value) noexcept
            {
                auto& self = TF_VectorOps::from_handle(vector);
                self.push_back(value);
            },
            .get =
                [](const TF_Vector* vector,
                   size_t index,
                   const void** out_value,
                   TF_Status* out_status) noexcept
            {
                auto& self = TF_VectorOps::from_handle(vector);
                self.get(
                    index,
                    out_value,
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .set =
                [](TF_Vector* vector,
                   size_t index,
                   const void* value,
                   TF_Status* out_status) noexcept
            {
                auto& self = TF_VectorOps::from_handle(vector);
                self.set(
                    index,
                    value,
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .size =
                [](const TF_Vector* vector, size_t* out_size) noexcept
            {
                auto& self = TF_VectorOps::from_handle(vector);
                self.size(out_size);
            },
            .capacity =
                [](const TF_Vector* vector, size_t* out_capacity) noexcept
            {
                auto& self = TF_VectorOps::from_handle(vector);
                self.capacity(out_capacity);
            },
            .reserve =
                [](TF_Vector* vector, size_t new_capacity, TF_Status* out_status) noexcept
            {
                auto& self = TF_VectorOps::from_handle(vector);
                self.reserve(
                    new_capacity,
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .data =
                [](TF_Vector* vector, void** out_data) noexcept
            {
                auto& self = TF_VectorOps::from_handle(vector);
                self.data(out_data);
            },

        };
    }

    ice::sonic::Status
    wrap(std::type_identity<ice::sonic::Status>, const ::TF_Status* handle) const noexcept
    {
        return ice::sonic::Status{m_Status_ops, const_cast<::TF_Status*>(handle)};
    }

    const ::TF_VectorOps& get_vtable() const noexcept
    {
        return m_vtable;
    }

    const ::TF_Vector& get_handle() const noexcept
    {
        return m_handle;
    }

    template<typename Registry, typename StringType>
    void register_ops(
        Registry& registry,
        const StringType& type,
        const StringType& provider
    ) const noexcept
    {
        registry.register_op(type, provider, const_cast<::TF_VectorOps*>(&m_vtable));
    }

private:
    ::TF_VectorOps m_vtable;
    ::TF_Vector m_handle;

    const ::TF_StatusOps* m_Status_ops{nullptr};
};

} // namespace ice::builder
