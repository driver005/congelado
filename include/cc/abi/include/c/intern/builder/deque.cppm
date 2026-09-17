// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/intern/deque.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/intern/deque.h"

export module cc_abi_builder_intern;

import std;
import cc_abi_primitives;
import cc_abi_sonic_intern;

export namespace ice::builder {

class TF_DequeOps
{
public:
    static TF_DequeOps* create(void* ctx) noexcept
    {
        return static_cast<TF_DequeOps*>(ctx);
    }

    template<typename HandleT>
    static TF_DequeOps* create(HandleT* handle) noexcept
    {
        return static_cast<TF_DequeOps*>(handle->plugin_data);
    }

    virtual ~TF_DequeOps() = default;
    [[nodiscard]] std::expected<void, ice::Status>
    set_element_size(size_t element_size) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status> push_front(const void* value) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status> push_back(const void* value) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status> pop_front() noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status> pop_back() noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status>
    get(size_t index, const void** out_value) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status> size(size_t* out_size) noexcept = 0;
    virtual ice::String get_name() const noexcept = 0;

    static TF_DequeOps* get_generic_vtable()
    {
        static TF_DequeOps vtable = {
            .struct_size = TF_DEQUE_STRUCT_SIZE,
            .set_element_size =
                [](TF_Deque* deque, size_t element_size) noexcept
            {
                auto* self = TF_DequeOps::create(deque);
                auto res = self->set_element_size(element_size);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .push_front =
                [](TF_Deque* deque, const void* value) noexcept
            {
                auto* self = TF_DequeOps::create(deque);
                auto res = self->push_front(value);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .push_back =
                [](TF_Deque* deque, const void* value) noexcept
            {
                auto* self = TF_DequeOps::create(deque);
                auto res = self->push_back(value);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .pop_front =
                [](TF_Deque* deque) noexcept
            {
                auto* self = TF_DequeOps::create(deque);
                auto res = self->pop_front();
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .pop_back =
                [](TF_Deque* deque) noexcept
            {
                auto* self = TF_DequeOps::create(deque);
                auto res = self->pop_back();
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .get =
                [](const TF_Deque* deque,
                   size_t index,
                   const void** out_value,
                   TF_Status* out_status) noexcept
            {
                auto* self = TF_DequeOps::create(deque);
                auto res = self->get(index, out_value);
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .size =
                [](const TF_Deque* deque, size_t* out_size) noexcept
            {
                auto* self = TF_DequeOps::create(deque);
                auto res = self->size(out_size);
                if (!res) {
                    res.error().to_c(status);
                }
            },

            .destroy =
                [](void* plugin_context) noexcept
            {
                delete TF_DequeOps::create(plugin_context);
            },

        };

        return &vtable;
    }
};

} // namespace ice::builder
