// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from c/extern/vector/vector.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "c/extern/vector/vector.h"
#include "c/intern/tf_status.h"
#include "c/intern/tf_tstring.h"

export module cc_abi_builder_vector;

import std;
import cc_abi_primitives;
import cc_abi_sonic_intern;

export namespace ice::builder {

class Vector
{
public:
    static Vector* create(void* ctx) noexcept
    {
        return static_cast<Vector*>(ctx);
    }

    template<typename HandleT>
    static Vector* create(HandleT* handle) noexcept
    {
        return reinterpret_cast<Vector*>(handle);
    }

    virtual ~Vector() = default;
    [[nodiscard]] std::expected<void, ice::Status> new_vector(size_t element_size) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status> push_back(const void* value) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status> get(size_t index) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status>
    set(size_t index, const void* value) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status> size() noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status> capacity() noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status> reserve(size_t new_capacity) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status> data() noexcept = 0;
    virtual ice::String get_name() const noexcept = 0;

    static TF_Vector* get_generic_vtable()
    {
        static TF_Vector vtable = {
            .struct_size = TF_VECTOR_STRUCT_SIZE,
            .new_vector =
                [](void* plugin_context, size_t element_size) noexcept
            {
                auto* self = Vector::create(plugin_context);
                auto res = self->new_vector(element_size);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .push_back =
                [](TF_Vector_Handle* vector, const void* value) noexcept
            {
                auto* self = Vector::create(vector);
                auto res = self->push_back(value);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .get =
                [](const TF_Vector_Handle* vector, size_t index) noexcept
            {
                auto* self = Vector::create(vector);
                auto res = self->get(index);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .set =
                [](TF_Vector_Handle* vector, size_t index, const void* value) noexcept
            {
                auto* self = Vector::create(vector);
                auto res = self->set(index, value);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .size =
                [](const TF_Vector_Handle* vector) noexcept
            {
                auto* self = Vector::create(vector);
                auto res = self->size();
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .capacity =
                [](const TF_Vector_Handle* vector) noexcept
            {
                auto* self = Vector::create(vector);
                auto res = self->capacity();
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .reserve =
                [](TF_Vector_Handle* vector, size_t new_capacity) noexcept
            {
                auto* self = Vector::create(vector);
                auto res = self->reserve(new_capacity);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .data =
                [](TF_Vector_Handle* vector) noexcept
            {
                auto* self = Vector::create(vector);
                auto res = self->data();
                if (!res) {
                    res.error().to_c(status);
                }
            },

            .destroy =
                [](void* plugin_context) noexcept
            {
                delete Vector::create(plugin_context);
            },

        };

        return &vtable;
    }
};

} // namespace ice::builder
