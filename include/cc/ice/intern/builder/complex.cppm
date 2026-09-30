// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/intern/complex.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/intern/complex.h"

export module cc_ice_intern_builder:complex;

import std;

export namespace ice::builder {

class TF_ComplexOps
{
public:
    TF_ComplexOps() noexcept :
        m_handle{.plugin_data = this}
    {
    }

    TF_ComplexOps(const TF_ComplexOps&) = delete;
    TF_ComplexOps& operator=(const TF_ComplexOps&) = delete;

    static TF_ComplexOps& from_handle(void* ctx) noexcept
    {
        return *static_cast<TF_ComplexOps*>(ctx);
    }

    template<typename HandleT>
    static TF_ComplexOps& from_handle(HandleT* handle) noexcept
    {
        return *static_cast<TF_ComplexOps*>(handle->plugin_data);
    }

    virtual ~TF_ComplexOps() = default;
    virtual void get_real(double* out_real) noexcept = 0;
    virtual void get_imag(double* out_imag) noexcept = 0;
    virtual void set_real(double real) noexcept = 0;
    virtual void set_imag(double imag) noexcept = 0;
    virtual void destroy() noexcept = 0;

    void get_generic_vtable() noexcept
    {
        m_vtable = ::TF_ComplexOps{
            .struct_size = TF_COMPLEX_STRUCT_SIZE,
            .get_real =
                [](const TF_Complex* complex_value, double* out_real) noexcept
            {
                TF_ComplexOps::from_handle(complex_value).get_real(out_real);
            },
            .get_imag =
                [](const TF_Complex* complex_value, double* out_imag) noexcept
            {
                TF_ComplexOps::from_handle(complex_value).get_imag(out_imag);
            },
            .set_real =
                [](TF_Complex* complex_value, double real) noexcept
            {
                TF_ComplexOps::from_handle(complex_value).set_real(real);
            },
            .set_imag =
                [](TF_Complex* complex_value, double imag) noexcept
            {
                TF_ComplexOps::from_handle(complex_value).set_imag(imag);
            },
            .destroy =
                [](TF_Complex* complex_value) noexcept
            {
                TF_ComplexOps::from_handle(complex_value).destroy();
            },

        };
    }

    const ::TF_ComplexOps& get_vtable() const noexcept
    {
        return m_vtable;
    }

    const TF_Complex& get_handle() const noexcept
    {
        return m_handle;
    }


private:
    ::TF_ComplexOps m_vtable;
    TF_Complex m_handle;
};

} // namespace ice::builder
