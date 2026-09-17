// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/intern/complex.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/intern/complex.h"

export module cc_abi_sonic_intern;

import std;
import cc_abi_primitives;
import cc_abi_sonic_intern;
import cc_abi_sonic_registration;

export namespace ice::sonic {

class TF_ComplexOps : public ice::sonic::Runtime<TF_ComplexOps, TF_ComplexOps>
{
public:
    explicit TF_ComplexOps(TF_ComplexOps* ops, void* plugin_context) noexcept :
        Runtime(ops, plugin_context)
    {
    }

    static constexpr std::string_view domain_name = "intern";

    [[nodiscard]] std::expected<void, ice::Status> get_real(double* out_real) noexcept
    {
        ice::Status status;
        m_ops->get_real(get_handle(), out_real, status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> get_imag(double* out_imag) noexcept
    {
        ice::Status status;
        m_ops->get_imag(get_handle(), out_imag, status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> set_real(double real) noexcept
    {
        ice::Status status;
        m_ops->set_real(get_handle(), real, status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> set_imag(double imag) noexcept
    {
        ice::Status status;
        m_ops->set_imag(get_handle(), imag, status.get_handle());

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
