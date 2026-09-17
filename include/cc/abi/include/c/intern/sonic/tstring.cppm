// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/intern/tstring.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/intern/tstring.h"

export module cc_abi_sonic_intern;

import std;
import cc_abi_primitives;
import cc_abi_sonic_intern;
import cc_abi_sonic_registration;

export namespace ice::sonic {

class TF_StringOps : public ice::sonic::Runtime<TF_StringOps, TF_StringOps>
{
public:
    explicit TF_StringOps(TF_StringOps* ops, void* plugin_context) noexcept :
        Runtime(ops, plugin_context)
    {
    }

    static constexpr std::string_view domain_name = "intern";

    [[nodiscard]] std::expected<void, ice::Status> init() noexcept
    {
        ice::Status status;
        m_ops->init(get_handle(), status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> copy(const char* src, size_t size) noexcept
    {
        ice::Status status;
        m_ops->copy(get_handle(), src, size, status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status>
    assign_view(const char* src, size_t size) noexcept
    {
        ice::Status status;
        m_ops->assign_view(get_handle(), src, size, status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> get_data_pointer(const char** out_data) noexcept
    {
        ice::Status status;
        m_ops->get_data_pointer(get_handle(), out_data, status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> get_type(TFTStringType* out_type) noexcept
    {
        ice::Status status;
        m_ops->get_type(get_handle(), out_type, status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> get_size(size_t* out_size) noexcept
    {
        ice::Status status;
        m_ops->get_size(get_handle(), out_size, status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> get_capacity(size_t* out_capacity) noexcept
    {
        ice::Status status;
        m_ops->get_capacity(get_handle(), out_capacity, status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> dealloc() noexcept
    {
        ice::Status status;
        m_ops->dealloc(get_handle(), status.get_handle());

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
