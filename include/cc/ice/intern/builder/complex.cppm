// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/intern/complex.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/intern/complex.h"
#include "include/c/intern/registration.h"

export module cc_ice_intern_builder:complex;

import std;
import cc_ice_intern_sonic;

export namespace ice::builder {

class Complex
{
public:
    explicit Complex() noexcept :
        m_handle{.plugin_data = this}
    {
    }

    Complex(const Complex&) = delete;
    Complex& operator=(const Complex&) = delete;

    static Complex& from_handle(void* ctx) noexcept
    {
        return *static_cast<Complex*>(ctx);
    }

    template<typename HandleT>
    static Complex& from_handle(HandleT* handle) noexcept
    {
        return *static_cast<Complex*>(handle->plugin_data);
    }

    virtual ~Complex() = default;
    virtual void destroy() noexcept = 0;
    virtual void get_real(double* out_real) noexcept = 0;
    virtual void get_imag(double* out_imag) noexcept = 0;
    virtual void set_real(double real) noexcept = 0;
    virtual void set_imag(double imag) noexcept = 0;

    void get_generic_vtable(void (*create)(::TF_Complex*)) noexcept
    {
        m_vtable = ::TF_ComplexOps{
            .struct_size = TF_OFFSET_OF_END(::TF_ComplexOps, set_imag),

            .create = create,
            .destroy =
                [](::TF_Complex* handle) noexcept
            {
                auto& self = Complex::from_handle(handle);
                self.destroy();
            },
            .get_real =
                [](const ::TF_Complex* complex_value, double* out_real) noexcept
            {
                auto& self = Complex::from_handle(complex_value);
                self.get_real(out_real);
            },
            .get_imag =
                [](const ::TF_Complex* complex_value, double* out_imag) noexcept
            {
                auto& self = Complex::from_handle(complex_value);
                self.get_imag(out_imag);
            },
            .set_real =
                [](::TF_Complex* complex_value, double real) noexcept
            {
                auto& self = Complex::from_handle(complex_value);
                self.set_real(real);
            },
            .set_imag =
                [](::TF_Complex* complex_value, double imag) noexcept
            {
                auto& self = Complex::from_handle(complex_value);
                self.set_imag(imag);
            },

        };
    }

    const ::TF_ComplexOps& get_vtable() const noexcept
    {
        return m_vtable;
    }

    const ::TF_Complex& get_handle() const noexcept
    {
        return m_handle;
    }

    void register_ops(
        const ice::sonic::Registration& registry,
        const ice::sonic::String& type,
        const ice::sonic::String& provider
    ) const noexcept
    {
        registry.register_op(type, provider, const_cast<::TF_ComplexOps*>(&m_vtable));
    }

private:
    ::TF_ComplexOps m_vtable;
    ::TF_Complex m_handle;
};

} // namespace ice::builder
