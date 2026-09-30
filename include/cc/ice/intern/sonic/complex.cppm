// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/intern/complex.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/intern/complex.h"

export module cc_ice_intern_sonic:complex;

import std;
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

    void get_real(double* out_real) noexcept
    {
        m_ops->get_real(get_handle(), out_real);
    }

    void get_imag(double* out_imag) noexcept
    {
        m_ops->get_imag(get_handle(), out_imag);
    }

    void set_real(double real) noexcept
    {
        m_ops->set_real(get_handle(), real);
    }

    void set_imag(double imag) noexcept
    {
        m_ops->set_imag(get_handle(), imag);
    }

    void destroy() noexcept
    {
        m_ops->destroy(get_handle());
    }
};

} // namespace ice::sonic
