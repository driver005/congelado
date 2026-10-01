// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/intern/complex.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/intern/complex.h"

export module cc_ice_intern_sonic:complex;

import std;
import :runtime;
import :tstring;

export namespace ice::sonic {

class TF_ComplexOps : public ice::sonic::Runtime<::TF_ComplexOps, ::TF_Complex>
{
public:
    template<typename Registry>
    TF_ComplexOps(
        Registry& registry,
        const ice::sonic::String& type,
        const ice::sonic::String& provider
    ) noexcept :
        Runtime(registry, type, provider)
    {
    }

    template<typename Registry>
    TF_ComplexOps(
        Registry& registry,
        ::TF_Complex* handle,
        const ice::sonic::String& type,
        const ice::sonic::String& provider
    ) noexcept :
        Runtime(registry, handle, type, provider)
    {
    }

    explicit TF_ComplexOps(const ::TF_ComplexOps* ops) noexcept :
        Runtime(ops)
    {
    }

    TF_ComplexOps(const ::TF_ComplexOps* ops, ::TF_Complex* handle) noexcept :
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

    void get_real(double* out_real) const noexcept
    {
        m_ops->get_real(get_handle(), out_real);
    }

    void get_imag(double* out_imag) const noexcept
    {
        m_ops->get_imag(get_handle(), out_imag);
    }

    void set_real(double real) const noexcept
    {
        m_ops->set_real(get_handle(), real);
    }

    void set_imag(double imag) const noexcept
    {
        m_ops->set_imag(get_handle(), imag);
    }
};

} // namespace ice::sonic
