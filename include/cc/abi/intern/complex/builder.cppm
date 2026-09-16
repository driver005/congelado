// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from c/extern/complex/complex.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "c/extern/complex/complex.h"
#include "c/intern/tf_status.h"
#include "c/intern/tf_tstring.h"

export module cc_abi_builder_complex;

import std;
import cc_abi_primitives;
import cc_abi_sonic_intern;

export namespace ice::builder {

class Complex
{
public:
    static Complex* create(void* ctx) noexcept
    {
        return static_cast<Complex*>(ctx);
    }

    template<typename HandleT>
    static Complex* create(HandleT* handle) noexcept
    {
        return reinterpret_cast<Complex*>(handle);
    }

    virtual ~Complex() = default;
    [[nodiscard]] std::expected<void, ice::Status>
    new_complex(int is_double, double real, double imag) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status> get_real() noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status> get_imag() noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status> set_real(double real) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status> set_imag(double imag) noexcept = 0;
    virtual ice::String get_name() const noexcept = 0;

    static TF_Complex* get_generic_vtable()
    {
        static TF_Complex vtable = {
            .struct_size = TF_COMPLEX_STRUCT_SIZE,
            .new_complex =
                [](void* plugin_context, int is_double, double real, double imag) noexcept
            {
                auto* self = Complex::create(plugin_context);
                auto res = self->new_complex(is_double, real, imag);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .get_real =
                [](const TF_Complex_Handle* complex_value) noexcept
            {
                auto* self = Complex::create(complex_value);
                auto res = self->get_real();
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .get_imag =
                [](const TF_Complex_Handle* complex_value) noexcept
            {
                auto* self = Complex::create(complex_value);
                auto res = self->get_imag();
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .set_real =
                [](TF_Complex_Handle* complex_value, double real) noexcept
            {
                auto* self = Complex::create(complex_value);
                auto res = self->set_real(real);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .set_imag =
                [](TF_Complex_Handle* complex_value, double imag) noexcept
            {
                auto* self = Complex::create(complex_value);
                auto res = self->set_imag(imag);
                if (!res) {
                    res.error().to_c(status);
                }
            },

            .destroy =
                [](void* plugin_context) noexcept
            {
                delete Complex::create(plugin_context);
            },

        };

        return &vtable;
    }
};

} // namespace ice::builder
