// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/intern/vector.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/intern/vector.h"

export module cc_ice_intern_sonic:vector;

import std;
import cc_abi_sonic_registration;

export namespace ice::sonic {

class TF_VectorOps : public ice::sonic::Runtime<TF_VectorOps, TF_VectorOps>
{
public:
    explicit TF_VectorOps(TF_VectorOps* ops, void* plugin_context) noexcept :
        Runtime(ops, plugin_context)
    {
    }

    static constexpr std::string_view domain_name = "intern";

    void set_element_size(size_t element_size) noexcept
    {
        m_ops->set_element_size(get_handle(), element_size);
    }

    void push_back(const void* value) noexcept
    {
        m_ops->push_back(get_handle(), value);
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

    void capacity(size_t* out_capacity) noexcept
    {
        m_ops->capacity(get_handle(), out_capacity);
    }

    [[nodiscard]] std::expected<void, ice::sonic::Status> reserve(size_t new_capacity) noexcept
    {
        ice::sonic::Status status;
        m_ops->reserve(get_handle(), new_capacity, status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
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
