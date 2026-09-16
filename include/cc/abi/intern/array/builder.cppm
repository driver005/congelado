// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from c/extern/array/array.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "c/extern/array/array.h"
#include "c/intern/tf_status.h"
#include "c/intern/tf_tstring.h"

export module cc_abi_builder_array;

import std;
import cc_abi_primitives;
import cc_abi_sonic_intern;

export namespace ice::builder {

class Array
{
public:
    static Array* create(void* ctx) noexcept
    {
        return static_cast<Array*>(ctx);
    }

    template<typename HandleT>
    static Array* create(HandleT* handle) noexcept
    {
        return reinterpret_cast<Array*>(handle);
    }

    virtual ~Array() = default;
    [[nodiscard]] std::expected<void, ice::Status>
    new_array(size_t element_size, size_t count) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status> get(size_t index) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status>
    set(size_t index, const void* value) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status> size() noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status> data() noexcept = 0;
    virtual ice::String get_name() const noexcept = 0;

    static TF_Array* get_generic_vtable()
    {
        static TF_Array vtable = {
            .struct_size = TF_ARRAY_STRUCT_SIZE,
            .new_array =
                [](void* plugin_context, size_t element_size, size_t count) noexcept
            {
                auto* self = Array::create(plugin_context);
                auto res = self->new_array(element_size, count);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .get =
                [](const TF_Array_Handle* array, size_t index) noexcept
            {
                auto* self = Array::create(array);
                auto res = self->get(index);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .set =
                [](TF_Array_Handle* array, size_t index, const void* value) noexcept
            {
                auto* self = Array::create(array);
                auto res = self->set(index, value);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .size =
                [](const TF_Array_Handle* array) noexcept
            {
                auto* self = Array::create(array);
                auto res = self->size();
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .data =
                [](TF_Array_Handle* array) noexcept
            {
                auto* self = Array::create(array);
                auto res = self->data();
                if (!res) {
                    res.error().to_c(status);
                }
            },

            .destroy =
                [](void* plugin_context) noexcept
            {
                delete Array::create(plugin_context);
            },

        };

        return &vtable;
    }
};

} // namespace ice::builder
