// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/intern/array.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/intern/array.h"

export module cc_ice_intern_builder:array;

import std;

export namespace ice::builder {

class TF_ArrayOps
{
public:
    TF_ArrayOps() noexcept :
        m_handle{.plugin_data = this}
    {
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
    virtual void set_element_size(size_t element_size) noexcept = 0;
    virtual void set_count(size_t count) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::sonic::Status>
    get(size_t index, const void** out_value) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::sonic::Status>
    set(size_t index, const void* value) noexcept = 0;
    virtual void size(size_t* out_size) noexcept = 0;
    virtual void data(void** out_data) noexcept = 0;
    virtual void destroy() noexcept = 0;

    void get_generic_vtable() noexcept
    {
        m_vtable = ::TF_ArrayOps{
            .struct_size = TF_ARRAY_STRUCT_SIZE,
            .set_element_size =
                [](TF_Array* array, size_t element_size) noexcept
            {
                TF_ArrayOps::from_handle(array).set_element_size(element_size);
            },
            .set_count =
                [](TF_Array* array, size_t count) noexcept
            {
                TF_ArrayOps::from_handle(array).set_count(count);
            },
            .get =
                [](const TF_Array* array,
                   size_t index,
                   const void** out_value,
                   TF_Status* out_status) noexcept
            {
                auto res = TF_ArrayOps::from_handle(array).get(index, out_value);
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .set =
                [](TF_Array* array, size_t index, const void* value, TF_Status* out_status) noexcept
            {
                auto res = TF_ArrayOps::from_handle(array).set(index, value);
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .size =
                [](const TF_Array* array, size_t* out_size) noexcept
            {
                TF_ArrayOps::from_handle(array).size(out_size);
            },
            .data =
                [](TF_Array* array, void** out_data) noexcept
            {
                TF_ArrayOps::from_handle(array).data(out_data);
            },
            .destroy =
                [](TF_Array* array) noexcept
            {
                TF_ArrayOps::from_handle(array).destroy();
            },

        };
    }

    const ::TF_ArrayOps& get_vtable() const noexcept
    {
        return m_vtable;
    }

    const TF_Array& get_handle() const noexcept
    {
        return m_handle;
    }


private:
    ::TF_ArrayOps m_vtable;
    TF_Array m_handle;
};

} // namespace ice::builder
