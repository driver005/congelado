// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/intern/array.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/intern/array.h"

export module cc_ice_intern_sonic:array;

import std;
import cc_abi_sonic_registration;

export namespace ice::sonic {

class TF_ArrayOps : public ice::sonic::Runtime<TF_ArrayOps, TF_ArrayOps>
{
public:
    explicit TF_ArrayOps(TF_ArrayOps* ops, void* plugin_context) noexcept :
        Runtime(ops, plugin_context)
    {
    }

    static constexpr std::string_view domain_name = "intern";

    void set_element_size(size_t element_size) noexcept
    {
        m_ops->set_element_size(get_handle(), element_size);
    }

    void set_count(size_t count) noexcept
    {
        m_ops->set_count(get_handle(), count);
    }

    [[nodiscard]] std::expected<void, ice::sonic::Status>
    get(size_t index, const void** out_value) noexcept
    {
        ice::sonic::Status status;
        m_ops->get(get_handle(), index, out_value, status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::sonic::Status>
    set(size_t index, const void* value) noexcept
    {
        ice::sonic::Status status;
        m_ops->set(get_handle(), index, value, status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    void size(size_t* out_size) noexcept
    {
        m_ops->size(get_handle(), out_size);
    }

    void data(void** out_data) noexcept
    {
        m_ops->data(get_handle(), out_data);
    }

    void destroy() noexcept
    {
        m_ops->destroy(get_handle());
    }
};

} // namespace ice::sonic
