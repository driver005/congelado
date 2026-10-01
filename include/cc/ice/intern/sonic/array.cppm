// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/intern/array.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/intern/array.h"

export module cc_ice_intern_sonic:array;

import std;
import :runtime;
import :status;
import :tstring;

export namespace ice::sonic {

class TF_ArrayOps : public ice::sonic::Runtime<::TF_ArrayOps, ::TF_Array>
{
public:
    template<typename Registry>
    TF_ArrayOps(
        Registry& registry,
        const ice::sonic::String& type,
        const ice::sonic::String& provider
    ) noexcept :
        Runtime(registry, type, provider)
    {
    }

    template<typename Registry>
    TF_ArrayOps(
        Registry& registry,
        ::TF_Array* handle,
        const ice::sonic::String& type,
        const ice::sonic::String& provider
    ) noexcept :
        Runtime(registry, handle, type, provider)
    {
    }

    explicit TF_ArrayOps(const ::TF_ArrayOps* ops) noexcept :
        Runtime(ops)
    {
    }

    TF_ArrayOps(const ::TF_ArrayOps* ops, ::TF_Array* handle) noexcept :
        Runtime(ops, handle)
    {
    }

    void create() const noexcept
    {
        m_ops->create(get_handle());
    }

    void destroy() const noexcept
    {
        m_ops->destroy(get_handle());
    }

    void set_element_size(size_t element_size) const noexcept
    {
        m_ops->set_element_size(get_handle(), element_size);
    }

    void set_count(size_t count) const noexcept
    {
        m_ops->set_count(get_handle(), count);
    }

    void get(
        size_t index,
        const void** out_value,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops->get(get_handle(), index, out_value, out_status.get_handle());
    }

    void set(size_t index, const void* value, const ice::sonic::Status& out_status) const noexcept
    {
        m_ops->set(get_handle(), index, value, out_status.get_handle());
    }

    void size(size_t* out_size) const noexcept
    {
        m_ops->size(get_handle(), out_size);
    }

    void data(void** out_data) const noexcept
    {
        m_ops->data(get_handle(), out_data);
    }
};

} // namespace ice::sonic
