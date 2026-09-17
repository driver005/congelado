// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/intern/complex.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/intern/complex.h"

export module cc_abi_builder_intern;

import std;
import cc_abi_primitives;
import cc_abi_sonic_intern;

export namespace ice::builder {

class TF_ComplexOps
{
public:
    static TF_ComplexOps* create(void* ctx) noexcept
    {
        return static_cast<TF_ComplexOps*>(ctx);
    }

    template<typename HandleT>
    static TF_ComplexOps* create(HandleT* handle) noexcept
    {
        return static_cast<TF_ComplexOps*>(handle->plugin_data);
    }

    virtual ~TF_ComplexOps() = default;
    [[nodiscard]] virtual std::expected<void, ice::Status> get_real(double* out_real) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status> get_imag(double* out_imag) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status> set_real(double real) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status> set_imag(double imag) noexcept = 0;

    static TF_ComplexOps* get_generic_vtable()
    {
        static TF_ComplexOps vtable = {
            .struct_size = TF_COMPLEX_STRUCT_SIZE,
            .get_real =
                [](const TF_Complex* complex_value, double* out_real) noexcept
            {
                auto* self = TF_ComplexOps::create(complex_value);
                auto res = self->get_real(out_real);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .get_imag =
                [](const TF_Complex* complex_value, double* out_imag) noexcept
            {
                auto* self = TF_ComplexOps::create(complex_value);
                auto res = self->get_imag(out_imag);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .set_real =
                [](TF_Complex* complex_value, double real) noexcept
            {
                auto* self = TF_ComplexOps::create(complex_value);
                auto res = self->set_real(real);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .set_imag =
                [](TF_Complex* complex_value, double imag) noexcept
            {
                auto* self = TF_ComplexOps::create(complex_value);
                auto res = self->set_imag(imag);
                if (!res) {
                    res.error().to_c(status);
                }
            },

            .destroy =
                [](void* plugin_context) noexcept
            {
                delete TF_ComplexOps::create(plugin_context);
            },

        };

        return &vtable;
    }
};

} // namespace ice::builder
