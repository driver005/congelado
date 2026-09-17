// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/intern/array.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/intern/array.h"

export module cc_abi_builder_intern;

import std;
import cc_abi_primitives;
import cc_abi_sonic_intern;

export namespace ice::builder {

class TF_ArrayOps
{
public:
    static TF_ArrayOps* create(void* ctx) noexcept
    {
        return static_cast<TF_ArrayOps*>(ctx);
    }

    template<typename HandleT>
    static TF_ArrayOps* create(HandleT* handle) noexcept
    {
        return static_cast<TF_ArrayOps*>(handle->plugin_data);
    }

    virtual ~TF_ArrayOps() = default;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    set_element_size(size_t element_size) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status> set_count(size_t count) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    get(size_t index, const void** out_value) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    set(size_t index, const void* value) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status> size(size_t* out_size) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status> data(void** out_data) noexcept = 0;

    static TF_ArrayOps* get_generic_vtable()
    {
        static TF_ArrayOps vtable = {
            .struct_size = TF_ARRAY_STRUCT_SIZE,
            .set_element_size =
                [](TF_Array* array, size_t element_size) noexcept
            {
                auto* self = TF_ArrayOps::create(array);
                auto res = self->set_element_size(element_size);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .set_count =
                [](TF_Array* array, size_t count) noexcept
            {
                auto* self = TF_ArrayOps::create(array);
                auto res = self->set_count(count);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .get =
                [](const TF_Array* array,
                   size_t index,
                   const void** out_value,
                   TF_Status* out_status) noexcept
            {
                auto* self = TF_ArrayOps::create(array);
                auto res = self->get(index, out_value);
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .set =
                [](TF_Array* array, size_t index, const void* value, TF_Status* out_status) noexcept
            {
                auto* self = TF_ArrayOps::create(array);
                auto res = self->set(index, value);
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .size =
                [](const TF_Array* array, size_t* out_size) noexcept
            {
                auto* self = TF_ArrayOps::create(array);
                auto res = self->size(out_size);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .data =
                [](TF_Array* array, void** out_data) noexcept
            {
                auto* self = TF_ArrayOps::create(array);
                auto res = self->data(out_data);
                if (!res) {
                    res.error().to_c(status);
                }
            },

            .destroy =
                [](void* plugin_context) noexcept
            {
                delete TF_ArrayOps::create(plugin_context);
            },

        };

        return &vtable;
    }
};

} // namespace ice::builder
