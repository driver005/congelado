module;

#include "include/c/intern/complex.h"

export module aten_xpu_intern:complex;

import std;
import cc_ice_intern_builder;
import cc_ice_intern_sonic;

export namespace aten_xpu {

// A complex number kept as two doubles.
class Complex : public ice::builder::Complex
{
public:
    Complex() noexcept = default;

    ~Complex() override = default;
    Complex(const Complex&) = delete;
    Complex& operator=(const Complex&) = delete;
    Complex(Complex&&) = delete;
    Complex& operator=(Complex&&) = delete;

    void destroy() noexcept override
    {
        delete this;
    }

    void get_real(double& out_real) noexcept override
    {
        out_real = m_real;
    }

    void get_imag(double& out_imag) noexcept override
    {
        out_imag = m_imag;
    }

    void set_real(double real) noexcept override
    {
        m_real = real;
    }

    void set_imag(double imag) noexcept override
    {
        m_imag = imag;
    }

private:
    double m_real = 0.0;
    double m_imag = 0.0;
};

} // namespace aten_xpu
