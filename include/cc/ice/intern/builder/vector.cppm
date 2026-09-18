// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/intern/vector.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/intern/vector.h"

export module cc_ice_builder_intern:vector;

import std;

export namespace ice::builder {

class TF_VectorOps
{
public:
    static TF_VectorOps* create(void* ctx) noexcept
    {
        return static_cast<TF_VectorOps*>(ctx);
    }

    template<typename HandleT>
    static TF_VectorOps* create(HandleT* handle) noexcept
    {
        return static_cast<TF_VectorOps*>(handle->plugin_data);
    }

    virtual ~TF_VectorOps() = default;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    set_element_size(size_t element_size) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    push_back(const void* value) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    get(size_t index, const void** out_value) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    set(size_t index, const void* value) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status> size(size_t* out_size) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    capacity(size_t* out_capacity) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    reserve(size_t new_capacity) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status> data(void** out_data) noexcept = 0;

    static TF_VectorOps* get_generic_vtable()
    {
        static TF_VectorOps vtable = {
            .struct_size = TF_VECTOR_STRUCT_SIZE,
            .set_element_size =
                [](TF_Vector* vector, size_t element_size) noexcept
            {
                auto* self = TF_VectorOps::create(vector);
                auto res = self->set_element_size(element_size);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .push_back =
                [](TF_Vector* vector, const void* value) noexcept
            {
                auto* self = TF_VectorOps::create(vector);
                auto res = self->push_back(value);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .get =
                [](const TF_Vector* vector,
                   size_t index,
                   const void** out_value,
                   TF_Status* out_status) noexcept
            {
                auto* self = TF_VectorOps::create(vector);
                auto res = self->get(index, out_value);
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .set =
                [](TF_Vector* vector,
                   size_t index,
                   const void* value,
                   TF_Status* out_status) noexcept
            {
                auto* self = TF_VectorOps::create(vector);
                auto res = self->set(index, value);
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .size =
                [](const TF_Vector* vector, size_t* out_size) noexcept
            {
                auto* self = TF_VectorOps::create(vector);
                auto res = self->size(out_size);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .capacity =
                [](const TF_Vector* vector, size_t* out_capacity) noexcept
            {
                auto* self = TF_VectorOps::create(vector);
                auto res = self->capacity(out_capacity);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .reserve =
                [](TF_Vector* vector, size_t new_capacity, TF_Status* out_status) noexcept
            {
                auto* self = TF_VectorOps::create(vector);
                auto res = self->reserve(new_capacity);
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .data =
                [](TF_Vector* vector, void** out_data) noexcept
            {
                auto* self = TF_VectorOps::create(vector);
                auto res = self->data(out_data);
                if (!res) {
                    res.error().to_c(status);
                }
            },

            .destroy =
                [](void* plugin_context) noexcept
            {
                delete TF_VectorOps::create(plugin_context);
            },

        };

        return &vtable;
    }

    builder::String get_name() const noexcept
    {
        builder::String result;
        m_ops->get_name(get_handle(), result.get_handle());
        return result;
    }
};

} // namespace ice::builder
