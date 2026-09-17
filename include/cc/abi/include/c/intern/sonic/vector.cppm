// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/intern/vector.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/intern/vector.h"

export module cc_abi_sonic_intern;

import std;
import cc_abi_primitives;
import cc_abi_sonic_intern;
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

    [[nodiscard]] std::expected<void, ice::Status> set_element_size(size_t element_size) noexcept
    {
        ice::Status status;
        m_ops->set_element_size(get_handle(), element_size, status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> push_back(const void* value) noexcept
    {
        ice::Status status;
        m_ops->push_back(get_handle(), value, status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status>
    get(size_t index, const void** out_value) noexcept
    {
        ice::Status status;
        m_ops->get(get_handle(), index, out_value, status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> set(size_t index, const void* value) noexcept
    {
        ice::Status status;
        m_ops->set(get_handle(), index, value, status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> size(size_t* out_size) noexcept
    {
        ice::Status status;
        m_ops->size(get_handle(), out_size, status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> capacity(size_t* out_capacity) noexcept
    {
        ice::Status status;
        m_ops->capacity(get_handle(), out_capacity, status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> reserve(size_t new_capacity) noexcept
    {
        ice::Status status;
        m_ops->reserve(get_handle(), new_capacity, status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> data(void** out_data) noexcept
    {
        ice::Status status;
        m_ops->data(get_handle(), out_data, status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    ice::String get_name() const noexcept
    {
        ice::String result;
        m_ops->get_name(get_handle(), result.get_handle());
        return result;
    }
};

} // namespace ice::sonic
