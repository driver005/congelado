// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/intern/array.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/intern/array.h"
#include "include/c/intern/status.h"

export module cc_ice_intern_builder:array;

import std;
import cc_ice_intern_sonic;

export namespace ice::builder {

class TF_ArrayOps
{
public:
    explicit TF_ArrayOps(const ::TF_StatusOps* Status_ops) noexcept :
        m_handle{.plugin_data = this}
    {
        m_Status_ops = Status_ops;
    }

    TF_ArrayOps(const TF_ArrayOps&) = delete;
    TF_ArrayOps& operator=(const TF_ArrayOps&) = delete;

    static TF_ArrayOps& from_handle(void* ctx) noexcept
    {
        return *static_cast<TF_ArrayOps*>(ctx);
    }

    template<typename HandleT>
    static TF_ArrayOps& from_handle(HandleT* handle) noexcept
    {
        return *static_cast<TF_ArrayOps*>(handle->plugin_data);
    }

    virtual ~TF_ArrayOps() = default;
    virtual void destroy() noexcept = 0;
    virtual void set_element_size(size_t element_size) noexcept = 0;
    virtual void set_count(size_t count) noexcept = 0;
    virtual void
    get(size_t index, const void** out_value, const ice::sonic::Status& out_status) noexcept = 0;
    virtual void
    set(size_t index, const void* value, const ice::sonic::Status& out_status) noexcept = 0;
    virtual void size(size_t* out_size) noexcept = 0;
    virtual void data(void** out_data) noexcept = 0;

    void get_generic_vtable(void (*create)(::TF_Array*)) noexcept
    {
        m_vtable = ::TF_ArrayOps{
            .struct_size = TF_OFFSET_OF_END(::TF_ArrayOps, data),

            .create = create,
            .destroy =
                [](TF_Array* handle) noexcept
            {
                auto& self = TF_ArrayOps::from_handle(handle);
                self.destroy();
            },
            .set_element_size =
                [](TF_Array* array, size_t element_size) noexcept
            {
                auto& self = TF_ArrayOps::from_handle(array);
                self.set_element_size(element_size);
            },
            .set_count =
                [](TF_Array* array, size_t count) noexcept
            {
                auto& self = TF_ArrayOps::from_handle(array);
                self.set_count(count);
            },
            .get =
                [](const TF_Array* array,
                   size_t index,
                   const void** out_value,
                   TF_Status* out_status) noexcept
            {
                auto& self = TF_ArrayOps::from_handle(array);
                self.get(
                    index,
                    out_value,
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .set =
                [](TF_Array* array, size_t index, const void* value, TF_Status* out_status) noexcept
            {
                auto& self = TF_ArrayOps::from_handle(array);
                self.set(
                    index,
                    value,
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .size =
                [](const TF_Array* array, size_t* out_size) noexcept
            {
                auto& self = TF_ArrayOps::from_handle(array);
                self.size(out_size);
            },
            .data =
                [](TF_Array* array, void** out_data) noexcept
            {
                auto& self = TF_ArrayOps::from_handle(array);
                self.data(out_data);
            },

        };
    }

    ice::sonic::Status
    wrap(std::type_identity<ice::sonic::Status>, const ::TF_Status* handle) const noexcept
    {
        return ice::sonic::Status{m_Status_ops, const_cast<::TF_Status*>(handle)};
    }

    const ::TF_ArrayOps& get_vtable() const noexcept
    {
        return m_vtable;
    }

    const ::TF_Array& get_handle() const noexcept
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
        registry.register_op(type, provider, const_cast<::TF_ArrayOps*>(&m_vtable));
    }

private:
    ::TF_ArrayOps m_vtable;
    ::TF_Array m_handle;

    const ::TF_StatusOps* m_Status_ops{nullptr};
};

} // namespace ice::builder
